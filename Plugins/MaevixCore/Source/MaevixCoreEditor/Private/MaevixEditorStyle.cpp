// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "MaevixEditorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "Brushes/SlateImageBrush.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr<FSlateStyleSet> FMaevixEditorStyle::StyleInstance = nullptr;

const FName FMaevixEditorStyle::MaevixIconName = TEXT("MaevixEditor.MaevixIcon");
const FName FMaevixEditorStyle::SettingsIconName = TEXT("MaevixEditor.SettingsIcon");
const FName FMaevixEditorStyle::EcosystemIconName = TEXT("MaevixEditor.EcosystemIcon");
const FName FMaevixEditorStyle::DocsIconName = TEXT("MaevixEditor.DocsIcon");

namespace MaevixColors
{
	const FLinearColor Background = FLinearColor(0.110f, 0.110f, 0.12f, 1.0f);
	const FLinearColor SectionBackground = FLinearColor(0.145f, 0.145f, 0.15f, 1.0f);
	const FLinearColor HeaderBackground = FLinearColor(0.227f, 0.227f, 0.235f, 1.0f);

	const FLinearColor Accent = FLinearColor(1.0f, 0.420f, 0.1f, 1.0f);
	const FLinearColor Secondary = FLinearColor(0.0f, 0.75f, 1.0f, 1.0f);

	const FLinearColor Success = FLinearColor(0.2f, 0.9f, 0.4f, 1.0f);
	const FLinearColor Warning = FLinearColor(1.0f, 0.8f, 0.2f, 1.0f);
	const FLinearColor Error = FLinearColor(1.0f, 0.3f, 0.3f, 1.0f);

	const FLinearColor Disabled = FLinearColor(0.05f, 0.05f, 0.05f, 1.0f);

	const FLinearColor TextPrimary = FLinearColor(0.91f, 0.91f, 0.91f, 1.0f);
	const FLinearColor TextSecondary = FLinearColor(0.65f, 0.65f, 0.65f, 1.0f);
	const FLinearColor TextMuted = FLinearColor(0.43f, 0.43f, 0.43f, 1.0f);
}

void FMaevixEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FMaevixEditorStyle::Shutdown()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		StyleInstance.Reset();
	}
}

void FMaevixEditorStyle::ReloadTextures()
{
	if (StyleInstance.IsValid())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

FName FMaevixEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("MaevixEditorStyle"));
	return StyleSetName;
}

FLinearColor FMaevixEditorStyle::GetAccentColor() { return MaevixColors::Accent; }

FLinearColor FMaevixEditorStyle::GetSecondaryColor() { return MaevixColors::Secondary; }

FLinearColor FMaevixEditorStyle::GetBackgroundColor() { return MaevixColors::Background; }

FLinearColor FMaevixEditorStyle::GetHeaderBackgroundColor(){ return MaevixColors::HeaderBackground; }

FLinearColor FMaevixEditorStyle::GetSuccessColor() { return MaevixColors::Success; }

FLinearColor FMaevixEditorStyle::GetWarningColor() { return MaevixColors::Warning; }

FLinearColor FMaevixEditorStyle::GetErrorColor() {	return MaevixColors::Error; }

