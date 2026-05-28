// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "MaevixCoreEditor.h"
#include "SMaevixHubTab.h"
#include "MaevixEditorStyle.h"
#include "CoreEditorLogging/LogMaevixEditor.h"
#include "MaevixEditorCommands.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "FMaevixCoreEditorModule"

void FMaevixCoreEditorModule::StartupModule()
{
	FMaevixEditorStyle::Initialize();

	FMaevixEditorCommands::Register();

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		SMaevixHubTab::TabId,
		FOnSpawnTab::CreateRaw(this, &FMaevixCoreEditorModule::SpawnMaevixHubTab))
		.SetDisplayName(LOCTEXT("MaevixHubTabTitle", "Maevix Hub"))
		.SetTooltipText(LOCTEXT("MaevixHubTabTooltip", "Open Maevix Game Framework Hub"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
		.SetIcon(FSlateIcon(FMaevixEditorStyle::GetStyleSetName(), FMaevixEditorStyle::MaevixIconName));

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FMaevixCoreEditorModule::RegisterToolbarButton));

	UE_LOG(LogMaevixEditor, Log, TEXT("MaevixCoreEditorModule::StartupModule -- module started"));
}

void FMaevixCoreEditorModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SMaevixHubTab::TabId);
	UnregisterToolbarButton();

	FMaevixEditorStyle::Shutdown();

	UE_LOG(LogMaevixEditor, Log, TEXT("MaevixCoreEditorModule::ShutdownModule -- module shutdown"));
}

TSharedRef<SDockTab> FMaevixCoreEditorModule::SpawnMaevixHubTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SMaevixHubTab)
		];
}

void FMaevixCoreEditorModule::RegisterToolbarButton()
{
	UToolMenu* ToolBar = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	if (ToolBar)
	{
		FToolMenuSection& Section = ToolBar->FindOrAddSection("Maevix");

		Section.AddEntry(FToolMenuEntry::InitToolBarButton(
			"OpenMaevixHub",
			FUIAction(FExecuteAction::CreateStatic(&FMaevixCoreEditorModule::OpenMaevixHub)),
			LOCTEXT("MaevixHub", "Maevix"),
			LOCTEXT("MaevixHubTooltip", "Open Maevix Game Framework Hub"),
			FSlateIcon(FMaevixEditorStyle::GetStyleSetName(), FMaevixEditorStyle::MaevixIconName)
		));

		UE_LOG(LogMaevixEditor, Log, TEXT("MaevixCoreEditorModule::RegisterToolbarButton -- toolbar button registered"));
	}
	else
	{
		UE_LOG(LogMaevixEditor, Warning, TEXT("MaevixCoreEditorModule::RegisterToolbarButton -- failed to extend LevelEditor toolbar"));
	}
}

void FMaevixCoreEditorModule::UnregisterToolbarButton()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
}

void FMaevixCoreEditorModule::OpenMaevixHub()
{
	FGlobalTabmanager::Get()->TryInvokeTab(SMaevixHubTab::TabId);
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FMaevixCoreEditorModule, MaevixCoreEditor)