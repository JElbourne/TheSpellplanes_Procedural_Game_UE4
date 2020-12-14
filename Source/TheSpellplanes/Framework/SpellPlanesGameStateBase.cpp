// Fill out your copyright notice in the Description page of Project Settings.


#include "SpellPlanesGameStateBase.h"
#include "Framework/SpellPlanesGameInstance.h"
#include "Framework/SpellPlanesGameModeBase.h"
#include "Camera/SpellPlanesCamera.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "Player/SpellPlanesPlayerController.h"
#include "Player/SpellPlanesCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "World/TerrainUtilities.h"



ASpellPlanesGameStateBase::ASpellPlanesGameStateBase()
{
	
}

/**
* This is called from GameMode when the Play State changes to Loading
* @return bool - whether the level was created or not.
*/
bool ASpellPlanesGameStateBase::CreateLevel()
{
	UE_LOG(LogTemp, Warning, TEXT("Lets Create the Level"));
	if (GetWorld() != nullptr && HasAuthority())
	{
		SPGameInstance = GetGameInstance<USpellPlanesGameInstance>();
		SeedText = SPGameInstance->GetSeedText();

		FVector PlayerStartLocation = SPGameInstance->GetPlayerSavedLocation();
		m_CurrentElevation = SPGameInstance->GetPlayerStartElevation(); // This should come from Game Instance

		FIntVector ZoneStartLocation = UWorldUtilities::GetZoneGridCoordFromLocation(PlayerStartLocation, m_CurrentElevation);
		m_CurrentZoneLocation = FVector(ZoneStartLocation.X * UWorldUtilities::ZONE_SIZE, ZoneStartLocation.X * UWorldUtilities::ZONE_SIZE, 0.0f);
		//uint8 SolutionLength = SPGameInstance->GetSolutionLength();
		//UE_LOG(LogTemp, Warning, TEXT("Solution Length Goal is: %i"), SolutionLength);
		
		// Step 1 - Create A Solution Path
		// TODO: What happens on the Load Game, where is the Solution Path Set from?
		//SPGameInstance->SetSolutionPathInCurrentRegion(CreateSolutionPath(SolutionLength));

		// Step 2 - Set Region Blocks
		// TODO: This feature is not implemented yet

		// Step 3 - Spawn First Zone
		SpawnZone(ZoneStartLocation, EZoneVisibilityLevel::ZVL_Visible);

		// Step 4 - Spawn Neighbour Zones
		SpawnZoneNeighbours(ZoneStartLocation);

		// Step 5 - Spawn Player
		//SpawnPlayer(PlayerStartLocation);

		// Step 6 - Spawn A Camera
		//SpawnCamera(ZoneStartLocation);


		return true;
		
	}
	return false; // Couldn't even find the World, we fools.
}

TMap<FIntVector, EZoneType> ASpellPlanesGameStateBase::CreateSolutionPath(uint8 SolutionLength)
{
	UWorldUtilities::CreateSolutionPath(SolutionLength, m_SolutionPathData, m_SpecialZoneLocations);
	UE_LOG(LogTemp, Warning, TEXT("Created Length of solution path: %i"), m_SolutionPathData.Num());
	return m_SolutionPathData;
}

bool ASpellPlanesGameStateBase::SpawnZone(FIntVector ZoneGridLocation, EZoneVisibilityLevel ZoneVisibilityLevel)
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Lets Spawn the First Zone"));
		TArray<int32> GridIdLayout;
		GridIdLayout.Init(0, UWorldUtilities::ZONE_BLOCK_AREA);

		// Step 1 - Generate Random Terrain for the Zone.


		// TODO This step should be expanded to see if zone exists or needs to be generated.
		UTerrainUtilities::GenerateZoneTerrain(SeedText, ZoneGridLocation, GridIdLayout);
		//

		SpawnZoneOnClient(ZoneGridLocation, GridIdLayout, ZoneVisibilityLevel);

		return true;
	}
	return false;
}

