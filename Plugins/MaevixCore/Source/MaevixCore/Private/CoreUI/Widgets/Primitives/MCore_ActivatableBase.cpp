// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/Primitives/MCore_ActivatableBase.h"

#include "CoreData/Assets/UI/Themes/MCore_PDA_UITheme_Base.h"
#include "CoreData/Logging/LogMaevixUI.h"
#include "CoreData/DevSettings/MCore_CoreSettings.h"
#include "CoreUI/MCore_UISubsystem.h"

#include "GameplayTagAssetInterface.h"
#include "Input/CommonUIInputTypes.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"

#if WITH_EDITOR
#include "Editor/WidgetCompilerLog.h"
#endif

#define LOCTEXT_NAMESPACE "MaevixCoreUI"

namespace
{
	UWidget* FindFocusedDescendant(UWidget* Root)
	{
		if (!Root) { return nullptr; }
		if (Root->HasKeyboardFocus()) { return Root; }

		if (UPanelWidget* Panel = Cast<UPanelWidget>(Root))
		{
			for (int32 i = 0; i < Panel->GetChildrenCount(); i++)
			{
				if (UWidget* Found = FindFocusedDescendant(Panel->GetChildAt(i)))
				{
					return Found;
				}
			}
		}

		return nullptr;
	}
}

UMCore_ActivatableBase::UMCore_ActivatableBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoActivate = false;
}

void UMCore_ActivatableBase::NativeConstruct()
{
	/* Widget stacks manage activation via HandleActiveIndexChanged -> ActivateWidget().
	 * If bAutoActivate is true, NativeConstruct auto-activates BEFORE the stack calls
	 * ActivateWidget, making the stack's call a no-op (bIsActive already true).
	 * This silently prevents NativeOnActivated and BP_OnActivated from ever firing. */
	if (bAutoActivate)
	{
		UE_LOG(LogMaevixUI, Error,
			TEXT("ActivatableBase::NativeConstruct: bAutoActivate is TRUE, stack-managed widgets must not auto-activate. Forcing to false. widget=%s"),
			*GetNameSafe(this));
		bAutoActivate = false;
	}

	Super::NativeConstruct();
}

/**
 * @param InputAction          DataTable row reference to the input action to bind.
 * @param Callback             Delegate fired when the action triggers.
 * @param IABindingHandle      Output handle for optional manual unregistration.
 * @param OverrideDisplayName  Display name override for the CommonUI action bar. Empty uses default.
 * @param bShouldDisplayInActionBar  Whether this binding appears in the CommonUI action bar.
 */
void UMCore_ActivatableBase::RegisterBinding(
	FDataTableRowHandle InputAction,
	const FInputActionExecutedDelegate& Callback,
	FMCore_InputActionBindingHandle& IABindingHandle,
	FText OverrideDisplayName,
	bool bShouldDisplayInActionBar)
{
	if (InputAction.IsNull() || InputAction.RowName.IsNone())
	{
		UE_LOG(LogMaevixUI, Warning, TEXT("ActivatableBase::RegisterBinding: InputAction row handle is invalid [%s]"),
			*GetNameSafe(this));
		return;
	}

	if (!Callback.IsBound())
	{
		UE_LOG(LogMaevixUI, Warning, TEXT("ActivatableBase::RegisterBinding: Callback delegate not bound [%s]"),
			*GetNameSafe(this));
		return;
	}

	FBindUIActionArgs BindArgs(InputAction, FSimpleDelegate::CreateLambda([InputAction, Callback]()
	{
		Callback.ExecuteIfBound(InputAction.RowName);
	}));

	BindArgs.bDisplayInActionBar = bShouldDisplayInActionBar;

	if (!OverrideDisplayName.IsEmpty())
	{
		BindArgs.OverrideDisplayName = OverrideDisplayName;
	}

	FUIActionBindingHandle NewHandle = RegisterUIActionBinding(BindArgs);

	IABindingHandle.CommonHandle = NewHandle;

	IABindingHandles.Add(NewHandle);

	UE_LOG(LogMaevixUI, Log,
		TEXT("ActivatableBase::RegisterBinding: Registered input binding for action: %s (Display in ActionBar: %s) widget=%s"),
		*InputAction.RowName.ToString(),
		bShouldDisplayInActionBar ? TEXT("Yes") : TEXT("No"),
		*GetNameSafe(this));
}

