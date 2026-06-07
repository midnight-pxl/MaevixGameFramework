// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * Lookup key for a service registration: the service interface UClass plus an optional discriminator tag.
 *
 * The discriminator lets several providers of the same interface coexist under distinct tags. An empty
 * discriminator is the default bucket and is matched exactly; it is not a wildcard.
 */
struct FMCore_ServiceKey
{
	/* Raw UClass*: a native UINTERFACE class lives for the whole module lifetime, so it is a stable map key
	   that needs no GC tracking here. Leave it raw; TObjectKey would only add cost for no benefit. */
	UClass* InterfaceClass = nullptr;

	FGameplayTag Discriminator;

	bool operator==(const FMCore_ServiceKey& Other) const
	{
		return InterfaceClass == Other.InterfaceClass && Discriminator == Other.Discriminator;
	}
};

FORCEINLINE uint32 GetTypeHash(const FMCore_ServiceKey& Key)
{
	return HashCombine(GetTypeHash(Key.InterfaceClass), GetTypeHash(Key.Discriminator));
}

/**
 * One stored registration. The provider is held weakly so a destroyed provider is never kept alive by the
 * registry and is pruned on the next access. RegistrationId is the opaque id surfaced through FMCore_ServiceHandle.
 */
struct FMCore_ServiceEntry
{
	TWeakObjectPtr<UObject> Provider;
	uint32 RegistrationId = 0;
};

/** Outcome of a Register call, reported through its OutResult parameter so the caller can log or branch. */
enum class EMCore_ServiceRegisterResult : uint8
{
	Added,             // stored into a previously empty slot
	ReclaimedStale,    // prior provider had died; slot reclaimed for the newcomer
	RejectedDuplicate, // a LIVE provider already holds (interface, discriminator)
	InvalidArguments   // null interface class or null provider
};

/**
 * Pure, in process storage core for the interface keyed service registry.
 *
 * Deliberately a plain C++ type: not a UObject, no scope concept, no event bus, no networking. It only
 * stores providers, resolves them by (interface, discriminator), and prunes dead weak references. The
 * scope subsystems each embed one of these and layer scope, handles, and notifications on top.
 *
 * v1 is single provider per key: a second registration over a live provider is rejected (see Register).
 */
class MAEVIXCORE_API FMCore_ServiceRegistryCore
{
public:
	/**
	 * Stores Provider under (InterfaceClass, Discriminator), reports the outcome through OutResult, and
	 * returns a fresh registration id.
	 *
	 * v1 single provider policy: if a live provider already holds this key the call is rejected
	 * (RejectedDuplicate) and returns 0 without changing anything. A stale incumbent is reclaimed
	 * (ReclaimedStale); an empty slot yields Added; a null InterfaceClass or null Provider yields
	 * InvalidArguments and returns 0. The core sets the outcome but does not log it.
	 */
	uint32 Register(UClass* InterfaceClass, FGameplayTag Discriminator, UObject* Provider,
		EMCore_ServiceRegisterResult& OutResult);

	/**
	 * Returns the live provider registered under (InterfaceClass, Discriminator), or nullptr if none.
	 *
	 * Synchronous and total: never blocks, never asserts. A stored provider that has gone stale is pruned
	 * in place and nullptr is returned.
	 */
	UObject* Resolve(UClass* InterfaceClass, FGameplayTag Discriminator);

	/** Removes the registration with the given id. Returns true if an entry was found and removed. */
	bool Unregister(uint32 RegistrationId);

	/** Sweeps and removes every entry whose weak provider has gone stale. */
	void PruneStaleEntries();

private:
	/*
	 * v1: single provider per key, reject-when-live (see Register). One TMap, not a dual key/handle map:
	 * Resolve and Register are O(1)/amortized; Unregister's O(n) scan is acceptable because explicit
	 * Unregister is the RARE path. Removal model: roster providers are reclaimed by weak-ref pruning
	 * (provider dies, weak ref nulls, prune sweep reclaims), NOT by explicit Unregister, which is only for
	 * a still-alive provider deliberately deregistering. Do not optimize Unregister or assume providers
	 * deregister explicitly.
	 *
	 * Deferred multi-provider seam (do NOT build before a real gather-all-providers consumer exists, i.e.
	 * MaevixMemory's ISavable roster): flip the value type to
	 * TMap<FMCore_ServiceKey, TArray<FMCore_ServiceEntry>> and drop the live-incumbent rejection in
	 * Register. The discriminator, the handle, and ResolveAll already support many under one key, so this
	 * needs NO public API change. TArray is the right roster container (cache-friendly iterate, cheap
	 * append, RemoveAtSwap prune); escalate to TSparseArray plus an id-to-index map for O(1) removal ONLY
	 * if the Memory design ever needs deterministic high-frequency removal of LIVE providers (it should
	 * not, per the removal model). Prune scheduling and the query-a-held-pointer-via-its-domain-interface
	 * usage rule are piece 2/3 and contract obligations.
	 */
	TMap<FMCore_ServiceKey, FMCore_ServiceEntry> Services;

	/* Monotonic id source, mirrors FMCore_UIExtensionHandle's uint32 scheme. 0 stays reserved as invalid. */
	uint32 NextRegistrationId = 1;
};
