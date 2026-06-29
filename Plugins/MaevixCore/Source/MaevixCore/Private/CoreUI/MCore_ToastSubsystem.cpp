// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/MCore_ToastSubsystem.h"

#include "CoreUI/MCore_UISubsystem.h"
#include "CoreUI/Widgets/MCore_PrimaryGameLayout.h"
#include "CoreUI/Widgets/Toast/MCore_ToastHost.h"
#include "CoreUI/Widgets/Toast/MCore_ToastBase.h"
#include "CoreData/Tags/MCore_ToastTags.h"
#include "CoreData/DevSettings/MCore_CoreSettings.h"
#include "CoreData/Logging/LogMaevixToast.h"

#include "Blueprint/UserWidget.h"
#include "Components/PanelWidget.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

// ============================================================================
// LIFECYCLE
// ============================================================================

void UMCore_ToastSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (IsRunningDedicatedServer())
	{
		UE_LOG(LogMaevixToast, Log, TEXT("ToastSubsystem::Initialize: skipping on dedicated server"));
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	UMCore_UISubsystem* UISubsystem = LocalPlayer ? LocalPlayer->GetSubsystem<UMCore_UISubsystem>() : nullptr;
	if (!UISubsystem)
	{
		UE_LOG(LogMaevixToast, Warning, TEXT("ToastSubsystem::Initialize: no UISubsystem; toast host cannot attach"));
		return;
	}

	UISubsystem->OnPrimaryGameLayoutReady.AddDynamic(this, &UMCore_ToastSubsystem::HandlePrimaryGameLayoutReady);

	/* Already-ready case: the layout was created before this subsystem initialized,
	   so the one-shot OnPrimaryGameLayoutReady broadcast has already passed. */
	if (UISubsystem->HasPrimaryGameLayout())
	{
		AttachToHost(UISubsystem->GetPrimaryGameLayout());
	}

	UE_LOG(LogMaevixToast, Log, TEXT("ToastSubsystem::Initialize: initialized for LocalPlayer"));
}

void UMCore_ToastSubsystem::Deinitialize()
{
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>())
		{
			UISubsystem->OnPrimaryGameLayoutReady.RemoveDynamic(this, &UMCore_ToastSubsystem::HandlePrimaryGameLayoutReady);
		}
	}

	ClearAllToasts();
	ToastHost = nullptr;
	CachedDefaultWidgetClass = nullptr;

	Super::Deinitialize();
}

void UMCore_ToastSubsystem::HandlePrimaryGameLayoutReady(UMCore_PrimaryGameLayout* Layout)
{
	AttachToHost(Layout);
}

void UMCore_ToastSubsystem::AttachToHost(UMCore_PrimaryGameLayout* Layout)
{
	/* A new world recreated the layout, so any toasts and timers tied to the old one
	   are stale. Drop them before re-resolving the host. */
	ClearAllToasts();

	ToastHost = Layout ? Layout->GetToastHost() : nullptr;

	if (!ToastHost && !bHostWarningIssued)
	{
		bHostWarningIssued = true;
		UE_LOG(LogMaevixToast, Warning,
			TEXT("ToastSubsystem: PrimaryGameLayout has no MCore_ToastHost slot; RequestToast will no-op. ")
			TEXT("Add a UMCore_ToastHost widget named 'MCore_ToastHost' to the layout WBP."));
	}

	if (ToastHost)
	{
		PreloadDefaultWidgetClass();
	}
}

// ============================================================================
// REQUEST / DISMISS
// ============================================================================

