// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/Assets/UI/Themes/MCore_PDA_UITheme_Base.h"

#include "CoreData/Assets/UI/Themes/MCore_PDA_ExtensionStyle_Base.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

UMCore_PDA_ExtensionStyle_Base* UMCore_PDA_UITheme_Base::GetExtensionStyle(FGameplayTag StyleTag) const
{
	if (!StyleTag.IsValid()) { return nullptr; }
	const TObjectPtr<UMCore_PDA_ExtensionStyle_Base>* Found = ExtensionStyles.Find(StyleTag);
	return Found ? Found->Get() : nullptr;
}

#if WITH_EDITOR
EDataValidationResult UMCore_PDA_UITheme_Base::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	/* Walk ExtensionStyles map; verify each value's ExtensionTag matches its map key. */
	for (const TPair<FGameplayTag, TObjectPtr<UMCore_PDA_ExtensionStyle_Base>>& Pair : ExtensionStyles)
	{
		if (!Pair.Value) { continue; }
		if (Pair.Value->ExtensionTag != Pair.Key)
		{
			Context.AddError(FText::FromString(
				FString::Printf(TEXT("%s: ExtensionStyles key '%s' references style with mismatched ExtensionTag '%s'. Keys must match the assigned style's ExtensionTag."),
					*GetName(),
					*Pair.Key.ToString(),
					*Pair.Value->ExtensionTag.ToString())));
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}
#endif
