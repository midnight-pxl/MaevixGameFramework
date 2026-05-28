// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

/**
 * MaevixEditorCommands.h
 *
 * UI command definitions for the Maevix editor integration.
 */

#pragma once

#include "CoreMinimal.h"
#include "EditorStyleSet.h"
#include "Framework/Commands/Commands.h"

class FMaevixEditorCommands : public TCommands<FMaevixEditorCommands>
{
public:
    FMaevixEditorCommands() : TCommands<FMaevixEditorCommands>(
            TEXT("MaevixEditor"),
            NSLOCTEXT("Contexts", "MaevixEditor", "Maevix Editor"),
            NAME_None,
            FAppStyle::GetAppStyleSetName())
    {
    }

    virtual void RegisterCommands() override;

    TSharedPtr<FUICommandInfo> OpenMaevixHub;
};