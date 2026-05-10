// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/Libraries/MCore_InputDisplayLibrary.h"

#include "CoreData/Logging/LogModulusUI.h"
#include "CoreData/Settings/MCore_PlayerSettingsSubsystem.h"
#include "CoreData/Types/Settings/MCore_PlayerSettingsSave.h"

#include "EnhancedInputSubsystemInterface.h"
#include "EnhancedInput/Public/UserSettings/EnhancedInputUserSettings.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "GameFramework/PlayerController.h"
#include "CommonInputSubsystem.h"
#include "CommonInputBaseTypes.h"

// ============================================================================
// PRIVATE HELPERS
// ============================================================================

/*
 * Resolve a UInputAction* to its profile MappingName by iterating rows
 * and matching on GetAssociatedInputAction(). Returns NAME_None on failure.
 */
static FName FindMappingNameForAction(
	const UEnhancedPlayerMappableKeyProfile* Profile,
	const UInputAction* InputAction)
{
	const TMap<FName, FKeyMappingRow>& AllRows = Profile->GetPlayerMappingRows();
	for (const auto& Pair : AllRows)
	{
		for (const FPlayerKeyMapping& Mapping : Pair.Value.Mappings)
		{
			if (Mapping.GetAssociatedInputAction() == InputAction)
			{
				return Pair.Key;
			}
		}
	}
	return NAME_None;
}

UEnhancedInputLocalPlayerSubsystem* UMCore_InputDisplayLibrary::GetEnhancedInputSubsystem(
	APlayerController* OwningPlayer)
{
	if (!OwningPlayer)
	{
		UE_LOG(LogModulusUI, Error,
			TEXT("EnhancedInputDisplay::GetEnhancedInputSubsystem: OwningPlayer is null"));
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = OwningPlayer->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogModulusUI, Error,
			TEXT("EnhancedInputDisplay::GetEnhancedInputSubsystem: LocalPlayer is null"));
		return nullptr;
	}

	return LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}

UEnhancedPlayerMappableKeyProfile* UMCore_InputDisplayLibrary::GetActiveKeyProfile(
	APlayerController* OwningPlayer)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem(OwningPlayer);
	if (!Subsystem) { return nullptr; }

	UEnhancedInputUserSettings* UserSettings = Subsystem->GetUserSettings();
	if (!UserSettings) { return nullptr; }

	return UserSettings->GetActiveKeyProfile();
}

// ============================================================================
// KEY QUERIES
// ============================================================================

FKey UMCore_InputDisplayLibrary::GetCurrentKeyForAction(APlayerController* OwningPlayer,
	UInputAction* InputAction)
{
	if (!InputAction) { return EKeys::Invalid; }

	UEnhancedPlayerMappableKeyProfile* Profile = GetActiveKeyProfile(OwningPlayer);
	if (!Profile) { return EKeys::Invalid; }

	const FName MappingName = FindMappingNameForAction(Profile, InputAction);
	const FKeyMappingRow* KeyRow = MappingName.IsNone() ? nullptr : Profile->FindKeyMappingRow(MappingName);
	if (!KeyRow) { return EKeys::Invalid; }

	for (const FPlayerKeyMapping& KeyMapping : KeyRow->Mappings)
	{
		return KeyMapping.GetCurrentKey();
	}

	return EKeys::Invalid;
}

TArray<FKey> UMCore_InputDisplayLibrary::GetAllKeysForAction(APlayerController* OwningPlayer,
	UInputAction* InputAction)
{
	TArray<FKey> Keys;
	if (!InputAction) { return Keys; }

	UEnhancedPlayerMappableKeyProfile* KeyProfile = GetActiveKeyProfile(OwningPlayer);
	if (!KeyProfile) { return Keys; }

	const FName MappingName = FindMappingNameForAction(KeyProfile, InputAction);
	const FKeyMappingRow* KeyRow = MappingName.IsNone() ? nullptr : KeyProfile->FindKeyMappingRow(MappingName);
	if (!KeyRow) { return Keys; }

	for (const FPlayerKeyMapping& KeyMapping : KeyRow->Mappings)
	{
		Keys.Add(KeyMapping.GetCurrentKey());
	}

	return Keys;
}

