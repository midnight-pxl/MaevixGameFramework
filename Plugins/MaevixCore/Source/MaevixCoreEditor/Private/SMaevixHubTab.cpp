// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "SMaevixHubTab.h"
#include "MaevixEditorStyle.h"
#include "CoreEditorLogging/LogMaevixEditor.h"
#include "CoreData/DevSettings/MCore_CoreSettings.h"
#include "CoreData/Assets/UI/Themes/MCore_PDA_UITheme_Base.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Interfaces/IPluginManager.h"
#include "ISettingsModule.h"
#include "HAL/PlatformProcess.h"
#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"

const FName SMaevixHubTab::TabId = FName(TEXT("MaevixHubTab"));

#define LOCTEXT_NAMESPACE "SMaevixHubTab"

namespace MaevixHubLayout
{
	const FMargin ContentPadding(16.0f);
	const FMargin SectionPadding(0.0f, 8.0f);
	const FMargin ButtonPadding(0.0f, 0.0f, 8.0f, 0.0f);
	const FMargin ItemPadding(8.0f, 4.0f);
	const float SectionSpacing = 24.0f;
	const float HeaderHeight = 60.0f;
}

void SMaevixHubTab::Construct(const FArguments& InArgs)
{
	RefreshInstalledPlugins();

	const ISlateStyle& Style = FMaevixEditorStyle::Get();

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(Style.GetBrush("MaevixEditor.Background"))
		.Padding(0.0f)
		[
			SNew(SVerticalBox)

			// Header
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(Style.GetBrush("MaevixEditor.HeaderBox"))
				.Padding(MaevixHubLayout::ContentPadding)
				[
					SNew(SHorizontalBox)

					// Icon placeholder
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, 12.0f, 0.0f)
					[
						SNew(SBox)
						.WidthOverride(32.0f)
						.HeightOverride(32.0f)
						[
							SNew(SImage)
							.Image(Style.GetBrush("MaevixEditor.MaevixIcon.Large"))
						]
					]

					// Title and subtitle
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("HubTitle", "Maevix Game Framework"))
							.TextStyle(&Style, "MaevixEditor.Text.Header")
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("HubSubtitle", "Central hub for framework tools and documentation"))
							.TextStyle(&Style, "MaevixEditor.Text.Muted")
						]
					]
				]
			]

			// Main Content
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)

				+ SScrollBox::Slot()
				.Padding(MaevixHubLayout::ContentPadding)
				[
					SNew(SVerticalBox)

					// ============================================================================
					// ECOSYSTEM
					// ============================================================================
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(MaevixHubLayout::SectionPadding)
					[
						SNew(SBorder)
						.BorderImage(Style.GetBrush("MaevixEditor.RoundedBox"))
						.Padding(MaevixHubLayout::ContentPadding)
						[
							SNew(SVerticalBox)

							// Section Header
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 0.0f, 0.0f, 12.0f)
							[
								SNew(SHorizontalBox)

								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(0.0f, 0.0f, 8.0f, 0.0f)
								[
									SNew(SBox)
									.WidthOverride(16.0f)
									.HeightOverride(16.0f)
									[
										SNew(SImage)
										.Image(Style.GetBrush(FMaevixEditorStyle::EcosystemIconName))
									]
								]

								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("EcosystemHeader", "Installed Plugins"))
									.TextStyle(&Style, "MaevixEditor.Text.SectionHeader")
								]

								+ SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SButton)
									.ButtonStyle(&Style, "MaevixEditor.Button")
									.OnClicked_Lambda([this]()
									{
										RefreshInstalledPlugins();
										UpdatePluginListUI();
										return FReply::Handled();
									})
									[
										SNew(STextBlock)
										.Text(LOCTEXT("RefreshBtn", "Refresh"))
									]
								]
							]

							// Plugin List
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SAssignNew(PluginListBox, SVerticalBox)
							]
						]
					]

					// ============================================================================
					// SETTINGS
					// ============================================================================
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(MaevixHubLayout::SectionPadding)
					[
						SNew(SBorder)
						.BorderImage(Style.GetBrush("MaevixEditor.RoundedBox"))
						.Padding(MaevixHubLayout::ContentPadding)
						[
							SNew(SVerticalBox)

							// Section Header
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 0.0f, 0.0f, 12.0f)
							[
								SNew(SHorizontalBox)

								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(0.0f, 0.0f, 8.0f, 0.0f)
								[
									SNew(SBox)
									.WidthOverride(16.0f)
									.HeightOverride(16.0f)
									[
										SNew(SImage)
										.Image(Style.GetBrush(FMaevixEditorStyle::SettingsIconName))
									]
								]

								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("SettingsHeader", "Debug Settings"))
									.TextStyle(&Style, "MaevixEditor.Text.SectionHeader")
								]

								+ SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SButton)
									.ButtonStyle(&Style, "MaevixEditor.Button")
									.OnClicked(this, &SMaevixHubTab::OnOpenProjectSettingsClicked)
									[
										SNew(STextBlock)
										.Text(LOCTEXT("OpenSettingsBtn", "Open Project Settings"))
									]
								]
							]

							// Event Logging Toggle
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 4.0f)
							[
								SNew(SHorizontalBox)

								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(0.0f, 0.0f, 8.0f, 0.0f)
								[
									SAssignNew(EventLoggingCheckbox, SCheckBox)
									.IsChecked(this, &SMaevixHubTab::GetEventLoggingCheckState)
									.OnCheckStateChanged(this, &SMaevixHubTab::OnEventLoggingCheckStateChanged)
								]

								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.VAlign(VAlign_Center)
								[
									SNew(SVerticalBox)

									+ SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(STextBlock)
										.Text(LOCTEXT("EventLoggingLabel", "Event System Logging"))
										.TextStyle(&Style, "MaevixEditor.Text.Normal")
									]

									+ SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(STextBlock)
										.Text(LOCTEXT("EventLoggingDesc", "Log all event broadcasts and subscriptions to Output Log"))
										.TextStyle(&Style, "MaevixEditor.Text.Muted")
									]
								]
							]

							// UI Debug Overlay Toggle
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 4.0f)
							[
								SNew(SHorizontalBox)

								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(0.0f, 0.0f, 8.0f, 0.0f)
								[
									SAssignNew(UIDebugOverlayCheckbox, SCheckBox)
									.IsChecked(this, &SMaevixHubTab::GetUIDebugOverlayCheckState)
									.OnCheckStateChanged(this, &SMaevixHubTab::OnUIDebugOverlayCheckStateChanged)
								]

								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.VAlign(VAlign_Center)
								[
									SNew(SVerticalBox)

									+ SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(STextBlock)
										.Text(LOCTEXT("UIDebugOverlayLabel", "UI Layer Debug Overlay"))
										.TextStyle(&Style, "MaevixEditor.Text.Normal")
									]

									+ SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(STextBlock)
										.Text(LOCTEXT("UIDebugOverlayDesc", "Show debug overlay for CommonUI layer stack"))
										.TextStyle(&Style, "MaevixEditor.Text.Muted")
									]
								]
							]
						]
					]

					// ============================================================================
					// UI THEME
					// ============================================================================
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(MaevixHubLayout::SectionPadding)
					[
						SNew(SBorder)
						.BorderImage(Style.GetBrush("MaevixEditor.RoundedBox"))
						.Padding(MaevixHubLayout::ContentPadding)
						[
							SNew(SVerticalBox)

							// Section Header
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 0.0f, 0.0f, 12.0f)
							[
								SNew(SHorizontalBox)

								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(0.0f, 0.0f, 8.0f, 0.0f)
								[
									SNew(SBox)
									.WidthOverride(16.0f)
									.HeightOverride(16.0f)
									[
										SNew(SImage)
										.Image(Style.GetBrush(FMaevixEditorStyle::SettingsIconName))
									]
								]

								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("UIThemeHeader", "UI Theme"))
									.TextStyle(&Style, "MaevixEditor.Text.SectionHeader")
								]
							]

							// Active Theme Row
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 4.0f)
							[
								SNew(SHorizontalBox)

								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.VAlign(VAlign_Center)
								[
									SNew(SVerticalBox)

									+ SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(STextBlock)
										.Text(LOCTEXT("ActiveThemeLabel", "Active Theme"))
										.TextStyle(&Style, "MaevixEditor.Text.Normal")
									]

									+ SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(STextBlock)
										.Text(this, &SMaevixHubTab::GetActiveThemeName)
										.TextStyle(&Style, "MaevixEditor.Text.Muted")
									]
								]

								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(SButton)
									.ButtonStyle(&Style, "MaevixEditor.Button")
									.OnClicked(this, &SMaevixHubTab::OnEditThemeAssetClicked)
									[
										SNew(STextBlock)
										.Text(LOCTEXT("EditThemeBtn", "Edit Theme Asset"))
									]
								]
							]
						]
					]

					// ============================================================================
					// DOCUMENTATION
					// ============================================================================
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(MaevixHubLayout::SectionPadding)
					[
						SNew(SBorder)
						.BorderImage(Style.GetBrush("MaevixEditor.RoundedBox"))
						.Padding(MaevixHubLayout::ContentPadding)
						[
							SNew(SVerticalBox)

							// Section Header
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 0.0f, 0.0f, 12.0f)
							[
								SNew(SHorizontalBox)

								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(0.0f, 0.0f, 8.0f, 0.0f)
								[
									SNew(SBox)
									.WidthOverride(16.0f)
									.HeightOverride(16.0f)
									[
										SNew(SImage)
										.Image(Style.GetBrush(FMaevixEditorStyle::DocsIconName))
									]
								]

								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("DocsHeader", "Documentation"))
									.TextStyle(&Style, "MaevixEditor.Text.SectionHeader")
								]
							]

							// Doc Buttons
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(SHorizontalBox)

								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(MaevixHubLayout::ButtonPadding)
								[
									SNew(SButton)
									.ButtonStyle(&Style, "MaevixEditor.Button")
									.OnClicked(this, &SMaevixHubTab::OnQuickStartClicked)
									[
										SNew(STextBlock)
										.Text(LOCTEXT("QuickStartBtn", "Quick Start Guide"))
									]
								]

								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(0.0f)
								[
									SNew(SButton)
									.ButtonStyle(&Style, "MaevixEditor.Button")
									.OnClicked(this, &SMaevixHubTab::OnAPIReferenceClicked)
									[
										SNew(STextBlock)
										.Text(LOCTEXT("APIRefBtn", "API Reference"))
									]
								]
							]
						]
					]

					// Bottom spacer
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SSpacer)
						.Size(FVector2D(1.0f, 16.0f))
					]
				]
			]

			// Footer
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(Style.GetBrush("MaevixEditor.Separator"))
				.Padding(FMargin(MaevixHubLayout::ContentPadding.Left, 8.0f, MaevixHubLayout::ContentPadding.Right, 8.0f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("FooterText", "Maevix Game Framework - Midnight Pixel Studio"))
					.TextStyle(&Style, "MaevixEditor.Text.Muted")
				]
			]
		]
	];

	// Initialize plugin list UI
	UpdatePluginListUI();

	UE_LOG(LogMaevixEditor, Log, TEXT("MaevixHubTab::Construct -- hub opened"));
}