void UMCore_ActivatableBase::UnregisterAllBindings()
{
	if (IABindingHandles.Num() == 0) { return; }

	UE_LOG(LogMaevixUI, Log, TEXT("ActivatableBase::UnregisterAllBindings: Unregistering input binding(s), widget=%s"),
		*GetNameSafe(this));

	for (FUIActionBindingHandle& Handle : IABindingHandles)
	{
		if (Handle.IsValid()) { Handle.Unregister(); }
	}

	IABindingHandles.Empty();
}

void UMCore_ActivatableBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	/* Apply design-time theme for UMG editor preview. Runtime re-applies from UISubsystem. */
	if (IsDesignTime())
	{
		ApplyTheme(UMCore_CoreSettings::GetDesignTimeTheme());
	}
}

void UMCore_ActivatableBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	/* Guard: Reset stale bIsActive from Blueprint CDO serialization.
	 * bIsActive is a non-Transient UPROPERTY in CommonActivatableWidget.
	 * If a Blueprint was saved while its widget preview was in an active state,
	 * bIsActive=true gets baked into the CDO. Every new instance then starts
	 * "already activated", causing the stack's ActivateWidget() to silently
	 * skip InternalProcessActivation -> NativeOnActivated -> BP_OnActivated. */
	if (IsActivated())
	{
		UE_LOG(LogMaevixUI, Log,
			TEXT("ActivatableBase::NativeOnInitialized: Stale bIsActive detected in CDO, resetting for stack-managed activation, widget=%s"),
			*GetNameSafe(this));
		Reset();
	}

	BindThemeDelegate();

	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>())
		{
			/* Cache theme before ApplyTheme so BP K2_OnThemeApplied overrides and any
			 * subclass work during ApplyTheme's reflection dispatch see the resolved theme,
			 * not the prior null. ApplyTheme_Implementation also assigns CachedTheme,
			 * but only after the BP dispatch layer has already had a chance to read it. */
			UMCore_PDA_UITheme_Base* ActiveTheme = UISubsystem->GetActiveTheme();
			CachedTheme = ActiveTheme;
			ApplyTheme(ActiveTheme);
		}
	}
}

void UMCore_ActivatableBase::NativeOnActivated()
{
	if (bShouldBlockActivation())
	{
		UE_LOG(LogMaevixUI, Log, TEXT("ActivatableBase::NativeOnActivated: Activation blocked by BlockTags, deactivating, widget=%s"),
			*GetNameSafe(this));

		DeactivateWidget();
		return;
	}

	Super::NativeOnActivated();

	if (bShouldFocusOnActivation)
	{
		if (SavedFocusTarget.IsValid())
		{
			SavedFocusTarget->SetFocus();
			SavedFocusTarget.Reset();

			UE_LOG(LogMaevixUI, Log, TEXT("ActivatableBase::NativeOnActivated: Restored saved focus, widget=%s"),
				*GetNameSafe(this));
			return;
		}

		if (UWidget* WidgetToFocus = NativeGetDesiredFocusTarget())
		{
			WidgetToFocus->SetFocus();

			UE_LOG(LogMaevixUI, Log, TEXT("ActivatableBase::NativeOnActivated: Auto focusing on desired focus target, widget=%s"),
				*GetNameSafe(this));
			return;
		}
	}

}

void UMCore_ActivatableBase::NativeOnDeactivated()
{
	/* Save focused descendant before input cleanup shifts focus */
	SavedFocusTarget = nullptr;
	if (WidgetTree && WidgetTree->RootWidget)
	{
		SavedFocusTarget = FindFocusedDescendant(WidgetTree->RootWidget);
	}

	/* Cleanup input bindings BEFORE calling Super to prevent memory leaks */
	UnregisterAllBindings();

	Super::NativeOnDeactivated();

	UE_LOG(LogMaevixUI, Log, TEXT("ActivatableBase::NativeOnDeactivated: Deactivated, input bindings cleared, widget=%s"),
		*GetNameSafe(this));
}

