// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/KeyBindings/MCore_KeyBindingPanel_Base.h"

#include "CoreUI/Widgets/KeyBindings/MCore_KeyBindingRow.h"
#include "CoreUI/Widgets/KeyBindings/MCore_KeyBindingButton.h"
#include "CoreUI/Widgets/KeyBindings/MCore_KeyBindingCaptureDialog.h"
#include "CoreUI/Widgets/Primitives/MCore_TabbedContainer.h"
#include "CoreUI/Widgets/Primitives/MCore_ButtonBase.h"
#include "CoreUI/Widgets/Primitives/MCore_ConfirmationDialog.h"
#include "CoreUI/MCore_UISubsystem.h"
#include "CoreData/Libraries/MCore_InputDisplayLibrary.h"
#include "CoreData/Libraries/MCore_ThemeLibrary.h"
#include "CoreData/Assets/UI/Themes/MCore_PDA_UITheme_Base.h"
#include "CoreData/DevSettings/MCore_CoreSettings.h"
#include "CoreData/Types/Input/MCore_KeyBindingTypes.h"
#include "CoreData/Tags/MCore_UILayerTags.h"
#include "CoreData/Logging/LogModulusUI.h"

#include "CommonTextBlock.h"
#include "EnhancedInputSubsystems.h"
#include "Components/ScrollBox.h"
#include "InputMappingContext.h"
#include "Components/ScrollBoxSlot.h"
#include "CoreData/Libraries/MCore_GameSettingsLibrary.h"
#include "CoreData/Logging/LogModulusSettings.h"
#include "GameFramework/PlayerController.h"

// ============================================================================
// LIFECYCLE
// ============================================================================

void UMCore_KeyBindingPanel_Base::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (IsDesignTime())
	{
		ApplyTheme(UMCore_CoreSettings::GetDesignTimeTheme());
	}
}

void UMCore_KeyBindingPanel_Base::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	/* Set default prompt text if not configured */
	if (CapturePromptText.IsEmpty())
	{
		CapturePromptText = NSLOCTEXT("ModulusCore", "CapturePrompt", "Press any key...");
	}
	if (CapturePromptGamepadText.IsEmpty())
	{
		CapturePromptGamepadText = NSLOCTEXT("ModulusCore", "CapturePromptGamepad", "Press any button...");
	}

	BindThemeDelegate();

	/* Apply initial theme */
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UMCore_UISubsystem* UI = LocalPlayer->GetSubsystem<UMCore_UISubsystem>())
		{
			ApplyTheme(UI->GetActiveTheme());
		}
	}

	/* Bind tab selection */
	if (TabbedContainer_Bindings)
	{
		TabbedContainer_Bindings->OnTabSelected.AddDynamic(this, &ThisClass::HandleContextTabSelected);
	}

	/* Bind optional reset buttons */
	if (Btn_ResetAllBindings)
	{
		Btn_ResetAllBindings->OnButtonClicked.AddDynamic(this, &ThisClass::HandleResetAllClicked);
	}
	if (Btn_ResetCategory)
	{
		Btn_ResetCategory->OnButtonClicked.AddDynamic(this, &ThisClass::HandleResetCategoryClicked);
	}

	PopulateBindings(GetOwningPlayer());
}

void UMCore_KeyBindingPanel_Base::NativeDestruct()
{
	UnbindThemeDelegate();

	if (TabbedContainer_Bindings)
	{
		TabbedContainer_Bindings->OnTabSelected.RemoveAll(this);
	}

	if (Btn_ResetAllBindings)
	{
		Btn_ResetAllBindings->OnButtonClicked.RemoveAll(this);
	}
	if (Btn_ResetCategory)
	{
		Btn_ResetCategory->OnButtonClicked.RemoveAll(this);
	}

	for (UMCore_KeyBindingRow* Row : AllRows)
	{
		if (Row)
		{
			Row->OnRowRebindCompleted.RemoveAll(this);
			Row->OnRowCaptureStateChanged.RemoveAll(this);
			Row->OnRowRebindResult.RemoveAll(this);
		}
	}
	AllRows.Reset();

	SpawnedHeaders.Reset();

	DismissActiveCaptureDialog();
	DismissActiveConfirmationDialog();

	Super::NativeDestruct();
}

