// Fill out your copyright notice in the Description page of Project Settings.

#include "Variant_Shooter/ShooterGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Variant_Shooter/ShooterPlayerController.h"
#include "Engine/Engine.h"

AShooterGameState::AShooterGameState()
{
	bReplicates = true;

	TeamScores.Init(0, TeamsCount);
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

	// TeamScores is a replicated property; updating it on server will replicate to clients
	// Optionally force OnRep on server for server-side UI by calling NotifyLocalPlayerControllers() here if needed.
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

void AShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterGameState, TeamScores);
}