bool ASpellPlanesGameStateBase::SpawnZoneNeighbours(FIntVector ZoneLocation)
{
	if (HasAuthority())
	{
		//UE_LOG(LogTemp, Warning, TEXT("Lets Spawn the Neighbour Zones"));
		for (int8 x = -1; x <= 1; x++)
		{
			for (int8 y = -1; y <= 1; y++)
			{
				if (x == 0 && y == 0) continue;
				int32 AdjX = x + ZoneLocation.X;
				int32 AdjY = y + ZoneLocation.Y;
				FIntVector NeighbourLocation = FIntVector(AdjX, AdjY, ZoneLocation.Z);
				if (!m_SpawnedZones.Contains(NeighbourLocation))
				{
					SpawnZone(NeighbourLocation, EZoneVisibilityLevel::ZVL_FOG);
					
				}
				else
				{
					UpdateFogZone(NeighbourLocation, EZoneVisibilityLevel::ZVL_FOG);
				}
				
			}
		}
		return true;
	}
	return false;
}

void ASpellPlanesGameStateBase::DestroyDistantZones()
{
	int32 CurrentZoneX = (int32)(m_CurrentZoneLocation.X / UWorldUtilities::ZONE_SIZE);
	int32 CurrentZoneY = (int32)(m_CurrentZoneLocation.Y / UWorldUtilities::ZONE_SIZE);

	TMap<FIntVector, AZone*> ZonesToDestroy;
	for (const TPair<FIntVector, AZone*>& SpawnedZones : m_SpawnedZones)
	{
		int32 ZoneDistanceX = abs(CurrentZoneX - SpawnedZones.Key.X);
		int32 ZoneDistanceY = abs(CurrentZoneY - SpawnedZones.Key.Y);
		if (ZoneDistanceX >= UWorldUtilities::MAX_ZONE_DISTANCE || ZoneDistanceY >= UWorldUtilities::MAX_ZONE_DISTANCE)
		{
			ZonesToDestroy.Add(SpawnedZones.Key, SpawnedZones.Value);
		}
	}

	for (const TPair<FIntVector, AZone*>& ZoneToDestroy : ZonesToDestroy)
	{
		ZoneToDestroy.Value->Destroy();
		m_SpawnedZones.Remove(ZoneToDestroy.Key);
		UE_LOG(LogTemp, Warning, TEXT("Destroyed Zone at location: %i, %i"), ZoneToDestroy.Key.X, ZoneToDestroy.Key.Y);
	}
}

EZoneType ASpellPlanesGameStateBase::GetZoneTypeAtLocation(FVector Location)
{
	FIntVector ZoneGridCoord = UWorldUtilities::GetZoneGridCoordFromLocation(Location, m_CurrentElevation);
	if (m_SolutionPathData.Contains(ZoneGridCoord))
	{
		return m_SolutionPathData[ZoneGridCoord];
	}
	else
	{
		return m_SolutionPathData.Add(ZoneGridCoord, EZoneType::PZT_Fill_Base);
	}
}



ESpecialZoneType ASpellPlanesGameStateBase::GetSpecialZoneAtLocation(FIntVector Location)
{
	return *m_SpecialZoneLocations.Find(Location);
}

void ASpellPlanesGameStateBase::SetSpecialZoneAtLocation(FIntVector Location, ESpecialZoneType ZoneType)
{
	m_SpecialZoneLocations.Add(Location, ZoneType);
}


void ASpellPlanesGameStateBase::SetSolutionPathAtLocation(FIntVector Location, EZoneType ZoneType)
{
	m_SolutionPathData.Add(Location, ZoneType);
}


void ASpellPlanesGameStateBase::ResetSpecialZones()
{
	m_SpecialZoneLocations.Empty();
}