FMCore_ToastHandle UMCore_ToastSubsystem::RequestToastInternal(const FMCore_ToastRequest& Request)
{
	if (!ToastHost)
	{
		/* Missing host was already warned at attach; nothing to place a toast into. */
		return FMCore_ToastHandle();
	}

	const FGameplayTag AnchorTag = ResolveAnchorTag(Request.AnchorTag);

	UPanelWidget* AnchorPanel = ResolveAnchorPanel(AnchorTag);
	if (!AnchorPanel)
	{
		if (!bHostWarningIssued)
		{
			bHostWarningIssued = true;
			UE_LOG(LogMaevixToast, Warning,
				TEXT("ToastSubsystem: anchor '%s' and the default fallback are not authored in the host; RequestToast no-ops."),
				*AnchorTag.ToString());
		}
		return FMCore_ToastHandle();
	}

	FMCore_ToastQueue& Queue = QueuesByAnchor.FindOrAdd(AnchorTag);
	Queue.SetMaxVisible(ResolveMaxVisible(AnchorTag));

	/* Dedup is anchor-local: a live entry with the same key refreshes in place. */
	if (!Request.DedupKey.IsNone())
	{
		uint32 ExistingHandleID = 0;
		if (Queue.FindByDedupKey(Request.DedupKey, ExistingHandleID))
		{
			RefreshToast(ExistingHandleID, Request);
			return FMCore_ToastHandle(ExistingHandleID);
		}
	}

	/* Fail loud before allocating: a missing widget class must not silently no-op. */
	const TSubclassOf<UMCore_ToastBase> WidgetClass = ResolveWidgetClass(Request);
	if (!WidgetClass)
	{
		return FMCore_ToastHandle();
	}

	const uint32 HandleID = NextHandleID++;

	FMCore_ToastQueueEntry Entry;
	Entry.HandleID = HandleID;
	Entry.DedupKey = Request.DedupKey;
	Entry.Priority = Request.Priority;
	Entry.bPersistent = (Request.Duration <= 0.f);

	uint32 UnusedExisting = 0;
	const EMCore_ToastAdmission Admission = Queue.Admit(Entry, UnusedExisting);
	if (Admission == EMCore_ToastAdmission::ShowNow)
	{
		ShowToast(Request, HandleID, AnchorTag);
	}
	else
	{
		/* Queued: hold the request for promotion when a slot frees. DedupRefreshed
		   cannot occur here because the dedup pre-check above already returned. */
		PendingRequests.Add(HandleID, Request);
	}

	return FMCore_ToastHandle(HandleID);
}

void UMCore_ToastSubsystem::DismissToastInternal(const FMCore_ToastHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return;
	}

	const uint32 HandleID = Handle.GetID();
	if (FindActiveByHandle(HandleID))
	{
		BeginDismiss(HandleID);
		return;
	}

	RemovePendingEverywhere(HandleID);
}

// ============================================================================
// SHOW / REFRESH / LIFETIME
// ============================================================================

