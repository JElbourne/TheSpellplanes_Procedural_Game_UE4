// Fill out your copyright notice in the Description page of Project Settings.


#include "SpellPlanesCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Player/SpellPlanesPlayerController.h"
#include "Components/InteractionComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Items/EquipableItem.h"
#include "Items/GearItem.h"
#include "Items/ThrowableItem.h"
#include "Materials/MaterialInstance.h"
#include "World/Pickup.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "TheSpellplanes.h"
#include "Weapons/MeleeDamage.h"
#include "Camera/SpellPlanesCamera.h"
#include "Weapons/ThrowableWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/SpellPlanesSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "World/WorldUtilities.h"


#define LOCTEXT_NAMESPACE "SpellPlanesCharacter"

// Sets default values
ASpellPlanesCharacter::ASpellPlanesCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup Equippable Meshes
	HairMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Hair, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HairMesh")));
	EarMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Ears, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EarsMesh")));
	ChestMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Chest, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ChestMesh")));
	HandsMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Hands, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandsMesh")));
	LegsMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Legs, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh")));
	FeetMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Feet, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FeetMesh")));
	TailMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Tail, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TailMesh")));

	for (auto& PlayerMesh : PlayerMeshes)
	{
		USkeletalMeshComponent* MeshComponent = PlayerMesh.Value;
		MeshComponent->SetupAttachment(GetMesh());
		MeshComponent->SetMasterPoseComponent(GetMesh());
	}

	PlayerMeshes.Add(EEquippableSlot::EIS_Head, GetMesh());

	// For Interaction - How often we use a sphere trace to look at our surroundings
	InteractionCheckFrequency = 0.25f;
	InteractionCheckDistance = 64.f;

	// Give the player an inventory with 20 Slots, and an 80Kg capacity
	PlayerInventory = CreateDefaultSubobject<UInventoryComponent>("PlayerInventory");
	PlayerInventory->SetSlotCapacity(20);
	PlayerInventory->SetWeightCapacity(80.f);

	// For LootPlayerInteraction
	LootPlayerInteraction = CreateDefaultSubobject<UInteractionComponent>("PlayerInteraction");
	LootPlayerInteraction->InteractableActionText = LOCTEXT("LootPlayerText", "Loot");
	LootPlayerInteraction->InteractableNameText = LOCTEXT("LootPlayerName", "Player");
	LootPlayerInteraction->SetupAttachment(GetRootComponent());
	LootPlayerInteraction->SetActive(false, true);
	LootPlayerInteraction->bAutoActivate = false;

	// Player Health
	Health = 100.f;
	MaxHealth = 100.f;

	// Move Speed
	SprintSpeed = GetCharacterMovement()->MaxWalkSpeed * 1.3f;
	WalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	// Melee Attack Values
	MeleeAttackDistance = 150.f;
	MeleeAttackDamage = 20.f;

}

// Called when the game starts or when spawned
void ASpellPlanesCharacter::BeginPlay()
{
	Super::BeginPlay();

	LootPlayerInteraction->OnInteract.AddDynamic(this, &ASpellPlanesCharacter::BeginLootingPlayer);

	// Try to display the players platform name on their loot card
	if (APlayerState* SPlayerState = GetPlayerState())
	{
		LootPlayerInteraction->SetInteractableNameText(FText::FromString(SPlayerState->GetPlayerName()));
	}


	// When the player spawns in they have no items equipped, so cache these items (That way, if a player unequips an item we can set the mesh back to the naked character)
	for (const TPair<EEquippableSlot, USkeletalMeshComponent*>& MapPair : PlayerMeshes)
	{
		if (!NakedMeshes.Contains(MapPair.Key))
		{
			UE_LOG(LogTemp, Error, TEXT("Missing a Naked Mesh Setup for, %d, on the SpellPlanesCharacter."), MapPair.Key);
		}
	}

	// Setup a reference to the PlayerController
	SPlayerController = Cast<ASpellPlanesPlayerController>(GetController());

}

void ASpellPlanesCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpellPlanesCharacter, bSprinting);
	DOREPLIFETIME(ASpellPlanesCharacter, LootSource);
	DOREPLIFETIME(ASpellPlanesCharacter, Killer);

	DOREPLIFETIME_CONDITION(ASpellPlanesCharacter, Health, COND_OwnerOnly); // Other Players can not see effects caused by health.
}

bool ASpellPlanesCharacter::IsInteracting() const
{
	return GetWorldTimerManager().IsTimerActive(TimerHandle_Interact);
}

float ASpellPlanesCharacter::GetRemainingInteractTime() const
{
	return GetWorldTimerManager().GetTimerRemaining(TimerHandle_Interact);
}

void ASpellPlanesCharacter::UseItem(class UItem* Item)
{
	// Make sure this is the server calling the method.
	if (!HasAuthority() && Item)
	{
		ServerUseItem(Item);
	}

	// Make sure the player isn't cheating and make sure it is in their inventory.
	if (HasAuthority())
	{
		if (PlayerInventory && !PlayerInventory->FindItem(Item))
		{
			return;
		}
	}

	// Now we can use the item using it's own implementation of USE
	// Both Server and Client will hit this part
	if (Item)
	{
		Item->OnUse(this);
		Item->Use(this);
	}
}

void ASpellPlanesCharacter::ServerUseItem_Implementation(class UItem* Item)
{
	UseItem(Item);
}

bool ASpellPlanesCharacter::ServerUseItem_Validate(class UItem* Item)
{
	return true;
}

void ASpellPlanesCharacter::DropItem(class UItem* Item, const int32 Quantity)
{
	if (PlayerInventory && Item && PlayerInventory->FindItem(Item))
	{
		// Make sure this is the server calling the method.
		if (!HasAuthority())
		{
			ServerDropItem(Item, Quantity);
			return;
		}

		if (HasAuthority())
		{
			const int32 ItemQuantity = Item->GetQuantity();
			const int32 DropQuantity = PlayerInventory->ConsumeItem(Item, Quantity);

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.bNoFail = true;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			FVector SpawnLocation = GetActorLocation();
			SpawnLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

			FTransform SpawnTransform(GetActorRotation(), SpawnLocation);

			ensure(PickupClass);

			APickup* Pickup = GetWorld()->SpawnActor<APickup>(PickupClass, SpawnTransform, SpawnParams);
			Pickup->InitializePickup(Item->GetClass(), DropQuantity);
		}
	}
}

void ASpellPlanesCharacter::ServerDropItem_Implementation(class UItem* Item, const int32 Quantity)
{
	DropItem(Item, Quantity);
}

bool ASpellPlanesCharacter::ServerDropItem_Validate(class UItem* Item, const int32 Quantity)
{
	return true;
}

bool ASpellPlanesCharacter::EquipItem(class UEquipableItem* Item)
{
	EquippedItems.Add(Item->Slot, Item);
	OnEquippedItemsChanged.Broadcast(Item->Slot, Item);
	return true;
}

bool ASpellPlanesCharacter::UnEquipItem(class UEquipableItem* Item)
{
	if (Item)
	{
		if (EquippedItems.Contains(Item->Slot))
		{
			if (Item == *EquippedItems.Find(Item->Slot))
			{
				EquippedItems.Remove(Item->Slot);
				OnEquippedItemsChanged.Broadcast(Item->Slot, nullptr);
				return true;
			}
		}
	}
	return false;
}

void ASpellPlanesCharacter::EquipGear(class UGearItem* Gear)
{
	OnEquippedGearChanged.Broadcast(Gear->Slot, Gear);
	UE_LOG(LogTemp, Warning, TEXT("Skeletal Mesh has been Equiped."));
	if (USkeletalMeshComponent* GearMesh = GetSlotSkeletalMeshComponent(Gear->Slot))
	{
		//USkeletalMesh* BodyMesh = *NakedMeshes.Find(Gear->Slot);
		
		
		//GearMesh->SetSkeletalMesh(Gear->Mesh);
		//GearMesh->SetMaterial(GearMesh->GetMaterials().Num() - 1, Gear->MaterialInstance);
		
	}
}

