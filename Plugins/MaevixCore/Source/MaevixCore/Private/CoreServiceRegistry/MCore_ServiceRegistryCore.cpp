// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreServiceRegistry/MCore_ServiceRegistryCore.h"

/**
 * @param OutResult Set on every path to the registration outcome so the caller can log or branch without
 *                  re-deriving why the id is 0.
 * @return The new registration id, or 0 when OutResult is RejectedDuplicate or InvalidArguments.
 */
uint32 FMCore_ServiceRegistryCore::Register(UClass* InterfaceClass, FGameplayTag Discriminator,
	UObject* Provider, EMCore_ServiceRegisterResult& OutResult)
{
	/* Reject invalid input rather than storing an entry that could never resolve. */
	if (!InterfaceClass || !Provider)
	{
		OutResult = EMCore_ServiceRegisterResult::InvalidArguments;
		return 0;
	}

	const FMCore_ServiceKey Key{ InterfaceClass, Discriminator };

	bool bReclaimedStale = false;
	if (const FMCore_ServiceEntry* Existing = Services.Find(Key))
	{
		/* A live provider already owns this key. v1 is single provider, so refuse the duplicate and leave
		   the incumbent in place. Existing is only read here; it is never touched after the Add below. */
		if (Existing->Provider.IsValid())
		{
			OutResult = EMCore_ServiceRegisterResult::RejectedDuplicate;
			return 0;
		}
		/* Incumbent went stale. Fall through so the Add below reclaims the slot for the newcomer. */
		bReclaimedStale = true;
	}

	const uint32 NewRegistrationId = NextRegistrationId++;

	FMCore_ServiceEntry NewEntry;
	NewEntry.Provider = Provider;
	NewEntry.RegistrationId = NewRegistrationId;
	Services.Add(Key, NewEntry);

	OutResult = bReclaimedStale ? EMCore_ServiceRegisterResult::ReclaimedStale
	                            : EMCore_ServiceRegisterResult::Added;
	return NewRegistrationId;
}

UObject* FMCore_ServiceRegistryCore::Resolve(UClass* InterfaceClass, FGameplayTag Discriminator)
{
	if (!InterfaceClass)
	{
		return nullptr;
	}

	const FMCore_ServiceKey Key{ InterfaceClass, Discriminator };

	const FMCore_ServiceEntry* Entry = Services.Find(Key);
	if (!Entry)
	{
		return nullptr;
	}

	if (UObject* Provider = Entry->Provider.Get())
	{
		return Provider;
	}

	/* Prune on access. Find returned a pointer into map storage that Remove invalidates, so we remove by
	   the local Key (not through Entry) and never read Entry again after this point. */
	Services.Remove(Key);
	return nullptr;
}

bool FMCore_ServiceRegistryCore::Unregister(uint32 RegistrationId)
{
	if (RegistrationId == 0)
	{
		return false;
	}

	/* The handle carries only the id, so scan for the matching entry. v1 holds at most one provider per key
	   and only a handful of keys, so a linear scan is fine; add an id -> key index if that ever grows.
	   RemoveCurrent is iterator safe, so there is no Find pointer for the removal to invalidate. */
	for (auto It = Services.CreateIterator(); It; ++It)
	{
		if (It->Value.RegistrationId == RegistrationId)
		{
			It.RemoveCurrent();
			return true;
		}
	}

	return false;
}

void FMCore_ServiceRegistryCore::PruneStaleEntries()
{
	/* TMap analog of the event subsystems' reverse iterate + RemoveAtSwap prune: walk the map and drop every
	   entry whose weak provider has died. RemoveCurrent does not invalidate the iterator. */
	for (auto It = Services.CreateIterator(); It; ++It)
	{
		if (!It->Value.Provider.IsValid())
		{
			It.RemoveCurrent();
		}
	}
}
