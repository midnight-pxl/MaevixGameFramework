// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "Components/MInt_InteractorComponent.h"

#include "Components/MInt_InteractableComponent.h"
#include "CoreData/Interfaces/MCore_Interactable.h"
#include "Interfaces/MInt_ExecutionConfigProvider.h"
#include "Logging/LogMaevixInteract.h"
#include "MInt_CollisionChannels.h"
#include "Settings/MInt_DeveloperSettings.h"

#include "CollisionQueryParams.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

static const TCHAR* MInt_FocusResolveOutcomeToString(EMInt_FocusResolveOutcome Outcome)
{
	switch (Outcome)
	{
	case EMInt_FocusResolveOutcome::NoCandidates:    return TEXT("NoCandidates");
	case EMInt_FocusResolveOutcome::NoViewRay:       return TEXT("NoViewRay");
	case EMInt_FocusResolveOutcome::SweepMiss:       return TEXT("SweepMiss");
	case EMInt_FocusResolveOutcome::HitNotCandidate: return TEXT("HitNotCandidate");
	case EMInt_FocusResolveOutcome::NoProvider:      return TEXT("NoProvider");
	case EMInt_FocusResolveOutcome::CannotInteract:  return TEXT("CannotInteract");
	case EMInt_FocusResolveOutcome::Focused:         return TEXT("Focused");
	}
	return TEXT("Unknown");
}

UMInt_InteractorComponent::UMInt_InteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;            // required so SetComponentTickEnabled(true) can work
	PrimaryComponentTick.bStartWithTickEnabled = false;  // gated on a non-empty candidate set

	InitSphereRadius(VolumeRadius);
	SetGenerateOverlapEvents(true);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionEnabled(ECollisionEnabled::NoCollision); // inert until BeginPlay confirms local control
}

void UMInt_InteractorComponent::BeginPlay()
{
	Super::BeginPlay();

	const UMInt_DeveloperSettings* Settings = UMInt_DeveloperSettings::Get();
	const ECollisionChannel MyChannel = Settings->InteractorObjectChannel.GetValue();
	const ECollisionChannel InteractableChannel = Settings->InteractableObjectChannel.GetValue();
	const ECollisionChannel TraceChannel = Settings->InteractionTraceChannel.GetValue();

	if (!MInt_IsValidObjectChannel(MyChannel) || !MInt_IsValidObjectChannel(InteractableChannel) || !MInt_IsValidTraceChannel(TraceChannel))
	{
		UE_LOG(LogMaevixInteract, Error,
			TEXT("UMInt_InteractorComponent on actor '%s' has invalid interaction channels (Interactor: '%s', Interactable: '%s', Trace: '%s'). ")
			TEXT("Create the two Object channels and the Trace channel under Project Settings > Engine > Collision, then set all three ")
			TEXT("under Project Settings > Game > Maevix Interact. Detection is disabled until configured."),
			*GetNameSafe(GetOwner()), *MInt_ChannelDisplayName(MyChannel), *MInt_ChannelDisplayName(InteractableChannel), *MInt_ChannelDisplayName(TraceChannel));
		GoInert();
		return;
	}

	CachedTraceChannel = TraceChannel;
	bChannelsConfigured = true;

	UE_LOG(LogMaevixInteract, Log,
		TEXT("UMInt_InteractorComponent::BeginPlay: detection channels configured: Owner='%s', TraceChannel='%s'"),
		*GetNameSafe(GetOwner()), *MInt_ChannelDisplayName(TraceChannel));

	// This volume is an object on the Interactor channel and overlaps only the Interactable channel.
	SetCollisionObjectType(MyChannel);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(InteractableChannel, ECR_Overlap);
	// MANDATORY: ignore the interaction Trace channel. It is Block-by-default, so without this the volume
	// would block its own camera sweep (the sweep runs on that channel from inside this volume). The
	// all-ignore above covers it; do not add a Block or Overlap response for it.

	OnComponentBeginOverlap.AddDynamic(this, &UMInt_InteractorComponent::HandleBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UMInt_InteractorComponent::HandleEndOverlap);

	// Detection is local-only, and locality changes after BeginPlay: a client pawn's Controller often
	// replicates in later, and possession is lost on spectate or death. Evaluate now and on every controller
	// change so a client not yet controlled at BeginPlay does not stay inert forever.
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		Pawn->ReceiveControllerChangedDelegate.AddDynamic(this, &UMInt_InteractorComponent::HandleControllerChanged);
	}
	RefreshLocality();
}