void ASpellPlanesCharacter::UnEquipGear(const EEquippableSlot Slot)
{
	if (USkeletalMeshComponent* EquippableMesh = GetSlotSkeletalMeshComponent(Slot))
	{
		if (NakedMeshes.Contains(Slot))
		{
			USkeletalMesh* BodyMesh = *NakedMeshes.Find(Slot);
			EquippableMesh->SetSkeletalMesh(BodyMesh);
			UE_LOG(LogTemp, Warning, TEXT("Skeletal Mesh has been UnEquiped."));
			//Put the materials back on the body mesh (Since gear may have applied a different material)
			for (int32 i = 0; i < BodyMesh->Materials.Num(); ++i)
			{
				if (BodyMesh->Materials.IsValidIndex(i))
				{
					EquippableMesh->SetMaterial(i, BodyMesh->Materials[i].MaterialInterface);
				}
			}
		}
		else
		{
			//For some gear like backpacks, there is no naked mesh
			EquippableMesh->SetSkeletalMesh(nullptr);
		}
	}
}

class USkeletalMeshComponent* ASpellPlanesCharacter::GetSlotSkeletalMeshComponent(const EEquippableSlot Slot)
{
	if (PlayerMeshes.Contains(Slot))
	{
		return *PlayerMeshes.Find(Slot);
	}
	return nullptr;
}

void ASpellPlanesCharacter::ServerUseThrowable_Implementation()
{
	UseThrowable();
}

bool ASpellPlanesCharacter::ServerUseThrowable_Validate()
{
	return true;
}

void ASpellPlanesCharacter::MulticastPlayThrowableTossFX_Implementation(UAnimMontage* MontageToPlay)
{
	/** Local player already instantly played grenade throw animation */
	if (GetNetMode() != NM_DedicatedServer && !IsLocallyControlled())
	{
		PlayAnimMontage(MontageToPlay);
	}
}

class UThrowableItem* ASpellPlanesCharacter::GetThrowable() const
{
	UThrowableItem* EquippedThrowable = nullptr;

	if (EquippedItems.Contains(EEquippableSlot::EIS_Throwable))
	{
		EquippedThrowable = Cast<UThrowableItem>(*EquippedItems.Find(EEquippableSlot::EIS_Throwable));
	}

	return EquippedThrowable;
}

void ASpellPlanesCharacter::UseThrowable()
{
	if (CanUseThrowable())
	{
		if (UThrowableItem* Throwable = GetThrowable())
		{
			if (HasAuthority())
			{
				SpawnThrowable();

				if (PlayerInventory)
				{
					PlayerInventory->ConsumeItem(Throwable, 1);
				}
			}
			else
			{
				// If this is the last Throwable Item we must update the UI before we hand off to the Server to destroy item from inventory.
				if (Throwable->GetQuantity() <= 1)
				{
					EquippedItems.Remove(EEquippableSlot::EIS_Throwable);
					OnEquippedItemsChanged.Broadcast(EEquippableSlot::EIS_Throwable, nullptr);
				}

				// Locally play grenade throw instantly - by the time server spawns the grenade in the throw animation should roughly sync up.
				PlayAnimMontage(Throwable->ThrowableTossAnimation);
				ServerUseThrowable();
			}
		}
	}
}

void ASpellPlanesCharacter::SpawnThrowable()
{
	if (HasAuthority())
	{
		if (UThrowableItem* CurrentThrowable = GetThrowable())
		{
			if (CurrentThrowable->ThrowableClass)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = SpawnParams.Instigator = this;
				SpawnParams.bNoFail = true;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				// TODO: For throw-ables there should be a grid system to pick location to throw toward.
				FVector EyesLoc = GetActorLocation();
				FRotator EyesRot = GetActorRotation();

				// Spawn throw-able slightly in front of our face so it doest collide with our player.
				EyesLoc = (EyesLoc.ForwardVector * 20.f) + EyesLoc;

				if (AThrowableWeapon* ThrowableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(CurrentThrowable->ThrowableClass, FTransform(EyesRot, EyesLoc)))
				{
					MulticastPlayThrowableTossFX(CurrentThrowable->ThrowableTossAnimation);
				}
			}
		}
	}
}

