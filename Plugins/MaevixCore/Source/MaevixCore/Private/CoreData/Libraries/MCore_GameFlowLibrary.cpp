// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/Libraries/MCore_GameFlowLibrary.h"

#include "CoreData/DevSettings/MCore_CoreSettings.h"
#include "CoreData/Logging/LogMaevixLoading.h"
#include "CoreData/Tags/MCore_LoadingTags.h"
#include "CoreLoading/MCore_LoadingScreenSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UMCore_GameFlowLibrary::StartNewGame(const UObject* WorldContextObject)
{
	const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get();
	if (!Settings || Settings->NewGameTargetLevel.IsNull())
	{
		UE_LOG(LogMaevixLoading, Warning,
			TEXT("GameFlowLibrary::StartNewGame: no NewGameTargetLevel configured; aborting travel"));
		return;
	}

	/* Prime the loading-screen context before travel so the PreLoadMap handler can
	   select an in-game screen. Best-effort: the subsystem is absent on dedicated
	   servers (ShouldCreateSubsystem returns false), and a missing subsystem must
	   not block travel. */
	if (const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		if (UMCore_LoadingScreenSubsystem* LoadingSubsystem =
			GameInstance->GetSubsystem<UMCore_LoadingScreenSubsystem>())
		{
			LoadingSubsystem->SetNextLoadingContext(
				FGameplayTagContainer(MCore_LoadingTags::MCore_Loading_Context_InGame.GetTag()));
		}
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(WorldContextObject, Settings->NewGameTargetLevel);
}
