// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Styling/SlateTypes.h"
#include "MCore_ThemeLibrary.generated.h"

class UCommonTextBlock;
class UCommonTextStyle;
class UCommonButtonStyle;
class ULocalPlayer;
class UMCore_PDA_SliderStyle;

/**
 * Stateless theme helpers shared across all Modulus widget classes.
 * Centralizes text style resolution, slider style construction, and button style logic.
 */
UCLASS()
class MODULUSCORE_API UMCore_ThemeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/**
	 * Resolves text style from an array (based on accessibility text size)
	 * and applies it to the target text block.
	 */
	UFUNCTION(BlueprintCallable, Category="ModulusCore|Theme")
	static void ApplyTextStyleFromTheme(const ULocalPlayer* LocalPlayer,
		UCommonTextBlock* TextBlock,
		const TArray<TSubclassOf<UCommonTextStyle>>& TextStyleArray);
	
	/**
	 * Construct FSliderStyle based on SliderStyle DataAsset.
	 * Returns unchanged if DataAsset is null.
	 */
	UFUNCTION(BlueprintPure, Category="ModulusCore|Theme")
	static FSliderStyle BuildSliderStyle(
		const UMCore_PDA_SliderStyle* SliderStyleDA,
		const FSliderStyle& BaseStyle);
	
	/** Returns StyleOverride if valid, ThemeDefault otherwise. */
	UFUNCTION(BlueprintPure, Category="ModulusCore|Theme")
	static TSubclassOf<UCommonButtonStyle> ResolveButtonStyle(
		TSubclassOf<UCommonButtonStyle> StyleOverride,
		TSubclassOf<UCommonButtonStyle> ThemeDefault);
};
