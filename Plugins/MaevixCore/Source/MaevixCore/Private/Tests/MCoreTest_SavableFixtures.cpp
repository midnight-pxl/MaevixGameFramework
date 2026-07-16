// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "Tests/MCoreTest_SavableFixtures.h"

UMCoreTest_SavableProvider::UMCoreTest_SavableProvider()
{
	// Stable cross-session identity for this provider instance.
	SaveGuid = FGuid::NewGuid();
}

FGuid UMCoreTest_SavableProvider::GetSaveGuid_Implementation() const
{
	return SaveGuid;
}

void UMCoreTest_SavableProvider::OnPreSave_Implementation(const FMCore_SaveLoadContext& Context)
{
	// Flush the live value into the serialized field before the orchestrator writes it.
	SavedValue = RuntimeValue;
	bPreSaveFired = true;

	// Record the context so the test can prove it crossed the Execute_ thunk intact.
	PreSavePhaseSeen = Context.Phase;
	PreSaveSlotSeen = Context.SlotName;
}

void UMCoreTest_SavableProvider::OnPostLoad_Implementation(const FMCore_SaveLoadContext& Context)
{
	// Rehydrate the live value from the restored serialized field.
	RuntimeValue = SavedValue;
	bPostLoadFired = true;

	// Record the context so the test can prove it crossed the Execute_ thunk intact.
	PostLoadPhaseSeen = Context.Phase;
	PostLoadSlotSeen = Context.SlotName;
}
