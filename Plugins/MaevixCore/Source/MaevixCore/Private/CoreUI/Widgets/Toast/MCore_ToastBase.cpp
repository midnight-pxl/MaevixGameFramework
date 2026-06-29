// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/Toast/MCore_ToastBase.h"

#include "CoreData/Logging/LogMaevixToast.h"

#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

namespace
{
	/* One warning per (widget class, bind name) per process: a misnamed BindWidgetOptional
	   surfaces once as a log line rather than spamming on every populate. */
	void WarnMissingBindOnce(const UMCore_ToastBase* Toast, const TCHAR* BindName)
	{
		static TSet<FString> Warned;
		const FString ClassName = GetNameSafe(Toast ? Toast->GetClass() : nullptr);
		const FString Key = FString::Printf(TEXT("%s.%s"), *ClassName, BindName);

		bool bAlreadyWarned = false;
		Warned.Add(Key, &bAlreadyWarned);
		if (!bAlreadyWarned)
		{
			UE_LOG(LogMaevixToast, Warning,
				TEXT("ToastBase::PopulateToast: '%s' has non-empty text but no '%s' bind; check the WBP BindWidgetOptional name."),
				*ClassName, BindName);
		}
	}
}

bool UMCore_ToastBase::PlayIntro()
{
	K2_OnPlayIntro();

	if (bAwaitIntroAnimation)
	{
		/* WBP owns completion; the subsystem arms the intro watchdog. */
		return true;
	}

	NotifyIntroComplete();
	return false;
}

bool UMCore_ToastBase::PlayOutro()
{
	K2_OnPlayOutro();

	if (bAwaitOutroAnimation)
	{
		return true;
	}

	NotifyOutroComplete();
	return false;
}

void UMCore_ToastBase::NotifyIntroComplete()
{
	if (bIntroDone)
	{
		return;
	}
	bIntroDone = true;
	OnIntroComplete.Broadcast(this);
}

void UMCore_ToastBase::NotifyOutroComplete()
{
	if (bOutroDone)
	{
		return;
	}
	bOutroDone = true;
	OnOutroComplete.Broadcast(this);
}

void UMCore_ToastBase::PopulateToast_Implementation(const FMCore_ToastRequest& Request)
{
	if (Txt_Title)
	{
		Txt_Title->SetText(Request.Title);
	}
	else if (!Request.Title.IsEmpty())
	{
		WarnMissingBindOnce(this, TEXT("Txt_Title"));
	}

	if (Txt_Body)
	{
		Txt_Body->SetText(Request.Body);
		/* Collapse-when-empty default: a body-less toast lays out as title-only. */
		Txt_Body->SetVisibility(Request.Body.IsEmpty()
			? ESlateVisibility::Collapsed
			: ESlateVisibility::SelfHitTestInvisible);
	}
	else if (!Request.Body.IsEmpty())
	{
		WarnMissingBindOnce(this, TEXT("Txt_Body"));
	}

	if (Img_Icon)
	{
		UTexture2D* IconTexture = Request.Icon.IsNull() ? nullptr : Request.Icon.LoadSynchronous();
		if (IconTexture)
		{
			Img_Icon->SetBrushFromTexture(IconTexture);
			Img_Icon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			/* Collapse-when-empty default: no icon supplied (or failed to load). */
			Img_Icon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	K2_OnPopulateToast(Request);
}
