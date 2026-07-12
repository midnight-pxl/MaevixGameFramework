// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplayTagContainer.h"
#include "Engine/TimerHandle.h"
#include "CoreData/Types/UI/MCore_ToastTypes.h"
#include "CoreUI/MCore_ToastQueue.h"
#include "MCore_ToastSubsystem.generated.h"

class UMCore_ToastBase;
class UMCore_ToastHost;
class UMCore_PrimaryGameLayout;
class UPanelWidget;

/**
 * Per-LocalPlayer toast service. Owns queueing, placement, and lifetime. Attaches to
 * the player's PrimaryGameLayout toast host, routes requests through a per-anchor
 * queue, and drives each toast through intro / dwell / outro with a watchdog so a
 * wedged WBP animation cannot pin a slot. Scope routing (Local vs AllLocal) lives in
 * UMCore_ToastFunctionLibrary; these methods always act on this player only.
 *
 * The engine-native FSlateNotificationManager / FNotificationInfo path is deliberately not
 * used: it is a global editor-style Slate overlay, not per-player split-screen aware and not
 * wired to the CommonUI theme, so it does not fit a per-player themed toast service.
 */
UCLASS()
class MAEVIXCORE_API UMCore_ToastSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Submit a toast for THIS player. Returns an invalid handle when no host / widget class is available. */
	FMCore_ToastHandle RequestToastInternal(const FMCore_ToastRequest& Request);

	/** Dismiss an active toast (plays outro) or drop a queued one. */
	void DismissToastInternal(const FMCore_ToastHandle& Handle);

private:
	/** Live toast bookkeeping. Widget is weak: the anchor panel owns the strong ref. */
	struct FActiveToastRecord
	{
		uint32 HandleID = 0;
		FGameplayTag AnchorTag;
		FName DedupKey = NAME_None;
		TWeakObjectPtr<UMCore_ToastBase> Widget;
		float Duration = 0.f;
		bool bIntroComplete = false;
		FTimerHandle DurationTimer;
		FTimerHandle IntroWatchdogTimer;
		FTimerHandle OutroWatchdogTimer;
	};

	UFUNCTION()
	void HandlePrimaryGameLayoutReady(UMCore_PrimaryGameLayout* Layout);
	void AttachToHost(UMCore_PrimaryGameLayout* Layout);

	FGameplayTag  ResolveAnchorTag(const FGameplayTag& Requested) const;
	UPanelWidget* ResolveAnchorPanel(const FGameplayTag& AnchorTag) const;
	int32         ResolveMaxVisible(const FGameplayTag& AnchorTag) const;
	TSubclassOf<UMCore_ToastBase> ResolveWidgetClass(const FMCore_ToastRequest& Request);
	void          PreloadDefaultWidgetClass();
	float         GetAwaitTimeoutSeconds() const;

	void ShowToast(const FMCore_ToastRequest& Request, uint32 HandleID, const FGameplayTag& AnchorTag);
	void RefreshToast(uint32 ExistingHandleID, const FMCore_ToastRequest& Request);
	void BeginDismiss(uint32 HandleID);
	void HandleToastIntroComplete(UMCore_ToastBase* Toast);
	void HandleToastOutroComplete(UMCore_ToastBase* Toast);
	void HandleIntroWatchdog(uint32 HandleID);
	void HandleOutroWatchdog(uint32 HandleID);
	void HandleDurationElapsed(uint32 HandleID);
	void RetireToast(uint32 HandleID);
	void PromoteNextForAnchor(const FGameplayTag& AnchorTag);
	void ClearAllToasts();

	void StartDurationTimer(FActiveToastRecord& Record);
	void ArmIntroWatchdog(uint32 HandleID);
	void ArmOutroWatchdog(uint32 HandleID);
	void ClearTimerHandle(FTimerHandle& Handle);
	void RemovePendingEverywhere(uint32 HandleID);

	FActiveToastRecord* FindActiveByHandle(uint32 HandleID);
	FActiveToastRecord* FindActiveByWidget(const UMCore_ToastBase* Toast);

	UPROPERTY(Transient)
	TObjectPtr<UMCore_ToastHost> ToastHost;

	UPROPERTY(Transient)
	TSubclassOf<UMCore_ToastBase> CachedDefaultWidgetClass;

	TMap<FGameplayTag, FMCore_ToastQueue> QueuesByAnchor;
	TArray<FActiveToastRecord> ActiveToasts;
	TMap<uint32, FMCore_ToastRequest> PendingRequests; // handleID maps to request, for promotion

	uint32 NextHandleID = 1;
	bool bHostWarningIssued = false; // one-shot missing-host / missing-anchor warning
	bool bWidgetClassWarningIssued = false; // one-shot missing-DefaultToastWidgetClass warning
};