void SMaevixHubTab::UpdatePluginListUI()
{
	if (!PluginListBox.IsValid())
	{
		return;
	}

	const ISlateStyle& Style = FMaevixEditorStyle::Get();
	PluginListBox->ClearChildren();

	if (InstalledPluginNames.Num() == 0)
	{
		PluginListBox->AddSlot()
		.AutoHeight()
		.Padding(MaevixHubLayout::ItemPadding)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NoPlugins", "No Maevix plugins found"))
			.TextStyle(&Style, "MaevixEditor.Text.Muted")
		];
		return;
	}

	for (int32 i = 0; i < InstalledPluginNames.Num(); ++i)
	{
		const bool bEnabled = InstalledPluginEnabled[i];

		PluginListBox->AddSlot()
		.AutoHeight()
		.Padding(MaevixHubLayout::ItemPadding)
		[
			SNew(SBorder)
			.BorderImage(Style.GetBrush("MaevixEditor.AccentBox"))
			.Padding(FMargin(12.0f, 8.0f))
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(InstalledPluginNames[i]))
					.TextStyle(&Style, "MaevixEditor.Text.Normal")
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(bEnabled ? LOCTEXT("Enabled", "Enabled") : LOCTEXT("Disabled", "Disabled"))
					.ColorAndOpacity(bEnabled ? FMaevixEditorStyle::GetSuccessColor() : FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
				]
			]
		];
	}
}

