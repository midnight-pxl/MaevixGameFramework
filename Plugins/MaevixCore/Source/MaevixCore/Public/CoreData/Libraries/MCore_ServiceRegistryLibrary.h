// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "UObject/ScriptInterface.h"
#include "Templates/SubclassOf.h"
#include "Templates/Casts.h"
#include "CoreServiceRegistry/MCore_ServiceRegistryTypes.h"
#include "MCore_ServiceRegistryLibrary.generated.h"

/**
 * Public facade for the interface keyed service registry: resolve and register providers by UINTERFACE type.
 *
 * A synchronous, in process service locator that fills the gap UE subsystems leave (subsystems resolve by
 * concrete UClass, never by interface). Scope is threaded through a WorldContext: a player owned context
 * routes to that LocalPlayer's registry, otherwise to the GameInstance registry. Resolve is most specific
 * first, so a per player provider wins over a global one, and returns null when absent without ever blocking.
 *
 * Usage discipline: when you hold a live pointer to a game object, query its domain interface directly. Do
 * NOT register game objects (chests, NPCs, items, structures) as registry services. The registry is for
 * cross plugin service resolution and the savable roster, which keeps the registered population small.
 */
UCLASS()
class MAEVIXCORE_API UMCore_ServiceRegistryLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ============================================================================
	// C++ TEMPLATED CONVENIENCE  (header inline; delegates to the UClass keyed functions below)
	// ============================================================================

	/** Resolves the provider implementing IX for the WorldContext's scope, or an empty TScriptInterface if absent. */
	template<class IX>
	static TScriptInterface<IX> Resolve(const UObject* WorldContext, FGameplayTag Discriminator = FGameplayTag())
	{
		static_assert(TIsIInterface<IX>::Value,
			"UMCore_ServiceRegistryLibrary::Resolve<IX> requires a UINTERFACE I-type (e.g. IMCore_Foo).");

		UObject* Provider = nullptr;
		ResolveService(WorldContext, IX::UClassType::StaticClass(), Discriminator, Provider);
		return TScriptInterface<IX>(Provider); // converting ctor casts internally; empty when Provider is null
	}

	/** Resolves IX as a 0 or 1 element array (v1 is single provider). Empty on miss; never a null element. */
	template<class IX>
	static TArray<TScriptInterface<IX>> ResolveAll(const UObject* WorldContext)
	{
		static_assert(TIsIInterface<IX>::Value,
			"UMCore_ServiceRegistryLibrary::ResolveAll<IX> requires a UINTERFACE I-type.");

		TArray<TScriptInterface<IX>> Result;
		const TScriptInterface<IX> Single = Resolve<IX>(WorldContext);
		if (Single) // operator bool: true only when both the object and the interface pointer are set
		{
			Result.Add(Single);
		}
		return Result;
	}

	/** Registers Provider as the IX implementation for the WorldContext's scope; returns a handle (invalid if rejected). */
	template<class IX>
	static FMCore_ServiceHandle RegisterService(const UObject* WorldContext, TScriptInterface<IX> Provider,
		FGameplayTag Discriminator = FGameplayTag())
	{
		static_assert(TIsIInterface<IX>::Value,
			"UMCore_ServiceRegistryLibrary::RegisterService<IX> requires a UINTERFACE I-type.");

		return RegisterService(WorldContext, IX::UClassType::StaticClass(), Provider.GetObject(), Discriminator);
	}

	// ============================================================================
	// BLUEPRINT SURFACE  (UClass keyed; the implementation the templates delegate to)
	// ============================================================================

	/**
	 * Resolves the provider registered for ServiceInterface in the WorldContext's scope into OutProvider.
	 * Most specific first: a per player provider wins over a GameInstance one. Returns true when found.
	 */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|ServiceRegistry",
		meta = (DefaultToSelf = "WorldContext"))
	static bool ResolveService(const UObject* WorldContext, TSubclassOf<UInterface> ServiceInterface,
		FGameplayTag Discriminator, UObject*& OutProvider);

	/**
	 * Registers Provider as the implementation of ServiceInterface in the WorldContext's scope.
	 * Provider must implement ServiceInterface, otherwise the call is refused and an invalid handle returned.
	 */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|ServiceRegistry",
		meta = (DefaultToSelf = "WorldContext"))
	static FMCore_ServiceHandle RegisterService(const UObject* WorldContext, TSubclassOf<UInterface> ServiceInterface,
		UObject* Provider, FGameplayTag Discriminator);

	/** Unregisters the provider referenced by Handle through the handle's owning subsystem, then invalidates Handle. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|ServiceRegistry")
	static void UnregisterService(UPARAM(ref) FMCore_ServiceHandle& Handle);
};
