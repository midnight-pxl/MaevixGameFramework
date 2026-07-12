// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CommonTabListWidgetBase.h"
#include "MCore_TabListBase.generated.h"

class UHorizontalBox;

/**
 * Maevix TabList for housing tab buttons within UIs.
 */
UCLASS(Blueprintable, meta = (DisableNativeTick))
class MAEVIXCORE_API UMCore_TabListBase : public UCommonTabListWidgetBase
{
	GENERATED_BODY()
	
protected:
	virtual void HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton) override;
	virtual void HandleTabRemoval_Implementation(FName TabNameID, UCommonButtonBase* TabButton) override;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HBox_Tabs;
};
