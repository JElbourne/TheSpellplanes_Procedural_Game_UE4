// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Zone.h"
#include "TerrainUtilities.generated.h"


/**
 * 
 */
UCLASS()
class THESPELLPLANES_API UTerrainUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UTerrainUtilities();

	static const int32 STAGE_ONE_DIST = UWorldUtilities::ZONE_SIZE / 2 + (UWorldUtilities::BLOCK_SIZE * (UWorldUtilities::FOG_DISTANCE / 2));
	static const int32 STAGE_TWO_DIST = UWorldUtilities::ZONE_SIZE / 2 + (UWorldUtilities::BLOCK_SIZE * UWorldUtilities::FOG_DISTANCE);
	static const int32 HALF_ZONE = UWorldUtilities::ZONE_SIZE / 2;

	/** This will create the 'organic' layout of the Terrain using PerlinNoise.
	* @param SeedText: Seed that was created when game started
	* @param ZoneLocation: This is used to give each Zone a unique Seed.
	* @param GridIdLayout: This will populate the given Array with all the Block Id's this zone has.
	*/
	static void GenerateZoneTerrain(FString SeedText, FIntVector ZoneLocation, TArray<int32>& GridIdLayout);

	/** This will instantiate each Block actor from GridIdLayout into the world on a grid, offset by the ZoneLocation. */
	static void UpdateZoneInstanceArray(AZone& Zone, TArray<int32> GridIdLayout);

	static void UpdateFogTerrain(AZone& Zone, EZoneVisibilityLevel ZoneVisibilityLevel, FVector CurrentZoneLocation);

private:

	static int32 ConvertStringInputToIntegerSeed(FString SeedText, FIntVector ZoneLocation);

	static void CreatePerlinNoise2D(float fBias, int nOctaves, TArray<float> fSeed, TArray<float>& fOutNoiseMap);

	static void UpdateInstanceOpacity(UInstancedStaticMeshComponent& InstancedMeshComponent, int32 index, FVector CurrentZoneLocation, EZoneVisibilityLevel ZoneVisibilityLevel);
};


