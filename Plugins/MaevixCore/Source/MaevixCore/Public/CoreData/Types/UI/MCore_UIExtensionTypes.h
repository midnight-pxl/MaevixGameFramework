// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MCore_UIExtensionTypes.generated.h"

class UUserWidget;
class UMCore_UIExtensionSubsystem;

/** Action taken on a UI extension (added or removed from extension point). */
UENUM(BlueprintType)
enum class EMCore_UIExtensionAction : uint8
{
	Added,
	Removed
};

/** Opaque handle referencing a UI extension. Call Subsystem->UnregisterExtension(Handle) to unregister. */
USTRUCT(BlueprintType)
struct MAEVIXCORE_API FMCore_UIExtensionHandle
{
	GENERATED_BODY()

public:
	FMCore_UIExtensionHandle()
		: HandleID(0)
	{}

	explicit FMCore_UIExtensionHandle(uint32 InID)
		: HandleID(InID)
	{}

	bool IsValid() const { return HandleID != 0; }
	uint32 GetID() const { return HandleID; }
	void Invalidate() { HandleID = 0; }

	bool operator==(const FMCore_UIExtensionHandle& Other) const { return HandleID == Other.HandleID; }
	bool operator!=(const FMCore_UIExtensionHandle& Other) const { return HandleID != Other.HandleID; }

private:
	UPROPERTY()
	uint32 HandleID;
};

/** Opaque handle referencing a UI extension point. Call Subsystem->UnregisterExtensionPoint(Handle) to unregister. */
USTRUCT(BlueprintType)
struct MAEVIXCORE_API FMCore_UIExtensionPointHandle
{
	GENERATED_BODY()

public:
	FMCore_UIExtensionPointHandle()
		: HandleID(0)
	{}

	explicit FMCore_UIExtensionPointHandle(uint32 InID)
		: HandleID(InID)
	{}

	bool IsValid() const { return HandleID != 0; }
	uint32 GetID() const { return HandleID; }
	void Invalidate() { HandleID = 0; }

	bool operator==(const FMCore_UIExtensionPointHandle& Other) const { return HandleID == Other.HandleID; }
	bool operator!=(const FMCore_UIExtensionPointHandle& Other) const { return HandleID != Other.HandleID; }

private:
	UPROPERTY()
	uint32 HandleID;
};

/** Invoked when extensions are added to or removed from an extension point. */
DECLARE_DYNAMIC_DELEGATE_TwoParams(
	FOnUIExtensionChanged,
	EMCore_UIExtensionAction, Action,
	FMCore_UIExtensionHandle, ExtensionHandle);

/**
 * Internal data representing a UI extension (widget injected into extension points).
 * Not exposed to Blueprint; managed by subsystem.
 */
struct FMCore_UIExtension
{
	FGameplayTag ExtensionPointTag;

	/** Sort priority; higher values appear first (default: 0). */
	int32 Priority{0};

	/**
	 * Optional context for filtering (LocalPlayer, PlayerState, or nullptr for global).
	 * Both nullptr = match. Both valid and same object = match. Otherwise no match.
	 */
	TWeakObjectPtr<UObject> ContextObject;

	TSubclassOf<UUserWidget> WidgetClass;

	uint32 HandleID{0};

	/** Higher priority extensions sort first (fixes Lyra's missing sort). */
	bool operator<(const FMCore_UIExtension& Other) const
	{
		return Priority > Other.Priority;
	}
};

/**
 * Internal data representing a UI extension point (slot that receives injected widgets).
 * Not exposed to Blueprint; managed by subsystem.
 */
struct FMCore_UIExtensionPoint
{
	FGameplayTag ExtensionPointTag;

	/**
	 * Optional context for filtering (LocalPlayer, PlayerState, or nullptr for global).
	 * Both nullptr = match. Both valid and same object = match. Otherwise no match.
	 */
	TWeakObjectPtr<UObject> ContextObject;

	FOnUIExtensionChanged OnChangeCallback;

	/** Validates context matching between this point and the given extension. */
	bool DoesExtensionPassContract(const FMCore_UIExtension* Extension) const;
};
