// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterCharacter.h"
#include "ShooterWeapon.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "ShooterGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "ShooterPlayerController.h"

AShooterCharacter::AShooterCharacter()
{
	// create the noise emitter component
	PawnNoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Pawn Noise Emitter"));

	// configure movement
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);

	UE_LOG(LogTemp, Log, TEXT("Set ShooterCharacter BReplicates!"));
	bReplicates = true; 
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// reset HP to max
	CurrentHP = MaxHP;

	BindPawnBroadcast();

	// update the HUD
	OnDamaged.Broadcast(1.0f);
	UE_LOG(LogTemp, Warning, TEXT("%s OnDamaged Broadcast in AShooterCharacter::BeginPlay"), *GetName());
}

void AShooterCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the respawn timer
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// base class handles move, aim and jump inputs
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AShooterCharacter::DoStartFiring);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterCharacter::DoStopFiring);

		// Switch weapon
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::DoSwitchWeapon);
	}

}

float AShooterCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 仅服务端处理伤害逻辑（客户端不处理，避免数据不一致）
	if (GetLocalRole() != ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("Client received TakeDamage - ignoring (should be handled by server)"));
		return 0.0f;
	}
	float oldHP = CurrentHP;
	// ignore if already dead
	if (CurrentHP <= 0.0f)
	{
		return 0.0f;
	}

	// Reduce HP
	CurrentHP -= Damage;
	UE_LOG(LogTemp, Log, TEXT("%s TakeDamage: Damage=%f, OldHP=%f, NewHP=%f"),
		*GetName(), Damage, oldHP, CurrentHP);

	// Have we depleted HP?
	if (CurrentHP <= 0.0f)
	{
		Die();
	}

	// update the HUD
	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));
	UE_LOG(LogTemp, Warning, TEXT("%s OnDamaged Broadcast in AShooterCharacter::TakeDamage"), *GetName());

	return Damage;
}

void AShooterCharacter::DoStartFiring()
{
	if (!IsLocallyControlled()) return;
	if (!CurrentWeapon)
	{
		return;
	}

	//Client
	if (GetLocalRole() != ROLE_Authority)
	{
		UE_LOG(LogTemp, Log, TEXT("Client DoStartFiring - Send RPC to server"));
		Server_RequestWeaponFire();
		// To Implement: Client Only firing effects
		//CurrentWeapon->PlayFireFX_ClientOnly();
		return;
	}
	// Server: fire the current weapon
	UE_LOG(LogTemp, Log, TEXT("Server DoStartFiring - Direct fire weapon"));
	CurrentWeapon->StartFiring();
}

void AShooterCharacter::DoStopFiring()
{
	// stop firing the current weapon
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFiring();
	}
}

void AShooterCharacter::DoSwitchWeapon()
{
	// ensure we have at least two weapons two switch between
	if (OwnedWeapons.Num() > 1)
	{
		// deactivate the old weapon
		CurrentWeapon->DeactivateWeapon();

		// find the index of the current weapon in the owned list
		int32 WeaponIndex = OwnedWeapons.Find(CurrentWeapon);

		// is this the last weapon?
		if (WeaponIndex == OwnedWeapons.Num() - 1)
		{
			// loop back to the beginning of the array
			WeaponIndex = 0;
		}
		else {
			// select the next weapon index
			++WeaponIndex;
		}

		// set the new weapon as current
		CurrentWeapon = OwnedWeapons[WeaponIndex];

		// activate the new weapon
		CurrentWeapon->ActivateWeapon();
	}
}

void AShooterCharacter::AttachWeaponMeshes(AShooterWeapon* Weapon)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

	// attach the weapon actor
	Weapon->AttachToActor(this, AttachmentRule);

	// attach the weapon meshes
	Weapon->GetFirstPersonMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);
	Weapon->GetThirdPersonMesh()->AttachToComponent(GetMesh(), AttachmentRule, FirstPersonWeaponSocket);
	
}

void AShooterCharacter::PlayFiringMontage(UAnimMontage* Montage)
{
	
}

