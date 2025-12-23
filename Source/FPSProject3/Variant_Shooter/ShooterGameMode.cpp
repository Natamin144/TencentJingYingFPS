// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterGameMode.h"
#include "ShooterUI.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();
    UE_LOG(LogGameMode, Warning, TEXT("AShooterGameMode::BeginPlay - GameMode loaded, Authority: %d"), HasAuthority());
	//DefaultPawnClass = AShooterCharacter::StaticClass();
	// create the UI
	ShooterUI = CreateWidget<UShooterUI>(UGameplayStatics::GetPlayerController(GetWorld(), 0), ShooterUIClass);
	ShooterUI -> AddToViewport(0);
}

AActor* AShooterGameMode::ChoosePlayerStart(AController* PlayerController)
{

    if (!HasAuthority())
    {
        UE_LOG(LogGameMode, Warning, TEXT("ChoosePlayerStart - Client side, skip execution"));
        return Super::ChoosePlayerStart_Implementation(PlayerController);
    }

    UE_LOG(LogTemp, Log, TEXT("Choose PlayerStart"));
    // 2. Validate Player controller
    if (!IsValid(PlayerController))
    {
        UE_LOG(LogGameMode, Warning, TEXT("ChoosePlayerStart - Player controller is invalid"));
        return Super::ChoosePlayerStart_Implementation(PlayerController);
    }

    // 3. Log with player ID (ensure log output)
    int32 PlayerId = PlayerController->PlayerState ? PlayerController->PlayerState->GetPlayerId() : -1;
    UE_LOG(LogGameMode, Log, TEXT("ChoosePlayerStart - Server execute, Player ID: %d"), PlayerId);

    // 4. Original logic (English comment only)
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

    if (PlayerStarts.Num() == 0)
    {
        UE_LOG(LogGameMode, Warning, TEXT("ChoosePlayerStart - No PlayerStart found, use super logic"));
        return Super::ChoosePlayerStart_Implementation(PlayerController);
    }

    // Assign start by player ID
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

    // Fallback to first PlayerStart
    return PlayerStarts[0];
}

void AShooterGameMode::IncrementTeamScore(uint8 TeamByte)
{
	// retrieve the team score if any
	int32 Score = 0;
	if (int32* FoundScore = TeamScores.Find(TeamByte))
	{
		Score = *FoundScore;
	}

	// increment the score for the given team
	++Score;
	TeamScores.Add(TeamByte, Score);

	// update the UI
	ShooterUI->BP_UpdateScore(TeamByte, Score);
}

/*int AShooterGameMode::RestartPlayer_Implementation(AController* NewPlayerController)
{
    Super::RestartPlayer(NewPlayerController);

    APlayerState* PlayerState = NewPlayerController->PlayerState;
    if (PlayerState)
    {
        if (PlayerState->GetPlayerId() == 0)
        {
            PlayerState->SetPlayerName("Player1");
        }
        else
        {
            PlayerState->SetPlayerName("Player2");
        }
    }
}*/