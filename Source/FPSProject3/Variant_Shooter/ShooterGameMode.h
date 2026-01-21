// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

class UShooterUI;

/**
 *  Simple GameMode for a first person shooter game
 *  Manages game UI
 *  Keeps track of team scores
 */
UCLASS(abstract)
class FPSPROJECT3_API AShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	/** Respawn timer handles keyed by controller */
	TMap<TWeakObjectPtr<AController>, FTimerHandle> RespawnTimerHandles;

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

public:

	/** Increases the score for the given team */
	void IncrementTeamScore(uint8 TeamByte);
	AActor* ChoosePlayerStart(AController* Player);

	/** Schedule a respawn for this controller after Delay seconds (server only) */
	void ScheduleRespawn(AController* Controller, float Delay);
	/** Blueprint event called on server when a team wins */

	UFUNCTION(BlueprintImplementableEvent, Category = "Shooter", meta = (DisplayName = "On Game Over"))
	void BP_OnGameOver(uint8 WinningTeam);

protected:
	/** Called by timer to actually respawn controller's pawn */
	void RespawnController(AController* Controller);
};