void UMCore_ToastSubsystem::ShowToast(const FMCore_ToastRequest& Request, uint32 HandleID, const FGameplayTag& AnchorTag)
{
	/* Admission already reserved an active queue slot for this handle. If creation fails
	   after that point, release the slot so the anchor is not wedged with a phantom toast. */
	auto ReleaseSlot = [this, HandleID, &AnchorTag]()
	{
		if (FMCore_ToastQueue* Queue = QueuesByAnchor.Find(AnchorTag))
		{
			Queue->RemoveActive(HandleID);
		}
	};

	UPanelWidget* AnchorPanel = ResolveAnchorPanel(AnchorTag);
	if (!AnchorPanel)
	{
		ReleaseSlot();
		return;
	}

	const TSubclassOf<UMCore_ToastBase> WidgetClass = ResolveWidgetClass(Request);
	if (!WidgetClass)
	{
		ReleaseSlot();
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	APlayerController* OwningPlayer = LocalPlayer ? LocalPlayer->GetPlayerController(GetWorld()) : nullptr;
	if (!OwningPlayer)
	{
		UE_LOG(LogMaevixToast, Warning, TEXT("ToastSubsystem::ShowToast: no PlayerController; cannot create toast widget"));
		ReleaseSlot();
		return;
	}

	UMCore_ToastBase* Toast = CreateWidget<UMCore_ToastBase>(OwningPlayer, WidgetClass);
	if (!Toast)
	{
		UE_LOG(LogMaevixToast, Warning, TEXT("ToastSubsystem::ShowToast: CreateWidget failed for '%s'"), *GetNameSafe(WidgetClass));
		ReleaseSlot();
		return;
	}

	Toast->PopulateToast(Request);

	/* Newest toast on top; existing toasts in this anchor reflow below. */
	AnchorPanel->InsertChildAt(0, Toast);

	FActiveToastRecord Record;
	Record.HandleID = HandleID;
	Record.AnchorTag = AnchorTag;
	Record.DedupKey = Request.DedupKey;
	Record.Widget = Toast;
	Record.Duration = Request.Duration;
	Record.bIntroComplete = false;
	ActiveToasts.Add(Record);

	/* Bind before PlayIntro so a synchronous (no-await) completion is caught. */
	Toast->OnIntroComplete.AddUObject(this, &UMCore_ToastSubsystem::HandleToastIntroComplete);
	Toast->OnOutroComplete.AddUObject(this, &UMCore_ToastSubsystem::HandleToastOutroComplete);

	const bool bAwaitIntro = Toast->PlayIntro();
	if (bAwaitIntro)
	{
		ArmIntroWatchdog(HandleID);
	}
}

void UMCore_ToastSubsystem::RefreshToast(uint32 ExistingHandleID, const FMCore_ToastRequest& Request)
{
	if (FActiveToastRecord* Record = FindActiveByHandle(ExistingHandleID))
	{
		if (UMCore_ToastBase* Toast = Record->Widget.Get())
		{
			Toast->PopulateToast(Request);
		}

		Record->Duration = Request.Duration;

		if (Record->bIntroComplete)
		{
			/* Already displaying: reset the dwell. A now-persistent refresh clears the timer. */
			ClearTimerHandle(Record->DurationTimer);
			if (Request.Duration > 0.f)
			{
				StartDurationTimer(*Record);
			}
		}
		/* Mid-intro: no duration timer yet; the intro completion path starts it with the updated Duration. */
		return;
	}

	/* Pending entry: update the stored content in place, keep queue position. */
	if (FMCore_ToastRequest* PendingRequest = PendingRequests.Find(ExistingHandleID))
	{
		*PendingRequest = Request;
	}
}

void UMCore_ToastSubsystem::HandleToastIntroComplete(UMCore_ToastBase* Toast)
{
	FActiveToastRecord* Record = FindActiveByWidget(Toast);
	if (!Record)
	{
		return;
	}

	ClearTimerHandle(Record->IntroWatchdogTimer);

	if (Record->bIntroComplete)
	{
		return;
	}
	Record->bIntroComplete = true;

	if (Record->Duration > 0.f)
	{
		StartDurationTimer(*Record);
	}
	/* Persistent (Duration <= 0): no auto-dismiss timer; only DismissToast or a dedup refresh ends it. */
}

void UMCore_ToastSubsystem::HandleDurationElapsed(uint32 HandleID)
{
	BeginDismiss(HandleID);
}

void UMCore_ToastSubsystem::BeginDismiss(uint32 HandleID)
{
	FActiveToastRecord* Record = FindActiveByHandle(HandleID);
	if (!Record)
	{
		return;
	}

	ClearTimerHandle(Record->DurationTimer);

	UMCore_ToastBase* Toast = Record->Widget.Get();
	if (!Toast)
	{
		RetireToast(HandleID);
		return;
	}

	const bool bAwaitOutro = Toast->PlayOutro();
	if (bAwaitOutro)
	{
		ArmOutroWatchdog(HandleID);
	}
}

void UMCore_ToastSubsystem::HandleToastOutroComplete(UMCore_ToastBase* Toast)
{
	if (FActiveToastRecord* Record = FindActiveByWidget(Toast))
	{
		RetireToast(Record->HandleID);
	}
}

void UMCore_ToastSubsystem::RetireToast(uint32 HandleID)
{
	const int32 Index = ActiveToasts.IndexOfByPredicate(
		[HandleID](const FActiveToastRecord& R) { return R.HandleID == HandleID; });
	if (Index == INDEX_NONE)
	{
		return;
	}

	/* Copy out before removal so the queue / promote work below is not aliasing a dead element. */
	FActiveToastRecord Record = ActiveToasts[Index];

	ClearTimerHandle(Record.DurationTimer);
	ClearTimerHandle(Record.IntroWatchdogTimer);
	ClearTimerHandle(Record.OutroWatchdogTimer);

	if (UMCore_ToastBase* Toast = Record.Widget.Get())
	{
		Toast->OnIntroComplete.RemoveAll(this);
		Toast->OnOutroComplete.RemoveAll(this);
		Toast->RemoveFromParent();
	}

	ActiveToasts.RemoveAt(Index);

	if (FMCore_ToastQueue* Queue = QueuesByAnchor.Find(Record.AnchorTag))
	{
		Queue->RemoveActive(HandleID);
	}

	PromoteNextForAnchor(Record.AnchorTag);
}

void UMCore_ToastSubsystem::PromoteNextForAnchor(const FGameplayTag& AnchorTag)
{
	FMCore_ToastQueue* Queue = QueuesByAnchor.Find(AnchorTag);
	if (!Queue)
	{
		return;
	}

	FMCore_ToastQueueEntry Promoted;
	if (!Queue->PromoteNext(Promoted))
	{
		return;
	}

	const FMCore_ToastRequest* PendingRequest = PendingRequests.Find(Promoted.HandleID);
	if (!PendingRequest)
	{
		/* Pending payload missing (should not happen); release the slot so the anchor is not wedged. */
		Queue->RemoveActive(Promoted.HandleID);
		return;
	}

	const FMCore_ToastRequest RequestCopy = *PendingRequest;
	PendingRequests.Remove(Promoted.HandleID);
	ShowToast(RequestCopy, Promoted.HandleID, AnchorTag);
}

void UMCore_ToastSubsystem::ClearAllToasts()
{
	UWorld* World = GetWorld();
	FTimerManager* TimerManager = World ? &World->GetTimerManager() : nullptr;

	for (FActiveToastRecord& Record : ActiveToasts)
	{
		if (TimerManager)
		{
			TimerManager->ClearTimer(Record.DurationTimer);
			TimerManager->ClearTimer(Record.IntroWatchdogTimer);
			TimerManager->ClearTimer(Record.OutroWatchdogTimer);
		}
		if (UMCore_ToastBase* Toast = Record.Widget.Get())
		{
			Toast->OnIntroComplete.RemoveAll(this);
			Toast->OnOutroComplete.RemoveAll(this);
			Toast->RemoveFromParent();
		}
	}

	ActiveToasts.Reset();
	PendingRequests.Reset();
	QueuesByAnchor.Reset();
}

// ============================================================================
// TIMERS
// ============================================================================

void UMCore_ToastSubsystem::StartDurationTimer(FActiveToastRecord& Record)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const uint32 HandleID = Record.HandleID;
	FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &UMCore_ToastSubsystem::HandleDurationElapsed, HandleID);
	World->GetTimerManager().SetTimer(Record.DurationTimer, Delegate, Record.Duration, false);
}

