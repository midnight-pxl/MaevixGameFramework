// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "CoreData/Types/UI/MCore_UIExtensionTypes.h"
#include "MCore_UIExtensionSubsystem.generated.h"

class UUserWidget;

/**
 * World subsystem for tag-based widget injection between decoupled plugins.
 * Extension points declare UI slots; extensions provide widgets, matched by GameplayTag
 * with optional per-LocalPlayer context filtering and priority ordering. Event-driven
 * (zero per-frame cost).
 */
UCLASS()
class MODULUSCORE_API UMCore_UIExtensionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Register an extension point that receives widget injections.
	 * Callback is immediately invoked for any existing matching extensions, sorted by priority.
	 */
	UFUNCTION(BlueprintCallable, Category="ModulusCore|UI Extension", 
		meta=(AutoCreateRefTerm="ExtensionCallback"))
	FMCore_UIExtensionPointHandle RegisterExtensionPoint(
		const FGameplayTag& ExtensionPointTag,
		UObject* ContextObject,
		const FOnUIExtensionChanged& ExtensionCallback);

	/**
	 * Unregister extension point (stop receiving extensions).
	 * Safe to call with invalid handle.
	 */
	UFUNCTION(BlueprintCallable, Category="ModulusCore|UI Extension")
	void UnregisterExtensionPoint(UPARAM(ref) FMCore_UIExtensionPointHandle& ExtensionPointHandle);

	/**
	 * Register a widget to inject at matching extension points.
	 * Immediately notifies all matching extension points of the new extension.
	 */
	UFUNCTION(BlueprintCallable, Category="ModulusCore|UI Extension")
	FMCore_UIExtensionHandle RegisterExtension(
		const FGameplayTag& ExtensionPointTag,
		UObject* ContextObject,
		TSubclassOf<UUserWidget> WidgetClass,
		int32 Priority = 0);

	/**
	 * Unregister extension (stop injecting into extension points).
	 * Notifies all extension points to remove widget.
	 * Safe to call with invalid handle.
	 */
	UFUNCTION(BlueprintCallable, Category="ModulusCore|UI Extension")
	void UnregisterExtension(UPARAM(ref) FMCore_UIExtensionHandle& ExtensionHandle);

	/**
	 * Get widget class for extension handle.
	 * Used by extension point widgets to create instances.
	 */
	UFUNCTION(BlueprintCallable, Category="ModulusCore|UI Extension")
	TSubclassOf<UUserWidget> GetWidgetClassForExtension(const FMCore_UIExtensionHandle& ExtensionHandle) const;

	/** Check if extension handle is currently registered */
	UFUNCTION(BlueprintPure, Category="ModulusCore|UI Extension")
	bool IsExtensionRegistered(const FMCore_UIExtensionHandle& ExtensionHandle) const;

	/** Check if extension point handle is currently registered */
	UFUNCTION(BlueprintPure, Category="ModulusCore|UI Extension")
	bool IsExtensionPointRegistered(const FMCore_UIExtensionPointHandle& ExtensionPointHandle) const;

private:
	void NotifyExtensionPointsOfExtension(EMCore_UIExtensionAction Action, const FMCore_UIExtension* Extension);
	void NotifyExtensionPointOfExtensions(const FMCore_UIExtensionPoint* ExtensionPoint);

	TMap<FGameplayTag, TArray<TSharedPtr<FMCore_UIExtensionPoint>>> ExtensionPointMap;
	TMap<FGameplayTag, TArray<TSharedPtr<FMCore_UIExtension>>> ExtensionMap;
	TMap<uint32, TSharedPtr<FMCore_UIExtensionPoint>> ExtensionPointHandles;
	TMap<uint32, TSharedPtr<FMCore_UIExtension>> ExtensionHandles;
	uint32 NextHandleID{1};
};