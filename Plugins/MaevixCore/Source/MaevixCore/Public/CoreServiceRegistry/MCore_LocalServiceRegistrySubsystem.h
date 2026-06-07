// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "CoreServiceRegistry/MCore_ServiceRegistryTypes.h"
#include "CoreServiceRegistry/MCore_ServiceRegistryCore.h"
#include "MCore_LocalServiceRegistrySubsystem.generated.h"

/**
 * LocalPlayer scoped service registry: resolves providers by UINTERFACE type for one LocalPlayer.
 *
 * Scope and lifetime: one per LocalPlayer (split screen safe), and naturally absent on a dedicated server
 * because dedicated servers have no LocalPlayers, so it holds client side providers only. Use
 * UMCore_GlobalServiceRegistrySubsystem for GameInstance scoped, server authoritative providers. This is a
 * storage layer; all Blueprint exposure, interface templating, and WorldContext routing live in the facade.
 *
 * Prune cadence (deferred): PruneStale is a passthrough and is uncalled in v1 (single provider, no roster
 * churn). When the multi provider Memory roster consumer lands, the facade must prune inline as ResolveAll
 * iterates the roster key plus a low frequency tick. Do not build that here.
 *
 * Re-resolution notification: OnServiceRegistered (a BlueprintAssignable delegate, not the event bus) fires
 * in process on this subsystem after a successful register, carrying the interface and discriminator, so a
 * consumer re-resolves only when its own service registers. It works in single player, listen, and
 * dedicated server (the Global subsystem fires server side) with zero network traffic. Consumer pattern:
 * resolve this subsystem, bind OnServiceRegistered (BP Assign or C++ AddDynamic), filter on ServiceInterface
 * (and Discriminator), then re-resolve. Server side consumers bind the Global subsystem's delegate.
 *
 * Roster storm obligation (deferred, do not build): when the multi provider / Memory roster flip lands,
 * ROSTER (collection) registrations must NOT broadcast OnServiceRegistered (savables are gathered via
 * ResolveAll at save time, not waited on as services, so per savable firing would storm). Single provider
 * service registrations keep firing it. Per interface filtering is already handled by the delegate args.
 */
UCLASS()
class MAEVIXCORE_API UMCore_LocalServiceRegistrySubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	/** Stores Provider under (InterfaceClass, Discriminator) and returns a stamped handle, or an invalid handle if rejected. */
	FMCore_ServiceHandle RegisterProvider(UClass* InterfaceClass, FGameplayTag Discriminator, UObject* Provider);

	/** Returns the live provider for (InterfaceClass, Discriminator), or nullptr. Non const: prunes a stale entry on access. */
	UObject* ResolveProvider(UClass* InterfaceClass, FGameplayTag Discriminator);

	/** Removes the registration with the given id. Returns true if an entry was found and removed. */
	bool UnregisterById(uint32 RegistrationId);

	/** Passthrough to the core's stale sweep. Uncalled in v1; reserved for the future roster prune cadence. */
	void PruneStale();

	/** Fires on a successful registration into this scope's registry; carries the registered interface
	 *  and discriminator. Bind here to re-resolve a service you resolved before it registered. */
	UPROPERTY(BlueprintAssignable, Category = "MaevixCore|ServiceRegistry")
	FMCore_OnServiceRegistered OnServiceRegistered;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	FMCore_ServiceRegistryCore Registry;
};
