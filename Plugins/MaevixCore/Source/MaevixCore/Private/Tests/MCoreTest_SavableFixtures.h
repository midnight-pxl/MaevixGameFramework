// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreData/Interfaces/MCore_Savable.h"
#include "MCoreTest_SavableFixtures.generated.h"

// Dev-only reflected fixture for the persistence (IMCore_Savable) harness. Left UNGUARDED for the same reason
// as the service-registry fixtures: UHT reflects UCLASS regardless of WITH_AUTOMATION_TESTS, so a guard would
// desync reflection from compilation. The test bodies that use it are guarded by WITH_DEV_AUTOMATION_TESTS.

// Reflected provider implementing IMCore_Savable; proves the reflected SaveGame round trip plus the hooks.
UCLASS()
class UMCoreTest_SavableProvider : public UObject, public IMCore_Savable
{
	GENERATED_BODY()

public:
	UMCoreTest_SavableProvider();

	// SaveGame: the only field flagged for serialization, so the only one an ArIsSaveGame round trip restores.
	UPROPERTY(SaveGame)
	int32 SavedValue = 0;

	// Live/derived value the hooks bridge to and from SavedValue. Reflected but NOT SaveGame.
	UPROPERTY()
	int32 RuntimeValue = 0;

	// Proves the ArIsSaveGame filter excludes non-SaveGame fields: it must survive the round trip untouched.
	UPROPERTY()
	int32 TransientNoise = 0;

	// Hook-fired flags, asserted by the tests.
	bool bPreSaveFired = false;
	bool bPostLoadFired = false;

	// Context observation. Defaults are the OPPOSITE of what each test expects, so a hook that ignores
	// Context fails the assert instead of passing by default.
	EMCore_SaveLoadPhase PreSavePhaseSeen  = EMCore_SaveLoadPhase::Loading;
	EMCore_SaveLoadPhase PostLoadPhaseSeen = EMCore_SaveLoadPhase::Saving;
	FString PreSaveSlotSeen;
	FString PostLoadSlotSeen;

	// Stable identity stamped at construction; returned by GetSaveGuid_Implementation.
	FGuid SaveGuid;

	virtual FGuid GetSaveGuid_Implementation() const override;
	virtual void OnPreSave_Implementation(const FMCore_SaveLoadContext& Context) override;
	virtual void OnPostLoad_Implementation(const FMCore_SaveLoadContext& Context) override;
};
