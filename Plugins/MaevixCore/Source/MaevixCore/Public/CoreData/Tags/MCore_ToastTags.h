// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "NativeGameplayTags.h"

namespace MCore_ToastTags
{
	/* Seven anchors: four corners plus three center-column positions. A toast-private
	   placement taxonomy mirroring MCore.UI.Layer.*; not promoted to a shared set. */
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_UI_Toast_Anchor_TopLeft);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_UI_Toast_Anchor_TopCenter);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_UI_Toast_Anchor_TopRight);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_UI_Toast_Anchor_MiddleCenter);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_UI_Toast_Anchor_BottomLeft);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_UI_Toast_Anchor_BottomCenter);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_UI_Toast_Anchor_BottomRight);

	/* Style variants. Theme-resolved look is mapped to visuals inside the toast WBP. */
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_UI_Toast_Style_Info);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_UI_Toast_Style_Success);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_UI_Toast_Style_Warning);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_UI_Toast_Style_Error);
}
