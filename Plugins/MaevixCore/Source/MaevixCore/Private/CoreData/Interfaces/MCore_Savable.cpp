// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/Interfaces/MCore_Savable.h"

FGuid IMCore_Savable::GetSaveGuid_Implementation() const
{
	// No persistent identity by default; a provider with stable identity overrides this.
	return FGuid();
}

void IMCore_Savable::OnPreSave_Implementation(const FMCore_SaveLoadContext& Context)
{
	// Default no-op; providers flush transient state into their SaveGame fields here.
}

void IMCore_Savable::OnPostLoad_Implementation(const FMCore_SaveLoadContext& Context)
{
	// Default no-op; providers rebuild derived state from restored fields here.
}
