// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved


#include "CoreData/Libraries/MCore_ThemeLibrary.h"

#include "CoreData/Assets/UI/Styles/MCore_PDA_SliderStyle.h"
#include "CoreData/Settings/MCore_PlayerSettingsSubsystem.h"
#include "CoreData/Logging/LogMaevixUI.h"

#include "CommonTextBlock.h"
#include "CommonButtonBase.h"
#include "Engine/LocalPlayer.h"

void UMCore_ThemeLibrary::ApplyTextStyleFromTheme(const ULocalPlayer* LocalPlayer, UCommonTextBlock* TextBlock,
	const TArray<TSubclassOf<UCommonTextStyle>>& TextStyleArray)
{
	if (!TextBlock || TextStyleArray.IsEmpty()) { return; }

	int32 SizeIndex{0};
	if (LocalPlayer)
	{
		if (const UMCore_PlayerSettingsSubsystem* SettingsSubsystem = LocalPlayer->GetSubsystem<UMCore_PlayerSettingsSubsystem>())
		{
			SizeIndex = SettingsSubsystem->GetActiveTextSizeIndex();
		}
	}
	
	const TSubclassOf<UCommonTextStyle> ResolvedStyle =
		TextStyleArray.IsValidIndex(SizeIndex)
		? TextStyleArray[SizeIndex]
		: TextStyleArray[0];
	
	if (ResolvedStyle) { TextBlock->SetStyle(ResolvedStyle); }
	UE_LOG(LogMaevixUI, VeryVerbose, TEXT("ThemeLibrary::ApplyTextStyleFromTheme: applied style at size index %d"), SizeIndex);
}

FSliderStyle UMCore_ThemeLibrary::BuildSliderStyle(const UMCore_PDA_SliderStyle* SliderStyleDA,
	const FSliderStyle& BaseStyle)
{
	if (!SliderStyleDA) { return BaseStyle; }
	
	FSliderStyle NewStyle = BaseStyle;
	
	FSlateBrush BarBrush;
	if (SliderStyleDA->BarImage)
	{
		BarBrush.SetResourceObject(SliderStyleDA->BarImage);
	}
	
	BarBrush.TintColor = SliderStyleDA->BarNormalTint;
	NewStyle.SetNormalBarImage(BarBrush);
	NewStyle.SetHoveredBarImage(BarBrush);
	NewStyle.SetDisabledBarImage(BarBrush);
	
	FSlateBrush ThumbNormalBrush;
	if (SliderStyleDA->ThumbImage)
	{
		ThumbNormalBrush.SetResourceObject(SliderStyleDA->ThumbImage);
	}
	
	ThumbNormalBrush.TintColor = SliderStyleDA->ThumbNormalTint;
	ThumbNormalBrush.SetImageSize(SliderStyleDA->ThumbSize);
	NewStyle.SetNormalThumbImage(ThumbNormalBrush);
	NewStyle.SetDisabledThumbImage(ThumbNormalBrush);
	
	FSlateBrush ThumbHoverBrush;
	if (SliderStyleDA->ThumbImage)
	{
		ThumbHoverBrush.SetResourceObject(SliderStyleDA->ThumbImage);
	}
	
	ThumbHoverBrush.TintColor = SliderStyleDA->ThumbHoveredTint;
	ThumbHoverBrush.SetImageSize(SliderStyleDA->ThumbSize);
	NewStyle.SetHoveredThumbImage(ThumbHoverBrush);
	
	NewStyle.BarThickness = SliderStyleDA->BarThickness;
	
	UE_LOG(LogMaevixUI, VeryVerbose, TEXT("ThemeLibrary::BuildSliderStyle: slider style built from DataAsset"));
	return NewStyle;
}

TSubclassOf<UCommonButtonStyle> UMCore_ThemeLibrary::ResolveButtonStyle(TSubclassOf<UCommonButtonStyle> StyleOverride,
	TSubclassOf<UCommonButtonStyle> ThemeDefault)
{
	return StyleOverride ? StyleOverride : ThemeDefault;
}

UMCore_PDA_ExtensionStyle_Base* UMCore_ThemeLibrary::GetExtensionStyleOfClass(
	const UMCore_PDA_UITheme_Base* Theme,
	FGameplayTag StyleTag,
	TSubclassOf<UMCore_PDA_ExtensionStyle_Base> TargetClass)
{
	if (!Theme || !TargetClass) { return nullptr; }
	UMCore_PDA_ExtensionStyle_Base* Style = Theme->GetExtensionStyle(StyleTag);
	return (Style && Style->IsA(TargetClass)) ? Style : nullptr;
}