// ============================================================================
// ACTION METADATA
// ============================================================================

FText UMCore_InputDisplayLibrary::GetActionDisplayName(APlayerController* OwningPlayer,
	UInputAction* InputAction)
{
	if (!InputAction) { return FText::FromString(TEXT("Invalid InputAction")); }

	UEnhancedPlayerMappableKeyProfile* KeyProfile = GetActiveKeyProfile(OwningPlayer);
	if (!KeyProfile) { return FText::FromString(InputAction->GetName()); }

	const FName MappingName = FindMappingNameForAction(KeyProfile, InputAction);
	const FKeyMappingRow* KeyRow = MappingName.IsNone() ? nullptr : KeyProfile->FindKeyMappingRow(MappingName);
	if (!KeyRow) { return FText::FromString(InputAction->GetName()); }

	for (const FPlayerKeyMapping& KeyMapping : KeyRow->Mappings)
	{
		FText DisplayName = KeyMapping.GetDisplayName();
		if (!DisplayName.IsEmpty()) { return DisplayName; }
	}

	return FText::FromString(InputAction->GetName());
}

FText UMCore_InputDisplayLibrary::GetActionDisplayCategory(APlayerController* OwningPlayer,
	UInputAction* InputAction)
{
	if (!InputAction) { return FText::FromString(TEXT("Invalid InputAction")); }

	UEnhancedPlayerMappableKeyProfile* KeyProfile = GetActiveKeyProfile(OwningPlayer);
	if (!KeyProfile) { return FText::FromString(TEXT("General")); }

	const FName MappingName = FindMappingNameForAction(KeyProfile, InputAction);
	const FKeyMappingRow* KeyRow = MappingName.IsNone() ? nullptr : KeyProfile->FindKeyMappingRow(MappingName);
	if (!KeyRow) { return FText::FromString(TEXT("General")); }

	for (const FPlayerKeyMapping& KeyMapping : KeyRow->Mappings)
	{
		FText DisplayCat = KeyMapping.GetDisplayCategory();
		if (!DisplayCat.IsEmpty()) { return DisplayCat; }
	}

	return FText::FromString(TEXT("General"));
}

FName UMCore_InputDisplayLibrary::GetActionMappingName(APlayerController* OwningPlayer,
	UInputAction* InputAction)
{
	if (!InputAction) { return NAME_None; }

	UEnhancedPlayerMappableKeyProfile* KeyProfile = GetActiveKeyProfile(OwningPlayer);
	if (!KeyProfile) { return NAME_None; }

	const FName MappingName = FindMappingNameForAction(KeyProfile, InputAction);
	const FKeyMappingRow* KeyRow = MappingName.IsNone() ? nullptr : KeyProfile->FindKeyMappingRow(MappingName);
	if (!KeyRow) { return NAME_None; }

	return MappingName;
}

bool UMCore_InputDisplayLibrary::IsActionRemappable(APlayerController* OwningPlayer,
	UInputAction* InputAction)
{
	if (!InputAction) { return false; }

	UEnhancedPlayerMappableKeyProfile* KeyProfile = GetActiveKeyProfile(OwningPlayer);
	if (!KeyProfile) { return false; }

	return !FindMappingNameForAction(KeyProfile, InputAction).IsNone();
}

// ============================================================================
// REBINDING OPERATIONS
// ============================================================================

/**
 * @param InputAction  Must reference a remappable action with PlayerMappableKeySettings.
 * @param NewKey       Must be a valid key.
 * @param OutError     Populated with a description on failure.
 * @return             True on successful rebind, false on validation or mapping failure.
 */