void UMCore_KeyBindingPanel_Base::DismissActiveCaptureDialog()
{
	if (ActiveCaptureDialog.IsValid())
	{
		ActiveCaptureDialog->OnDialogDismissed.RemoveAll(this);
		ActiveCaptureDialog->OnReadyForCapture.RemoveAll(this);
		ActiveCaptureDialog->DeactivateWidget();
		ActiveCaptureDialog.Reset();
	}

	if (ActiveCaptureButton.IsValid())
	{
		ActiveCaptureButton->ExitCaptureMode();
		ActiveCaptureButton.Reset();
	}
}

void UMCore_KeyBindingPanel_Base::DismissActiveConfirmationDialog()
{
	if (PendingConfirmationDialog.IsValid())
	{
		PendingConfirmationDialog->OnDialogResult.RemoveAll(this);
		PendingConfirmationDialog->DeactivateWidget();
		PendingConfirmationDialog.Reset();
	}
}

// ============================================================================
// PUBLIC API
// ============================================================================

void UMCore_KeyBindingPanel_Base::PopulateBindings(APlayerController* OwningPlayer)
{
    if (!OwningPlayer || !TabbedContainer_Bindings || !KeyBindingRowClass)
    {
       UE_LOG(LogModulusUI, Warning,
          TEXT("KeyBindingPanel_Base::PopulateBindings -- missing PC, TabbedContainer, or KeyBindingRowClass [%s]"),
          *GetNameSafe(this));
       return;
    }

    /* Dismiss any active dialogs before rebuilding */
    DismissActiveCaptureDialog();
    DismissActiveConfirmationDialog();

    /* Clear previous state */
    TabbedContainer_Bindings->ClearAllTabs();
    for (UMCore_KeyBindingRow* Row : AllRows)
    {
       if (Row)
       {
          Row->OnRowRebindCompleted.RemoveAll(this);
          Row->OnRowCaptureStateChanged.RemoveAll(this);
          Row->OnRowRebindResult.RemoveAll(this);
       }
    }
    AllRows.Reset();
    SpawnedHeaders.Reset();

    const UMCore_CoreSettings* CoreSettings = UMCore_CoreSettings::Get();
    FName FirstTabID{NAME_None};
    int32 ContextCount{0};

    if (CoreSettings && CoreSettings->KeyBindingContexts.Num() > 0)
    {
       /* Register all configured IMCs with EnhancedInputUserSettings so
        * profile rows exist before BuildContextPage queries them.
        * Idempotent -- safe on repeated calls. */
       if (const ULocalPlayer* LocalPlayer = OwningPlayer->GetLocalPlayer())
       {
          if (UEnhancedInputLocalPlayerSubsystem* EISubsystem =
             LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
          {
             if (UEnhancedInputUserSettings* UserSettings = EISubsystem->GetUserSettings())
             {
                for (const FMCore_KeyBindingContext& Context : CoreSettings->KeyBindingContexts)
                {
                   if (UInputMappingContext* IMC = Context.MappingContext.LoadSynchronous())
                   {
                      UserSettings->RegisterInputMappingContext(IMC);
                   }
                }
             }
          }
       }

       /* Tabbed mode: one tab per configured InputMappingContext */
       for (const FMCore_KeyBindingContext& Context : CoreSettings->KeyBindingContexts)
       {
          UInputMappingContext* IMC = Context.MappingContext.LoadSynchronous();
          if (!IMC) { continue; }

          UScrollBox* Page = BuildContextPage(OwningPlayer, IMC);
          if (!Page || Page->GetChildrenCount() == 0) { continue; }

          const FName TabID = FName(*IMC->GetName());
          if (TabbedContainer_Bindings->AddTab(TabID, Page))
          {
             /* Set tab button label from context display name */
             if (UMCore_ButtonBase* TabButton = Cast<UMCore_ButtonBase>(
                TabbedContainer_Bindings->GetTabButton(TabID)))
             {
                TabButton->SetButtonText(Context.DisplayName);
             }

             OnContextTabCreated(TabID, Page, Context.DisplayName);
             ++ContextCount;

             if (FirstTabID.IsNone()) { FirstTabID = TabID; }
          }
       }
    }
    else
    {
       /* Fallback: single flat page using GetAllRemappableActions */
       UScrollBox* Page = BuildFallbackPage(OwningPlayer);
       if (Page && Page->GetChildrenCount() > 0)
       {
          const FName TabID{TEXT("AllBindings")};
          TabbedContainer_Bindings->AddTab(TabID, Page);
          FirstTabID = TabID;
          ContextCount = 1;
       }
    }

    if (!FirstTabID.IsNone())
    {
       TabbedContainer_Bindings->SelectTab(FirstTabID);
    }

    UE_LOG(LogModulusUI, Log,
       TEXT("KeyBindingPanel_Base::PopulateBindings -- populated %d binding rows across %d context tabs [%s]"),
       AllRows.Num(), ContextCount, *GetNameSafe(this));
}

void UMCore_KeyBindingPanel_Base::RefreshAllRows()
{
	for (UMCore_KeyBindingRow* Row : AllRows)
	{
		if (Row) { Row->RefreshDisplay(); }
	}
}

// ============================================================================
// PANEL BUILD
// ============================================================================

UScrollBox* UMCore_KeyBindingPanel_Base::BuildContextPage(APlayerController* OwningPlayer,
	const UInputMappingContext* MappingContext)
{
	TArray<FPlayerKeyMapping> AllMappings =
		UMCore_InputDisplayLibrary::GetRemappableActionsForContext(OwningPlayer, MappingContext);

    if (AllMappings.IsEmpty()) { return nullptr; }

    /* Deduplicate by InputAction. Multiple profile rows may reference the same action */
    TMap<TObjectPtr<const UInputAction>, const FPlayerKeyMapping*> UniqueActions;
    for (const FPlayerKeyMapping& Mapping : AllMappings)
    {
       const UInputAction* Action = Mapping.GetAssociatedInputAction();
       if (!Action) { continue; }

       if (!UniqueActions.Contains(Action))
       {
          UniqueActions.Add(Action, &Mapping);
       }
    }

    /* Group by display category */
    TMap<FString, TArray<const FPlayerKeyMapping*>> CategorizedActions;
    for (const auto& Pair : UniqueActions)
    {
       const FText Category = Pair.Value->GetDisplayCategory();
       const FString CategoryKey = Category.ToString();
       CategorizedActions.FindOrAdd(CategoryKey).Add(Pair.Value);
    }

	/* Sort categories by IMC action ordering */
	TArray<FString> SortedCategories;
	for (const FEnhancedActionKeyMapping& ActionMapping : MappingContext->GetMappings())
	{
		const UInputAction* Action = ActionMapping.Action;
		if (!Action || !UniqueActions.Contains(Action)) { continue; }

		const FText Category = UniqueActions[Action]->GetDisplayCategory();
		SortedCategories.AddUnique(Category.ToString());
	}

    const bool bShowSecondary = UMCore_CoreSettings::Get()
       ? UMCore_CoreSettings::Get()->bShowSecondaryBindings
       : false;

    UScrollBox* ScrollBox = NewObject<UScrollBox>(this);

    for (const FString& CategoryKey : SortedCategories)
    {
       const FText CategoryDisplayName = FText::FromString(CategoryKey);

       UCommonTextBlock* Header = CreateThemedCategoryHeader(CategoryDisplayName);
       if (Header)
       {
          if (UScrollBoxSlot* HeaderSlot = Cast<UScrollBoxSlot>(ScrollBox->AddChild(Header)))
          {
	          HeaderSlot->SetPadding(FMargin(16.0f, 0.0f, 0.0f, 4.0f));
          }
          OnCategoryHeaderCreated(CategoryDisplayName, Header);
       }

       for (const FPlayerKeyMapping* Mapping : CategorizedActions[CategoryKey])
       {
          UMCore_KeyBindingRow* Row = CreateWidget<UMCore_KeyBindingRow>(
             this, KeyBindingRowClass);
          if (!Row) { continue; }

          const UInputAction* Action = Mapping->GetAssociatedInputAction();
          if (!Action)
          {
             UE_LOG(LogModulusUI, Warning,
                TEXT("KeyBindingPanel_Base::BuildContextPage -- mapping '%s' has no associated InputAction [%s]"),
                *Mapping->GetMappingName().ToString(), *GetNameSafe(this));
             continue;
          }

          Row->InitFromAction(OwningPlayer, const_cast<UInputAction*>(Action), bShowSecondary);
          Row->OnRowRebindCompleted.AddDynamic(this, &ThisClass::HandleRowRebindCompleted);
          Row->OnRowCaptureStateChanged.AddDynamic(this, &ThisClass::HandleRowCaptureStateChanged);
          Row->OnRowRebindResult.AddDynamic(this, &ThisClass::HandleRowRebindResult);

          ScrollBox->AddChild(Row);
          AllRows.Add(Row);

          OnRowCreated(Row);
       }
    }

    return ScrollBox;	
}

UScrollBox* UMCore_KeyBindingPanel_Base::BuildFallbackPage(APlayerController* OwningPlayer)
{
	TArray<FPlayerKeyMapping> AllMappings =
		UMCore_InputDisplayLibrary::GetAllRemappableActions(OwningPlayer);

	if (AllMappings.IsEmpty())
	{
		UE_LOG(LogModulusUI, Log,
			TEXT("KeyBindingPanel_Base::BuildFallbackPage -- no remappable actions found [%s]"),
			*GetNameSafe(this));
		return nullptr;
	}

	/* Deduplicate by mapping name */
	TMap<FName, const FPlayerKeyMapping*> UniqueActions;
	for (const FPlayerKeyMapping& Mapping : AllMappings)
	{
		const FName Name = Mapping.GetMappingName();
		if (!UniqueActions.Contains(Name))
		{
			UniqueActions.Add(Name, &Mapping);
		}
	}

	/* Group by display category */
	TMap<FString, TArray<const FPlayerKeyMapping*>> CategorizedActions;
	for (const auto& Pair : UniqueActions)
	{
		const FText Category = Pair.Value->GetDisplayCategory();
		const FString CategoryKey = Category.ToString();
		CategorizedActions.FindOrAdd(CategoryKey).Add(Pair.Value);
	}

	/* Sort categories alphabetically */
	TArray<FString> SortedCategories;
	CategorizedActions.GetKeys(SortedCategories);
	SortedCategories.Sort();

	const bool bShowSecondary = UMCore_CoreSettings::Get()
		? UMCore_CoreSettings::Get()->bShowSecondaryBindings
		: false;

	UScrollBox* ScrollBox = NewObject<UScrollBox>(this);

	for (const FString& CategoryKey : SortedCategories)
	{
		const FText CategoryDisplayName = FText::FromString(CategoryKey);

		UCommonTextBlock* Header = CreateThemedCategoryHeader(CategoryDisplayName);
		if (Header)
		{
			ScrollBox->AddChild(Header);
			OnCategoryHeaderCreated(CategoryDisplayName, Header);
		}

		for (const FPlayerKeyMapping* Mapping : CategorizedActions[CategoryKey])
		{
			UMCore_KeyBindingRow* Row = CreateWidget<UMCore_KeyBindingRow>(
				this, KeyBindingRowClass);
			if (!Row) { continue; }

			const UInputAction* Action = Mapping->GetAssociatedInputAction();
			if (!Action)
			{
				UE_LOG(LogModulusUI, Warning,
					TEXT("KeyBindingPanel_Base::BuildFallbackPage -- mapping '%s' has no associated InputAction [%s]"),
					*Mapping->GetMappingName().ToString(), *GetNameSafe(this));
				continue;
			}

			Row->InitFromAction(OwningPlayer, const_cast<UInputAction*>(Action), bShowSecondary);
			Row->OnRowRebindCompleted.AddDynamic(this, &ThisClass::HandleRowRebindCompleted);
			Row->OnRowCaptureStateChanged.AddDynamic(this, &ThisClass::HandleRowCaptureStateChanged);
			Row->OnRowRebindResult.AddDynamic(this, &ThisClass::HandleRowRebindResult);

			ScrollBox->AddChild(Row);
			AllRows.Add(Row);

			OnRowCreated(Row);
		}
	}

	return ScrollBox;
}

UCommonTextBlock* UMCore_KeyBindingPanel_Base::CreateThemedCategoryHeader(
	const FText& CategoryDisplayName)
{
	UCommonTextBlock* Header = NewObject<UCommonTextBlock>(this);
	if (Header)
	{
		Header->SetText(CategoryDisplayName);
		SpawnedHeaders.Add(Header);

		if (CachedTheme.IsValid())
		{
			UMCore_ThemeLibrary::ApplyTextStyleFromTheme(
				GetOwningLocalPlayer(), Header, CachedTheme->HeadingTextStyle);
		}
	}
	return Header;
}

// ============================================================================
// TAB CALLBACKS
// ============================================================================

void UMCore_KeyBindingPanel_Base::HandleContextTabSelected(FName TabID)
{
	ActiveContextTabID = TabID;
}

// ============================================================================
// RESET HANDLERS
// ============================================================================

void UMCore_KeyBindingPanel_Base::HandleResetAllClicked()
{
	DismissActiveConfirmationDialog();
	
	/** Resolve dialog class: per-widget override w/ CoreSettings fallback */
	TSubclassOf<UMCore_ConfirmationDialog> DialogClass = ResetConfirmationDialogClass;
	if (!DialogClass)
	{
		const UMCore_CoreSettings* CoreSettings = UMCore_CoreSettings::Get();
		if (CoreSettings) { DialogClass = CoreSettings->DefaultConfirmationDialogClass; }
		UE_LOG(LogModulusSettings, Log,
			TEXT("KeyBindingPanel::ResetAllClicked -- No local Dialog set, falling back to CoreSettings."));
	}
	
	if (!DialogClass)
	{
		/** No dialog configured, reset directly as safety net */
		UMCore_InputDisplayLibrary::ResetAllBindingsToDefault(GetOwningPlayer());
		RefreshAllRows();
		return;
	}

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer) { return; }

	UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (!UISubsystem) { return; }

	UCommonActivatableWidget* CAWidget = UISubsystem->OpenScreen(
		DialogClass,
		MCore_UILayerTags::MCore_UI_Layer_Modal);

	if (UMCore_ConfirmationDialog* Dialog = Cast<UMCore_ConfirmationDialog>(CAWidget))
	{
		PendingConfirmationDialog = Dialog;
		Dialog->SetDialogMessage(
			NSLOCTEXT("ModulusCore", "ResetAllBindingsMessage", "Reset all key bindings to defaults?"));
		Dialog->SetButtonLabels(
			NSLOCTEXT("ModulusCore", "ResetConfirm", "Reset"),
			NSLOCTEXT("ModulusCore", "ResetCancel", "Cancel"));
		Dialog->OnDialogResult.AddDynamic(this, &ThisClass::HandleResetAllConfirmResult);
	}
}

