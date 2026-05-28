// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Styling/SlateTypes.h"
#include "Templates/SubclassOf.h"
#include "CoreData/Assets/UI/Themes/MCore_PDA_UITheme_Base.h"
#include "CoreData/Assets/UI/Themes/MCore_PDA_ExtensionStyle_Base.h"
#include "MCore_ThemeLibrary.generated.h"

class UCommonTextBlock;
class UCommonTextStyle;
class UCommonButtonStyle;
class ULocalPlayer;
class UMCore_PDA_SliderStyle;

/**
 * Stateless theme helpers shared across all Maevix widget classes.
 * Centralizes text style resolution, slider style construction, and button style logic.
 */
UCLASS()
class MAEVIXCORE_API UMCore_ThemeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/**
	 * Resolves text style from an array (based on accessibility text size)
	 * and applies it to the target text block.
	 */
	UFUNCTION(BlueprintCallable, Category="MaevixCore|Theme")
	static void ApplyTextStyleFromTheme(const ULocalPlayer* LocalPlayer,
		UCommonTextBlock* TextBlock,
		const TArray<TSubclassOf<UCommonTextStyle>>& TextStyleArray);
	
	/**
	 * Construct FSliderStyle based on SliderStyle DataAsset.
	 * Returns unchanged if DataAsset is null.
	 */
	UFUNCTION(BlueprintPure, Category="MaevixCore|Theme")
	static FSliderStyle BuildSliderStyle(
		const UMCore_PDA_SliderStyle* SliderStyleDA,
		const FSliderStyle& BaseStyle);
	
	/** Returns StyleOverride if valid, ThemeDefault otherwise. */
	UFUNCTION(BlueprintPure, Category="MaevixCore|Theme")
	static TSubclassOf<UCommonButtonStyle> ResolveButtonStyle(
		TSubclassOf<UCommonButtonStyle> StyleOverride,
		TSubclassOf<UCommonButtonStyle> ThemeDefault);

	/**
	 * BP-friendly typed lookup. Returns the extension style at StyleTag cast to TargetClass,
	 * or nullptr if absent or wrong type. The DeterminesOutputType meta lets the BP node's
	 * output pin auto-match TargetClass for chained-cast ergonomics.
	 */
	UFUNCTION(BlueprintPure, Category="MaevixCore|Theme",
		meta=(DeterminesOutputType="TargetClass"))
	static UMCore_PDA_ExtensionStyle_Base* GetExtensionStyleOfClass(
		const UMCore_PDA_UITheme_Base* Theme,
		FGameplayTag StyleTag,
		TSubclassOf<UMCore_PDA_ExtensionStyle_Base> TargetClass);

	/** Templated C++ helper with compile-time guard. Use for type-safe extension lookup. */
	template<typename T>
	static T* GetExtensionStyle(const UMCore_PDA_UITheme_Base* Theme, FGameplayTag Tag)
	{
		static_assert(TIsDerivedFrom<T, UMCore_PDA_ExtensionStyle_Base>::Value,
			"T must derive from UMCore_PDA_ExtensionStyle_Base");
		return Theme ? Cast<T>(Theme->GetExtensionStyle(Tag)) : nullptr;
	}
};
