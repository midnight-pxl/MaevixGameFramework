// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "Tests/MCoreTest_ServiceRegistryFixtures.h"
#include "CoreData/Libraries/MCore_ServiceRegistryLibrary.h"

void UMCoreTest_Listener::HandleServiceRegistered(TSubclassOf<UInterface> ServiceInterface, FGameplayTag Discriminator)
{
	++FireCount;
	LastInterface = ServiceInterface;
	LastDiscriminator = Discriminator;

	// Documented discipline: re-resolve (read) from inside the callback; never register/unregister here.
	if (UObject* WorldContext = ReResolveWorldContext.Get())
	{
		UObject* OutProvider = nullptr;
		UMCore_ServiceRegistryLibrary::ResolveService(WorldContext, ServiceInterface, Discriminator, OutProvider);
		bReResolvedNonNull = (OutProvider != nullptr);
	}
}
