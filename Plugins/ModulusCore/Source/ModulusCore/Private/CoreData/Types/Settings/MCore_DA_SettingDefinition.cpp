// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/Types/Settings/MCore_DA_SettingDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

FString UMCore_DA_SettingDefinition::GetSaveKey() const
{
	if (!SettingTag.IsValid()) { return FString(); }
	/* GameplayTag "Audio.SFXVolume" becomes "Audio_SFXVolume" */
	return SettingTag.ToString().Replace(TEXT("."), TEXT("_"));
}

bool UMCore_DA_SettingDefinition::IsDefinitionValid() const
{
	if (SettingDisplayName.IsEmpty() || !SettingTag.IsValid()) { return false; }

	switch (SettingType)
	{
	case EMCore_SettingType::Slider:
		return MinValue <= MaxValue
		&& DefaultValue >= MinValue
		&& DefaultValue <= MaxValue;

	case EMCore_SettingType::Dropdown:
	{
		const int32 OptionCount = ThemeOptions.Num() > 0 ? ThemeOptions.Num() : DropdownOptions.Num();
		return OptionCount > 0
		&& DefaultDropdownIndex >= 0
		&& DefaultDropdownIndex < OptionCount;
	}

	case EMCore_SettingType::Toggle:
	default:
		return true;
	}
}

#if WITH_EDITOR
EDataValidationResult UMCore_DA_SettingDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!SettingTag.IsValid())
	{
		Context.AddError(FText::FromString(
			FString::Printf(TEXT("%s: SettingTag not set. Setting definitions require a valid gameplay tag."),
				*GetName())));
		Result = EDataValidationResult::Invalid;
	}

	if (SettingDisplayName.IsEmpty())
	{
		Context.AddError(FText::FromString(
			FString::Printf(TEXT("SettingDisplayName: %s is empty"), *GetName())));
		Result = EDataValidationResult::Invalid;
	}

	switch (SettingType)
	{
	case EMCore_SettingType::Slider:
		if (MinValue > MaxValue)
		{
			Context.AddError(FText::FromString(
				FString::Printf(TEXT("%s: MinValue is greater than MaxValue"), *GetName())));
			Result = EDataValidationResult::Invalid;
		}
		if (DefaultValue < MinValue || DefaultValue > MaxValue)
		{
			Context.AddError(FText::FromString(
				FString::Printf(TEXT("%s: DefaultValue is out of range [%.2f, %.2f]"),
					*GetName(), MinValue, MaxValue)));
			Result = EDataValidationResult::Invalid;
		}
		break;

	case EMCore_SettingType::Dropdown:
	{
		const bool bUsesThemeOptions = ThemeOptions.Num() > 0;
		const int32 OptionCount = bUsesThemeOptions ? ThemeOptions.Num() : DropdownOptions.Num();

		if (OptionCount == 0)
		{
			Context.AddError(FText::FromString(
				FString::Printf(TEXT("%s: Dropdown has no options"), *GetName())));
			Result = EDataValidationResult::Invalid;
		}
		else if (DefaultDropdownIndex < 0 || DefaultDropdownIndex >= OptionCount)
		{
			Context.AddError(FText::FromString(
				FString::Printf(TEXT("%s: Dropdown index is out of range [0, %d]"),
					*GetName(), OptionCount-1)));
			Result = EDataValidationResult::Invalid;
		}

		if (bUsesThemeOptions)
		{
			for (int32 i = 0; i < ThemeOptions.Num(); ++i)
			{
				if (ThemeOptions[i].ThemeAsset.IsNull())
				{
					Context.AddError(FText::FromString(FString::Printf(
						TEXT("%s: ThemeOptions[%d] has null ThemeAsset"),
						*GetName(), i)));
					Result = EDataValidationResult::Invalid;
				}
			}

			if (NamedSetter != TEXT("MCore.SetActiveTheme"))
			{
				Context.AddWarning(FText::FromString(FString::Printf(
					TEXT("%s: ThemeOptions is non-empty but NamedSetter is '%s' (expected 'MCore.SetActiveTheme')"),
					*GetName(), *NamedSetter.ToString())));
			}
		}
		break;
	}

	default:
		break;
	}

	return Result;
}
#endif