FReply SMaevixHubTab::OnOpenProjectSettingsClicked()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->ShowViewer("Project", "Game", "Maevix Core");
	}
	return FReply::Handled();
}

void SMaevixHubTab::OnEventLoggingCheckStateChanged(ECheckBoxState NewState)
{
#if WITH_EDITORONLY_DATA
	UMCore_CoreSettings* Settings = GetMutableDefault<UMCore_CoreSettings>();
	if (Settings)
	{
		Settings->bEnableEventSystemLogging = (NewState == ECheckBoxState::Checked);
		Settings->TryUpdateDefaultConfigFile();
		UE_LOG(LogMaevixEditor, Log, TEXT("MaevixHubTab::OnEventLoggingCheckStateChanged -- event system logging %s"),
			Settings->bEnableEventSystemLogging ? TEXT("enabled") : TEXT("disabled"));
	}
#endif
}

void SMaevixHubTab::OnUIDebugOverlayCheckStateChanged(ECheckBoxState NewState)
{
#if WITH_EDITORONLY_DATA
	UMCore_CoreSettings* Settings = GetMutableDefault<UMCore_CoreSettings>();
	if (Settings)
	{
		Settings->bShowUILayerDebugOverlay = (NewState == ECheckBoxState::Checked);
		Settings->TryUpdateDefaultConfigFile();
		UE_LOG(LogMaevixEditor, Log, TEXT("MaevixHubTab::OnUIDebugOverlayCheckStateChanged -- UI layer debug overlay %s"),
			Settings->bShowUILayerDebugOverlay ? TEXT("enabled") : TEXT("disabled"));
	}
#endif
}

