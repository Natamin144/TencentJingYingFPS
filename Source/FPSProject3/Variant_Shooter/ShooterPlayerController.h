// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/ShooterUI.h"
#include "ShooterPlayerController.generated.h"

class UInputMappingContext;
class AShooterCharacter;
class UShooterBulletCounterUI;

/**
 *  Simple PlayerController for a first person shooter game
 *  Manages input mappings
 *  Respawns the player pawn when it's destroyed
 */
UCLASS(abstract)
class FPSPROJECT3_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input mapping contexts for this player */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** Character class to respawn when the possessed pawn is destroyed */
	UPROPERTY(EditAnywhere, Category="Shooter|Respawn")
	TSubclassOf<AShooterCharacter> CharacterClass;

	/** Type of bullet counter UI widget to spawn */
	UPROPERTY(EditAnywhere, Category="Shooter|UI")
	TSubclassOf<UShooterBulletCounterUI> BulletCounterUIClass;


	/** Pointer to the bullet counter UI widget */
	TObjectPtr<UShooterBulletCounterUI> BulletCounterUI;

	/** Type of global shooter UI widget to spawn (score board etc.) */
	/** Type of UI widget to spawn */
	UPROPERTY(EditAnywhere, Category = "Shooter")
	TSubclassOf<UShooterUI> ShooterUIClass;

	/** Pointer to the global shooter UI widget */
	TObjectPtr<UShooterUI> ShooterUI;

public:

	/** Tag to grant the possessed pawn to flag it as the player */
	UPROPERTY(EditAnywhere, Category = "Shooter|Player")
	FName PlayerPawnTag = FName("Player");
protected:

	/** Gameplay Initialization */
	virtual void BeginPlay() override;

	/** Initialize input bindings */
	virtual void SetupInputComponent() override;

	/** Pawn initialization */
	virtual void OnPossess(APawn* InPawn) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


public:
	/** Called if the possessed pawn is destroyed */
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);
	/** Called when the bullet count on the possessed pawn is updated */
	UFUNCTION()
	void OnBulletCountUpdated(int32 MagazineSize, int32 Bullets);

	/** Called when the possessed pawn is damaged */
	UFUNCTION()
	void OnPawnDamaged(float LifePercent);

	/** Client RPC: server notifies this controller's client to update team score UI */
	UFUNCTION(Client, Reliable)
	void Client_UpdateTeamScore(uint8 TeamByte, int32 Score);

	/** Local helper used by GameState on clients to update the team score UI */
	UFUNCTION()
	void UpdateLocalTeamScore(uint8 TeamByte, int32 Score);

	UPROPERTY(EditAnywhere, Replicated, Category = "Shooter|Player")
	FString CustomPlayerName;
	//在整个网络中的玩家ID
	UPROPERTY(EditAnywhere, Replicated, Category = "Shooter|Player")
	uint16 NetworkPlayerID = 0;
	/** Team ID for this character*/
	UPROPERTY(EditAnywhere, Replicated, Category = "Team")
	uint8 PlayerTeamByte = 0;

	/** Client RPC: notify owning client that they were respawned (server calls after possess) */
	UFUNCTION(Client, Reliable)
	void Client_OnRespawned();

	/** Getter for respawn CharacterClass (used by GameMode when respawning) */
	UFUNCTION()
	TSubclassOf<AShooterCharacter> GetRespawnCharacterClass() const { return CharacterClass; }
};