void UMInt_InteractorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		Pawn->ReceiveControllerChangedDelegate.RemoveDynamic(this, &UMInt_InteractorComponent::HandleControllerChanged);
	}
	Super::EndPlay(EndPlayReason);
}

void UMInt_InteractorComponent::HandleControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	RefreshLocality();
}

void UMInt_InteractorComponent::RefreshLocality()
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (bChannelsConfigured && Pawn && Pawn->IsLocallyControlled())
	{
		UE_LOG(LogMaevixInteract, Log,
			TEXT("UMInt_InteractorComponent::RefreshLocality: detection active: Owner='%s'"),
			*GetNameSafe(GetOwner()));
		GoActive();
	}
	else
	{
		const TCHAR* InertReason = !bChannelsConfigured ? TEXT("channels unconfigured")
			: !Pawn ? TEXT("owner is not a pawn")
			: TEXT("pawn is not locally controlled");
		UE_LOG(LogMaevixInteract, Log,
			TEXT("UMInt_InteractorComponent::RefreshLocality: detection inert: Owner='%s', Reason='%s'"),
			*GetNameSafe(GetOwner()), InertReason);
		GoInert();
	}
}

void UMInt_InteractorComponent::GoActive()
{
	if (bDetectionActive)
	{
		return;
	}
	bDetectionActive = true;
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	UpdateOverlaps(); // catch interactables already in range
}

void UMInt_InteractorComponent::GoInert()
{
	bDetectionActive = false;
	SetComponentTickEnabled(false);
	if (GetCollisionEnabled() != ECollisionEnabled::NoCollision)
	{
		SetCollisionEnabled(ECollisionEnabled::NoCollision); // ends overlaps
	}
	Candidates.Empty();
	ClearFocusIfAny();
}

void UMInt_InteractorComponent::HandleBeginOverlap(UPrimitiveComponent* /*OverlappedComponent*/, AActor* /*OtherActor*/,
	UPrimitiveComponent* OtherComp, int32 /*OtherBodyIndex*/, bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
	if (!OtherComp)
	{
		return;
	}
	const bool bWasEmpty = Candidates.IsEmpty();
	Candidates.Add(OtherComp);
	if (bWasEmpty)
	{
		SetComponentTickEnabled(true);
	}

	UE_LOG(LogMaevixInteract, Verbose,
		TEXT("UMInt_InteractorComponent::HandleBeginOverlap: candidate added: Owner='%s', Candidate='%s', Count=%d, TickEnabled=%s"),
		*GetNameSafe(GetOwner()), *GetNameSafe(OtherComp),
		Candidates.Num(), bWasEmpty ? TEXT("true") : TEXT("false"));
}

void UMInt_InteractorComponent::HandleEndOverlap(UPrimitiveComponent* /*OverlappedComponent*/, AActor* /*OtherActor*/,
	UPrimitiveComponent* OtherComp, int32 /*OtherBodyIndex*/)
{
	if (!OtherComp)
	{
		return;
	}
	Candidates.Remove(OtherComp);
	if (Candidates.IsEmpty())
	{
		SetComponentTickEnabled(false);
		ClearFocusIfAny();
	}

	UE_LOG(LogMaevixInteract, Verbose,
		TEXT("UMInt_InteractorComponent::HandleEndOverlap: candidate removed: Owner='%s', Candidate='%s', Count=%d, TickDisabled=%s"),
		*GetNameSafe(GetOwner()), *GetNameSafe(OtherComp),
		Candidates.Num(), Candidates.IsEmpty() ? TEXT("true") : TEXT("false"));
}

