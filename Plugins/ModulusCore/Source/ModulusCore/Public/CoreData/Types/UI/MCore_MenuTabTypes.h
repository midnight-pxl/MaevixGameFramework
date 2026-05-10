// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MCore_MenuTabTypes.generated.h"

class UCommonActivatableWidget;

/**
 * Menu tab registration data for the in-game menu hub.
 * Stores tab identity, widget class, icon, and sort priority for plugin screen registration.
 */
USTRUCT(BlueprintType)
struct MODULUSCORE_API FMCore_MenuTab
{
  	GENERATED_BODY()

public:

  	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu Tab")
  	FGameplayTag TabID;

  	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu Tab")
  	TSubclassOf<UCommonActivatableWidget> ScreenWidgetClass;

  	/** Sort order (lower = left, higher = right) */
  	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu Tab")
  	int32 Priority{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu Tab")
	TObjectPtr<UTexture2D> TabIcon{nullptr};

	/** Extracts display name from the last segment of TabID (e.g. "Inventory" from "MVault.UI.Categories.Inventory") */
	FText GetDisplayName() const
	{
		if (!TabID.IsValid())
		{
			return FText::FromString("Invalid Tab");
		}

		FString TagString = TabID.ToString();
		int32 LastDotIndex;
		if (TagString.FindLastChar('.', LastDotIndex))
		{
			return FText::FromString(TagString.RightChop(LastDotIndex + 1));
		}

		return FText::FromString(TagString);
	}

  	FMCore_MenuTab()
  		: Priority(0)
		, TabIcon(nullptr)
  	{}
};
