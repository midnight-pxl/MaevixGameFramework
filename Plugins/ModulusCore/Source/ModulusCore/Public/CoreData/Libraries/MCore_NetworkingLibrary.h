// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MCore_NetworkingLibrary.generated.h"

/** Network mode types for UE multiplayer */
UENUM(BlueprintType)
enum class EMCore_NetMode : uint8
{
	DedicatedServer UMETA(DisplayName = "Dedicated Server"),
	ListenServer	UMETA(DisplayName = "Listen Server (Host)"),
	Client			UMETA(DisplayName = "Client"),
    Standalone		UMETA(DisplayName = "Single-Player")
};

/**
 * Static network helpers for UObjects that cannot use UMCore_NetworkingComponent.
 * Provides authority checks and network mode queries for widgets, subsystems, and data assets.
 *
 * Use UMCore_NetworkingComponent instead for Actors and ActorComponents.
 */
UCLASS()
class MODULUSCORE_API UMCore_NetworkingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// ============================================================================
	// NETWORK MODE QUERIES
	// ============================================================================

	/** Returns the current network mode. For simple checks, use IsServer/IsClient instead. */
	UFUNCTION(BlueprintPure, Category = "Modulus|Network",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Get Network Mode"))
	static EMCore_NetMode GetNetworkMode(const UObject* WorldContextObject);

	/** Returns true if running on server (dedicated or listen). */
	UFUNCTION(BlueprintPure, Category = "Modulus|Network",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Is Server"))
	static bool IsServer(const UObject* WorldContextObject);

	/** Returns true if running on client (remote or listen). */
	UFUNCTION(BlueprintPure, Category = "Modulus|Network",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Is Client"))
	static bool IsClient(const UObject* WorldContextObject);

	/** Returns true only if running on dedicated server (no local player). */
	UFUNCTION(BlueprintPure, Category = "Modulus|Network",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Is Dedicated Server"))
	static bool IsDedicatedServer(const UObject* WorldContextObject);

	/** Returns true only if running as listen server (hosting with local player). */
	UFUNCTION(BlueprintPure, Category = "Modulus|Network",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Is Listen Server"))
	static bool IsListenServer(const UObject* WorldContextObject);

	// ============================================================================
	// WIDGET AUTHORITY HELPERS
	// ============================================================================

	/** Returns true if widget's owning player has network authority. Use before triggering server RPCs from UI. */
	UFUNCTION(BlueprintPure, Category = "Modulus|Network|Widget",
		meta = (DefaultToSelf = "Widget", DisplayName = "Has Network Authority (Widget)"))
	static bool Widget_HasNetworkAuthority(const UUserWidget* Widget);

	/** Returns true if widget can execute server operations. Validates authority and valid player controller. */
	UFUNCTION(BlueprintPure, Category = "Modulus|Network|Widget",
		meta = (DefaultToSelf = "Widget", DisplayName = "Can Execute Server Operation (Widget)"))
	static bool Widget_CanExecuteServerOperation(const UUserWidget* Widget);

	/** Returns true if widget can execute client operations. Returns false on dedicated server. */
	UFUNCTION(BlueprintPure, Category = "Modulus|Network|Widget",
		meta = (DefaultToSelf = "Widget", DisplayName = "Can Execute Client Operation (Widget)"))
	static bool Widget_CanExecuteClientOperation(const UUserWidget* Widget);

	// ============================================================================
	// WIDGET CONTEXT ACCESS
	// ============================================================================

	/** Returns the widget's owning player controller, or nullptr on dedicated server. */
	UFUNCTION(BlueprintPure, Category = "Modulus|Network|Widget",
		meta = (DefaultToSelf = "Widget", DisplayName = "Get Owning Player Controller (Widget)"))
	static APlayerController* Widget_GetOwningPlayerController(const UUserWidget* Widget);

	/** Returns the widget's owning player's pawn, or nullptr if not possessed. */
	UFUNCTION(BlueprintPure, Category = "Modulus|Network|Widget",
		meta = (DefaultToSelf = "Widget", DisplayName = "Get Owning Pawn (Widget)"))
	static APawn* Widget_GetOwningPawn(const UUserWidget* Widget);

	// ============================================================================
	// SUBSYSTEM HELPERS
	// ============================================================================

	/** Returns true if subsystem is running on server. */
	UFUNCTION(BlueprintPure, Category = "Modulus|Network|Subsystem",
		meta = (DefaultToSelf = "Subsystem", DisplayName = "Is Server (Subsystem)"))
	static bool Subsystem_IsServer(const USubsystem* Subsystem);

	/** Currently equivalent to Subsystem_IsServer. Provided as a separate intent-named API for caller readability. */
	UFUNCTION(BlueprintPure, Category = "Modulus|Network|Subsystem",
		meta = (DefaultToSelf = "Subsystem", DisplayName = "Can Execute Server Operation (Subsystem)"))
	static bool Subsystem_CanExecuteServerOperation(const USubsystem* Subsystem);
};