bool UMCore_InputDisplayLibrary::RemapActionKey(APlayerController* OwningPlayer,
	UInputAction* InputAction,
	FKey NewKey,
	FText& OutError)
{
	OutError = FText::GetEmpty();

	if (!InputAction)
	{
		OutError = FText::FromString(TEXT("Input Action is null"));
		return false;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem(OwningPlayer);
	if (!Subsystem)
	{
		OutError = FText::FromString(TEXT("Enhanced Input subsystem not available"));
		return false;
	}

	UEnhancedInputUserSettings* UserSettings = Subsystem->GetUserSettings();
	if (!UserSettings)
	{
		OutError = FText::FromString(TEXT("User settings not initialized"));
		return false;
	}

	/* Confirm action is remappable before attempt */
	UEnhancedPlayerMappableKeyProfile* KeyProfile = UserSettings->GetActiveKeyProfile();
	if (!KeyProfile)
	{
		OutError = FText::FromString(TEXT("Action not remappable or mapping invalid"));
		return false;
	}
	FName MappingName = FindMappingNameForAction(KeyProfile, InputAction);
	if (MappingName.IsNone())
	{
		OutError = FText::FromString(TEXT("Action not remappable or mapping invalid"));
		return false;
	}

	FMapPlayerKeyArgs KeyArgs;
	KeyArgs.MappingName = MappingName;
	KeyArgs.NewKey = NewKey;
	KeyArgs.Slot = EPlayerMappableKeySlot::First;

	FGameplayTagContainer FailureReason;
	UserSettings->MapPlayerKey(KeyArgs, FailureReason);

	if (FailureReason.IsEmpty())
	{
		UserSettings->SaveSettings();
		UE_LOG(LogModulusUI, Log,
			TEXT("EnhancedInputDisplay::RemapActionKey: successfully remapped action %s to key %s"),
			*InputAction->GetName(), *NewKey.ToString());
		return true;
	}

	OutError = FText::FromString(FailureReason.ToString());
	UE_LOG(LogModulusUI, Warning,
		TEXT("EnhancedInputDisplay::RemapActionKey: failed to remap action %s to key %s: %s"),
		*InputAction->GetName(), *NewKey.ToString(), *FailureReason.ToString());
	return false;
}

/**
 * @param InputAction  Must reference a remappable action with PlayerMappableKeySettings.
 * @param OutError     Populated with a description on failure.
 * @return             True on successful reset, false on validation or mapping failure.
 */
bool UMCore_InputDisplayLibrary::ResetActionToDefault(APlayerController* OwningPlayer,
	UInputAction* InputAction,
	FText& OutError)
{
	OutError = FText::GetEmpty();

	if (!InputAction)
	{
		OutError = FText::FromString(TEXT("Input Action is null"));
		return false;
	}

	UEnhancedPlayerMappableKeyProfile* KeyProfile = GetActiveKeyProfile(OwningPlayer);
	if (!KeyProfile)
	{
		OutError = FText::FromString(TEXT("No active key profile"));
		return false;
	}

	const FName MappingName = FindMappingNameForAction(KeyProfile, InputAction);
	const FKeyMappingRow* MappingRow = MappingName.IsNone() ? nullptr : KeyProfile->FindKeyMappingRow(MappingName);
	if (!MappingRow)
	{
		OutError = FText::FromString(TEXT("Action not found in key profile"));
		return false;
	}

	for (const FPlayerKeyMapping& Mapping : MappingRow->Mappings)
	{
		FKey DefaultKey = Mapping.GetDefaultKey();
		return RemapActionKey(OwningPlayer, InputAction, DefaultKey, OutError);
	}

	OutError = FText::FromString(TEXT("No mappings found for action"));
	return false;
}

// ============================================================================
// REMAPPABLE ACTION QUERIES
// ============================================================================

TArray<FPlayerKeyMapping> UMCore_InputDisplayLibrary::GetAllRemappableActions(
	APlayerController* OwningPlayer)
{
	TArray<FPlayerKeyMapping> Result;

	UEnhancedPlayerMappableKeyProfile* Profile = GetActiveKeyProfile(OwningPlayer);
	if (!Profile)
	{
		UE_LOG(LogModulusUI, Warning,
			TEXT("EnhancedInputDisplay::GetAllRemappableActions: no active key profile"));
		return Result;
	}

	const TMap<FName, FKeyMappingRow>& Rows = Profile->GetPlayerMappingRows();
	for (const TPair<FName, FKeyMappingRow>& Pair : Rows)
	{
		for (const FPlayerKeyMapping& Mapping : Pair.Value.Mappings)
		{
			Result.Add(Mapping);
		}
	}

	return Result;
}

TArray<FPlayerKeyMapping> UMCore_InputDisplayLibrary::GetRemappableActionsForDevice(
	APlayerController* OwningPlayer, ECommonInputType DeviceType)
{
	TArray<FPlayerKeyMapping> Result;

	UEnhancedPlayerMappableKeyProfile* Profile = GetActiveKeyProfile(OwningPlayer);
	if (!Profile)
	{
		UE_LOG(LogModulusUI, Warning,
			TEXT("EnhancedInputDisplay::GetRemappableActionsForDevice: no active key profile"));
		return Result;
	}

	const TMap<FName, FKeyMappingRow>& Rows = Profile->GetPlayerMappingRows();
	for (const TPair<FName, FKeyMappingRow>& Pair : Rows)
	{
		for (const FPlayerKeyMapping& Mapping : Pair.Value.Mappings)
		{
			/* Use current key for device check; fall back to default if current is invalid */
			FKey KeyToCheck = Mapping.GetCurrentKey();
			if (!KeyToCheck.IsValid())
			{
				KeyToCheck = Mapping.GetDefaultKey();
			}

			bool bInclude = false;
			switch (DeviceType)
			{
			case ECommonInputType::Gamepad:
				bInclude = KeyToCheck.IsGamepadKey();
				break;
			case ECommonInputType::MouseAndKeyboard:
				bInclude = !KeyToCheck.IsGamepadKey() && !KeyToCheck.IsTouch();
				break;
			case ECommonInputType::Touch:
				bInclude = KeyToCheck.IsTouch();
				break;
			default:
				break;
			}

			if (bInclude)
			{
				Result.Add(Mapping);
			}
		}
	}

	return Result;
}

// ============================================================================
// ICON RESOLUTION
// ============================================================================

bool UMCore_InputDisplayLibrary::GetIconBrushForKey(const ULocalPlayer* LocalPlayer,
	FKey Key, FSlateBrush& OutBrush)
{
	if (!LocalPlayer || !Key.IsValid()) { return false; }

	UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer);
	if (!CommonInputSubsystem)
	{
		UE_LOG(LogModulusUI, Warning,
			TEXT("EnhancedInputDisplay::GetIconBrushForKey: CommonInputSubsystem unavailable"));
		return false;
	}

	UCommonInputPlatformSettings* PlatformSettings = UCommonInputPlatformSettings::Get();
	if (!PlatformSettings) { return false; }

	return PlatformSettings->TryGetInputBrush(
		OutBrush, Key, CommonInputSubsystem->GetCurrentInputType(), CommonInputSubsystem->GetCurrentGamepadName());
}

