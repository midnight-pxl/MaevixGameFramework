// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/Assets/UI/Themes/MCore_PDA_ExtensionStyle_Base.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

EDataValidationResult UMCore_PDA_ExtensionStyle_Base::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!ExtensionTag.IsValid())
	{
		Context.AddError(FText::FromString(
			FString::Printf(TEXT("%s: ExtensionTag not set. Extension styles must declare which tag they belong to."),
				*GetName())));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
