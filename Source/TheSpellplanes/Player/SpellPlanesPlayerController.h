// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "../Camera/SpellPlanesCamera.h"
#include "../Framework/SpellPlanesGameStateBase.h"
#include "SpellPlanesPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class THESPELLPLANES_API ASpellPlanesPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	ASpellPlanesPlayerController();

	UFUNCTION()
	virtual void BeginPlay() override;

	UFUNCTION()
	void SpawnMainCamera();

	UFUNCTION()
	float GetCameraYaw();

	UFUNCTION()
	FVector GetCameraLocation();

	UFUNCTION()
	bool IsTransitioningZones();

	UFUNCTION()
	void TransitionZones(FVector2D TransitionDirection);

	// Call this instead of show notification if on the server
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void ClientShowNotification(const FText& Message);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowNotification(const FText& Message);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowDeathScreen(class ASpellPlanesCharacter* Killer);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowLootMenu(const class UInventoryComponent* LootSource);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowIngameUI();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void HideLootMenu();

	UFUNCTION(BlueprintImplementableEvent)
	void OnHitPlayer();

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void Respawn();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRespawn();

	void StartReload();

	// User Input to drive Camera Rotation
	void RotateCameraLeft();
	void RotateCameraRight();
	//


private:

	class ASpellPlanesCamera* m_MainCamera;
	ASpellPlanesGameStateBase* m_SpellPlanesGameState;

protected:
	// Called to bind functionality to input
	virtual void SetupInputComponent() override;
	//
};