void AShooterCharacter::AddWeaponRecoil(float Recoil)
{
	// apply the recoil as pitch input
	AddControllerPitchInput(Recoil);
}

void AShooterCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	// When called on server, the delegate broadcast won't reach the owning client (bindings exist only on client).
	// Forward the update to the owning client when this actor is server-authoritative and not locally controlled.
	if (GetLocalRole() == ROLE_Authority && !IsLocallyControlled())
	{
		// send to owning client
		Client_UpdateWeaponHUD(CurrentAmmo, MagazineSize);
		return;
	}

	// Local client (or listen-server local player) -> broadcast so blueprints bound to OnBulletCountUpdated run.
	OnBulletCountUpdated.Broadcast(MagazineSize, CurrentAmmo);
}

void AShooterCharacter::Client_UpdateWeaponHUD_Implementation(int32 CurrentAmmo, int32 MagazineSize)
{
	// Running on owning client: broadcast to trigger Blueprint HUD update bound to the delegate.
	OnBulletCountUpdated.Broadcast(MagazineSize, CurrentAmmo);
}

FVector AShooterCharacter::GetWeaponTargetLocation()
{
	// trace ahead from the camera viewpoint
	FHitResult OutHit;

	const FVector Start = GetFirstPersonCameraComponent()->GetComponentLocation();
	const FVector End = Start + (GetFirstPersonCameraComponent()->GetForwardVector() * MaxAimDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);

	// return either the impact point or the trace end
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}

void AShooterCharacter::AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass)
{
	// do we already own this weapon?
	AShooterWeapon* OwnedWeapon = FindWeaponOfType(WeaponClass);

	if (!OwnedWeapon)
	{
		// spawn the new weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

		AShooterWeapon* AddedWeapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);

		if (AddedWeapon)
		{
			// add the weapon to the owned list
			OwnedWeapons.Add(AddedWeapon);

			// if we have an existing weapon, deactivate it
			if (CurrentWeapon)
			{
				CurrentWeapon->DeactivateWeapon();
			}

			// switch to the new weapon
			CurrentWeapon = AddedWeapon;
			CurrentWeapon->ActivateWeapon();
		}
	}
}

void AShooterCharacter::OnWeaponActivated(AShooterWeapon* Weapon)
{
	UE_LOG(LogTemp, Log, TEXT("%s 激活武器: %s"), *GetName(), *Weapon->GetName());
	// update the bullet counter
	OnBulletCountUpdated.Broadcast(Weapon->GetMagazineSize(), Weapon->GetBulletCount());

	// set the character mesh AnimInstances
	GetFirstPersonMesh()->SetAnimInstanceClass(Weapon->GetFirstPersonAnimInstanceClass());
	GetMesh()->SetAnimInstanceClass(Weapon->GetThirdPersonAnimInstanceClass());
}

void AShooterCharacter::OnWeaponDeactivated(AShooterWeapon* Weapon)
{
	// unused
}

void AShooterCharacter::OnSemiWeaponRefire()
{
	// unused
}

AShooterWeapon* AShooterCharacter::FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const
{
	// check each owned weapon
	for (AShooterWeapon* Weapon : OwnedWeapons)
	{
		if (Weapon->IsA(WeaponClass))
		{
			return Weapon;
		}
	}

	// weapon not found
	return nullptr;

}

void AShooterCharacter::Die()
{
	if (!HasAuthority()) {
		UE_LOG(LogTemp, Error, TEXT("Client attempted to call Die() - ignoring (should be handled by server)"));
		return;
	}
	Die_Local();
	Multicast_NotifyDie();
	// increment the team score
	if (AShooterGameMode* GM = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->IncrementTeamScore(GetTeamByte());
	}
	// schedule character respawn
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AShooterCharacter::OnRespawn, RespawnTime, false);
}

void AShooterCharacter::Die_Local()
{
	// deactivate the weapon
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->DeactivateWeapon();
	}
	// stop character movement
	GetCharacterMovement()->StopMovementImmediately();
	// disable controls
	DisableInput(nullptr);
	// reset the bullet counter UI
	OnBulletCountUpdated.Broadcast(0, 0);
	// call the BP handler
	BP_OnDeath();
}

