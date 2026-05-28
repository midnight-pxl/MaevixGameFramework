// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/Assets/Loading/MCore_PDA_LoadingScreens.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "MCoreLoadingScreens"

FPrimaryAssetId UMCore_PDA_LoadingScreens::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("MCore_LoadingScreens"), GetFName());
}

#if WITH_EDITOR
EDataValidationResult UMCore_PDA_LoadingScreens::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (Entries.Num() == 0)
	{
		Context.AddWarning(LOCTEXT("NoEntries", "Loading screen library has no entries. Configure at least one entry or this library contributes nothing to the selection pool."));
		Result = CombineDataValidationResults(Result, EDataValidationResult::Valid);
	}

	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		const FMCore_LoadingScreenEntry& Entry = Entries[i];

		if (Entry.BackgroundImage.IsNull())
		{
			Context.AddWarning(FText::Format(
				LOCTEXT("MissingBackground", "Entry [{0}] has no BackgroundImage. The loading screen will display the widget over an empty background."),
				FText::AsNumber(i)));
		}

		for (int32 MapIdx = 0; MapIdx < Entry.AssociatedMaps.Num(); ++MapIdx)
		{
			if (Entry.AssociatedMaps[MapIdx].IsNone())
			{
				Context.AddWarning(FText::Format(
					LOCTEXT("EmptyAssociatedMap", "Entry [{0}] has an empty FName in AssociatedMaps[{1}]. It will never match any map."),
					FText::AsNumber(i), FText::AsNumber(MapIdx)));
			}
		}
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