bool ASpellPlanesCharacter::CanUseThrowable() const
{
	return GetThrowable() != nullptr && GetThrowable()->ThrowableClass != nullptr;
}

float ASpellPlanesCharacter::ModifyHealth(const float Delta)
{
	const float OldHealth = Health;

	Health = FMath::Clamp<float>(Health + Delta, 0.f, MaxHealth);

	return Health - OldHealth;
}

void ASpellPlanesCharacter::OnRep_Health(float OldHealth)
{
	OnHealthModified(Health - OldHealth);
}

void ASpellPlanesCharacter::StartFire()
{
	BeginMeleeAttack();
}

void ASpellPlanesCharacter::StopFire()
{

}

void ASpellPlanesCharacter::BeginMeleeAttack()
{
	float AttackTimeLength = MeleeAttackMontage ? MeleeAttackMontage->GetPlayLength() : 1.f;
	if (GetWorld()->TimeSince(LastMeleeAttackTime) > AttackTimeLength)
	{
		UE_LOG(LogTemp, Warning, TEXT("Punched"));
		FHitResult Hit;
		FCollisionShape Shape = FCollisionShape::MakeSphere(32.f);

		FVector StartTrace = GetActorLocation();
		FVector EndTrace = (GetActorRotation().Vector() * MeleeAttackDistance) + StartTrace;

		FCollisionQueryParams QueryParams = FCollisionQueryParams("MeleeSweep", false, this);

		if (MeleeAttackMontage)
			PlayAnimMontage(MeleeAttackMontage);
		DrawDebugSphere(GetWorld(), StartTrace, 32, 6, FColor::Blue, false, 1.f);
		DrawDebugSphere(GetWorld(), EndTrace, 32, 6, FColor::Blue, false, 1.f);
		if (GetWorld()->SweepSingleByChannel(Hit, StartTrace, EndTrace, FQuat::Identity, COLLISION_WEAPON, Shape, QueryParams))
		{
			UE_LOG(LogTemp, Warning, TEXT("We actually Punched Something!!!"));
			
			if (ASpellPlanesCharacter* HitPlayer = Cast<ASpellPlanesCharacter>(Hit.GetActor()))
			{
				if (SPlayerController)
				{
					SPlayerController->OnHitPlayer();
				}
			}
		}

		ServerProcessMeleeHit(Hit);

		LastMeleeAttackTime = GetWorld()->GetTimeSeconds();
	}
}

void ASpellPlanesCharacter::ServerProcessMeleeHit_Implementation(const FHitResult& MeleeHit)
{
	float AttackTimeLength = MeleeAttackMontage ? MeleeAttackMontage->GetPlayLength() : 1.f;
	if (GetWorld()->TimeSince(LastMeleeAttackTime) > AttackTimeLength && (GetActorLocation() - MeleeHit.ImpactPoint).Size() <= MeleeAttackDistance)
	{
		MulticastPlayMeleeFX();

		UGameplayStatics::ApplyPointDamage(MeleeHit.GetActor(), MeleeAttackDamage, (MeleeHit.TraceStart - MeleeHit.TraceEnd).GetSafeNormal(), MeleeHit, GetController(), this, UMeleeDamage::StaticClass());
	}

	LastMeleeAttackTime = GetWorld()->GetTimeSeconds();
}

void ASpellPlanesCharacter::MulticastPlayMeleeFX_Implementation()
{
	if (!IsLocallyControlled() && MeleeAttackMontage)
	{
		PlayAnimMontage(MeleeAttackMontage);
	}
}

void ASpellPlanesCharacter::Suicide(struct FDamageEvent const& DamageEvent, const AActor* DamageCauser)
{
	Killer = this;
	OnRep_Killer();
}

void ASpellPlanesCharacter::KilledByPlayer(struct FDamageEvent const& DamageEvent, class ASpellPlanesCharacter* Character, const AActor* DamageCauser)
{
	Killer = Character;
	OnRep_Killer();
}

