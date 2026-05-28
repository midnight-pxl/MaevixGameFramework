// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "MCore_SettingsCollectionSubsystem.generated.h"

class UMCore_DA_SettingsCollection;
class UMCore_DA_SettingDefinition;

/**
 * GameInstance-scoped runtime cache for resolved settings collection DataAssets.
 * One instance per GameInstance (process, or PIE session).
 *
 * Soft-ref source-of-truth stays on UMCore_CoreSettings::SettingsCollections (CDO,
 * config-driven). This subsystem resolves those soft refs on first access and
 * holds the loaded DAs UPROPERTY-tracked so they survive GC across alt-tab.
 *
 * The cache previously lived on UMCore_CoreSettings (UDeveloperSettings CDO), which
 * sits in the disregard-for-GC permanent pool. UPROPERTY strong refs from the
 * permanent pool to runtime-loaded objects violate GC invariants and trip
 * GarbageCollectionVerification at L155. Hosting the cache on a runtime UObject
 * (this subsystem) makes the strong refs legal and GC-tracked.
 *
 * Public Blueprint API stays on UMCore_CoreSettings via thin proxies that forward
 * here.
 */
UCLASS()
class MAEVIXCORE_API UMCore_SettingsCollectionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Static accessor; looks up the subsystem from a world context. Returns nullptr
	 *  if WorldContextObject is null, has no world, or the world has no GameInstance. */
	static UMCore_SettingsCollectionSubsystem* Get(const UObject* WorldContextObject);

	/** Cache-consuming methods migrated from UMCore_CoreSettings. Same semantics, same
	 *  return shapes; only the host changes. Not BlueprintCallable on the subsystem
	 *  itself; BP access remains via the CoreSettings proxies for API stability. */
	const TArray<UMCore_DA_SettingsCollection*>& GetAllSettingsCollections();
	UMCore_DA_SettingDefinition* FindSettingDefinitionByTag(const FGameplayTag& SettingTag);
	TArray<UMCore_DA_SettingDefinition*> GetSettingsForCategory(const FGameplayTag& CategoryTag);
	TArray<FGameplayTag> GetAllSettingsCategories();

	/** Returns the configured display name for CategoryTag, or the tag's leaf segment if no display name is set. */
	FText GetCategoryDisplayName(const FGameplayTag& CategoryTag);

	bool HasValidSettingsCollections();

	/** Drops the cache; next read re-resolves. Called by the CoreSettings proxy from
	 *  PostEditChangeProperty (editor-only invalidation). */
	void InvalidateCollectionCache();

private:
	/** GC-rooted via UPROPERTY. Legal here; subsystem is a runtime UObject, not in
	   the disregard-for-GC permanent pool. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMCore_DA_SettingsCollection>> ResolvedCollections;

	bool bCollectionsCacheValid = false;
};
