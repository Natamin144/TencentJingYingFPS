// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "ShooterCharacter.h"
#include "ShooterBulletCounterUI.h"
#include "FPSProject3.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "UI/ShooterUI.h"
#include "Net/UnrealNetwork.h"
#include "Variant_Shooter/ShooterGameState.h"
void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController())
	{
		if (SVirtualJoystick::ShouldDisplayTouchInterface())
		{
			// spawn the mobile controls widget
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

			if (MobileControlsWidget)
			{
				// add the controls to the player screen
				MobileControlsWidget->AddToPlayerScreen(0);
			} else {
				UE_LOG(LogFPSProject3, Error, TEXT("Could not spawn mobile controls widget."));
			}
		}

		// create the bullet counter widget and add it to the screen
		BulletCounterUI = CreateWidget<UShooterBulletCounterUI>(this, BulletCounterUIClass);

		if (BulletCounterUI)
		{
			BulletCounterUI->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogFPSProject3, Error, TEXT("Could not spawn bullet counter widget."));

		}

		// create the global shooter UI (score) for this local player controller
		if (ShooterUIClass) {
			ShooterUI = CreateWidget<UShooterUI>(this, ShooterUIClass);
			ShooterUI->CustomPlayerName = CustomPlayerName;
			if (ShooterUI)
			{
				ShooterUI->AddToPlayerScreen(0);
			}
			else
			{
				UE_LOG(LogFPSProject3, Error, TEXT("Could not spawn ShooterUI widget."));
			}
		}
		else {
			UE_LOG(LogFPSProject3, Error, TEXT("ShooterUIClass is not set in ShooterPlayerController."));
		}
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		AShooterGameState* GameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(GetWorld()));
		if(IsLocalController())
		{
			PlayerTeamByte = 0;
			CustomPlayerName = FString::Printf(TEXT("Server Player"));
			NetworkPlayerID = GameState -> RegisteredServerControllersCount;
			GameState->RegisteredServerControllersCount = GameState->RegisteredServerControllersCount + 1;
		}
		else
		{
			PlayerTeamByte = 1;
			NetworkPlayerID = GameState->RegisteredServerControllersCount;
			GameState->RegisteredServerControllersCount = GameState->RegisteredServerControllersCount + 1;
			CustomPlayerName = FString::Printf(TEXT("Client Player %d"), NetworkPlayerID);
		}
	}
}

void AShooterPlayerController::SetupInputComponent()
{
	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// add the input mapping contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AShooterPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// reset the bullet counter HUD
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(0, 0);
	}

	// Respawn is now handled by GameMode. Just log and let server schedule/perform respawn.
	UE_LOG(LogTemp, Log, TEXT("OnPawnDestroyed: pawn destroyed for controller %s - respawn will be handled by server GameMode"), *GetName());
}

void AShooterPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	if(IsLocalPlayerController())
		UE_LOG(LogTemp, Log, TEXT("调用AShooterPlayerController::OnBulletCountUpdated"));
	// update the UI
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(MagazineSize, Bullets);
	}
}

void AShooterPlayerController::OnPawnDamaged(float LifePercent)
{
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_Damaged(LifePercent);
	}
}

void AShooterPlayerController::Client_UpdateTeamScore_Implementation(uint8 TeamByte, int32 Score)
{
	// Ensure this runs on owning client and UI exists
	if (!IsLocalPlayerController()) return;

	// Lazy-create ShooterUI if not present
	if (!ShooterUI && ShooterUIClass)
	{
		ShooterUI = CreateWidget<UShooterUI>(this, ShooterUIClass);
		if (ShooterUI)
		{
			ShooterUI->AddToPlayerScreen(0);
		}
	}

	if (ShooterUI)
	{
		ShooterUI->BP_UpdateScore(TeamByte, Score);
	}
}

void AShooterPlayerController::UpdateLocalTeamScore(uint8 TeamByte, int32 Score)
{
	// Ensure this runs only on the owning client
	if (!IsLocalPlayerController()) return;

	// Lazy-create ShooterUI if not present
	if (!ShooterUI && ShooterUIClass)
	{
		ShooterUI = CreateWidget<UShooterUI>(this, ShooterUIClass);
		if (ShooterUI)
		{
			ShooterUI->AddToPlayerScreen(0);
		}
	}

	if (ShooterUI)
	{
		ShooterUI->BP_UpdateScore(TeamByte, Score);
	}
}

void AShooterPlayerController::Client_OnRespawned_Implementation()
{
	// Client-side hook after server respawned and possessed a new pawn.
	// Blueprints can override or bind to this event. For now just log.
	if (IsLocalPlayerController())
	{
		UE_LOG(LogTemp, Log, TEXT("Client_OnRespawned: local player has been respawned by server."));
	}
}

void AShooterPlayerController::Client_OnGameOver_Implementation(bool bWin, uint8 WinningTeam)
{
	// Ensure this runs on owning client
	if (!IsLocalPlayerController()) return;

	// Lazy-create ShooterUI if not present
	if (!ShooterUI && ShooterUIClass)
	{
		ShooterUI = CreateWidget<UShooterUI>(this, ShooterUIClass);
		if (ShooterUI)
		{
			ShooterUI->AddToPlayerScreen(0);
		}
	}

	// Trigger blueprint UI event
	if (ShooterUI)
	{
		ShooterUI->BP_GameOver(bWin);
	}

	UE_LOG(LogTemp, Log, TEXT("Client_OnGameOver: WinningTeam=%d, bWin=%d"), WinningTeam, bWin);
}

void AShooterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// 配置TeamByte从服务器复制到所有客户端
	DOREPLIFETIME(AShooterPlayerController, PlayerTeamByte);
}