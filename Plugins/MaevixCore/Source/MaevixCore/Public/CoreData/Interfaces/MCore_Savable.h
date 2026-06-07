// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Misc/Guid.h"
#include "CoreData/Types/Persistence/MCore_SaveLoadContext.h"
#include "MCore_Savable.generated.h"

// MinimalAPI (not a bare UINTERFACE) so the future MaevixMemory module can reference UMCore_Savable::StaticClass()
// to resolve the savable roster through the service registry cross-module.
UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UMCore_Savable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Save-participation contract. A provider implements this to be gathered and serialized by the persistence
 * orchestrator (the future MaevixMemory) without the orchestrator knowing the provider's concrete type;
 * resolve providers of this interface through the service registry.
 *
 * The data path is deliberately NOT a method here: the orchestrator owns the FArchive and captures
 * UPROPERTY(SaveGame) fields via Provider->Serialize(Ar) with ArIsSaveGame=true (the reflected path, which
 * works for C++ and Blueprint providers alike). Core owns neither the archive nor any save-format policy.
 *
 * Deferred seams (not declared yet; all additive because these hooks are default-no-op BlueprintNativeEvents):
 *   - SerializeState(FArchive&) for non-reflected / computed state, added only when a real provider needs it.
 *   - OnPreLoad (clear-before-load) and OnPostSave (resume), added later without breaking implementers.
 */
class MAEVIXCORE_API IMCore_Savable
{
	GENERATED_BODY()

public:
	/**
	 * Stable cross-session identity used to match a saved record back to its provider on load.
	 * The default returns an invalid FGuid; a provider with persistent identity overrides this.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MaevixCore|Persistence")
	FGuid GetSaveGuid() const;
	virtual FGuid GetSaveGuid_Implementation() const;

	/**
	 * Called just before the orchestrator serializes this provider, so it can flush transient state into its
	 * SaveGame fields. Non-const because the provider mutates itself.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MaevixCore|Persistence")
	void OnPreSave(const FMCore_SaveLoadContext& Context);
	virtual void OnPreSave_Implementation(const FMCore_SaveLoadContext& Context);

	/**
	 * Called after the orchestrator restores this provider's serialized fields, so it can rebuild derived state
	 * from them. Non-const because the provider mutates itself.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MaevixCore|Persistence")
	void OnPostLoad(const FMCore_SaveLoadContext& Context);
	virtual void OnPostLoad_Implementation(const FMCore_SaveLoadContext& Context);
};
