// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

/**
 * MaevixCoreEditor.h
 *
 * Editor module for MaevixCore. Registers the Maevix Hub tab,
 * toolbar button, and editor-only utilities.
 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class SDockTab;
class FSpawnTabArgs;

class FMaevixCoreEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static void OpenMaevixHub();

private:
    TSharedRef<SDockTab> SpawnMaevixHubTab(const FSpawnTabArgs& SpawnTabArgs);
    void RegisterToolbarButton();
    void UnregisterToolbarButton();
};