void UMCore_KeyBindingPanel_Base::HandleResetAllConfirmResult(bool bConfirmed)
{
	if (PendingConfirmationDialog.IsValid())
	{
		PendingConfirmationDialog->OnDialogResult.RemoveAll(this);
	}
	PendingConfirmationDialog.Reset();

	if (bConfirmed)
	{
		UMCore_InputDisplayLibrary::ResetAllBindingsToDefault(GetOwningPlayer());
		RefreshAllRows();
	}
}

void UMCore_KeyBindingPanel_Base::HandleResetCategoryClicked()
{
	DismissActiveConfirmationDialog();

	const UMCore_CoreSettings* CoreSettings = UMCore_CoreSettings::Get();
	if (!CoreSettings) { return; }

	/* Resolve the active IMC from the active tab ID */
	const UInputMappingContext* ActiveIMC = nullptr;
	FText ActiveContextName;

	for (const FMCore_KeyBindingContext& Context : CoreSettings->KeyBindingContexts)
	{
		UInputMappingContext* IMC = Context.MappingContext.LoadSynchronous();
		if (IMC && FName(*IMC->GetName()) == ActiveContextTabID)
		{
			ActiveIMC = IMC;
			ActiveContextName = Context.DisplayName;
			break;
		}
	}

	if (!ActiveIMC)
	{
		UE_LOG(LogModulusUI, Warning,
			TEXT("KeyBindingPanel_Base::HandleResetCategoryClicked -- no active IMC found for tab '%s' [%s]"),
			*ActiveContextTabID.ToString(), *GetNameSafe(this));
		return;
	}
	
	/** Resolve dialog class: per-widget override w/ CoreSettings fallback */
	TSubclassOf<UMCore_ConfirmationDialog> DialogClass = ResetConfirmationDialogClass;
	if (!DialogClass)
	{
		if (CoreSettings) { DialogClass = CoreSettings->DefaultConfirmationDialogClass; }
		UE_LOG(LogModulusSettings, Log,
			TEXT("KeyBindingPanel::ResetAllClicked -- No local Dialog set, falling back to CoreSettings."));
	}
	
	if (!DialogClass)
	{
		/** No dialog configured, reset directly as safety net */
		UMCore_InputDisplayLibrary::ResetBindingsForContext(GetOwningPlayer(), ActiveIMC);
		RefreshAllRows();
		return;
	}
	
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer) { return; }

	UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (!UISubsystem) { return; }

	UCommonActivatableWidget* CAWidget = UISubsystem->OpenScreen(
		DialogClass,
		MCore_UILayerTags::MCore_UI_Layer_Modal);

	if (UMCore_ConfirmationDialog* Dialog = Cast<UMCore_ConfirmationDialog>(CAWidget))
	{
		PendingConfirmationDialog = Dialog;
		Dialog->SetDialogMessage(
			FText::Format(NSLOCTEXT("ModulusCore", "ResetCategoryBindingsMessage",
				"Reset {0} key bindings to defaults?"), ActiveContextName));
		Dialog->SetButtonLabels(
			NSLOCTEXT("ModulusCore", "ResetConfirm", "Reset"),
			NSLOCTEXT("ModulusCore", "ResetCancel", "Cancel"));
		Dialog->OnDialogResult.AddDynamic(this, &ThisClass::HandleResetCategoryConfirmResult);
	}
}