void UMInt_InteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TraceInterval > 0.0f)
	{
		TimeSinceTrace += DeltaTime;
		if (TimeSinceTrace < TraceInterval)
		{
			return;
		}
		TimeSinceTrace = 0.0f;
	}
	ResolveAndBroadcastIfChanged();
}

void UMInt_InteractorComponent::RequestImmediateInteractionRefresh()
{
	ResolveAndBroadcastIfChanged();
}

void UMInt_InteractorComponent::ResolveAndBroadcastIfChanged()
{
	PruneCandidates();

	AActor* Target{nullptr};
	UObject* Provider{nullptr};
	FHitResult Hit;
	EMInt_FocusResolveOutcome Outcome = EMInt_FocusResolveOutcome::NoCandidates;
	ResolveFocus(Hit, Target, Provider, Outcome);

	if (Outcome != LastResolveOutcome)
	{
		UE_LOG(LogMaevixInteract, Verbose,
			TEXT("UMInt_InteractorComponent::ResolveAndBroadcastIfChanged: resolve outcome changed: Owner='%s', Outcome='%s', Count=%d"),
			*GetNameSafe(GetOwner()), MInt_FocusResolveOutcomeToString(Outcome), Candidates.Num());
		LastResolveOutcome = Outcome;
	}

	const bool bNowFocused = (Target != nullptr);
	const bool bIdentityChanged = (CachedTarget.Get() != Target) || (CachedProvider.Get() != Provider);

	if (bIdentityChanged || (bFocused && !bNowFocused))
	{
		BroadcastResolved(Target, Provider, Hit);
	}
	else if (bNowFocused)
	{
		const int32 Version = IMCore_Interactable::Execute_GetInteractionStateVersion(Provider);
		if (Version != CachedStateVersion)
		{
			BroadcastResolved(Target, Provider, Hit);
		}
	}
}

bool UMInt_InteractorComponent::ResolveFocus(FHitResult& OutHit, AActor*& OutTarget, UObject*& OutProvider,
	EMInt_FocusResolveOutcome& OutOutcome) const
{
	OutTarget = nullptr;
	OutProvider = nullptr;

	if (Candidates.IsEmpty())
	{
		OutOutcome = EMInt_FocusResolveOutcome::NoCandidates;
		return false;
	}

	FVector ViewPos = FVector::ZeroVector;
	FVector ViewDir = FVector::ZeroVector;
	GetInteractionTraceRay(ViewPos, ViewDir);
	ViewDir = ViewDir.GetSafeNormal();
	if (ViewDir.IsNearlyZero())
	{
		OutOutcome = EMInt_FocusResolveOutcome::NoViewRay;
		return false; // no valid view (a non-player owner without a GetInteractionTraceRay override)
	}

	const FVector PawnLocation = GetOwner()->GetActorLocation();
	const double Depth = FMath::Max(0.0, FVector::DotProduct(PawnLocation - ViewPos, ViewDir));
	const FVector Start = ViewPos + ViewDir * Depth;
	const FVector End = Start + ViewDir * InteractDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(MIntInteractorFocus), /*bTraceComplex=*/false, GetOwner());
	if (!GetWorld()->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, CachedTraceChannel,
			FCollisionShape::MakeSphere(TraceRadius), Params))
	{
		OutOutcome = EMInt_FocusResolveOutcome::SweepMiss;
		return false;
	}

	AActor* HitActor = OutHit.GetActor();
	if (!HitActor || !IsCandidateActor(HitActor)) // the trace answers "looking at"; the candidate set answers "in reach"
	{
		OutOutcome = EMInt_FocusResolveOutcome::HitNotCandidate;
		return false;
	}

	UObject* Provider = ResolveProvider(HitActor);
	if (!Provider || !IMCore_Interactable::Execute_CanInteract(Provider, MakeContext()))
	{
		OutOutcome = !Provider ? EMInt_FocusResolveOutcome::NoProvider : EMInt_FocusResolveOutcome::CannotInteract;
		return false;
	}

	OutTarget = HitActor;
	OutProvider = Provider;
	OutOutcome = EMInt_FocusResolveOutcome::Focused;
	return true;
}