void ASpellPlanesCharacter::OnRep_Killer()
{
	SetLifeSpan(20.f);

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	SetReplicateMovement(false);

	TurnOff();

	LootPlayerInteraction->Activate();

	// Un-equip all equipped item so they can be looted from the inventory.
	if (HasAuthority())
	{
		TArray<UEquipableItem*> Equippables;
		EquippedItems.GenerateValueArray(Equippables);

		for (auto& EquippedItem : Equippables)
		{
			EquippedItem->SetEquipped(false);
		}
	}

	if (IsLocallyControlled())
	{
		if (SPlayerController)
		{
			SPlayerController->ShowDeathScreen(Killer);
		}
	}
}

void ASpellPlanesCharacter::MoveForward(float Val)
{
	if (Val != 0.f) {
		FRotator CtrlRotation = GetControlRotation();
		FRotator OrthoAdjustedRotation = FRotator(0.f, (CtrlRotation.Yaw + 45.f), 0.f);
		FVector ForwardVector = FRotationMatrix(OrthoAdjustedRotation).GetScaledAxis(EAxis::X);
		FVector MoveDirection = ModifyMoveDirection(ForwardVector);
		AddMovementInput(MoveDirection, Val);
	}
}

void ASpellPlanesCharacter::MoveRight(float Val)
{
	if (Val != 0.f) {
		FRotator CtrlRotation = GetControlRotation();
		FRotator OrthoAdjustedRotation = FRotator(0.f, (CtrlRotation.Yaw + 45.f), 0.f);
		FVector RightVector = FRotationMatrix(OrthoAdjustedRotation).GetScaledAxis(EAxis::Y);
		FVector MoveDirection = ModifyMoveDirection(RightVector);
		AddMovementInput(MoveDirection, Val);
	}
}

FVector ASpellPlanesCharacter::ModifyMoveDirection(FVector Dir)
{
	// When the Camera Rotates we want the Player to still move character UP on screen, etc.
	if (SPlayerController)
	{	
		float Yaw = abs(SPlayerController->GetCameraYaw());

		if (Yaw == 90.f) {
			// Swap the X and Y with some negative action on Y
			return FVector((Dir.Y * -1.f), Dir.X, Dir.Z);
		}
		else if (Yaw == 180.f) {
			return FVector((Dir.X * -1.f), (Dir.Y * -1.f), Dir.Z);
		}
		else if (Yaw == 270.f) {
			// Swap the X and Y with some negative action on X
			return FVector(Dir.Y, (Dir.X * -1.f), Dir.Z);
		}
	}
	return Dir;
}


// Called every frame
void ASpellPlanesCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const bool bIsInteractingOnServer = (HasAuthority() && IsInteracting());
	

	if ((!HasAuthority() || bIsInteractingOnServer) && GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFrequency)
	{
		PerformInteractionCheck();
	}

	if (SPlayerController && !SPlayerController->IsTransitioningZones())
	{
		// Check for world edge
		// TODO redo this implementation as I already dont think I want it this way.
		// This is for initial prototyping of the concept.
		FVector CameraLocation = SPlayerController->GetCameraLocation();
		FVector PlayerLocation = GetActorLocation();
		int32 MaxTravelDistance = (UWorldUtilities::ZONE_SIZE / 2);
		int32 CurrentDistanceX = PlayerLocation.X - CameraLocation.X;
		int32 CurrentDistanceY = PlayerLocation.Y - CameraLocation.Y;

		if (abs(CurrentDistanceX) > MaxTravelDistance)
		{
			if (CurrentDistanceX > 0)
			{
				SPlayerController->TransitionZones(FVector2D(1, 0));
			}
			else
			{
				SPlayerController->TransitionZones(FVector2D(-1, 0));
			}
			
		}
		else if (abs(CurrentDistanceY) > MaxTravelDistance)
		{
			if (CurrentDistanceY > 0)
			{
				SPlayerController->TransitionZones(FVector2D(0, 1));
			}
			else
			{
				SPlayerController->TransitionZones(FVector2D(0, -1));
			}
		}
	}

}

