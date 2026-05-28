// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/Types/UI/MCore_UIExtensionTypes.h"

bool FMCore_UIExtensionPoint::DoesExtensionPassContract(const FMCore_UIExtension* Extension) const
{
	if (!Extension) { return false; }

	/* Context matching: both nullptr = match, both same object = match, otherwise no match */
	const bool bExtensionPointHasContext = ContextObject.IsValid();
	const bool bExtensionHasContext = Extension->ContextObject.IsValid();

	/* Both global (no context) */
	if (!bExtensionPointHasContext && !bExtensionHasContext) { return true; }

	/* Both have context - check if same object */
	if (bExtensionPointHasContext && bExtensionHasContext)
	{
		return ContextObject.Get() == Extension->ContextObject.Get();
	}
	/* One has context, other doesn't - no match */
	return false;
}
