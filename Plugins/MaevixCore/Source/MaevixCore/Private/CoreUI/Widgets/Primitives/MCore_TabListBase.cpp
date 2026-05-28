// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/Primitives/MCore_TabListBase.h"

#include "CommonButtonBase.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"

void UMCore_TabListBase::HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton)
{
	if (HBox_Tabs && TabButton)
	{
		if (UHorizontalBoxSlot* TabSlot = HBox_Tabs->AddChildToHorizontalBox(TabButton))
		{
			TabSlot->SetHorizontalAlignment(HAlign_Fill);
			TabSlot->SetVerticalAlignment(VAlign_Fill);
		}
	}
}

void UMCore_TabListBase::HandleTabRemoval_Implementation(FName TabNameID, UCommonButtonBase* TabButton)
{
	if (HBox_Tabs && TabButton) { HBox_Tabs->RemoveChild(TabButton); }
}