void ASpellPlanesCharacter::Restart()
{
	Super::Restart();

	if (SPlayerController)
	{
		SPlayerController->ShowIngameUI();
	}
}

float ASpellPlanesCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	const float DamageDelt = ModifyHealth(-Damage);

	if (Health <= 0.f)
	{
		if (ASpellPlanesCharacter* KillerCharacter = Cast<ASpellPlanesCharacter>(DamageCauser->GetOwner()))
		{
			KilledByPlayer(DamageEvent, KillerCharacter, DamageCauser);
		}
		else
		{
			Suicide(DamageEvent, DamageCauser);
		}
	}

	return DamageDelt;
}

void ASpellPlanesCharacter::SaveGame()
{
	//// Create instance of the save game class
	//USpellPlanesSaveGame* SaveGameInstance = Cast<USpellPlanesSaveGame>(UGameplayStatics::CreateSaveGameObject(USpellPlanesSaveGame::StaticClass()));
	//
	//// Set the save game data
	////SaveGameInstance->PlayerData.PlayerLocation = this->GetActorLocation();
	////Save the save game instance
	//UGameplayStatics::SaveGameToSlot(SaveGameInstance, TEXT("MySlot"), 0);

	//FVector pos = this->GetActorLocation();
	//UE_LOG(LogTemp, Warning, TEXT("Game Saved: x: %f y: %f z: %f"), pos.X, pos.Y, pos.Z);

}

void ASpellPlanesCharacter::LoadGame()
{	
	//// Create instance of the save game class
	//USpellPlanesSaveGame* SaveGameInstance = Cast<USpellPlanesSaveGame>(UGameplayStatics::CreateSaveGameObject(USpellPlanesSaveGame::StaticClass()));
	//SaveGameInstance = Cast<USpellPlanesSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("MySlot"), 0));
	//FVector pos = SaveGameInstance->PlayerData.PlayerLocation;
	//ServerSetLovation(pos);

	//
	//UE_LOG(LogTemp, Warning, TEXT("Game Loaded: x: %f y: %f z: %f"), pos.X, pos.Y, pos.Z);
}

void ASpellPlanesCharacter::ServerSetLovation_Implementation(FVector pos)
{
	this->SetActorLocation(pos, false, 0, ETeleportType::ResetPhysics);
}

void ASpellPlanesCharacter::SetLootSource(class UInventoryComponent* NewLootSource)
{

	/** If the item we are looting gets destroyed, we need to tell the client to remove their Loot screen. */
	if (NewLootSource && NewLootSource->GetOwner())
	{
		NewLootSource->GetOwner()->OnDestroyed.AddUniqueDynamic(this, &ASpellPlanesCharacter::OnLootSourceOwnerDestroyed);
	}


	if (HasAuthority())
	{
		if (NewLootSource)
		{
			// Looting a player keeps their body alive for an extra 2 minutes to provide enough time to loot their items
			if (ASpellPlanesCharacter* Character = Cast<ASpellPlanesCharacter>(NewLootSource->GetOwner()))
			{
				Character->SetLifeSpan(120.f);
			}
		}

		LootSource = NewLootSource;
	}
	else
	{
		ServerSetLootSource(NewLootSource);
	}
}

bool ASpellPlanesCharacter::IsLooting() const
{
	return LootSource != nullptr;
}

void ASpellPlanesCharacter::BeginLootingPlayer(class ASpellPlanesCharacter* Character)
{
	if (Character)
	{
		Character->SetLootSource(PlayerInventory);
	}
}

void ASpellPlanesCharacter::ServerSetLootSource_Implementation(class UInventoryComponent* NewLootSource)
{
	SetLootSource(NewLootSource);
}

bool ASpellPlanesCharacter::ServerSetLootSource_Validate(class UInventoryComponent* NewLootSource)
{
	return true;
}

void ASpellPlanesCharacter::OnLootSourceOwnerDestroyed(AActor* DestroyedActor)
{
	// Remove loot source
	if (HasAuthority() && LootSource && DestroyedActor == LootSource->GetOwner())
	{
		ServerSetLootSource(nullptr);
	}
}

