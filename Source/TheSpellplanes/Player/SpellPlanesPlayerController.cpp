// Fill out your copyright notice in the Description page of Project Settings.


#include "SpellPlanesPlayerController.h"
#include "SpellPlanesCharacter.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/SpellPlanesCamera.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "World/WorldUtilities.h"
#include "Framework/SpellPlanesGameInstance.h"
#include "Framework/SpellPlanesGameStateBase.h"
#include "Kismet/GameplayStatics.h"



ASpellPlanesPlayerController::ASpellPlanesPlayerController()
{
	
}

void ASpellPlanesPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SpawnMainCamera();

	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	m_SpellPlanesGameState = GetWorld() != NULL ? GetWorld()->GetGameState<ASpellPlanesGameStateBase>() : NULL;
}

void ASpellPlanesPlayerController::SpawnMainCamera()
{
	if (GetWorld())
	{
		USpellPlanesGameInstance* SPGameInstance = GetGameInstance<USpellPlanesGameInstance>();

		if (SPGameInstance)
		{
			FVector PlayerStartLocation = SPGameInstance->GetPlayerSavedLocation();
			FVector ZoneStartLocation = UWorldUtilities::GetZoneCenterFromLocation(PlayerStartLocation);

			UE_LOG(LogTemp, Warning, TEXT("SetMainCamera on PlayerController"));
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.bNoFail = true;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			m_MainCamera = GetWorld()->SpawnActor<ASpellPlanesCamera>(ASpellPlanesCamera::StaticClass(), ZoneStartLocation, FRotator(0.f, 0.f, 0.f), SpawnParams);

			if (m_MainCamera)
			{
				FViewTargetTransitionParams TransitionParams;
				TransitionParams.BlendTime = 0.f;
				TransitionParams.BlendFunction = EViewTargetBlendFunction::VTBlend_Linear;
				TransitionParams.BlendExp = 0.f;
				TransitionParams.bLockOutgoing = false;
				SetViewTarget(m_MainCamera, TransitionParams);
				UE_LOG(LogTemp, Error, TEXT("Here"));
			}
		}
	}
}

float ASpellPlanesPlayerController::GetCameraYaw()
{
	if (m_MainCamera)
	{
		return m_MainCamera->CameraYaw;
	}
	return 0.f;
}

FVector ASpellPlanesPlayerController::GetCameraLocation()
{
	return m_MainCamera->GetActorLocation();
}

bool ASpellPlanesPlayerController::IsTransitioningZones()
{
	return m_MainCamera->m_IsMoving;
}

void ASpellPlanesPlayerController::TransitionZones(FVector2D TransitionDirection)
{
	m_MainCamera->m_IsMoving = true;
	UE_LOG(LogTemp, Warning, TEXT("ASpellPlanesPlayerController::TransitionZones"));
	FVector CamLoc = m_MainCamera->GetActorLocation();
	if (m_SpellPlanesGameState->MoveToZone(CamLoc, TransitionDirection))
	{
		// TODO this needs to move into the WorldUtilities.??
		//FVector NewCameraLocation = FVector(
		//	CamLoc.X + (TransitionDirection.X * UWorldUtilities::ZONE_SIZE),
		//	CamLoc.Y + (TransitionDirection.Y * UWorldUtilities::ZONE_SIZE),
		//	CamLoc.Z);
		m_MainCamera->MoveCamera(m_SpellPlanesGameState->m_CurrentZoneLocation);

		UE_LOG(LogTemp, Warning, TEXT("Finished Move Camera"));
	}
}

void ASpellPlanesPlayerController::ClientShowNotification_Implementation(const FText& Message)
{
	ShowNotification(Message);
}

void ASpellPlanesPlayerController::Respawn()
{
	UnPossess();
	ChangeState(NAME_Inactive);

	if (!HasAuthority())
	{
		ServerRespawn();
	}
	else
	{
		ServerRestartPlayer();
	}
}

void ASpellPlanesPlayerController::ServerRespawn_Implementation()
{
	Respawn();
}

bool ASpellPlanesPlayerController::ServerRespawn_Validate()
{
	return true;
}

void ASpellPlanesPlayerController::StartReload()
{
	if (ASpellPlanesCharacter* SpellPlanesCharacter = Cast<ASpellPlanesCharacter>(GetPawn()))
	{
		if (SpellPlanesCharacter->IsAlive())
		{
			//Character->StartReload();
			UE_LOG(LogTemp, Warning, TEXT("Need to implement weapon reloading"));
		}
		else
		{
			Respawn();
		}
	}
}

void ASpellPlanesPlayerController::RotateCameraLeft()
{
	if (m_MainCamera)
	{
		m_MainCamera->RotateCamera(-1.f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainCamera not set!"));
	}
}

void ASpellPlanesPlayerController::RotateCameraRight()
{
	if (m_MainCamera)
	{
		m_MainCamera->RotateCamera(1.f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainCamera not set!"));
	}
}

// Called to bind functionality to input
void ASpellPlanesPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("TurnCameraRight", IE_Pressed, this, &ASpellPlanesPlayerController::RotateCameraRight);
	InputComponent->BindAction("TurnCameraLeft", IE_Pressed, this, &ASpellPlanesPlayerController::RotateCameraLeft);
}
