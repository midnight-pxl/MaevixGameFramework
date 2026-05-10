// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "MCore_NetworkingInterface.h"
#include "MCore_NetworkingComponent.generated.h"

/**
 * Abstract base networking component providing authority validation, replication
 * helpers, and Iris detection. Derive from this for actor components that need
 * network-aware functionality.
 */
UCLASS(Abstract, BlueprintType, ClassGroup=(ModulusCore))
class MODULUSCORE_API UMCore_NetworkingComponent : public UActorComponent, public IMCore_NetworkingInterface
{
	GENERATED_BODY()

public:
	UMCore_NetworkingComponent();

	virtual bool HasNetworkAuthority() const override
	{
		return GetOwner() ? GetOwner()->HasAuthority() : false;
	};

	/** Equivalent to HasNetworkAuthority. Provided as a separate intent-named API for caller readability. */
	UFUNCTION(BlueprintPure, Category="Networking|Authority")
	bool CanExecuteServerOperation() const { return HasNetworkAuthority(); }

	/** False on dedicated server (no local client to execute against), true everywhere else. */
	UFUNCTION(BlueprintPure, Category="Networking|Authority")
	bool CanExecuteClientOperation() const
	{
		return GetOwner() ? (GetOwner()->GetNetMode() != NM_DedicatedServer) : false;
	}

	/** True if the owning actor uses Iris replication. Result is cached on BeginPlay via DetectNetworkingSystem. */
	UFUNCTION(BlueprintPure, Category="Networking|Replication")
	bool IsUsingIrisReplication() const;

	/**
	 * Force immediate network update for this component's owning actor.
	 * Use sparingly due to performance cost.
	 */
	UFUNCTION(BlueprintCallable, Category = "Modulus|Replication")
	void ForceNetUpdate();

protected:
	// ============================================================================
	// ACTORCOMPONENT OVERRIDES
	// ============================================================================

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/*
	 * NOT YET IMPLEMENTED
	 * Reserved extension point for cross-plugin authority delegation.
	 */
	template<typename TOperation>
	bool ExecuteWithAuthority(TOperation&& Operation, bool bRequireServerAuthority = true);

private:

	UPROPERTY(Replicated)
	FGuid ComponentNetworkID{FGuid::NewGuid()};

	UPROPERTY(Transient)
	bool bIrisDetected{false};

	void DetectNetworkingSystem();
	bool IsIrisAvailable() const;

};
