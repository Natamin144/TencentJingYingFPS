// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ShooterGameState.generated.h"

UCLASS()
class FPSPROJECT3_API AShooterGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AShooterGameState();

	/** Server: increment the score for a team (call from GameMode) */
	void AddTeamScore(uint8 TeamByte);

	UPROPERTY(EditAnywhere, Category = "Shooter")
	int32 TeamsCount = 2;

protected:
	/** Array of scores by team ID - replicated to clients. Fixed size: 4 teams. */
	UPROPERTY(ReplicatedUsing = OnRep_TeamScores)
	TArray<int32> TeamScores;

	/** Called on clients when TeamScores is updated */
	UFUNCTION()
	void OnRep_TeamScores();

	/** Helper to notify local player controller UI on clients */
	void NotifyLocalPlayerControllers();

public:
	// replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