ECheckBoxState SMaevixHubTab::GetEventLoggingCheckState() const
{
#if WITH_EDITORONLY_DATA
	const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get();
	if (Settings)
	{
		return Settings->bEnableEventSystemLogging ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
#endif
	return ECheckBoxState::Unchecked;
}

ECheckBoxState SMaevixHubTab::GetUIDebugOverlayCheckState() const
{
#if WITH_EDITORONLY_DATA
	const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get();
	if (Settings)
	{
		return Settings->bShowUILayerDebugOverlay ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
#endif
	return ECheckBoxState::Unchecked;
}

FText SMaevixHubTab::GetActiveThemeName() const
{
	const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get();
	if (!Settings)
	{
		return LOCTEXT("NoSettingsError", "Settings unavailable");
	}

	if (!Settings->IsValidThemeIndex(Settings->DefaultThemeIndex))
	{
		return LOCTEXT("NoThemeConfigured", "No theme configured");
	}

	const FMCore_ThemeEntry& ThemeEntry = Settings->AvailableThemes[Settings->DefaultThemeIndex];
	if (ThemeEntry.DisplayName.IsEmpty())
	{
		// Fall back to asset name if no display name set
		if (!ThemeEntry.ThemeAsset.IsNull())
		{
			return FText::FromString(ThemeEntry.ThemeAsset.GetAssetName());
		}
		return LOCTEXT("UnnamedTheme", "Unnamed Theme");
	}

	return ThemeEntry.DisplayName;
}

FReply SMaevixHubTab::OnEditThemeAssetClicked()
{
	const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get();
	if (!Settings || !Settings->IsValidThemeIndex(Settings->DefaultThemeIndex))
	{
		UE_LOG(LogMaevixEditor, Warning, TEXT("MaevixHubTab::OnEditThemeAssetClicked -- no active theme configured, configure a theme in Project Settings > Game > Maevix Core"));
		return FReply::Handled();
	}

	const FMCore_ThemeEntry& ThemeEntry = Settings->AvailableThemes[Settings->DefaultThemeIndex];
	if (ThemeEntry.ThemeAsset.IsNull())
	{
		UE_LOG(LogMaevixEditor, Warning, TEXT("MaevixHubTab::OnEditThemeAssetClicked -- active theme entry has no asset assigned"));
		return FReply::Handled();
	}

	// Load the asset and open it in the editor
	if (UMCore_PDA_UITheme_Base* ThemeAsset = ThemeEntry.ThemeAsset.LoadSynchronous())
	{
		if (GEditor)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(ThemeAsset);
			UE_LOG(LogMaevixEditor, Log, TEXT("MaevixHubTab::OnEditThemeAssetClicked -- opened theme asset: %s"), *ThemeAsset->GetName());
		}
	}

	return FReply::Handled();
}

void SMaevixHubTab::RefreshInstalledPlugins()
{
	InstalledPluginNames.Empty();
	InstalledPluginEnabled.Empty();

	TArray<TSharedRef<IPlugin>> AllPlugins = IPluginManager::Get().GetDiscoveredPlugins();
	for (const TSharedRef<IPlugin>& Plugin : AllPlugins)
	{
		if (Plugin->GetName().Contains(TEXT("Maevix")))
		{
			InstalledPluginNames.Add(Plugin->GetName());
			InstalledPluginEnabled.Add(Plugin->IsEnabled());
		}
	}

	UE_LOG(LogMaevixEditor, Log, TEXT("MaevixHubTab::RefreshInstalledPlugins -- found %d Maevix plugin(s)"), InstalledPluginNames.Num());
}

FReply SMaevixHubTab::OnQuickStartClicked()
{
	// TODO: Add actual documentation URL when available
	UE_LOG(LogMaevixEditor, Log, TEXT("MaevixHubTab::OnQuickStartClicked -- Quick Start Guide requested"));
	return FReply::Handled();
}

FReply SMaevixHubTab::OnAPIReferenceClicked()
{
	// TODO: Add actual documentation URL when available
	UE_LOG(LogMaevixEditor, Log, TEXT("MaevixHubTab::OnAPIReferenceClicked -- API Reference requested"));
	return FReply::Handled();
}

void SMaevixHubTab::OpenURL(const FString& URL)
{
	if (!URL.IsEmpty())
	{
		FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
	}
}

#undef LOCTEXT_NAMESPACE
