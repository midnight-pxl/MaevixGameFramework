// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "MCore_SettingsTypes.h"
#include "MCore_DA_SettingsCollection.generated.h"

class UMCore_DA_SettingDefinition;

/**
 * Collection of setting definitions for a settings panel.
 * Provides category-based filtering and tag-based lookup. Display order
 * follows the position of each entry in the Settings array.
 *
 * Create as a DataAsset; add to UMCore_CoreSettings::SettingsCollections.
 */
UCLASS(BlueprintType, Const)
class MAEVIXCORE_API UMCore_DA_SettingsCollection : public UDataAsset
{
	GENERATED_BODY()

public:
	// ============================================================================
	// PROPERTIES
	// ============================================================================

	/** Display name for this collection (e.g. "Default Settings", "PC Settings") */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|Settings|Collection")
	FText CollectionName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|Settings|Collection")
	TArray<TObjectPtr<UMCore_DA_SettingDefinition>> Settings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|Settings|Collection",
		meta = (TitleProperty = "SectionDisplayName"))
	TArray<FMCore_SettingsSection> Sections;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|Settings|Collection")
	TMap<FGameplayTag, FText> CategoryDisplayName;

	// ============================================================================
	// METHODS
	// ============================================================================

	/** Returns all settings matching a category tag, in Settings array order. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MaevixCore|Settings")
	TArray<UMCore_DA_SettingDefinition*> GetSettingsInCategory(const FGameplayTag& CategoryTag) const;

	/** Returns all unique category tags in first-seen order across the Settings array. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MaevixCore|Settings")
	TArray<FGameplayTag> GetAllCategories() const;

	const TArray<TObjectPtr<UMCore_DA_SettingDefinition>>& GetAllSettings() const { return Settings; }

	/** Find a single setting by its tag. Returns nullptr if not found. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MaevixCore|Settings")
	UMCore_DA_SettingDefinition* FindSettingByTag(const FGameplayTag& SettingTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MaevixCore|Settings")
	int32 GetSettingCount() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
