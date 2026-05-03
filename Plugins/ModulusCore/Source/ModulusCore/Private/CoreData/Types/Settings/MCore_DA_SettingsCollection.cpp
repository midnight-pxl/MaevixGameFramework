// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/Types/Settings/MCore_DA_SettingsCollection.h"

#include "CoreData/Types/Settings/MCore_DA_SettingDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

TArray<UMCore_DA_SettingDefinition*> UMCore_DA_SettingsCollection::GetSettingsInCategory(
	const FGameplayTag& CategoryTag) const
{
	TArray<UMCore_DA_SettingDefinition*> SettingOrder;

	for (const TObjectPtr<UMCore_DA_SettingDefinition>& Setting : Settings)
	{
		if (Setting && Setting->CategoryTag == CategoryTag) { SettingOrder.Add(Setting); }
	}

	return SettingOrder;
}

TArray<FGameplayTag> UMCore_DA_SettingsCollection::GetAllCategories() const
{
	TArray<FGameplayTag> ResultArray;

	for (const TObjectPtr<UMCore_DA_SettingDefinition>& Setting : Settings)
	{
		if (!Setting || !Setting->CategoryTag.IsValid()) { continue; }
		ResultArray.AddUnique(Setting->CategoryTag);
	}

	return ResultArray;
}

UMCore_DA_SettingDefinition* UMCore_DA_SettingsCollection::FindSettingByTag(const FGameplayTag& SettingTag) const
{
	for (const TObjectPtr<UMCore_DA_SettingDefinition>& Setting : Settings)
	{
		if (Setting && Setting->SettingTag == SettingTag) { return Setting; }
	}

	return nullptr;
}

int32 UMCore_DA_SettingsCollection::GetSettingCount() const
{
	int32 Count{0};

	for (const TObjectPtr<UMCore_DA_SettingDefinition>& Setting : Settings)
	{
		if (Setting) { Count++; }
	}

	return Count;
}

#if WITH_EDITOR
EDataValidationResult UMCore_DA_SettingsCollection::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (CollectionName.IsEmpty())
	{
		Context.AddWarning((FText::FromString(
			FString::Printf(TEXT("%s: CollectionName is empty"), *GetName()))));
		Result = EDataValidationResult::Invalid;
	}

	if (Settings.Num() == 0)
	{
		Context.AddWarning((FText::FromString(
			FString::Printf(TEXT("%s: Settings array is zero"), *GetName()))));
		Result = EDataValidationResult::Invalid;
	}

	/* Check for duplicates and null tags */
	TSet<FGameplayTag> SeenTags;

	for (int32 idx = 0; idx < Settings.Num(); ++idx)
	{
		if (!Settings[idx])
		{
			Context.AddWarning((FText::FromString(
				FString::Printf(TEXT("%s: Null entry at index %d"),
					*GetName(), idx))));
			Result = EDataValidationResult::Invalid;
			continue;
		}

		const FGameplayTag& SettingTag = Settings[idx]->SettingTag;

		if (!SettingTag.IsValid())
		{
			Context.AddWarning((FText::FromString(
				FString::Printf(TEXT("%s: SettingTag at index %d is invalid"),
					*GetName(), idx))));
			Result = EDataValidationResult::Invalid;
		}
		else
		{
			SeenTags.Add(SettingTag);
		}
	}

	return Result;
}
#endif