void UMCore_KeyBindingPanel_Base::HandleResetCategoryConfirmResult(bool bConfirmed)
{
	if (PendingConfirmationDialog.IsValid())
	{
		PendingConfirmationDialog->OnDialogResult.RemoveAll(this);
	}
	PendingConfirmationDialog.Reset();

	if (bConfirmed)
	{
		const UMCore_CoreSettings* CoreSettings = UMCore_CoreSettings::Get();
		if (CoreSettings)
		{
			for (const FMCore_KeyBindingContext& Context : CoreSettings->KeyBindingContexts)
			{
				UInputMappingContext* IMC = Context.MappingContext.LoadSynchronous();
				if (IMC && FName(*IMC->GetName()) == ActiveContextTabID)
				{
					UMCore_InputDisplayLibrary::ResetBindingsForContext(GetOwningPlayer(), IMC);
					break;
				}
			}
		}
		RefreshAllRows();
	}
}

void UMCore_KeyBindingPanel_Base::HandleRowRebindCompleted()
{
	RefreshAllRows();
}

void UMCore_KeyBindingPanel_Base::HandleRowCaptureStateChanged(
	UMCore_KeyBindingButton* Button, bool bCapturing)
{
	if (bCapturing)
	{
		DismissActiveCaptureDialog();

		ActiveCaptureButton = Button;

		if (!CaptureDialogClass)
		{
			UE_LOG(LogModulusUI, Warning,
				TEXT("KeyBindingPanel_Base::HandleRowCaptureStateChanged -- CaptureDialogClass not set [%s]"),
				*GetNameSafe(this));
			return;
		}

		ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
		if (!LocalPlayer) { return; }

		UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
		if (!UISubsystem) { return; }

		UCommonActivatableWidget* CAWidget = UISubsystem->OpenScreen(
			CaptureDialogClass, MCore_UILayerTags::MCore_UI_Layer_Modal);

		UMCore_KeyBindingCaptureDialog* Dialog = Cast<UMCore_KeyBindingCaptureDialog>(CAWidget);
		if (!Dialog) { return; }

		ActiveCaptureDialog = Dialog;

		/* Build dialog configuration */
		const FText PromptText = (Button && Button->IsGamepadSlot())
			? CapturePromptGamepadText
			: CapturePromptText;

		FText ActionName;
		if (Button && Button->GetInputAction())
		{
			ActionName = UMCore_InputDisplayLibrary::GetActionDisplayName(
				GetOwningPlayer(), Button->GetInputAction());
		}

		/* Build slot context: "Keyboard/Mouse - Primary" or "Gamepad - Secondary" */
		FText SlotContext;
		if (Button)
		{
			const FString DeviceStr = Button->IsGamepadSlot() ? TEXT("Gamepad") : TEXT("Keyboard/Mouse");
			const FString SlotStr = (Button->GetSlot() == EPlayerMappableKeySlot::First)
				? TEXT("Primary") : TEXT("Secondary");
			SlotContext = FText::FromString(FString::Printf(TEXT("%s - %s"), *DeviceStr, *SlotStr));
		}

		Dialog->ConfigureForCapture(ActionName, SlotContext, PromptText);
		Dialog->OnDialogDismissed.AddDynamic(this, &ThisClass::HandleCaptureDialogDismissed);
		Dialog->OnReadyForCapture.AddDynamic(this, &ThisClass::HandleCaptureDialogReadyForCapture);
	}
	/* On !bCapturing: do nothing. ActiveCaptureButton stays valid for retry after error cooldown.
	   It gets cleared when dialog dismisses or rebind succeeds. */
}

