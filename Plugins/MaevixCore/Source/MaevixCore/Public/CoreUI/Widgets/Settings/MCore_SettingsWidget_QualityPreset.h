// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreUI/Widgets/Settings/MCore_SettingsWidget_Switcher.h"
#include "MCore_SettingsWidget_QualityPreset.generated.h"

/**
 * Switcher subclass for the QualityPreset (OverallScalabilityLevel) setting.
 *
 * Display index diverges from the setting's saved int value when the user has
 * tweaked an individual scalability; the save's LastSelectedQualityPreset
 * intent (-1 = Custom, 0..3 = preset) drives the displayed dropdown index
 * (4 = Custom, 0..3 = preset row).
 */
UCLASS(Blueprintable, ClassGroup = "MaevixUI", meta = (DisableNativeTick))
class MAEVIXCORE_API UMCore_SettingsWidget_QualityPreset : public UMCore_SettingsWidget_Switcher
{
	GENERATED_BODY()

protected:
	virtual int32 ResolveDisplayedIndex_Implementation() override;
};
