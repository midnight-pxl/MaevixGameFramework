// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MCore_LoadingScreenTypes.generated.h"

class UTexture2D;

/**
 * Selection strategy for picking a loading screen entry from the configured library.
 * Read by UMCore_LoadingScreenSubsystem on each level transition.
 */
UENUM(BlueprintType)
enum class EMCore_LoadingScreenSelectionMode : uint8
{
	Random       UMETA(DisplayName="Random",
	                   ToolTip="Pick uniformly at random from the library"),
	Sequential   UMETA(DisplayName="Sequential",
	                   ToolTip="Cycle through entries in order. Cursor is in-memory only and resets on game restart."),
	TagBased     UMETA(DisplayName="Tag-Based",
	                   ToolTip="Filter by context tags or associated maps, random within the filtered set. Falls back to the any-context pool, then to the full library."),
	Custom       UMETA(DisplayName="Custom",
	                   ToolTip="Override SelectLoadingScreenEntry in a subclass. Default behavior falls through to Random.")
};

/**
 * Controls when the loading screen disappears after the level finishes loading.
 * Read by UMCore_LoadingScreenSubsystem on each transition.
 */
UENUM(BlueprintType)
enum class EMCore_LoadingDismissalMode : uint8
{
	AutoOnLoadComplete UMETA(DisplayName="Auto On Load Complete",
	                         ToolTip="Dismiss as soon as the map finishes loading. Chunk 1 default behavior."),
	InputRequired      UMETA(DisplayName="Input Required",
	                         ToolTip="Wait for any player input after the map finishes loading. Default."),
	Manual             UMETA(DisplayName="Manual",
	                         ToolTip="Wait for an explicit DismissLoadingScreen() call. Use for async post-load or server-authoritative ready barriers.")
};

/**
 * One configurable loading screen instance: background, tip text, and selection hints.
 * Aggregated across all UMCore_PDA_LoadingScreens libraries listed in Project Settings.
 */
USTRUCT(BlueprintType)
struct MODULUSCORE_API FMCore_LoadingScreenEntry
{
	GENERATED_BODY()

	/** Background texture displayed behind the loading widget. Loaded synchronously at PreLoadMap. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ModulusCore|Loading|Screen")
	TSoftObjectPtr<UTexture2D> BackgroundImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ModulusCore|Loading|Screen",
		meta=(MultiLine="true"))
	FText TipText;

	/** Hints for Tag-Based selection. An entry matches when any of its tags overlap the pending context. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ModulusCore|Loading|Screen",
		meta=(Categories="MCore.Loading.Context"))
	FGameplayTagContainer ContextTags;

	/** Map names this entry is preferred for. Matched against the destination map in Tag-Based mode. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ModulusCore|Loading|Screen")
	TArray<FName> AssociatedMaps;
};
