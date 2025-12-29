// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterGameMode.h"
#include "ShooterUI.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "Variant_Shooter/ShooterPlayerController.h"
#include "Variant_Shooter/ShooterGameState.h"

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();
    UE_LOG(LogGameMode, Warning, TEXT("AShooterGameMode::BeginPlay - GameMode loaded, Authority: %d"), HasAuthority());
	// UI will be created on each client's PlayerController BeginPlay.
}

AActor* AShooterGameMode::ChoosePlayerStart(AController* PlayerController)
{
    if (!HasAuthority())
    {
        UE_LOG(LogGameMode, Warning, TEXT("ChoosePlayerStart - Client side, skip execution"));
        return Super::ChoosePlayerStart_Implementation(PlayerController);
    }

    UE_LOG(LogTemp, Log, TEXT("Choose PlayerStart"));
    if (!IsValid(PlayerController))
    {
        UE_LOG(LogGameMode, Warning, TEXT("ChoosePlayerStart - Player controller is invalid"));
        return Super::ChoosePlayerStart_Implementation(PlayerController);
    }

    int32 PlayerId = PlayerController->PlayerState ? PlayerController->PlayerState->GetPlayerId() : -1;
    UE_LOG(LogGameMode, Log, TEXT("ChoosePlayerStart - Server execute, Player ID: %d"), PlayerId);

    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

    if (PlayerStarts.Num() == 0)
    {
        UE_LOG(LogGameMode, Warning, TEXT("ChoosePlayerStart - No PlayerStart found, use super logic"));
        return Super::ChoosePlayerStart_Implementation(PlayerController);
    }

    if (PlayerId == 0)
    {
        UE_LOG(LogGameMode, Log, TEXT("ChoosePlayerStart - Assign PlayerStartA to Player %d"), PlayerId);
        for (AActor* Start : PlayerStarts)
        {
            if (IsValid(Start) && Start->ActorHasTag(FName("PlayerStartA")))
            {
                return Start;
            }
        }
    }
    else if (PlayerId == 1)
    {
        UE_LOG(LogGameMode, Log, TEXT("ChoosePlayerStart - Assign PlayerStartB to Player %d"), PlayerId);
        for (AActor* Start : PlayerStarts)
        {
            if (IsValid(Start) && Start->ActorHasTag(FName("PlayerStartB")))
            {
                return Start;
            }
        }
    }

    return PlayerStarts[0];
}

void AShooterGameMode::IncrementTeamScore(uint8 TeamByte)
{
	// Delegate the score change to GameState (replicated)
	if (AShooterGameState* GS = GetGameState<AShooterGameState>())
	{
		UE_LOG(LogTemp, Log, TEXT("IncrementTeamScore called for Team %d"), TeamByte);
		GS->AddTeamScore(TeamByte);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("IncrementTeamScore - Failed to get ShooterGameState"));
	}
}

void AShooterGameMode::ScheduleRespawn(AController* Controller, float Delay)
{
	if (!HasAuthority() || !IsValid(Controller)) return;

	FTimerHandle& Handle = RespawnTimerHandles.FindOrAdd(Controller);
	FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &AShooterGameMode::RespawnController, Controller);
	GetWorld()->GetTimerManager().SetTimer(Handle, Delegate, Delay, false);
	UE_LOG(LogTemp, Log, TEXT("Scheduled respawn for controller %s in %f seconds"), *Controller->GetName(), Delay);
}

void AShooterGameMode::RespawnController(AController* Controller)
{
	if (!HasAuthority() || !IsValid(Controller)) return;

	UE_LOG(LogTemp, Log, TEXT("Respawning controller %s"), *Controller->GetName());
	// Clear handle
	RespawnTimerHandles.Remove(Controller);

	// Determine spawn transform (use ChoosePlayerStart)
	AActor* StartSpot = ChoosePlayerStart(Controller);
	FTransform SpawnTransform = StartSpot ? StartSpot->GetActorTransform() : FTransform::Identity;

	AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(Controller);
	TSubclassOf<AShooterCharacter> CharClass = nullptr;
	if (ShooterPC)
	{
		// PlayerController exposes CharacterClass via getter (see PlayerController changes)
		CharClass = ShooterPC->GetRespawnCharacterClass();
	}

	// Fallback to DefaultPawnClass if none provided and if it's compatible
	if (!CharClass)
	{
		if (DefaultPawnClass && DefaultPawnClass->IsChildOf(AShooterCharacter::StaticClass()))
		{
			CharClass = DefaultPawnClass;
		}
	}

	if (!CharClass)
	{
		UE_LOG(LogTemp, Error, TEXT("RespawnController: No Character class available to spawn"));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = nullptr;
	SpawnParams.Instigator = nullptr;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AShooterCharacter* NewPawn = GetWorld()->SpawnActor<AShooterCharacter>(CharClass, SpawnTransform, SpawnParams);
	if (!NewPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("RespawnController: Failed to spawn pawn for controller %s"), *Controller->GetName());
		return;
	}

	// Possess the pawn with the controller
	Controller->Possess(NewPawn);

	// Make sure pawn binds its delegates to the local Controller
	NewPawn->BindPawnBroadcast();

	// Notify the owning client that respawn occurred (client-side effects / UI)
	if (ShooterPC)
	{
		ShooterPC->Client_OnRespawned();
	}

	UE_LOG(LogTemp, Log, TEXT("RespawnController: Respawned controller %s with pawn %s"), *Controller->GetName(), *NewPawn->GetName());
}