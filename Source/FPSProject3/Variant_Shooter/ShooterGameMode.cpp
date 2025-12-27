// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterGameMode.h"
#include "Variant_Shooter/ShooterGameState.h"
#include "Variant_Shooter/ShooterPlayerController.h"
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

    // Assign start to Player 1
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
    // Assign start to Player 2
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