bool UMCore_InputDisplayLibrary::GetIconBrushForKeyByDeviceType(const ULocalPlayer* LocalPlayer,
	FKey Key, ECommonInputType InputType, FSlateBrush& OutBrush)
{
	if (!LocalPlayer || !Key.IsValid()) { return false; }

	UCommonInputPlatformSettings* PlatformSettings = UCommonInputPlatformSettings::Get();
	if (!PlatformSettings) { return false; }

	/* Gamepad icons need the active gamepad name to pick the right controller data */
	FName GamepadName{NAME_None};
	if (InputType == ECommonInputType::Gamepad)
	{
		UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer);
		if (CommonInputSubsystem)
		{
			GamepadName = CommonInputSubsystem->GetCurrentGamepadName();
		}
	}

	return PlatformSettings->TryGetInputBrush(OutBrush, Key, InputType, GamepadName);
}

// ============================================================================
// GAMEPAD CONFIG HELPERS
// ============================================================================

TArray<FName> UMCore_InputDisplayLibrary::GetAvailableGamepadConfigs(
	const UObject* WorldContextObject)
{
	TArray<FName> Result;

	UCommonInputPlatformSettings* PlatformSettings = UCommonInputPlatformSettings::Get();
	if (!PlatformSettings) { return Result; }

	const TArray<TSoftClassPtr<UCommonInputBaseControllerData>> ControllerData =
		PlatformSettings->GetControllerData();

	for (const TSoftClassPtr<UCommonInputBaseControllerData>& SoftClass : ControllerData)
	{
		const UClass* LoadedClass = SoftClass.LoadSynchronous();
		if (!LoadedClass) { continue; }

		const UCommonInputBaseControllerData* CDO =
			LoadedClass->GetDefaultObject<UCommonInputBaseControllerData>();
		if (!CDO || CDO->InputType != ECommonInputType::Gamepad) { continue; }

		Result.AddUnique(CDO->GamepadName);
	}

	return Result;
}