void UMCore_ToastSubsystem::ArmIntroWatchdog(uint32 HandleID)
{
	UWorld* World = GetWorld();
	FActiveToastRecord* Record = FindActiveByHandle(HandleID);
	if (!World || !Record)
	{
		return;
	}

	FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &UMCore_ToastSubsystem::HandleIntroWatchdog, HandleID);
	World->GetTimerManager().SetTimer(Record->IntroWatchdogTimer, Delegate, GetAwaitTimeoutSeconds(), false);
}

void UMCore_ToastSubsystem::ArmOutroWatchdog(uint32 HandleID)
{
	UWorld* World = GetWorld();
	FActiveToastRecord* Record = FindActiveByHandle(HandleID);
	if (!World || !Record)
	{
		return;
	}

	FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &UMCore_ToastSubsystem::HandleOutroWatchdog, HandleID);
	World->GetTimerManager().SetTimer(Record->OutroWatchdogTimer, Delegate, GetAwaitTimeoutSeconds(), false);
}

void UMCore_ToastSubsystem::HandleIntroWatchdog(uint32 HandleID)
{
	FActiveToastRecord* Record = FindActiveByHandle(HandleID);
	if (!Record || Record->bIntroComplete)
	{
		return;
	}

	UMCore_ToastBase* Toast = Record->Widget.Get();
	UE_LOG(LogMaevixToast, Warning,
		TEXT("ToastSubsystem: intro watchdog fired for toast %u (anchor '%s', class '%s'); forcing intro complete. ")
		TEXT("Did the WBP set bAwaitIntroAnimation without calling NotifyIntroComplete?"),
		HandleID, *Record->AnchorTag.ToString(), *GetNameSafe(Toast ? Toast->GetClass() : nullptr));

	if (Toast)
	{
		/* Routes through OnIntroComplete; the widget's bIntroDone guard makes a late real Notify a no-op. */
		Toast->NotifyIntroComplete();
	}
	else
	{
		RetireToast(HandleID);
	}
}

void UMCore_ToastSubsystem::HandleOutroWatchdog(uint32 HandleID)
{
	FActiveToastRecord* Record = FindActiveByHandle(HandleID);
	if (!Record)
	{
		return;
	}

	UMCore_ToastBase* Toast = Record->Widget.Get();
	UE_LOG(LogMaevixToast, Warning,
		TEXT("ToastSubsystem: outro watchdog fired for toast %u (anchor '%s', class '%s'); forcing outro complete. ")
		TEXT("Did the WBP set bAwaitOutroAnimation without calling NotifyOutroComplete?"),
		HandleID, *Record->AnchorTag.ToString(), *GetNameSafe(Toast ? Toast->GetClass() : nullptr));

	if (Toast)
	{
		Toast->NotifyOutroComplete();
	}
	else
	{
		RetireToast(HandleID);
	}
}

void UMCore_ToastSubsystem::ClearTimerHandle(FTimerHandle& Handle)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Handle);
	}
	Handle.Invalidate();
}