void UMInt_InteractorComponent::BroadcastResolved(AActor* Target, UObject* Provider, const FHitResult& Hit)
{
	const bool bWasFocused = bFocused;
	const AActor* PreviousTarget = CachedTarget.Get();

	FMInt_ResolvedInteraction Resolved;
	Resolved.Target = Target;
	Resolved.Provider = Provider;
	Resolved.Hit = Hit;

	int32 Version = 0;
	if (Provider)
	{
		const FMCore_InteractionContext Context = MakeContext();
		Resolved.InteractionData = IMCore_Interactable::Execute_GetInteractionData(Provider, Context);
		if (Provider->Implements<UMInt_ExecutionConfigProvider>())
		{
			Resolved.ExecutionConfig = IMInt_ExecutionConfigProvider::Execute_GetExecutionConfig(Provider);
		}
		Version = IMCore_Interactable::Execute_GetInteractionStateVersion(Provider);
	}

	CachedTarget = Target;
	CachedProvider = Provider;
	CachedStateVersion = Version;
	bFocused = (Target != nullptr);

	if (!bWasFocused && bFocused)
	{
		UE_LOG(LogMaevixInteract, Log,
			TEXT("UMInt_InteractorComponent::BroadcastResolved: focus acquired: Owner='%s', Target='%s'"),
			*GetNameSafe(GetOwner()), *GetNameSafe(Target));
	}
	else if (bWasFocused && !bFocused)
	{
		UE_LOG(LogMaevixInteract, Log,
			TEXT("UMInt_InteractorComponent::BroadcastResolved: focus lost: Owner='%s', Target='%s'"),
			*GetNameSafe(GetOwner()), *GetNameSafe(PreviousTarget));
	}

	OnInteractFocusChanged.Broadcast(Resolved);
}

void UMInt_InteractorComponent::ClearFocusIfAny()
{
	if (bFocused)
	{
		BroadcastResolved(nullptr, nullptr, FHitResult());
	}
}

void UMInt_InteractorComponent::PruneCandidates()
{
	for (auto It = Candidates.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}
	if (Candidates.IsEmpty())
	{
		SetComponentTickEnabled(false);
	}
}

bool UMInt_InteractorComponent::IsCandidateActor(const AActor* Actor) const
{
	for (const TWeakObjectPtr<UPrimitiveComponent>& CandidateRef : Candidates)
	{
		const UPrimitiveComponent* CandidateComponent = CandidateRef.Get();
		if (CandidateComponent && CandidateComponent->GetOwner() == Actor)
		{
			return true;
		}
	}
	return false;
}

UObject* UMInt_InteractorComponent::ResolveProvider(AActor* Actor) const
{
	if (!Actor)
	{
		return nullptr;
	}
	// Multi-region actors (several interactable components) are out of scope: take the first.
	if (UMInt_InteractableComponent* Comp = Actor->FindComponentByClass<UMInt_InteractableComponent>())
	{
		return Comp;
	}
	if (Actor->Implements<UMCore_Interactable>())
	{
		return Actor;
	}
	return nullptr;
}

FMCore_InteractionContext UMInt_InteractorComponent::MakeContext() const
{
	FMCore_InteractionContext Context;
	Context.Interactor = GetOwner();
	if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		Context.InteractorController = Pawn->GetController();
	}
	return Context;
}

void UMInt_InteractorComponent::GetInteractionTraceRay_Implementation(FVector& OutViewLocation, FVector& OutViewDirection) const
{
	OutViewLocation = FVector::ZeroVector;
	OutViewDirection = FVector::ZeroVector;

	// Default view model: the local player's camera view point (through the camera manager). Non-player view
	// models (cursor, AI) override this seam; the default intentionally yields no ray without a PlayerController.
	if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
		{
			FRotator ViewRotation = FRotator::ZeroRotator;
			PlayerController->GetPlayerViewPoint(OutViewLocation, ViewRotation);
			OutViewDirection = ViewRotation.Vector();
		}
	}
}

#if WITH_EDITOR
void UMInt_InteractorComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UMInt_InteractorComponent, VolumeRadius))
	{
		SetSphereRadius(VolumeRadius);
	}
}
#endif
