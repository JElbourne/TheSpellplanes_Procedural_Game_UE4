// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "World/WorldUtilities.h"
#include "Framework/SpellPlanesSaveGame.h"
#include "../World/Zone.h"
#include "SpellPlanesGameInstance.h"
#include "SpellPlanesGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class THESPELLPLANES_API ASpellPlanesGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	ASpellPlanesGameStateBase();

	USpellPlanesGameInstance* SPGameInstance;

	/**
	* This manages the current Elevation Level the player is on.
	* Typically used in determining location as the Z value.
	*/
	UPROPERTY(VisibleAnywhere, Category = "Game Play State")
	int32 m_CurrentElevation;

	/** This is the current Zone the player is moving in. Also the Camera Location which is how this is set. */
	UPROPERTY(VisibleAnywhere, Category = "Game Play State")
	FVector m_CurrentZoneLocation;

private:

	UPROPERTY()
	FString SeedText;

	/** 
	* This has all the room types that were procedurally generated at startup of world.
	* -Should be HERE in Game State
	* -The CLIENT will update the zones they are walking into and need to know if that new zone is on the PATH.
	*/
	UPROPERTY(VisibleAnywhere, Category = "Initialized World")
	TMap<FIntVector, EZoneType> m_SolutionPathData;

	/** This has all the Special Zones that were procedurally generated at startup of world.
	ie Start and end of initial path, or a special dungeon. */
	UPROPERTY(VisibleAnywhere, Category = "Initialized World")
	TMap<FIntVector, ESpecialZoneType> m_SpecialZoneLocations;

	UPROPERTY()
	TMap<FIntVector, FZoneGridData> ZoneData;

	UPROPERTY()
	TMap<FIntVector, AZone*> m_SpawnedZones;

public:

	UFUNCTION()
	bool CreateLevel();

	// Specialty Zones
	UFUNCTION()
	ESpecialZoneType GetSpecialZoneAtLocation(FIntVector Location);

	UFUNCTION()
	void SetSpecialZoneAtLocation(FIntVector Location, ESpecialZoneType ZoneType);

	UFUNCTION()
	void SetSolutionPathAtLocation(FIntVector Location, EZoneType ZoneType);

	UFUNCTION()
	void ResetSpecialZones();

	UFUNCTION()
	bool MoveToZone(FVector CurrentZoneLocation, FVector2D TransitionDirection);

	UFUNCTION(Client, Reliable)
	void SpawnZoneOnClient(FIntVector ZoneCoord, const TArray<int32> & GridIdLayout, EZoneVisibilityLevel ZoneVisibilityLevel);
	
	UFUNCTION()
	void UpdateFogZone(FIntVector ZoneCoord, EZoneVisibilityLevel ZoneVisibilityLevel);
	UFUNCTION()
	void UpdateFogNeighbours(FIntVector ZoneCoord, EZoneVisibilityLevel ZoneVisibilityLevel);

private:

	TMap<FIntVector, EZoneType> CreateSolutionPath(uint8 SolutionLength);
	bool SpawnZone(FIntVector ZoneLocation, EZoneVisibilityLevel ZoneVisibilityLevel);
	bool SpawnZoneNeighbours(FIntVector ZoneLocation);
	
	void DestroyDistantZones();

	/** Will return the Zone type for a location either from the Solution Path or Standard Filled Zone */
	EZoneType GetZoneTypeAtLocation(FVector Location);
};
