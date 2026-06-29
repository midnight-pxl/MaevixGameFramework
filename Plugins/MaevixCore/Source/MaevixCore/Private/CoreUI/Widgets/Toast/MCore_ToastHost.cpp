// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/Toast/MCore_ToastHost.h"

#include "CoreData/Tags/MCore_ToastTags.h"

#include "Components/PanelWidget.h"

void UMCore_ToastHost::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildAnchorMap();
}

void UMCore_ToastHost::BuildAnchorMap()
{
	using namespace MCore_ToastTags;

	AnchorMap.Reset();

	/* Only authored (bound) panels are mapped; a missing panel leaves its anchor unresolved,
	   which the subsystem handles via its default-anchor fallback. */
	auto MapIfBound = [this](const FGameplayTag& AnchorTag, UPanelWidget* Panel)
	{
		if (Panel)
		{
			AnchorMap.Add(AnchorTag, Panel);
		}
	};

	MapIfBound(MCore_UI_Toast_Anchor_TopLeft,      Panel_TopLeft);
	MapIfBound(MCore_UI_Toast_Anchor_TopCenter,    Panel_TopCenter);
	MapIfBound(MCore_UI_Toast_Anchor_TopRight,     Panel_TopRight);
	MapIfBound(MCore_UI_Toast_Anchor_MiddleCenter, Panel_MiddleCenter);
	MapIfBound(MCore_UI_Toast_Anchor_BottomLeft,   Panel_BottomLeft);
	MapIfBound(MCore_UI_Toast_Anchor_BottomCenter, Panel_BottomCenter);
	MapIfBound(MCore_UI_Toast_Anchor_BottomRight,  Panel_BottomRight);
}

UPanelWidget* UMCore_ToastHost::GetAnchorPanel(FGameplayTag AnchorTag) const
{
	const TObjectPtr<UPanelWidget>* Found = AnchorMap.Find(AnchorTag);
	return Found ? Found->Get() : nullptr;
}

bool UMCore_ToastHost::HasAnchor(FGameplayTag AnchorTag) const
{
	return AnchorMap.Contains(AnchorTag);
}
