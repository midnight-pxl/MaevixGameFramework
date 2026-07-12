// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/Subsystem.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "UObject/Interface.h"
#include "MCore_ServiceRegistryTypes.generated.h"

/**
 * Broadcast by a registry subsystem on a successful registration so a consumer that resolved early
 * (and got null) can re-resolve. In process, fires on the instance where the register happened (so it
 * works on a dedicated server via the Global subsystem). Carries the interface and discriminator so a
 * consumer re-resolves only when its own service registers.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMCore_OnServiceRegistered,
	TSubclassOf<UInterface>, ServiceInterface, FGameplayTag, Discriminator);

/**
 * Opaque handle to a single service registration.
 *
 * Returned by RegisterService and consumed by UnregisterService. The handle is self routing: it carries
 * a weak reference to the scope subsystem (LocalPlayer or GameInstance) that owns the registration, so
 * UnregisterService can reach the correct registry instance without a WorldContext. That weak owner also
 * keeps unregister split screen safe, because it resolves to the exact owning subsystem rather than a
 * scope category.
 *
 * RegistrationId 0 is the reserved invalid handle. The owning scope subsystem stamps both fields when it
 * hands a handle back to a caller; the storage core mints only the id.
 */
USTRUCT(BlueprintType)
struct MAEVIXCORE_API FMCore_ServiceHandle
{
	GENERATED_BODY()

public:
	FMCore_ServiceHandle()
		: RegistrationId(0)
	{}

	bool IsValid() const { return RegistrationId != 0; }
	uint32 GetID() const { return RegistrationId; }
	void Invalidate() { RegistrationId = 0; OwningRegistry.Reset(); }

	bool operator==(const FMCore_ServiceHandle& Other) const { return RegistrationId == Other.RegistrationId; }
	bool operator!=(const FMCore_ServiceHandle& Other) const { return RegistrationId != Other.RegistrationId; }

private:
	// Stamps id and owner together. Reserved for the scope subsystems so a valid handle
	// always carries its owner.
	FMCore_ServiceHandle(uint32 InRegistrationId, USubsystem* InOwningRegistry)
		: RegistrationId(InRegistrationId)
		, OwningRegistry(InOwningRegistry)
	{}

	UPROPERTY()
	uint32 RegistrationId;

	// Weak back reference to the owning scope subsystem (Local or Global). Reflected so it survives
	// Blueprint variable round trips, but intentionally not BP visible or editable, and never replicated
	// (handles are in process only). Weak: a torn down subsystem resolves to null and makes unregister a
	// safe no-op.
	UPROPERTY()
	TWeakObjectPtr<USubsystem> OwningRegistry;

	/* The scope subsystems stamp OwningRegistry through the private constructor above. These names are
	   provisional until the subsystem prompt; update the friends if those classes are renamed. */
	friend class UMCore_LocalServiceRegistrySubsystem;
	friend class UMCore_GlobalServiceRegistrySubsystem;
	friend class UMCore_ServiceRegistryLibrary;   // facade reads OwningRegistry to route UnregisterService
};