void ASpellPlanesCharacter::OnRep_LootSource()
{
	// Bring up or remove the looting menu
	if (SPlayerController)
	{
		if (SPlayerController->IsLocalController())
		{
			if (LootSource)
			{
				SPlayerController->ShowLootMenu(LootSource);
			}
			else
			{
				SPlayerController->HideLootMenu();
			}
		}
	}
}

void ASpellPlanesCharacter::LootItem(class UItem* ItemToLoot)
{
	if (HasAuthority())
	{
		if (PlayerInventory && LootSource && ItemToLoot && LootSource->HasItem(ItemToLoot->GetClass(), ItemToLoot->GetQuantity()))
		{
			const FItemAddResult AddResult = PlayerInventory->TryAddItem(ItemToLoot);

			if (AddResult.ActualAmountGiven > 0)
			{
				LootSource->ConsumeItem(ItemToLoot, AddResult.ActualAmountGiven);
			}
			else
			{
				// Tell the player why they couldn't loot the item
				if (SPlayerController)
				{
					SPlayerController->ClientShowNotification(AddResult.ErrorText);
				}
			}
		}
	}
	else {
		ServerLootItem(ItemToLoot);
	}
}

void ASpellPlanesCharacter::ServerLootItem_Implementation(class UItem* ItemToLoot)
{
	LootItem(ItemToLoot);
}

bool ASpellPlanesCharacter::ServerLootItem_Validate(class UItem* ItemToLoot)
{
	return true;
}

void ASpellPlanesCharacter::PerformInteractionCheck()
{
	
	if (GetController() == nullptr)
	{
		return;
	}

	InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

	FRotator EyesRot = GetActorRotation();
	FVector EyesLoc = GetActorLocation();

	FVector TraceStart = EyesLoc;
	//FVector TraceEnd = (EyesRot.Vector() * InteractionCheckDistance) + TraceStart;
	FVector TraceEnd = (EyesRot.Vector() * InteractionCheckDistance) + TraceStart;
	FQuat TraceRot = FQuat::Identity;
	FCollisionShape TraceShape = FCollisionShape::MakeSphere(32.f);
	FHitResult TraceHit;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bDebugQuery = true;

	if (GetWorld()->SweepSingleByChannel(TraceHit, TraceEnd, TraceEnd, TraceRot, ECC_Visibility, TraceShape, QueryParams))
	{
		//DrawDebugSphere(GetWorld(), TraceEnd, 32, 6, FColor::Blue, false, 1.f);
		if (TraceHit.GetActor())
		{
			if (UInteractionComponent* InteractionComponent = Cast<UInteractionComponent>(TraceHit.GetActor()->GetComponentByClass(UInteractionComponent::StaticClass())))
			{
				
				//float Distance = (TraceStart - TraceHit.ImpactPoint).Size();
				float Distance = InteractionCheckDistance;

				if (InteractionComponent != GetInteractable() && Distance <= InteractionComponent->InteractionDistance)
				{
					DrawDebugSphere(GetWorld(), TraceEnd, 32, 6, FColor::Blue, false, 1.f);
					FoundNewInteractable(InteractionComponent);
				}
				else if (Distance > InteractionComponent->InteractionDistance && GetInteractable())
				{
					CouldntFindInteractable();
				}

				return;
			}
		}
	}

	CouldntFindInteractable();
}

void ASpellPlanesCharacter::CouldntFindInteractable()
{

	if (GetWorldTimerManager().IsTimerActive(TimerHandle_Interact))
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_Interact);
	}

	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->EndFocus(this);

		if (InteractionData.bInteractHeld)
		{
			EndInteract();
		}
	}

	InteractionData.ViewedInteractionComponent = nullptr;
}

void ASpellPlanesCharacter::FoundNewInteractable(UInteractionComponent* Interactable)
{
	UE_LOG(LogTemp, Warning, TEXT("FoundNewInteractable"));
	
	EndInteract();

	if (UInteractionComponent* OldInteractable = GetInteractable())
	{
		OldInteractable->EndFocus(this);
	}

	InteractionData.ViewedInteractionComponent = Interactable;
	Interactable->BeginFocus(this);
}

