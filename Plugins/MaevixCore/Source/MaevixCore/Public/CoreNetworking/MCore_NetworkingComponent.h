// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "MCore_NetworkingInterface.h"
#include "CoreData/Logging/LogMaevixNetworking.h"
#include "MCore_NetworkingComponent.generated.h"

/** Outcome of an authority-gated execution: whether the operation ran and, if not, why. */
UENUM(BlueprintType)
enum class EMCore_AuthorityExecResult : uint8
{
	Executed,           /* Owner was authoritative; the operation ran. */
	SkippedNoAuthority, /* Owner exists but is not authoritative; operation did not run. */
	SkippedNoOwner      /* No owning Actor resolved; operation could not be gated or run. */
};

/**
 * Abstract base networking component providing authority validation and replication helpers.
 * Derive from this for actor components that need network-aware functionality.
 *
 * Legacy-replication-based and Iris-compatible (not Iris-optimized). Iris is enabled per
 * project via bUseIris in *.Target.cs; this component's standard replicated state and RPCs
 * ride that switch unchanged, so a downstream project on 5.7+ gets Iris support with no Core
 * change.
 *
 * OnAuthorityChanged fires once at BeginPlay with the init-time authority verdict. Components
 * placed on deferred or post-spawn-reowned actors, or that need the autonomous-proxy
 * distinction at initialization, must read authority at point-of-use rather than caching this
 * verdict. ExecuteWithAuthority already re-reads authority live on every call, so it is
 * unaffected. OnRep_Owner-driven correction is the additive, non-breaking extension to add if
 * and when such a component is built.
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
	}

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
	 * Runs Operation only when the owning Actor is the network authority, re-reading authority
	 * live on every call rather than trusting an init-time cache.
	 *
	 * @warning This gates execution on authority; it does not make the call network transparent.
	 * The subclass still owns its own Server RPC. ExecuteWithAuthority is meant to run inside that
	 * RPC handler, as the authority gate around the authoritative mutation. This is a C++ inherited
	 * surface (a template cannot be a UFUNCTION), so Blueprint-only downstream code does not call
	 * it and instead branches on the pure CanExecuteServerOperation check.
	 *
	 * @param Operation Callable invoked with no arguments when authoritative. Surface any operation
	 * result through a captured reference; this method returns the authority outcome only.
	 * @return Executed when the operation ran, SkippedNoAuthority when the owner is not
	 * authoritative, SkippedNoOwner when no owning Actor resolved.
	 */
	template<typename TOperation>
	EMCore_AuthorityExecResult ExecuteWithAuthority(TOperation&& Operation)
	{
		const AActor* Owner = GetOwner();
		if (!Owner)
		{
			return EMCore_AuthorityExecResult::SkippedNoOwner;
		}
		if (!Owner->HasAuthority())
		{
			return EMCore_AuthorityExecResult::SkippedNoAuthority;
		}

		Operation();
		return EMCore_AuthorityExecResult::Executed;
	}

private:

	UPROPERTY(Replicated)
	FGuid ComponentNetworkID{FGuid::NewGuid()};

};