FName UMCore_InputDisplayLibrary::GetEffectiveGamepadName(
	const UObject* WorldContextObject, const ULocalPlayer* LocalPlayer)
{
	if (!LocalPlayer) { return NAME_None; }

	/* Read the player's override index */
	int32 OverrideIndex{0};
	UMCore_PlayerSettingsSubsystem* SettingsSubsystem = LocalPlayer->GetSubsystem<UMCore_PlayerSettingsSubsystem>();
	if (SettingsSubsystem)
	{
		if (UMCore_PlayerSettingsSave* Save = SettingsSubsystem->GetPlayerSettings())
		{
			OverrideIndex = Save->GamepadIconSetIndex;
		}
	}

	/* 0 = Auto-Detect: use whatever CommonInput reports */
	if (OverrideIndex == 0)
	{
		UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer);
		return CommonInputSubsystem ? CommonInputSubsystem->GetCurrentGamepadName() : NAME_None;
	}

	/* Non-zero: look up the config at (index - 1) */
	TArray<FName> Configs = GetAvailableGamepadConfigs(WorldContextObject);
	const int32 AdjustedIndex = OverrideIndex - 1;

	if (Configs.IsValidIndex(AdjustedIndex))
	{
		return Configs[AdjustedIndex];
	}

	/* Out of range: fall back to auto-detect */
	UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer);
	return CommonInputSubsystem ? CommonInputSubsystem->GetCurrentGamepadName() : NAME_None;
}

// ============================================================================
// SLOT-BASED OPERATIONS
// ============================================================================

FKey UMCore_InputDisplayLibrary::GetBoundKeyForSlot(APlayerController* OwningPlayer,
	UInputAction* InputAction, EPlayerMappableKeySlot Slot, bool bGamepad)
{
	if (!InputAction) { return EKeys::Invalid; }

	UEnhancedPlayerMappableKeyProfile* Profile = GetActiveKeyProfile(OwningPlayer);
	if (!Profile) { return EKeys::Invalid; }

	const FName MappingName = FindMappingNameForAction(Profile, InputAction);
	const FKeyMappingRow* Row = MappingName.IsNone() ? nullptr : Profile->FindKeyMappingRow(MappingName);
	if (!Row) { return EKeys::Invalid; }

	for (const FPlayerKeyMapping& Mapping : Row->Mappings)
	{
		if (Mapping.GetSlot() == Slot && Mapping.GetCurrentKey().IsGamepadKey() == bGamepad)
		{
			return Mapping.GetCurrentKey();
		}
	}

	return EKeys::Invalid;
}