void ASpellPlanesCharacter::BeginInteract()
{

	if(!HasAuthority())
	{
		ServerBeginInteract();
	}

/** As an optimization, the server only checks that we're looking at an item once we begin interacting with it.
This saves the server doing a check every tick for an interactable Item. The exception is a non-instance interact.
In this case, the server will check every tick for the duration of the interact. */
if (HasAuthority())
{
	PerformInteractionCheck();
}

InteractionData.bInteractHeld = true;

if (UInteractionComponent* Interactable = GetInteractable())
{
	Interactable->BeginInteract(this);

	if (FMath::IsNearlyZero(Interactable->InteractionTime))
	{
		Interact();
	}
	else
	{
		GetWorldTimerManager().SetTimer(TimerHandle_Interact, this, &ASpellPlanesCharacter::Interact, Interactable->InteractionTime, false);
	}
}
}

void ASpellPlanesCharacter::EndInteract()
{
	UE_LOG(LogTemp, Warning, TEXT("EndInteract"));
	if (!HasAuthority())
	{
		ServerEndInteract();
	}

	InteractionData.bInteractHeld = false;

	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->EndInteract(this);
	}
}

void ASpellPlanesCharacter::ServerBeginInteract_Implementation()
{
	BeginInteract();
}

bool ASpellPlanesCharacter::ServerBeginInteract_Validate()
{
	return true;
}

void ASpellPlanesCharacter::ServerEndInteract_Implementation()
{
	EndInteract();
}

bool ASpellPlanesCharacter::ServerEndInteract_Validate()
{
	return true;
}

void ASpellPlanesCharacter::Interact()
{
	UE_LOG(LogTemp, Warning, TEXT("Interact on Character"));

	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->Interact(this);
	}
}


void ASpellPlanesCharacter::StartReload()
{
	// May not have reload-able weapons
}

bool ASpellPlanesCharacter::CanSprint() const
{
	return true;
	//return !IsAiming;
}

void ASpellPlanesCharacter::StartSprinting()
{
	SetSprinting(true);
}

void ASpellPlanesCharacter::StopSprinting()
{
	SetSprinting(false);
}

void ASpellPlanesCharacter::SetSprinting(const bool bNewSprinting)
{
	if ((bNewSprinting && !CanSprint()) || bNewSprinting == bSprinting)
	{
		return;
	}

	if (!HasAuthority())
	{
		ServerSetSprinting(bNewSprinting);
	}

	bSprinting = bNewSprinting;

	GetCharacterMovement()->MaxWalkSpeed = bSprinting ? SprintSpeed : WalkSpeed;
}

void ASpellPlanesCharacter::ServerSetSprinting_Implementation(const bool bNewSprinting)
{
	SetSprinting(bNewSprinting);
}

bool ASpellPlanesCharacter::ServerSetSprinting_Validate(const bool bNewSprinting)
{
	return true;
}

void ASpellPlanesCharacter::StartCrouching()
{
	Crouch();
}

void ASpellPlanesCharacter::StopCrouching()
{

}

// Called to bind functionality to input
void ASpellPlanesCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Save", IE_Pressed, this, &ASpellPlanesCharacter::SaveGame);
	PlayerInputComponent->BindAction("Load", IE_Pressed, this, &ASpellPlanesCharacter::LoadGame);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASpellPlanesCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASpellPlanesCharacter::MoveRight);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASpellPlanesCharacter::BeginInteract);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &ASpellPlanesCharacter::EndInteract);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASpellPlanesCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASpellPlanesCharacter::StopFire);

	PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &ASpellPlanesCharacter::UseThrowable);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASpellPlanesCharacter::StartSprinting);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASpellPlanesCharacter::StopSprinting);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASpellPlanesCharacter::StartCrouching);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASpellPlanesCharacter::StopCrouching);
}

#undef LOCTEXT_NAMESPACE