// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "Settings/MInt_DeveloperSettings.h"

const UMInt_DeveloperSettings* UMInt_DeveloperSettings::Get()
{
	return GetDefault<UMInt_DeveloperSettings>();
}

#if WITH_EDITOR
FText UMInt_DeveloperSettings::GetSectionText() const
{
	return NSLOCTEXT("MInt_DeveloperSettings", "SectionText", "Maevix Interact");
}

FText UMInt_DeveloperSettings::GetSectionDescription() const
{
	return NSLOCTEXT("MInt_DeveloperSettings", "SectionDesc",
		"Configure interaction detection channels for the Maevix Interact plugin.");
}
#endif