void UMCore_ToastSubsystem::RemovePendingEverywhere(uint32 HandleID)
{
	for (TPair<FGameplayTag, FMCore_ToastQueue>& Pair : QueuesByAnchor)
	{
		Pair.Value.RemovePending(HandleID);
	}
	PendingRequests.Remove(HandleID);
}

// ============================================================================
// RESOLUTION
// ============================================================================

FGameplayTag UMCore_ToastSubsystem::ResolveAnchorTag(const FGameplayTag& Requested) const
{
	const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get();

	/* Code-resolved default (TopRight) unless config overrides it. Avoids reading a
	   native gameplay tag during static CDO init. */
	FGameplayTag DefaultAnchor = MCore_ToastTags::MCore_UI_Toast_Anchor_TopRight;
	if (Settings && Settings->DefaultToastAnchorTag.IsValid())
	{
		DefaultAnchor = Settings->DefaultToastAnchorTag;
	}

	if (!Requested.IsValid())
	{
		/* Empty request anchor takes the configured default; this is expected, not a warning. */
		return DefaultAnchor;
	}

	if (ToastHost && ToastHost->HasAnchor(Requested))
	{
		return Requested;
	}

	UE_LOG(LogMaevixToast, Verbose,
		TEXT("ToastSubsystem::ResolveAnchorTag: anchor '%s' not authored in host; using default '%s'."),
		*Requested.ToString(), *DefaultAnchor.ToString());
	return DefaultAnchor;
}

UPanelWidget* UMCore_ToastSubsystem::ResolveAnchorPanel(const FGameplayTag& AnchorTag) const
{
	return ToastHost ? ToastHost->GetAnchorPanel(AnchorTag) : nullptr;
}

int32 UMCore_ToastSubsystem::ResolveMaxVisible(const FGameplayTag& AnchorTag) const
{
	const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get();
	if (!Settings)
	{
		return 1;
	}

	if (const int32* Override = Settings->ToastMaxVisiblePerAnchor.Find(AnchorTag))
	{
		return FMath::Max(1, *Override);
	}
	return FMath::Max(1, Settings->DefaultMaxVisibleToasts);
}

TSubclassOf<UMCore_ToastBase> UMCore_ToastSubsystem::ResolveWidgetClass(const FMCore_ToastRequest& Request)
{
	if (Request.WidgetOverride)
	{
		return Request.WidgetOverride;
	}

	if (CachedDefaultWidgetClass)
	{
		return CachedDefaultWidgetClass;
	}

	if (const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get())
	{
		if (!Settings->DefaultToastWidgetClass.IsNull())
		{
			if (UClass* Loaded = Settings->DefaultToastWidgetClass.LoadSynchronous())
			{
				CachedDefaultWidgetClass = Loaded;
				return CachedDefaultWidgetClass;
			}
		}
	}

	if (!bWidgetClassWarningIssued)
	{
		bWidgetClassWarningIssued = true;
		UE_LOG(LogMaevixToast, Warning,
			TEXT("ToastSubsystem: request has no WidgetOverride and DefaultToastWidgetClass ")
			TEXT("(Project Settings > Game > Maevix Core > Toast) is unset or failed to load; RequestToast returns an invalid handle."));
	}
	return nullptr;
}

void UMCore_ToastSubsystem::PreloadDefaultWidgetClass()
{
	if (CachedDefaultWidgetClass) { return; }

	const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get();
	if (Settings && !Settings->DefaultToastWidgetClass.IsNull())
	{
		CachedDefaultWidgetClass = Settings->DefaultToastWidgetClass.LoadSynchronous();
	}
}

float UMCore_ToastSubsystem::GetAwaitTimeoutSeconds() const
{
	const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get();
	return Settings ? FMath::Max(1.0f, Settings->AwaitAnimationTimeoutSeconds) : 5.0f;
}

// ============================================================================
// LOOKUP
// ============================================================================

UMCore_ToastSubsystem::FActiveToastRecord* UMCore_ToastSubsystem::FindActiveByHandle(uint32 HandleID)
{
	return ActiveToasts.FindByPredicate([HandleID](const FActiveToastRecord& R) { return R.HandleID == HandleID; });
}

UMCore_ToastSubsystem::FActiveToastRecord* UMCore_ToastSubsystem::FindActiveByWidget(const UMCore_ToastBase* Toast)
{
	return ActiveToasts.FindByPredicate([Toast](const FActiveToastRecord& R) { return R.Widget.Get() == Toast; });
}
