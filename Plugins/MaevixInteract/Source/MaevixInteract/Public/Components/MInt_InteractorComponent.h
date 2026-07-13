// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Types/Interaction/MInt_InteractionTypes.h"
#include "MInt_InteractorComponent.generated.h"

class APawn;
class AController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMInt_OnInteractFocusChanged, const FMInt_ResolvedInteraction&, Resolved);

/**
 * Player-side interaction driver. A local pawn carries this Interactor volume; it tracks nearby interactable
 * volumes by overlap, sweeps the camera ray to decide which one is focused, and broadcasts the resolved
 * focus. It never executes an interaction.
 */
UCLASS(ClassGroup=(MaevixInteract), BlueprintType, meta=(BlueprintSpawnableComponent, DisplayName="Maevix Interactor"))
class MAEVIXINTERACT_API UMInt_InteractorComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UMInt_InteractorComponent();

	/** Reach from the character to the focused interactable, in centimeters. Camera-relative: boom length cancels out. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaevixInteract|Interactor", meta=(ClampMin="0.0", UIMin="0.0", Units="cm"))
	float InteractDistance = 200.0f;

	/** Focus sweep radius, cm. The real cost knob: broadphase scales with radius squared, and a fat radius grabs things beside the reticle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaevixInteract|Interactor", meta=(ClampMin="0.0", UIMin="0.0", Units="cm"))
	float TraceRadius = 5.0f;

	/** Minimum seconds between focus sweeps. 0 sweeps every gated tick; raise to throttle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaevixInteract|Interactor", meta=(ClampMin="0.0", UIMin="0.0", Units="s"))
	float TraceInterval = 0.0f;

	/**
	 * This Interactor's presence volume radius, in centimeters. Does NOT define interaction range: range is
	 * each interactable's DetectionRadius intersected with InteractDistance. This only sets candidate proximity.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaevixInteract|Interactor", meta=(ClampMin="0.0", UIMin="0.0", Units="cm"))
	float VolumeRadius = 32.0f;

	/** Broadcast on focus gained, lost (null Target), changed, or the focused target's state version change. */
	UPROPERTY(BlueprintAssignable, Category="MaevixInteract|Interactor")
	FMInt_OnInteractFocusChanged OnInteractFocusChanged;

	/** Forces an immediate re-resolve and broadcast, bypassing TraceInterval. Zero cost if unused. */
	UFUNCTION(BlueprintCallable, Category="MaevixInteract|Interactor")
	void RequestImmediateInteractionRefresh();

	/**
	 * View ray the focus sweep runs along. Default returns the local PlayerController's view point (through the
	 * camera manager, so view targets and camera modifiers apply). Override for other view models (cursor
	 * deprojection, AI eyes); the default produces no ray without a local PlayerController.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="MaevixInteract|Interactor")
	void GetInteractionTraceRay(FVector& OutViewLocation, FVector& OutViewDirection) const;
	virtual void GetInteractionTraceRay_Implementation(FVector& OutViewLocation, FVector& OutViewDirection) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	/** Bound to the owning pawn's controller-changed delegate; re-evaluates locality when possession changes. */
	UFUNCTION()
	void HandleControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	/** Enables detection when the owner is a locally-controlled pawn, disables it otherwise. Idempotent. */
	void RefreshLocality();
	void GoActive();
	void GoInert();

	void ResolveAndBroadcastIfChanged();
	bool ResolveFocus(FHitResult& OutHit, AActor*& OutTarget, UObject*& OutProvider) const;
	void BroadcastResolved(AActor* Target, UObject* Provider, const FHitResult& Hit);
	void ClearFocusIfAny();
	void PruneCandidates();
	bool IsCandidateActor(const AActor* Actor) const;
	UObject* ResolveProvider(AActor* Actor) const;
	FMCore_InteractionContext MakeContext() const;

	/** Overlapping interactable volumes. Weak: a destroyed interactable does not reliably fire EndOverlap. */
	TSet<TWeakObjectPtr<UPrimitiveComponent>> Candidates;

	TWeakObjectPtr<AActor> CachedTarget;
	TWeakObjectPtr<UObject> CachedProvider;
	int32 CachedStateVersion = 0;
	float TimeSinceTrace = 0.0f;
	TEnumAsByte<ECollisionChannel> CachedTraceChannel = ECC_Visibility;
	bool bChannelsConfigured = false;
	bool bDetectionActive = false;
	/** Whether the last broadcast reported a focus. Tracked separately from CachedTarget so a destroyed focus still emits focus-lost. */
	bool bFocused = false;
};