TSharedRef<FSlateStyleSet> FMaevixEditorStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("MaevixCore"))->GetBaseDir() / TEXT("Resources"));

	// Color brushes
	Style->Set("MaevixEditor.Background", new FSlateColorBrush(MaevixColors::Background));
	Style->Set("MaevixEditor.HeaderBackground", new FSlateColorBrush(MaevixColors::HeaderBackground));
	Style->Set("MaevixEditor.SectionBackground", new FSlateColorBrush(MaevixColors::SectionBackground));
	Style->Set("MaevixEditor.AccentBrush", new FSlateColorBrush(MaevixColors::Accent));
	Style->Set("MaevixEditor.SecondaryBrush", new FSlateColorBrush(MaevixColors::Secondary));

	// Rounded border brushes
	Style->Set("MaevixEditor.RoundedBox", new FSlateRoundedBoxBrush(
		MaevixColors::SectionBackground,
		4.0f
	));

	Style->Set("MaevixEditor.HeaderBox", new FSlateRoundedBoxBrush(
		MaevixColors::HeaderBackground,
		6.0f
	));

	Style->Set("MaevixEditor.AccentBox", new FSlateRoundedBoxBrush(
		FLinearColor(MaevixColors::Accent.R, MaevixColors::Accent.G, MaevixColors::Accent.B, 0.15f),
		4.0f
	));

	// Separator brush
	Style->Set("MaevixEditor.Separator", new FSlateColorBrush(FLinearColor(1.0f, 1.0f, 1.0f, 0.1f)));

	// Text styles
	const FTextBlockStyle NormalText = FTextBlockStyle()
		.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 10))
		.SetColorAndOpacity(FSlateColor(MaevixColors::TextPrimary));

	const FTextBlockStyle HeaderText = FTextBlockStyle()
		.SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 18))
		.SetColorAndOpacity(FSlateColor(MaevixColors::TextPrimary));

	const FTextBlockStyle SectionHeaderText = FTextBlockStyle()
		.SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 14))
		.SetColorAndOpacity(FSlateColor(MaevixColors::Secondary));

	const FTextBlockStyle SubHeaderText = FTextBlockStyle()
		.SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		.SetColorAndOpacity(FSlateColor(MaevixColors::TextPrimary));

	const FTextBlockStyle MutedText = FTextBlockStyle()
		.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 9))
		.SetColorAndOpacity(FSlateColor(MaevixColors::TextMuted));

	Style->Set("MaevixEditor.Text.Normal", NormalText);
	Style->Set("MaevixEditor.Text.Header", HeaderText);
	Style->Set("MaevixEditor.Text.SectionHeader", SectionHeaderText);
	Style->Set("MaevixEditor.Text.SubHeader", SubHeaderText);
	Style->Set("MaevixEditor.Text.Muted", MutedText);

	// Button style
	const FButtonStyle MaevixButton = FButtonStyle()
		.SetNormal(FSlateRoundedBoxBrush(MaevixColors::SectionBackground, 4.0f))
		.SetHovered(FSlateRoundedBoxBrush(MaevixColors::Accent * 0.8f, 4.0f))
		.SetPressed(FSlateRoundedBoxBrush(MaevixColors::Accent, 4.0f))
		.SetDisabled(FSlateRoundedBoxBrush(MaevixColors::Disabled, 4.0f))
		.SetNormalForeground(FSlateColor(MaevixColors::TextPrimary))
		.SetHoveredForeground(FSlateColor(MaevixColors::TextPrimary))
		.SetPressedForeground(FSlateColor(MaevixColors::TextPrimary))
		.SetDisabledForeground(FSlateColor(MaevixColors::TextMuted))
		.SetNormalPadding(FMargin(12.0f, 6.0f))
		.SetPressedPadding(FMargin(12.0f, 7.0f, 12.0f, 5.0f));

	Style->Set("MaevixEditor.Button", MaevixButton);

	// Primary action button (accent colored)
	const FButtonStyle PrimaryButton = FButtonStyle()
		.SetNormal(FSlateRoundedBoxBrush(MaevixColors::Accent * 0.9f, 4.0f))
		.SetHovered(FSlateRoundedBoxBrush(MaevixColors::Accent, 4.0f))
		.SetPressed(FSlateRoundedBoxBrush(MaevixColors::Accent * 1.1f, 4.0f))
		.SetDisabled(FSlateRoundedBoxBrush(MaevixColors::Disabled, 4.0f))
		.SetNormalForeground(FSlateColor(MaevixColors::TextPrimary))
		.SetHoveredForeground(FSlateColor(MaevixColors::TextPrimary))
		.SetPressedForeground(FSlateColor(MaevixColors::TextPrimary))
		.SetDisabledForeground(FSlateColor(MaevixColors::TextMuted))
		.SetNormalPadding(FMargin(12.0f, 6.0f))
		.SetPressedPadding(FMargin(12.0f, 7.0f, 12.0f, 5.0f));

	Style->Set("MaevixEditor.Button.Primary", PrimaryButton);

	// Icons
	Style->Set(MaevixIconName, new FSlateImageBrush(Style->RootToContentDir(TEXT("Maevix_M_Sm"), TEXT(".png")), FVector2D(16.0, 16.0)));
	Style->Set("MaevixEditor.MaevixIcon.Small", new FSlateImageBrush(Style->RootToContentDir(TEXT("Maevix_M_Sm"), TEXT(".png")), FVector2D(16.0, 16.0)));
	Style->Set("MaevixEditor.MaevixIcon.Large", new FSlateImageBrush(Style->RootToContentDir(TEXT("Maevix_M_Sm"), TEXT(".png")), FVector2D(40.0, 40.0)));

	// Section icons - placeholder colored boxes until final assets
	Style->Set(SettingsIconName, new FSlateRoundedBoxBrush(MaevixColors::HeaderBackground, 2.0f, FVector2f(16.0f, 16.0f)));
	Style->Set(EcosystemIconName, new FSlateRoundedBoxBrush(MaevixColors::TextPrimary, 2.0f, FVector2f(16.0f, 16.0f)));  // Monochrome logo
	Style->Set(DocsIconName, new FSlateRoundedBoxBrush(MaevixColors::Secondary, 2.0f, FVector2f(16.0f, 16.0f)));

	return Style;
}