bool UMCore_InputDisplayLibrary::RemapActionKeyForSlot(APlayerController* OwningPlayer,
	UInputAction* InputAction, FKey NewKey, EPlayerMappableKeySlot Slot, FText& OutError)
{
	OutError = FText::GetEmpty();

	if (!InputAction)
	{
		OutError = FText::FromString(TEXT("Input Action is null"));
		return false;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem(OwningPlayer);
	if (!Subsystem)
	{
		OutError = FText::FromString(TEXT("Enhanced Input subsystem not available"));
		return false;
	}

	UEnhancedInputUserSettings* UserSettings = Subsystem->GetUserSettings();
	if (!UserSettings)
	{
		OutError = FText::FromString(TEXT("User settings not initialized"));
		return false;
	}

	UEnhancedPlayerMappableKeyProfile* KeyProfile = UserSettings->GetActiveKeyProfile();
	if (!KeyProfile)
	{
		OutError = FText::FromString(TEXT("Action not remappable or mapping invalid"));
		return false;
	}
	FName MappingName = FindMappingNameForAction(KeyProfile, InputAction);
	if (MappingName.IsNone())
	{
		OutError = FText::FromString(TEXT("Action not remappable or mapping invalid"));
		return false;
	}

	FMapPlayerKeyArgs KeyArgs;
	KeyArgs.MappingName = MappingName;
	KeyArgs.NewKey = NewKey;
	KeyArgs.Slot = Slot;
	KeyArgs.bCreateMatchingSlotIfNeeded = true;

	FGameplayTagContainer FailureReason;
	UserSettings->MapPlayerKey(KeyArgs, FailureReason);

	if (FailureReason.IsEmpty())
	{
		UserSettings->SaveSettings();
		UE_LOG(LogModulusUI, Log,
			TEXT("EnhancedInputDisplay::RemapActionKeyForSlot: remapped action %s (slot %d) to key %s"),
			*InputAction->GetName(), static_cast<int32>(Slot), *NewKey.ToString());
		return true;
	}

	OutError = FText::FromString(FailureReason.ToString());
	UE_LOG(LogModulusUI, Warning,
		TEXT("EnhancedInputDisplay::RemapActionKeyForSlot: failed to remap action %s (slot %d) to key %s: %s"),
		*InputAction->GetName(), static_cast<int32>(Slot), *NewKey.ToString(),
		*FailureReason.ToString());
	return false;
}

bool UMCore_InputDisplayLibrary::ResetActionToDefaultForSlot(APlayerController* OwningPlayer,
	UInputAction* InputAction, EPlayerMappableKeySlot Slot, bool bGamepad, FText& OutError)
{
	OutError = FText::GetEmpty();

	if (!InputAction)
	{
		OutError = FText::FromString(TEXT("Input Action is null"));
		return false;
	}

	UEnhancedPlayerMappableKeyProfile* Profile = GetActiveKeyProfile(OwningPlayer);
	if (!Profile)
	{
		OutError = FText::FromString(TEXT("No active key profile"));
		return false;
	}

	const FName MappingName = FindMappingNameForAction(Profile, InputAction);
	const FKeyMappingRow* Row = MappingName.IsNone() ? nullptr : Profile->FindKeyMappingRow(MappingName);
	if (!Row)
	{
		OutError = FText::FromString(TEXT("Action not found in key profile"));
		return false;
	}

	/* Find the mapping matching the requested slot and device type */
	for (const FPlayerKeyMapping& Mapping : Row->Mappings)
	{
		if (Mapping.GetSlot() == Slot && Mapping.GetDefaultKey().IsGamepadKey() == bGamepad)
		{
			return RemapActionKeyForSlot(OwningPlayer, InputAction,
				Mapping.GetDefaultKey(), Slot, OutError);
		}
	}

	OutError = FText::FromString(TEXT("No matching mapping found for slot and device type"));
	return false;
}

// ============================================================================
// CONTEXT-SCOPED QUERIES
// ============================================================================

TArray<FPlayerKeyMapping> UMCore_InputDisplayLibrary::GetRemappableActionsForContext(
	APlayerController* OwningPlayer, const UInputMappingContext* MappingContext)
{
	TArray<FPlayerKeyMapping> Result;

	if (!MappingContext)
	{
		UE_LOG(LogModulusUI, Warning,
			TEXT("InputDisplayLibrary::GetRemappableActionsForContext: MappingContext is null"));
		return Result;
	}

	UEnhancedPlayerMappableKeyProfile* Profile = GetActiveKeyProfile(OwningPlayer);
	if (!Profile) { return Result; }

	/* Build a set of InputActions that belong to this IMC */
	TSet<TObjectPtr<const UInputAction>> ContextActions;
	for (const FEnhancedActionKeyMapping& ActionMapping : MappingContext->GetMappings())
	{
		if (ActionMapping.Action)
		{
			ContextActions.Add(ActionMapping.Action);
		}
	}

	if (ContextActions.IsEmpty()) { return Result; }

	/* Iterate all profile rows; include any whose associated action is in this IMC */
	const TMap<FName, FKeyMappingRow>& AllRows = Profile->GetPlayerMappingRows();

	for (const auto& Pair : AllRows)
	{
		for (const FPlayerKeyMapping& Mapping : Pair.Value.Mappings)
		{
			const UInputAction* Action = Mapping.GetAssociatedInputAction();
			if (!Action) { continue; }
			if (!ContextActions.Contains(Action)) { continue; }

			Result.Add(Mapping);
		}
	}

	return Result;
}

// ============================================================================
// BULK RESET OPERATIONS
// ============================================================================

void UMCore_InputDisplayLibrary::ResetAllBindingsToDefault(APlayerController* OwningPlayer)
{
	UEnhancedPlayerMappableKeyProfile* Profile = GetActiveKeyProfile(OwningPlayer);
	if (!Profile)
	{
		UE_LOG(LogModulusUI, Warning,
			TEXT("EnhancedInputDisplay::ResetAllBindingsToDefault: no active key profile"));
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem(OwningPlayer);
	if (!Subsystem) { return; }

	UEnhancedInputUserSettings* UserSettings = Subsystem->GetUserSettings();
	if (!UserSettings) { return; }

	const TMap<FName, FKeyMappingRow>& AllRows = Profile->GetPlayerMappingRows();
	int32 ResetCount{0};

	for (const TPair<FName, FKeyMappingRow>& Pair : AllRows)
	{
		for (const FPlayerKeyMapping& Mapping : Pair.Value.Mappings)
		{
			FMapPlayerKeyArgs KeyArgs;
			KeyArgs.MappingName = Pair.Key;
			KeyArgs.Slot = Mapping.GetSlot();
			KeyArgs.NewKey = Mapping.GetDefaultKey();

			FGameplayTagContainer FailureReason;
			UserSettings->MapPlayerKey(KeyArgs, FailureReason);

			if (FailureReason.IsEmpty()) { ++ResetCount; }
		}
	}

	UserSettings->SaveSettings();

	UE_LOG(LogModulusUI, Log,
		TEXT("EnhancedInputDisplay::ResetAllBindingsToDefault: reset %d bindings to defaults"),
		ResetCount);
}

void UMCore_InputDisplayLibrary::ResetBindingsForContext(APlayerController* OwningPlayer,
	const UInputMappingContext* MappingContext)
{
	if (!MappingContext)
	{
		UE_LOG(LogModulusUI, Warning,
			TEXT("EnhancedInputDisplay::ResetBindingsForContext: MappingContext is null"));
		return;
	}

	UEnhancedPlayerMappableKeyProfile* Profile = GetActiveKeyProfile(OwningPlayer);
	if (!Profile) { return; }

	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem(OwningPlayer);
	if (!Subsystem) { return; }

	UEnhancedInputUserSettings* UserSettings = Subsystem->GetUserSettings();
	if (!UserSettings) { return; }

	const TMap<FName, FKeyMappingRow>& AllRows = Profile->GetPlayerMappingRows();

	/* Collect remappable action names from this IMC */
	TSet<FName> ContextActionNames;
	for (const FEnhancedActionKeyMapping& ActionMapping : MappingContext->GetMappings())
	{
		const UInputAction* Action = ActionMapping.Action;
		if (!Action) { continue; }
		
		/* Only include actions that exist in the key profile */
		FName MappingName = FindMappingNameForAction(Profile, Action);
		if (MappingName.IsNone()) { continue; }

		ContextActionNames.Add(MappingName);
	}

	int32 ResetCount{0};

	for (const FName& ActionName : ContextActionNames)
	{
		const FKeyMappingRow* Row = AllRows.Find(ActionName);
		if (!Row) { continue; }

		for (const FPlayerKeyMapping& Mapping : Row->Mappings)
		{
			FMapPlayerKeyArgs KeyArgs;
			KeyArgs.MappingName = ActionName;
			KeyArgs.Slot = Mapping.GetSlot();
			KeyArgs.NewKey = Mapping.GetDefaultKey();

			FGameplayTagContainer FailureReason;
			UserSettings->MapPlayerKey(KeyArgs, FailureReason);

			if (FailureReason.IsEmpty()) { ++ResetCount; }
		}
	}

	UserSettings->SaveSettings();

	UE_LOG(LogModulusUI, Log,
		TEXT("EnhancedInputDisplay::ResetBindingsForContext: reset %d bindings for context '%s'"),
		ResetCount, *MappingContext->GetName());
}
