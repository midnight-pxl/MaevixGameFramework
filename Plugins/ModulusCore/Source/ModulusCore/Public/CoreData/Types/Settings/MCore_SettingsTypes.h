// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

/**
 * MCore_SettingsTypes.h
 *
 * Enums and structs supporting the DataAsset-driven settings system.
 * Defines setting widget types and batch change payloads.
 */

#pragma once

#include "CoreMinimal.h"
#include "MCore_SettingsTypes.generated.h"

class UMCore_DA_SettingDefinition;

/* Setting widget types for game settings menus */
UENUM(BlueprintType)
enum class EMCore_SettingType : uint8
{
    Toggle       UMETA(DisplayName = "Toggle (Boolean)"),
    Slider       UMETA(DisplayName = "Slider (Float)"),
    Dropdown     UMETA(DisplayName = "Dropdown (Selection)")
};

/* Float setting + value pair for batch setter operations */
USTRUCT(BlueprintType)
struct MODULUSCORE_API FMCore_FloatSettingChange
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings",
        meta = (DisplayName = "Setting"))
    TObjectPtr<UMCore_DA_SettingDefinition> Setting = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings",
        meta = (DisplayName = "Value"))
    float Value = 0.f;
};

/* Int setting + value pair for batch setter operations */
USTRUCT(BlueprintType)
struct MODULUSCORE_API FMCore_IntSettingChange
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings",
        meta = (DisplayName = "Setting"))
    TObjectPtr<UMCore_DA_SettingDefinition> Setting = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings",
        meta = (DisplayName = "Value"))
    int32 Value = 0;
};

/* Bool setting + value pair for batch setter operations */
USTRUCT(BlueprintType)
struct MODULUSCORE_API FMCore_BoolSettingChange
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings",
        meta = (DisplayName = "Setting"))
    TObjectPtr<UMCore_DA_SettingDefinition> Setting = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings",
        meta = (DisplayName = "Value"))
    bool Value = false;
};

/* Visual section grouping declared on a SettingsCollection. Settings whose
 * SectionID matches render under a shared header. */
USTRUCT(BlueprintType)
struct MODULUSCORE_API FMCore_SettingsSection
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Section")
    FName SectionID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Section")
    FText SectionDisplayName;
};