bool UMCore_ActivatableBase::bShouldBlockActivation() const
{
	if (BlockTags.IsEmpty()) { return false; }

	const APlayerController* OwningPlayer = GetOwningPlayer();
	if (!OwningPlayer)
	{
		UE_LOG(LogMaevixUI, Warning, TEXT("ActivatableBase::bShouldBlockActivation: No PlayerController detected"));
		return false;
	}

	const IGameplayTagAssetInterface* BlockTagInterface = Cast<IGameplayTagAssetInterface>(OwningPlayer);
	if (!BlockTagInterface)
	{
		UE_LOG(LogMaevixUI, Verbose, TEXT("ActivatableBase::bShouldBlockActivation: OwningPlayer doesn't implement the BlockTag Interface, allowing activation"));
		return false;
	}

	FGameplayTagContainer OwnedTags;
	BlockTagInterface->GetOwnedGameplayTags(OwnedTags);

	if (OwnedTags.HasAny(BlockTags))
	{
		UE_LOG(LogMaevixUI, Log, TEXT("ActivatableBase::bShouldBlockActivation: Activation blocked by BlockTags: %s, PlayerTags: %s"),
			*BlockTags.ToStringSimple(),
			*OwnedTags.ToStringSimple());
		return true;
	}

	return false;
}

/**
 * Assigns NewTheme to CachedTheme as its first action so Blueprint K2_OnThemeApplied
 * overrides may read CachedTheme to access the current theme. C++ subclasses
 * overriding this must either call Super::ApplyTheme_Implementation(NewTheme) or
 * assign CachedTheme = NewTheme themselves before performing any work; otherwise
 * CachedTheme will be stale relative to the theme that was just applied.
 */
void UMCore_ActivatableBase::ApplyTheme_Implementation(UMCore_PDA_UITheme_Base* Theme)
{
	CachedTheme = Theme;
	K2_OnThemeApplied(Theme);
}

void UMCore_ActivatableBase::HandleThemeChanged(UMCore_PDA_UITheme_Base* NewTheme)
{
	CachedTheme = NewTheme;
	ApplyTheme(NewTheme);
}

void UMCore_ActivatableBase::BindThemeDelegate()
{
	if (bThemeDelegateBound) { return; }

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer) { return; }

	UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (!UISubsystem) { return; }

	UISubsystem->OnThemeChanged.AddDynamic(this, &UMCore_ActivatableBase::HandleThemeChanged);
	bThemeDelegateBound = true;
}

void UMCore_ActivatableBase::UnbindThemeDelegate()
{
	if (!bThemeDelegateBound) { return; }

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer)
	{
		bThemeDelegateBound = false;
		return;
	}

	UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (UISubsystem)
	{
		UISubsystem->OnThemeChanged.RemoveDynamic(this, &UMCore_ActivatableBase::HandleThemeChanged);
	}

	bThemeDelegateBound = false;
}

void UMCore_ActivatableBase::NativeDestruct()
{
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>())
		{
			UISubsystem->NotifyWidgetDestroyed(this);
		}
	}

	UnbindThemeDelegate();
	Super::NativeDestruct();
}

#if WITH_EDITOR
void UMCore_ActivatableBase::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree,
                                                        class IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);

	if (GetClass()->ClassGeneratedBy == nullptr) { return; }

	/* CommonUI exposes BP_GetDesiredFocusTarget as the Blueprint-overridable function */
	static const FName BPGetDesiredFocusTargetName = TEXT("BP_GetDesiredFocusTarget");

	if (!GetClass()->IsFunctionImplementedInScript(BPGetDesiredFocusTargetName))
	{
		/* Only warn for direct children - intermediate C++ classes may implement it natively */
		const UClass* ParentClass = GetClass()->GetSuperClass();
		if (ParentClass == UMCore_ActivatableBase::StaticClass())
		{
			CompileLog.Warning(LOCTEXT("MissingFocusTarget_Warning",
				"GetDesiredFocusTarget not implemented. Gamepad nav won't work correctly. "
				"Override this function and return a valid, focusable widget."));
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE
