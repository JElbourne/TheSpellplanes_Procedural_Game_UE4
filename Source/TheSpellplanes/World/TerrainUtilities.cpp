// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainUtilities.h"
#include <numeric>
#include <string>
#include <limits>
#include "Zone.h"

UTerrainUtilities::UTerrainUtilities()
{

}

int32 UTerrainUtilities::ConvertStringInputToIntegerSeed(FString SeedText, FIntVector ZoneLocation)
{
	// TODO make sure the user cant harm us her with malicious input
	std::string BaseText = std::string(TCHAR_TO_UTF8(*SeedText));
	int32 UserSeedNumber = (uint32)std::accumulate(std::next(BaseText.begin()), BaseText.end(),
		static_cast<int>(BaseText.at(0)),
		[](int a, char b) {
		return a + static_cast<int>(b);
	});
	// This is the top end of out int32 number so we have a hard limit here for the final number (2.14 billion).
	if (UserSeedNumber > 4000) UserSeedNumber = 4000;


	int32 MaxWidth = UWorldUtilities::WORLD_MAX_WIDTH_IN_ZONES;
	int32 HalfWidth = UWorldUtilities::WORLD_MAX_WIDTH_IN_ZONES / 2;

	// This will take the Width of our Seeded World and convert the Negative Grid Coordinated to positive ones.
	// Also Use the modulo operator to reset location only for Seed data.
	// This keeps the seed within INT32 limit but makes world infinite.
	int32 SeedX = (ZoneLocation.X > 0) ? (ZoneLocation.X % HalfWidth) : ((ZoneLocation.X % HalfWidth) * -2);
	int32 SeedY = (ZoneLocation.Y > 0) ? (ZoneLocation.Y % HalfWidth) : ((ZoneLocation.Y % HalfWidth) * -2);

	int32 Seed = UserSeedNumber * 1000000; //This will get it up to the top end of the number
	Seed += (SeedY * MaxWidth + SeedX); // This will add the lower end in for our final Seed Number.
	return Seed;
}

void UTerrainUtilities::CreatePerlinNoise2D(float fBias, int nOctaves, TArray<float> fSeedNoise, TArray<float>& fOutNoiseMap)
{
	int ZoneSize = UWorldUtilities::ZONE_LENGTH_IN_BLOCKS;

	float fNoise = 0.f;
	float fScale = 1.0f;
	float fScaleAccumulate = 0.f;

	float fHighNoise = TNumericLimits<float>::Min();
	float fLowNoise = TNumericLimits<float>::Max();

	for (int16 y = 0; y < ZoneSize; y++)
	{
		for (int16 x = 0; x < ZoneSize; x++)
		{

			fNoise = 0.f;
			fScale = 1.0f;
			fScaleAccumulate = 0.f;

			for (int8 i = 0; i < nOctaves; i++)
			{
				int32 nPitch = ZoneSize >> i;
				int32 nSampleX1 = (x / nPitch) * nPitch;
				int32 nSampleY1 = (y / nPitch) * nPitch;

				int32 nSampleX2 = (nSampleX1 + nPitch) % ZoneSize;
				int32 nSampleY2 = (nSampleY1 + nPitch) % ZoneSize;

				float fBlendX = (float)(x - nSampleX1) / (float)nPitch;
				float fBlendY = (float)(y - nSampleY1) / (float)nPitch;

				float fSampleT = (1.0f - fBlendX) * fSeedNoise[nSampleY1 * ZoneSize + nSampleX1] + fBlendX * fSeedNoise[nSampleY1 * ZoneSize + nSampleX2];
				float fSampleB = (1.0f - fBlendX) * fSeedNoise[nSampleY2 * ZoneSize + nSampleX1] + fBlendX * fSeedNoise[nSampleY2 * ZoneSize + nSampleX2];

				fScaleAccumulate += fScale;
				fNoise += (fBlendY * (fSampleB - fSampleT) + fSampleT) * fScale;
				fScale = fScale / fBias;
			}
			int16 nNewX = (x < (ZoneSize / 2)) ? x + (ZoneSize / 2) : x - (ZoneSize / 2);
			int16 nNewY = (y < (ZoneSize / 2)) ? y + (ZoneSize / 2) : y - (ZoneSize / 2);
			float fNoiseAmount = fNoise / fScaleAccumulate;
						
			if (fNoiseAmount < fLowNoise)
				fLowNoise = fNoiseAmount;
			else if (fNoiseAmount > fHighNoise)
				fHighNoise = fNoiseAmount;

			fOutNoiseMap[nNewY * ZoneSize + nNewX] = fNoiseAmount;
		}
	}

	// Normalizes the noise to be a nice value between 0 and 1.
	for (int16 y = 0; y < ZoneSize; y++)
	{
		for (int16 x = 0; x < ZoneSize; x++)
		{
			fOutNoiseMap[y * ZoneSize + x] = (fOutNoiseMap[y * ZoneSize + x] - fLowNoise) / (fHighNoise - fLowNoise);
		}
	}
}


void UTerrainUtilities::UpdateInstanceOpacity(UInstancedStaticMeshComponent& InstancedMeshComponent, int32 index, FVector CurrentZoneLocation, EZoneVisibilityLevel ZoneVisibilityLevel)
{
	if (index >= InstancedMeshComponent.GetInstanceCount())
	{
		return;
	}

	const float STAGE_ONE_FADE = (float)(UWorldUtilities::FOG_FADE_STAGE_ONE) / 100.f;
	const float STAGE_TWO_FADE = (float)(UWorldUtilities::FOG_FADE_STAGE_TWO) / 100.f;

	if (ZoneVisibilityLevel == EZoneVisibilityLevel::ZVL_FOG)
	{
		FTransform InstanceTrans;
		InstancedMeshComponent.GetInstanceTransform(index, InstanceTrans, true);

		if (InstancedMeshComponent.GetInstanceTransform(index, InstanceTrans, true))
		{
			FVector InstanceLoc = InstanceTrans.GetLocation();
			float CurrDistX = abs(CurrentZoneLocation.X - InstanceLoc.X);
			float CurrDistY = abs(CurrentZoneLocation.Y - InstanceLoc.Y);

			// Check X direction for side zones
			if (CurrDistX <= STAGE_ONE_DIST && CurrDistX >= HALF_ZONE)
			{
				if (CurrDistY <= STAGE_TWO_DIST && CurrDistY > STAGE_ONE_DIST)
				{
					InstancedMeshComponent.SetCustomDataValue(index, 2, STAGE_TWO_FADE, true);
					return;
				}
				else if (CurrDistY > STAGE_TWO_DIST)
				{
					InstancedMeshComponent.SetCustomDataValue(index, 2, 0.0f, true);
					return;
				}
				InstancedMeshComponent.SetCustomDataValue(index, 2, STAGE_ONE_FADE, true);
				return;
			}
			else if (CurrDistX <= STAGE_TWO_DIST && CurrDistX > STAGE_ONE_DIST)
			{
				if (CurrDistY > STAGE_TWO_DIST)
				{
					InstancedMeshComponent.SetCustomDataValue(index, 2, 0.0f, true);
					return;
				}
				InstancedMeshComponent.SetCustomDataValue(index, 2, STAGE_TWO_FADE, true);
				return;
			}
			// Check Y direction for side zones
			if (CurrDistY <= STAGE_ONE_DIST && CurrDistY >= HALF_ZONE)
			{
				if (CurrDistX <= STAGE_TWO_DIST && CurrDistX > STAGE_ONE_DIST)
				{
					InstancedMeshComponent.SetCustomDataValue(index, 2, STAGE_TWO_FADE, true);
					return;
				}
				else if (CurrDistX > STAGE_TWO_DIST)
				{
					InstancedMeshComponent.SetCustomDataValue(index, 2, 0.0f, true);
					return;
				}
				InstancedMeshComponent.SetCustomDataValue(index, 2, STAGE_ONE_FADE, true);
				return;
			}
			else if (CurrDistY <= STAGE_TWO_DIST && CurrDistY > STAGE_ONE_DIST)
			{
				if (CurrDistX > STAGE_TWO_DIST)
				{
					InstancedMeshComponent.SetCustomDataValue(index, 2, 0.0f, true);
					return;
				}
				InstancedMeshComponent.SetCustomDataValue(index, 2, STAGE_TWO_FADE, true);
				return;
			}
			// Check Both direction for the corner zones
			if (CurrDistX > STAGE_TWO_DIST || CurrDistY > STAGE_TWO_DIST)
			{
				InstancedMeshComponent.SetCustomDataValue(index, 2, 0.0f, true);
				return;
			}
		}
	}
	else
	{
		InstancedMeshComponent.SetCustomDataValue(index, 2, 1.0f, true);
	}


}

void UTerrainUtilities::GenerateZoneTerrain(FString SeedText, FIntVector ZoneLocation, TArray<int32>& GridIdLayout)
{
	int32 Seed = ConvertStringInputToIntegerSeed(SeedText, ZoneLocation);
	UE_LOG(LogTemp, Warning, TEXT("GenerateZoneTerrain with Converted Seed: %i"), Seed);

	//FMath::RandInit(Seed);
	FMath::SRandInit(Seed);

	// Set some initial variables. TODO get this from a config or GameInstance.
	int nOctaveCount = 4;
	float fScalingBias = 2.5f;

	// Set the Base Random Noise. This will be our base to generate all the noise from.
	TArray<float> fNoiseSeed;
	fNoiseSeed.SetNum(UWorldUtilities::ZONE_BLOCK_AREA);
	for (int i = 0; i < UWorldUtilities::ZONE_BLOCK_AREA; i++) fNoiseSeed[i] = FMath::SRand();

	// Fill an array with our Perlin noise.
	TArray<float> fPerlinNoise;
	fPerlinNoise.SetNum(UWorldUtilities::ZONE_BLOCK_AREA);
	CreatePerlinNoise2D(fScalingBias, nOctaveCount, fNoiseSeed, fPerlinNoise);

	TArray<int8> StoneGrid;
	StoneGrid.Init(0, UWorldUtilities::ZONE_BLOCK_AREA);
	TArray<int8> VegetationGrid;
	VegetationGrid.Init(0, UWorldUtilities::ZONE_BLOCK_AREA);

	int8 ZoneSize = UWorldUtilities::ZONE_LENGTH_IN_BLOCKS;

	UDataTable* GroundTerrainTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), NULL, TEXT("DataTable'/Game/TheSpellplanes/Data/GroundTerrain.GroundTerrain'")));
	if (GroundTerrainTable)
	{
		for (int16 y = 0; y < ZoneSize; y++)
		{
			for (int16 x = 0; x < ZoneSize; x++)
			{
				int8 CurrentHeight = (int8)(fPerlinNoise[y * ZoneSize + x] * 24.0f);

				for (auto TerrainTypeRow : GroundTerrainTable->GetRowMap())
				{
					FGroundTerrainType* GroundTerrainType = (FGroundTerrainType*)(TerrainTypeRow.Value);
					if (CurrentHeight <= GroundTerrainType->MaxElevation && CurrentHeight >= GroundTerrainType->MinElevation)
					{
						UBlockUtilities::SetValueInGridId(GridIdLayout[(y * ZoneSize + x)], (int16)GroundTerrainType->Type, EBlockIdIndex::BII_GroundType);
						if (GroundTerrainType->Type == EGroundTerrainTypes::GBT_Rough_Stone ||
							GroundTerrainType->Type == EGroundTerrainTypes::GBT_Lava)
						{
							StoneGrid[(y * ZoneSize + x)]= 1;
						}
						else if (GroundTerrainType->Type == EGroundTerrainTypes::GBT_Soil)
						{
							VegetationGrid[(y * ZoneSize + x)] = 1;
						}
						break;
					}
				}
			}
		}
	}

	// Set Resource Block Types
	int32 fRandomSeedValue = ((fNoiseSeed[0] + fNoiseSeed[ZoneSize / 4] + fNoiseSeed[ZoneSize / 2] + fNoiseSeed[ZoneSize - 1]) / 4) * 10;
	int32 StoneTypeIndex = (fRandomSeedValue > 0) ? fRandomSeedValue : 1; // Must do this step so that number is 1-10 inclusive
	int32 TreeTypeIndex = (fRandomSeedValue*2 > 10) ? fRandomSeedValue*2 : 11; // Must do this step so that number is 11-20 inclusive

	for (int16 y = 0; y < ZoneSize; y++)
	{
		for (int16 x = 0; x < ZoneSize; x++)
		{
			// Mountains
			if (StoneGrid[(y * ZoneSize + x)] == 1)
			{
				int16 index1 = (y == (ZoneSize - 1)) ? (y * ZoneSize + x) : ((y + 1) * ZoneSize + x);
				int16 index2 = (y == 0) ? (y * ZoneSize + x) : ((y - 1) * ZoneSize + x);
				int16 index3 = (x == (ZoneSize - 1)) ? (y * ZoneSize + x) : (y * ZoneSize + (x + 1));
				int16 index4 = (x == 0) ? (y * ZoneSize + x) : (y * ZoneSize + (x - 1));

				if (StoneGrid[index1] == 1 &&
					StoneGrid[index2] == 1 &&
					StoneGrid[index3] == 1 &&
					StoneGrid[index4] == 1)
				{
					// This is a stone block 100%
					UBlockUtilities::SetValueInGridId(GridIdLayout[(y * ZoneSize + x)], StoneTypeIndex, EBlockIdIndex::BII_BlockType);
				}
				else
				{
					// This is a stone block 40% of the time
					if (fNoiseSeed[y * ZoneSize + x] < 0.4f)
					{
						UBlockUtilities::SetValueInGridId(GridIdLayout[(y * ZoneSize + x)], StoneTypeIndex, EBlockIdIndex::BII_BlockType);
					}
				}
			}
			// Vegetation
			if (VegetationGrid[(y * ZoneSize + x)] == 1)
			{
				int16 index1 = (y == (ZoneSize - 1)) ? (y * ZoneSize + x) : ((y + 1) * ZoneSize + x);
				int16 index2 = (y == 0) ? (y * ZoneSize + x) : ((y - 1) * ZoneSize + x);
				int16 index3 = (x == (ZoneSize - 1)) ? (y * ZoneSize + x) : (y * ZoneSize + (x + 1));
				int16 index4 = (x == 0) ? (y * ZoneSize + x) : (y * ZoneSize + (x - 1));

				if (VegetationGrid[index1] == 1 &&
					VegetationGrid[index2] == 1 &&
					VegetationGrid[index3] == 1 &&
					VegetationGrid[index4] == 1)
				{
					// This is a Tree block 100%
					UBlockUtilities::SetValueInGridId(GridIdLayout[(y * ZoneSize + x)], TreeTypeIndex, EBlockIdIndex::BII_BlockType);
				}
				else
				{
					// This is a Tree block 20% of the time and Grass 80%
					if (fNoiseSeed[y * ZoneSize + x] < 0.2f)
					{
						UBlockUtilities::SetValueInGridId(GridIdLayout[(y * ZoneSize + x)], TreeTypeIndex, EBlockIdIndex::BII_BlockType);
					}
					else
					{
						UBlockUtilities::SetValueInGridId(GridIdLayout[(y * ZoneSize + x)], 99, EBlockIdIndex::BII_BlockType);
					}
				}
			}

		}
	}
}

void UTerrainUtilities::UpdateZoneInstanceArray(AZone& Zone, TArray<int32> GridIdLayout)
{
	for (int16 i = 0; i < GridIdLayout.Num(); i++)
	{
		int32 GridId = GridIdLayout[i];

		// Determine the grid coordinate in a grid of blocks on the screen
		int16 GridX = i % UWorldUtilities::ZONE_LENGTH_IN_BLOCKS;
		int16 GridY = i / UWorldUtilities::ZONE_LENGTH_IN_BLOCKS;

		// Determine Actual Location of the Block to be spawned, this is Local Location to the ZoneActor
		float PosX = (float)((GridX * UWorldUtilities::BLOCK_SIZE) - (UWorldUtilities::ZONE_SIZE / 2));
		float PosY = (float)((GridY * UWorldUtilities::BLOCK_SIZE) - (UWorldUtilities::ZONE_SIZE / 2));

		int16 ModifierId = UBlockUtilities::GetValueFromGridId(GridId, EBlockIdIndex::BII_ModifierType);

		// 1. Spawn The Ground
		int16 GroundId = UBlockUtilities::GetValueFromGridId(GridId, EBlockIdIndex::BII_GroundType);
		Zone.AddTerrainInstance(FVector(PosX, PosY, UWorldUtilities::BLOCK_SIZE * -1), GroundId, ModifierId);

		// 2. Spawn The Block
		int16 BlockId = UBlockUtilities::GetValueFromGridId(GridId, EBlockIdIndex::BII_BlockType);
		if (BlockId > 0)
		{
			Zone.AddResourceInstance(FVector(PosX, PosY, 0.f), BlockId, ModifierId);
		}

		// 3. Set The Item & Quantity
		int16 ItemId = UBlockUtilities::GetValueFromGridId(GridId, EBlockIdIndex::BII_ItemType);
		int16 ItemQnt = UBlockUtilities::GetValueFromGridId(GridId, EBlockIdIndex::BII_ItemQuantity);
		if (ItemId > 0 && ItemQnt > 0)
		{

		}
	}
}

void UTerrainUtilities::UpdateFogTerrain(AZone& Zone, EZoneVisibilityLevel ZoneVisibilityLevel, FVector CurrentZoneLocation)
{
	int32 InstanceCount = Zone.TerrainBlocksComponent->GetInstanceCount();
	for (int32 i = 0; i < UWorldUtilities::ZONE_BLOCK_AREA; i++)
	{
		UpdateInstanceOpacity(*Zone.TerrainBlocksComponent, i, CurrentZoneLocation, ZoneVisibilityLevel);
		UpdateInstanceOpacity(*Zone.MountainBlocksComponent, i, CurrentZoneLocation, ZoneVisibilityLevel);
		UpdateInstanceOpacity(*Zone.TreeBlocksComponent, i, CurrentZoneLocation, ZoneVisibilityLevel);
	}
}