bool ASpellPlanesGameStateBase::MoveToZone(FVector CurrentCameraLocation, FVector2D TransitionDirection)
{
	UE_LOG(LogTemp, Warning, TEXT("ASpellPlanesGameStateBase::MoveToZone"));
	FIntVector CurrentZoneLocation = FIntVector(
		(int32)(CurrentCameraLocation.X / UWorldUtilities::ZONE_SIZE),
		(int32)(CurrentCameraLocation.Y / UWorldUtilities::ZONE_SIZE),
		(int32)(m_CurrentElevation)
	);
	FIntVector NextZoneLocation = FIntVector(
		(int32)(CurrentZoneLocation.X + TransitionDirection.X),
		(int32)(CurrentZoneLocation.Y + TransitionDirection.Y),
		CurrentZoneLocation.Z
	);

	if (m_SpawnedZones.Contains(CurrentZoneLocation) && m_SpawnedZones.Contains(NextZoneLocation))
	{
		UE_LOG(LogTemp, Warning, TEXT("m_SpawnedZones.Contains(CurrentZoneLocation) && m_SpawnedZones.Contains(NextZoneLocation)"));
		AZone* CurrentZone = *m_SpawnedZones.Find(CurrentZoneLocation);
		AZone* NextZone = *m_SpawnedZones.Find(NextZoneLocation);

		m_CurrentZoneLocation = FVector(
			CurrentCameraLocation.X + (TransitionDirection.X * UWorldUtilities::ZONE_SIZE),
			CurrentCameraLocation.Y + (TransitionDirection.Y * UWorldUtilities::ZONE_SIZE),
			CurrentCameraLocation.Z);

		UpdateFogZone(NextZoneLocation, EZoneVisibilityLevel::ZVL_Visible);

		SpawnZoneNeighbours(NextZoneLocation);
		DestroyDistantZones();
		return true;
	}

	return false;
}

void ASpellPlanesGameStateBase::SpawnZoneOnClient_Implementation(FIntVector ZoneGridLocation, const TArray<int32> & GridIdLayout, EZoneVisibilityLevel ZoneVisibilityLevel)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	FVector SpawnVector = FVector(
		(float)ZoneGridLocation.X * UWorldUtilities::ZONE_SIZE,
		(float)ZoneGridLocation.Y * UWorldUtilities::ZONE_SIZE,
		0.f);
	FRotator SpawnRotator = FRotator(0.f, 0.f, 0.f);
	AZone* Zone = GetWorld()->SpawnActor<AZone>(AZone::StaticClass(), SpawnVector, SpawnRotator, SpawnParams);
	UTerrainUtilities::UpdateZoneInstanceArray(*Zone, GridIdLayout);
	m_SpawnedZones.Add(ZoneGridLocation, Zone);

	UpdateFogZone(ZoneGridLocation, ZoneVisibilityLevel);
}

void ASpellPlanesGameStateBase::UpdateFogZone(FIntVector ZoneCoord, EZoneVisibilityLevel ZoneVisibilityLevel)
{
	if (m_SpawnedZones.Contains(ZoneCoord))
	{
		AZone* Zone = *m_SpawnedZones.Find(ZoneCoord);
		if (ZoneVisibilityLevel == EZoneVisibilityLevel::ZVL_Hidden)
		{
			Zone->SetActorHiddenInGame(true);
		}
		else
		{
			UTerrainUtilities::UpdateFogTerrain(*Zone, ZoneVisibilityLevel, m_CurrentZoneLocation);
			Zone->SetActorHiddenInGame(false);
		}
	}
}

void ASpellPlanesGameStateBase::UpdateFogNeighbours(FIntVector ZoneCoord, EZoneVisibilityLevel ZoneVisibilityLevel)
{
	for (int8 x = -1; x <= 1; x++)
	{
		for (int8 y = -1; y <= 1; y++)
		{
			if (x == 0 && y == 0) continue;
			int32 AdjX = x + ZoneCoord.X;
			int32 AdjY = y + ZoneCoord.Y;
			FIntVector NeighbourLocation = FIntVector(AdjX, AdjY, ZoneCoord.Z);
			if (m_SpawnedZones.Contains(NeighbourLocation))
			{
				UpdateFogZone(NeighbourLocation, ZoneVisibilityLevel);
			}

		}
	}
}
