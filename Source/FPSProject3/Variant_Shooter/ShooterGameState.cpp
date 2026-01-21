// Fill out your copyright notice in the Description page of Project Settings.

#include "Variant_Shooter/ShooterGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Variant_Shooter/ShooterPlayerController.h"
#include "Variant_Shooter/ShooterGameMode.h"
#include "Engine/Engine.h"

AShooterGameState::AShooterGameState()
{
	bReplicates = true;

	// initialize score array with TeamsCount entries
	TeamScores.Init(0, FMath::Max(1, TeamsCount));
}

void AShooterGameState::AddTeamScore(uint8 TeamByte)
{
	if (!HasAuthority()) return;

	if (TeamByte >= static_cast<uint8>(TeamScores.Num()))
	{
		UE_LOG(LogTemp, Warning, TEXT("AddTeamScore: TeamByte %d out of range (max %d)"), TeamByte, TeamScores.Num());
		return;
	}

	int32& ScoreRef = TeamScores[TeamByte];
	++ScoreRef;

	UE_LOG(LogTemp, Log, TEXT("Team %d scored. New score: %d"), TeamByte, ScoreRef);

	// If we've reached the winning score, notify game over
	if (ScoreRef >= WinningScore)
	{
		NotifyGameOver(TeamByte);
	}
	// replicated TeamScores will invoke OnRep_TeamScores on clients
	// TeamScores is a replicated property; updating it on server will replicate to clients
	// Also update server-local UI immediately so listen-server local player sees the change.
	NotifyLocalPlayerControllers();
}

void AShooterGameState::OnRep_TeamScores()
{
	// Runs on each client when TeamScores changes.
	NotifyLocalPlayerControllers();
}

void AShooterGameState::NotifyLocalPlayerControllers()
{
	// Iterate player controllers available on this machine and tell them to update UI.
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(It->Get()))
		{
			for (int32 Index = 0; Index < TeamScores.Num(); ++Index)
			{
				PC->UpdateLocalTeamScore(static_cast<uint8>(Index), TeamScores[Index]);
			}
		}
	}
}

void AShooterGameState::NotifyGameOver(uint8 WinningTeam)
{
	UE_LOG(LogTemp, Log, TEXT("Game Over! Winning Team: %d"), WinningTeam);
	// Runs on server only: notify all PlayerControllers via client RPC (clients will run UI logic).
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(It->Get()))
		{
			const bool bWin = (PC->PlayerTeamByte == WinningTeam);
			PC->Client_OnGameOver(bWin, WinningTeam);
		}
	}

	// Also invoke GameMode Blueprint event for server-side handling
	if (AShooterGameMode* GM = GetWorld()->GetAuthGameMode<AShooterGameMode>())
	{
		GM->BP_OnGameOver(WinningTeam);
	}
}

void AShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterGameState, TeamScores);
}