void UMCore_KeyBindingPanel_Base::HandleRowRebindResult(bool bSuccess, FText ErrorMessage)
{
	if (bSuccess)
	{
		/* Dismiss dialog on successful rebind */
		if (ActiveCaptureDialog.IsValid())
		{
			ActiveCaptureDialog->OnDialogDismissed.RemoveAll(this);
			ActiveCaptureDialog->OnReadyForCapture.RemoveAll(this);
			ActiveCaptureDialog->DeactivateWidget();
			ActiveCaptureDialog.Reset();
		}
		ActiveCaptureButton.Reset();
	}
	else if (!ErrorMessage.IsEmpty())
	{
		/* Show error in dialog, keep open for retry */
		if (ActiveCaptureDialog.IsValid())
		{
			ActiveCaptureDialog->ShowCaptureError(ErrorMessage);
		}
	}
}

void UMCore_KeyBindingPanel_Base::HandleCaptureDialogDismissed()
{
	/* User hit Cancel or Back */
	if (ActiveCaptureButton.IsValid())
	{
		ActiveCaptureButton->ExitCaptureMode();
	}
	ActiveCaptureButton.Reset();

	if (ActiveCaptureDialog.IsValid())
	{
		ActiveCaptureDialog->OnDialogDismissed.RemoveAll(this);
		ActiveCaptureDialog->OnReadyForCapture.RemoveAll(this);
		ActiveCaptureDialog.Reset();
	}
}

