// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "MCoreTest_ServiceRegistryFixtures.generated.h"

// Dev-only reflected fixtures for the Service Registry automation harness. Left UNGUARDED on purpose: UHT
// reflects UCLASS/UINTERFACE regardless of WITH_AUTOMATION_TESTS, so guarding them with that macro would
// desync reflection (always generated) from compilation (dev only) and break the shipping link. The test
// bodies that use them are guarded by WITH_DEV_AUTOMATION_TESTS in their .cpp files.

// Minimal UINTERFACE the harness resolves by (plain C++ method; no Blueprint needed).
UINTERFACE()
class UMCoreTest_PingInterface : public UInterface
{
	GENERATED_BODY()
};

class IMCoreTest_PingInterface
{
	GENERATED_BODY()
public:
	virtual int32 GetPingValue() const = 0;
};

// C++ implementer, so TScriptInterface<IMCoreTest_PingInterface> resolves a non-null interface pointer.
UCLASS()
class UMCoreTest_PingProvider : public UObject, public IMCoreTest_PingInterface
{
	GENERATED_BODY()
public:
	virtual int32 GetPingValue() const override { return PingValue; }
	int32 PingValue = 42;
};

// Does NOT implement the interface; exercises the facade ImplementsInterface gate.
UCLASS()
class UMCoreTest_NonProvider : public UObject
{
	GENERATED_BODY()
};

// UObject listener with a UFUNCTION handler matching FMCore_OnServiceRegistered, for the delegate round trip.
UCLASS()
class UMCoreTest_Listener : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void HandleServiceRegistered(TSubclassOf<UInterface> ServiceInterface, FGameplayTag Discriminator);

	int32 FireCount = 0;
	TSubclassOf<UInterface> LastInterface;
	FGameplayTag LastDiscriminator;

	// Usage-discipline demo: the handler RE-RESOLVES (read), never registers/unregisters. Set by the test.
	TWeakObjectPtr<UObject> ReResolveWorldContext;
	bool bReResolvedNonNull = false;
};
