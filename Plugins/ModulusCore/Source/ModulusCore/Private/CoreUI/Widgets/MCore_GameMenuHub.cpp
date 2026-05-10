// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/MCore_GameMenuHub.h"

#include "CoreUI/MCore_UISubsystem.h"
#include "CoreUI/Widgets/Primitives/MCore_TabbedContainer.h"
#include "CoreData/Logging/LogModulusUI.h"
#include "CoreData/Types/UI/MCore_MenuTabTypes.h"

#include "GameplayTagContainer.h"
#include "CommonButtonBase.h"
#include "CommonAnimatedSwitcher.h"

UMCore_GameMenuHub::UMCore_GameMenuHub(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
}

void UMCore_GameMenuHub::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (TabbedContainer)
    {
        TabbedContainer->OnTabAdded.AddDynamic(this, &ThisClass::HandleContainerTabAdded);
    }
}

void UMCore_GameMenuHub::NativeDestruct()
{
    if (TabbedContainer)
    {
        TabbedContainer->OnTabAdded.RemoveAll(this);
    }

    Super::NativeDestruct();
}

void UMCore_GameMenuHub::RebuildTabBar()
{
    if (!TabbedContainer)
    {
        UE_LOG(LogModulusUI, Error,
            TEXT("GameMenuHub::RebuildTabBar: TabbedContainer not bound, verify BindWidget in Blueprint"));
        return;
    }

    ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
    if (!LocalPlayer)
    {
        UE_LOG(LogModulusUI, Warning, TEXT("GameMenuHub::RebuildTabBar: no owning LocalPlayer"));
        return;
    }

    UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
    if (!UISubsystem)
    {
        UE_LOG(LogModulusUI, Warning, TEXT("GameMenuHub::RebuildTabBar: no UISubsystem found"));
        return;
    }

    const TArray<FMCore_MenuTab>& RegisteredScreens = UISubsystem->GetRegisteredMenuScreens();

    TabbedContainer->ClearAllTabs();

    if (RegisteredScreens.IsEmpty())
    {
        if (EmptyStateWidgetClass)
        {
            UCommonActivatableWidget* EmptyWidget = CreateWidget<UCommonActivatableWidget>(
                this, EmptyStateWidgetClass);

            if (EmptyWidget)
            {
                UCommonAnimatedSwitcher* Switcher = TabbedContainer->GetPageSwitcher();
                if (Switcher)
                {
                    Switcher->AddChild(EmptyWidget);
                    Switcher->SetActiveWidget(EmptyWidget);
                }
                UE_LOG(LogModulusUI, Log, TEXT("GameMenuHub::RebuildTabBar: showing empty state (no screens registered)"));
            }
        }
        else
        {
            UE_LOG(LogModulusUI, Warning, TEXT("GameMenuHub::RebuildTabBar: no screens registered and EmptyStateWidgetClass not set"));
        }
        return;
    }

    for (int32 Index = 0; Index < RegisteredScreens.Num(); ++Index)
    {
        const FMCore_MenuTab& Tab = RegisteredScreens[Index];
        FName TabNameID = FName(*Tab.TabID.ToString());

        UCommonActivatableWidget* ScreenWidget = CreateWidget<UCommonActivatableWidget>(
            this, Tab.ScreenWidgetClass);

        if (!ScreenWidget)
        {
            UE_LOG(LogModulusUI, Error,
                TEXT("GameMenuHub::RebuildTabBar: failed to create widget for '%s'"),
                *Tab.TabID.ToString());
            continue;
        }

        TabbedContainer->AddTab(TabNameID, ScreenWidget);
        OnPageCreated(TabNameID, ScreenWidget);
    }

    if (!RegisteredScreens.IsEmpty())
    {
        FName FirstTabID = FName(*RegisteredScreens[0].TabID.ToString());
        TabbedContainer->SelectTab(FirstTabID);

        UE_LOG(LogModulusUI, Log,
            TEXT("GameMenuHub::RebuildTabBar: rebuilt tab bar with %d tabs"),
            RegisteredScreens.Num());
    }
}

bool UMCore_GameMenuHub::SetTabEnabled(FGameplayTag TabID, bool bEnabled)
{
    if (!TabbedContainer || !TabID.IsValid()) { return false; }

    FName TabNameID = FName(*TabID.ToString());
    bool bSuccess = TabbedContainer->SetTabEnabled(TabNameID, bEnabled);

    if (bSuccess)
    {
        UE_LOG(LogModulusUI, Log, TEXT("GameMenuHub::SetTabEnabled: tab '%s' is %s"),
            *TabID.ToString(),
            bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
    }
    else
    {
        UE_LOG(LogModulusUI, Warning, TEXT("GameMenuHub::SetTabEnabled: tab '%s' not found"),
            *TabID.ToString());
    }

    return bSuccess;
}

bool UMCore_GameMenuHub::SetTabHidden(FGameplayTag TabID, bool bIsHidden)
{
    if (!TabbedContainer || !TabID.IsValid()) { return false; }

    FName TabNameID = FName(*TabID.ToString());
    bool bSuccess = TabbedContainer->SetTabHidden(TabNameID, bIsHidden);

    if (bSuccess)
    {
        UE_LOG(LogModulusUI, Log, TEXT("GameMenuHub::SetTabHidden: tab '%s' is %s"),
            *TabID.ToString(),
            bIsHidden ? TEXT("HIDDEN") : TEXT("VISIBLE"));
    }
    else
    {
        UE_LOG(LogModulusUI, Warning, TEXT("GameMenuHub::SetTabHidden: tab '%s' not found"),
            *TabID.ToString());
    }

    return bSuccess;
}

bool UMCore_GameMenuHub::IsTabEnabled(FGameplayTag TabID) const
{
    if (!TabbedContainer || !TabID.IsValid()) { return false; }

    return TabbedContainer->IsTabEnabled(FName(*TabID.ToString()));
}

bool UMCore_GameMenuHub::IsTabHidden(FGameplayTag TabID) const
{
    if (!TabbedContainer || !TabID.IsValid()) { return false; }

    return TabbedContainer->IsTabHidden(FName(*TabID.ToString()));
}

TOptional<FUIInputConfig> UMCore_GameMenuHub::GetDesiredInputConfig() const
{
    FUIInputConfig Config(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture, false);
    return Config;
}

UWidget* UMCore_GameMenuHub::NativeGetDesiredFocusTarget() const
{
    if (TabbedContainer)
    {
        const FName SelectedTab = TabbedContainer->GetSelectedTabID();
        if (!SelectedTab.IsNone())
        {
            if (UWidget* PageWidget = TabbedContainer->GetPageWidget(SelectedTab))
            {
                return PageWidget;
            }
        }

        return TabbedContainer;
    }

    return Super::NativeGetDesiredFocusTarget();
}

void UMCore_GameMenuHub::HandleContainerTabAdded(FName TabID, UCommonButtonBase* TabButton)
{
    OnTabCreated(TabID, TabButton);
}