void UMCore_KeyBindingPanel_Base::HandleCaptureDialogReadyForCapture()
{
	/* Error cooldown expired, dialog restored prompt. Re-enter capture. */
	if (ActiveCaptureButton.IsValid())
	{
		ActiveCaptureButton->EnterCaptureMode();
	}
	else
	{
		/* Button went stale (widget destroyed during cooldown). Dismiss dialog. */
		HandleCaptureDialogDismissed();
	}
}

// ============================================================================
// THEME
// ============================================================================

void UMCore_KeyBindingPanel_Base::ApplyTheme_Implementation(UMCore_PDA_UITheme_Base* NewTheme)
{
	CachedTheme = NewTheme;

	if (!NewTheme) { return; }

	for (UCommonTextBlock* Header : SpawnedHeaders)
	{
		if (Header)
		{
			UMCore_ThemeLibrary::ApplyTextStyleFromTheme(
				GetOwningLocalPlayer(), Header, NewTheme->HeadingTextStyle);
		}
	}

	K2_OnThemeApplied(NewTheme);
}

void UMCore_KeyBindingPanel_Base::HandleThemeChanged(UMCore_PDA_UITheme_Base* NewTheme)
{
	CachedTheme = NewTheme;
	ApplyTheme(NewTheme);
}

void UMCore_KeyBindingPanel_Base::BindThemeDelegate()
{
	if (bThemeDelegateBound) { return; }

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer) { return; }

	UMCore_UISubsystem* UI = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (!UI) { return; }

	UI->OnThemeChanged.AddDynamic(this, &UMCore_KeyBindingPanel_Base::HandleThemeChanged);
	bThemeDelegateBound = true;
}

void UMCore_KeyBindingPanel_Base::UnbindThemeDelegate()
{
	if (!bThemeDelegateBound) { return; }

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer)
	{
		bThemeDelegateBound = false;
		return;
	}

	UMCore_UISubsystem* UI = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (UI)
	{
		UI->OnThemeChanged.RemoveDynamic(this, &UMCore_KeyBindingPanel_Base::HandleThemeChanged);
	}

	bThemeDelegateBound = false;
}