void AShooterCharacter::Multicast_NotifyDie_Implementation()
{
	Die_Local(); // 客户端执行本地死亡逻辑
}

void AShooterCharacter::OnRespawn()
{
	// destroy the character to force the PC to respawn
	Destroy();
}

/*void AShooterCharacter::Server_RequestWeaponFire()
{
	UE_LOG(LogTemp, Warning, TEXT("Client RPC - Request weapon fire on server. Function Not Implemented."));
}*/

bool AShooterCharacter::Server_RequestWeaponFire_Validate()
{
	return CurrentWeapon != nullptr;
}

void AShooterCharacter::Server_RequestWeaponFire_Implementation()
{
	if (!HasAuthority() || !CurrentWeapon) return;

	UE_LOG(LogTemp, Warning, TEXT("Server RPC - Trigger weapon fire"));
	CurrentWeapon->StartFiring();
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterCharacter, CurrentHP);
}

void AShooterCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

void AShooterCharacter::OnHealthUpdate()
{
	UE_LOG(LogTemp, Log, TEXT("%s OnRep_CurrentHealth: NewHP=%f"), *GetName(), CurrentHP);
	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));
	UE_LOG(LogTemp, Warning, TEXT("%s OnDamaged Broadcast in AShooterCharacter::OnHealthUpdate"), *GetName());
	//Client
	if (IsLocallyControlled())
	{
		/*FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), CurrentHP);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

		if (CurrentHP <= 0)
		{
			FString deathMessage = FString::Printf(TEXT("You have been killed."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
		}*/
	}

	//Server
	if (GetLocalRole() == ROLE_Authority)
	{
		/*FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHP);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);*/
	}

	//在所有机器上都执行的函数。
	/*
		因任何因伤害或死亡而产生的特殊功能都应放在这里。
	*/
}

// 服务器 RPC 实现：仅服务端执行，用于触发多播
void AShooterCharacter::ServerNotifyPickUpWeapon_Implementation(AShooterPickup* Pickup)
{
	Pickup->GivePickupToHolder(this);
	// 广播到所有客户端执行
	MulticastPickUpWeapon(Pickup);
}

bool AShooterCharacter::ServerNotifyPickUpWeapon_Validate(AShooterPickup* Pickup)
{
	return Pickup != nullptr; // 简单验证：武器类不为空
}

// 多播 RPC 实现：所有客户端执行添加武器逻辑
void AShooterCharacter::MulticastPickUpWeapon_Implementation(AShooterPickup* Pickup)
{
	// 客户端执行添加武器（本地角色视角同步）
	//UE_LOG(LogTemp, Log, TEXT("MulticastPickUpWeapon - 通过网络传播，玩家获得武器: %s"), *Pickup->GetName());
	Pickup->GivePickupToHolder(this);
}

bool AShooterCharacter::MulticastPickUpWeapon_Validate(AShooterPickup* Pickup)
{
	return Pickup != nullptr;
}

void AShooterCharacter::BindPawnBroadcast()
{
	// 注册一些广播事件
	// 仅在客户端执行（本地角色）
	if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetController()))
	{
		if (PC->IsLocalPlayerController())
		{
			// 绑定子弹更新事件到 PlayerController 的处理函数
			// 注意：AddDynamic 的第一个参数必须是实现回调函数的对象实例（这里是 PC）
			OnBulletCountUpdated.AddDynamic(PC, &AShooterPlayerController::OnBulletCountUpdated);
			OnDamaged.AddDynamic(PC, &AShooterPlayerController::OnPawnDamaged);
			OnDestroyed.AddDynamic(PC, &AShooterPlayerController::OnPawnDestroyed);
			Tags.Add(PC->PlayerPawnTag);
		}
	}
}

uint8 AShooterCharacter::GetTeamByte() const
{
	// Try to get team from PlayerState
	if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetController()))
	{
		return PC->PlayerTeamByte;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s GetTeamByte: No PlayerController found, defaulting to team 0"), *GetName());
	}
	return 0; // Default team
}