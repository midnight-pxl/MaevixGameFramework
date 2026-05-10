// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/Settings/MCore_SettingsCollectionSubsystem.h"

#include "CoreData/DevSettings/MCore_CoreSettings.h"
#include "CoreData/Logging/LogModulusSettings.h"
#include "CoreData/Types/Settings/MCore_DA_SettingDefinition.h"
#include "CoreData/Types/Settings/MCore_DA_SettingsCollection.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UMCore_SettingsCollectionSubsystem* UMCore_SettingsCollectionSubsystem::Get(
	const UObject* WorldContextObject)
{
	if (!WorldContextObject) { return nullptr; }

	const UWorld* World = WorldContextObject->GetWorld();
	if (!World) { return nullptr; }

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance) { return nullptr; }

	return GameInstance->GetSubsystem<UMCore_SettingsCollectionSubsystem>();
}

const TArray<UMCore_DA_SettingsCollection*>&
UMCore_SettingsCollectionSubsystem::GetAllSettingsCollections()
{
	if (!bCollectionsCacheValid)
	{
		ResolvedCollections.Reset();

		const UMCore_CoreSettings* CoreSettings = UMCore_CoreSettings::Get();
		if (CoreSettings)
		{
			for (const TSoftObjectPtr<UMCore_DA_SettingsCollection>& SoftRef : CoreSettings->SettingsCollections)
			{
				if (SoftRef.IsNull())
				{
					continue;
				}
				UMCore_DA_SettingsCollection* Loaded = SoftRef.LoadSynchronous();
				if (Loaded)
				{
					ResolvedCollections.Add(Loaded);
				}
				else
				{
					UE_LOG(LogModulusSettings, Warning,
						TEXT("SettingsCollectionSubsystem::GetAllSettingsCollections: failed to load collection '%s'"),
						*SoftRef.ToString());
				}
			}
		}

		bCollectionsCacheValid = true;
		UE_LOG(LogModulusSettings, Log,
			TEXT("SettingsCollectionSubsystem::GetAllSettingsCollections: loaded %d collection(s)"),
			ResolvedCollections.Num());
	}

	return ResolvedCollections;
}

UMCore_DA_SettingDefinition* UMCore_SettingsCollectionSubsystem::FindSettingDefinitionByTag(
	const FGameplayTag& SettingTag)
{
	for (const UMCore_DA_SettingsCollection* Collection : GetAllSettingsCollections())
	{
		if (UMCore_DA_SettingDefinition* Found = Collection->FindSettingByTag(SettingTag))
		{
			return Found;
		}
	}
	return nullptr;
}

TArray<UMCore_DA_SettingDefinition*> UMCore_SettingsCollectionSubsystem::GetSettingsForCategory(
	const FGameplayTag& CategoryTag)
{
	TArray<UMCore_DA_SettingDefinition*> Combined;
	for (const UMCore_DA_SettingsCollection* Collection : GetAllSettingsCollections())
	{
		Combined.Append(Collection->GetSettingsInCategory(CategoryTag));
	}
	return Combined;
}

TArray<FGameplayTag> UMCore_SettingsCollectionSubsystem::GetAllSettingsCategories()
{
	TArray<FGameplayTag> Result;
	for (const UMCore_DA_SettingsCollection* Collection : GetAllSettingsCollections())
	{
		for (const TObjectPtr<UMCore_DA_SettingDefinition>& Setting : Collection->GetAllSettings())
		{
			if (!Setting || !Setting->CategoryTag.IsValid())
			{
				continue;
			}
			Result.AddUnique(Setting->CategoryTag);
		}
	}
	return Result;
}

FText UMCore_SettingsCollectionSubsystem::GetCategoryDisplayName(const FGameplayTag& CategoryTag)
{
	for (const UMCore_DA_SettingsCollection* Collection : GetAllSettingsCollections())
	{
		if (const FText* Name = Collection->CategoryDisplayName.Find(CategoryTag))
		{
			return *Name;
		}
	}
	/* Fallback: last segment of the tag path */
	const FString TagStr = CategoryTag.ToString();
	int32 LastDot;
	if (TagStr.FindLastChar(TEXT('.'), LastDot))
	{
		return FText::FromString(TagStr.Mid(LastDot + 1));
	}
	return FText::GetEmpty();
}

bool UMCore_SettingsCollectionSubsystem::HasValidSettingsCollections()
{
	return GetAllSettingsCollections().Num() > 0;
}

void UMCore_SettingsCollectionSubsystem::InvalidateCollectionCache()
{
	ResolvedCollections.Reset();
	bCollectionsCacheValid = false;
	UE_LOG(LogModulusSettings, Log,
		TEXT("SettingsCollectionSubsystem::InvalidateCollectionCache: collection cache invalidated"));
}
