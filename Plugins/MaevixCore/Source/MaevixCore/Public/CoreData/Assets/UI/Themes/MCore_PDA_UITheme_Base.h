// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
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
 * Create child DataAssets for each visual theme.
 * Set as active in Project Settings or via UISubsystem at runtime.
 */
UCLASS(Abstract, Blueprintable)
class MAEVIXCORE_API UMCore_PDA_UITheme_Base : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// ============================================================================
	// IDENTITY
	// ============================================================================

	/**
	 * Stable tag of theme. Plugins match per-theme styles
	 * against the active theme based on this tag. No hard-reference to
	 * DataAsset or dependency on registry ordering.
	 * Assign a unique tag per shipped theme.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|Theme")
	FGameplayTag ThemeId;

	// ============================================================================
	// COMMONUI STYLES
	// ============================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|Theme|CommonUI")
	TSubclassOf<UCommonBorderStyle> PanelBorderStyle;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|Theme|CommonUI")
	TSubclassOf<UCommonBorderStyle> HighlightBorderStyle;

	/** Main actions: play, confirm, submit. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|Theme|CommonUI")
	TSubclassOf<UCommonButtonStyle> PrimaryButtonStyle;

	/** Cancel, back, navigation, settings controls. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|Theme|CommonUI")
	TSubclassOf<UCommonButtonStyle> SecondaryButtonStyle;

	/** No background fill, text/icon only with subtle hover. Used for arrow buttons and de-emphasized actions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|Theme|CommonUI")
	TSubclassOf<UCommonButtonStyle> GhostButtonStyle;
	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|Theme|CommonUI")
	TArray<TSubclassOf<UCommonTextStyle>> HeadingTextStyle;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|Theme|CommonUI")
	TArray<TSubclassOf<UCommonTextStyle>> TabTextStyle;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|Theme|CommonUI")
	TArray<TSubclassOf<UCommonTextStyle>> LabelTextStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|Theme|CommonUI")
	TArray<TSubclassOf<UCommonTextStyle>> ValueTextStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|Theme|CommonUI")
	TArray<TSubclassOf<UCommonTextStyle>> DescriptionTextStyle;

	// ============================================================================
	// MAEVIXCORE STYLE DATA ASSETS
	// ============================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|Theme|Styles")
	TObjectPtr<UMCore_PDA_SliderStyle> SliderStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|Theme|Styles")
	TObjectPtr<UMCore_PDA_ScrollbarStyle> ScrollbarStyle;
	
};
