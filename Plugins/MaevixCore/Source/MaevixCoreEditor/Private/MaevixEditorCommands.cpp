// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "MaevixEditorCommands.h"

#define LOCTEXT_NAMESPACE "FMaevixEditorCommands"

void FMaevixEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenMaevixHub, "Maevix Hub", "Open Maevix Game Framework Hub", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE