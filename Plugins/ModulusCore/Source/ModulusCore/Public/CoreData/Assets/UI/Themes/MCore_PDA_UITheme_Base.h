// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MCore_PDA_UITheme_Base.generated.h"

class UCommonButtonStyle;
class UCommonTextStyle;
class UCommonBorderStyle;
class UMCore_PDA_SliderStyle;
class UMCore_PDA_ScrollbarStyle;

/**
 * Master UI theme DataAsset referencing CommonUI styles and modular style DataAssets.
 * Widgets query the active theme via UISubsystem->GetActiveTheme().
 *
 * Create child DataAssets for each visual theme (Minimal, Fantasy, Sci-Fi).
 * Set as active in Project Settings or via UISubsystem at runtime.
 */
UCLASS(Abstract, Blueprintable)
class MODULUSCORE_API UMCore_PDA_UITheme_Base : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// ============================================================================
	// COMMONUI STYLES
	// ============================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CommonUI Styles")
	TSubclassOf<UCommonBorderStyle> PanelBorderStyle;

	/** Main actions: play, confirm, submit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CommonUI Styles")
	TSubclassOf<UCommonButtonStyle> PrimaryButtonStyle;

	/** Cancel, back, navigation, settings controls */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CommonUI Styles")
	TSubclassOf<UCommonButtonStyle> SecondaryButtonStyle;

	/** No background fill, text/icon only with subtle hover. Used for arrow buttons and de-emphasized actions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CommonUI Styles")
	TSubclassOf<UCommonButtonStyle> GhostButtonStyle;
	
	/** 	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CommonUI Styles")
	TArray<TSubclassOf<UCommonTextStyle>> HeadingTextStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CommonUI Styles")
	TArray<TSubclassOf<UCommonTextStyle>> LabelTextStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CommonUI Styles")
	TArray<TSubclassOf<UCommonTextStyle>> ValueTextStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CommonUI Styles")
	TArray<TSubclassOf<UCommonTextStyle>> DescriptionTextStyle;

	// ============================================================================
	// MODULUSCORE STYLE DATA ASSETS
	// ============================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Style DataAssets")
	TObjectPtr<UMCore_PDA_SliderStyle> SliderStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Style DataAssets")
	TObjectPtr<UMCore_PDA_ScrollbarStyle> ScrollbarStyle;
};
