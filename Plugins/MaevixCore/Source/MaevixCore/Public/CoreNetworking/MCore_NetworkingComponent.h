// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "MCore_NetworkingInterface.h"
#include "CoreData/Logging/LogMaevixNetworking.h"
#include "MCore_NetworkingComponent.generated.h"

/**
 * Abstract base networking component providing authority validation, replication helpers,
 * and Iris detection. Derive from this for actor components that need network-aware
 * functionality.
 *
 * @warning ExecuteWithAuthority<T> is a reserved Phase-2 extension point. Its v1.0
 * implementation is a logged no-op that returns false; do not rely on it for shipped
 * gameplay. Real implementation lands with the first cross-plugin authority use case
 * (e.g. Dynomega). See MAEVIXCORE_WORK_TRACKER.md T-19.
 *
 * @note Iris is currently Beta in UE 5.6 (Epic documentation). Detection and dedicated-Iris
 * paths (IsUsingIrisReplication, DetectNetworkingSystem, IsIrisAvailable) are honesty stubs
 * until Iris graduates from Beta. See MAEVIXCORE_WORK_TRACKER.md T-13 (honesty doc) and
 * T-19 (real implementation when Iris reaches production status).
 */
UCLASS(Abstract, BlueprintType, ClassGroup=(MaevixCore))
class MAEVIXCORE_API UMCore_NetworkingComponent : public UActorComponent, public IMCore_NetworkingInterface
{
	GENERATED_BODY()

public:
	UMCore_NetworkingComponent();

	virtual bool HasNetworkAuthority() const override
	{
		return GetOwner() ? GetOwner()->HasAuthority() : false;
	};

	/** Equivalent to HasNetworkAuthority. Provided as a separate intent-named API for caller readability. */
	UFUNCTION(BlueprintPure, Category="MaevixCore|Networking|Authority")
	bool CanExecuteServerOperation() const { return HasNetworkAuthority(); }

	/** False on dedicated server (no local client to execute against), true everywhere else. */
	UFUNCTION(BlueprintPure, Category="MaevixCore|Networking|Authority")
	bool CanExecuteClientOperation() const
	{
		return GetOwner() ? (GetOwner()->GetNetMode() != NM_DedicatedServer) : false;
	}

	/**
	 * True if the owning actor uses Iris replication. Result is cached on BeginPlay via DetectNetworkingSystem.
	 *
	 * @note Honesty stub: Iris is currently Beta in UE 5.6 (Epic documentation). Detection and
	 * dedicated-Iris paths are targeted for v1.X once Iris reaches production status. Currently
	 * returns false unconditionally. See MAEVIXCORE_WORK_TRACKER.md T-13 / T-19.
	 */
	UFUNCTION(BlueprintPure, Category="MaevixCore|Networking|Replication")
	bool IsUsingIrisReplication() const;

	/**
	 * Force immediate network update for this component's owning actor.
	 * Use sparingly due to performance cost.
	 */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Networking|Replication")
	void ForceNetUpdate();

protected:
	// ============================================================================
	// ACTORCOMPONENT OVERRIDES
	// ============================================================================

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Reserved extension point for cross-plugin authority delegation.
	 *
	 * v1.0 ships as a logged no-op stub (returns false unconditionally). Real implementation
	 * arrives with the first downstream consumer (Dynomega / Catalyst). Subclasses that
	 * require authority-gated execution today should call HasNetworkAuthority() directly.
	 *
	 * @param Operation Forwarded operation to execute (signature TBD at Phase 2).
	 * @param bRequireServerAuthority If true, requires server authority; otherwise accepts client authority too.
	 * @return Always false in v1.0. Phase 2 will return true on successful authority-gated execution.
	 */
	template<typename TOperation>
	bool ExecuteWithAuthority(TOperation&& Operation, bool bRequireServerAuthority = true)
	{
		UE_LOG(LogMaevixNetworking, Warning,
			TEXT("NetworkingComponent::ExecuteWithAuthority: not yet implemented "
				 "(Phase-2 extension point - see T-19). Operation skipped, returning false."));
		return false;
	}

private:

	UPROPERTY(Replicated)
	FGuid ComponentNetworkID{FGuid::NewGuid()};

	/* Honesty stub: Iris is currently Beta in UE 5.6 (Epic documentation). Detection and
	 * dedicated-Iris paths are targeted for v1.X once Iris reaches production status.
	 * See MAEVIXCORE_WORK_TRACKER.md T-13 / T-19. */
	UPROPERTY(Transient)
	bool bIrisDetected{false};

	void DetectNetworkingSystem();
	bool IsIrisAvailable() const;

};
