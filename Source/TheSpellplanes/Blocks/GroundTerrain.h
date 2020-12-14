// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blocks/Block.h"
#include "GroundTerrain.generated.h"


UENUM(BlueprintType)
enum class EGroundTerrainTypes : uint8
{
	// 10 Ground Blocks
	GBT_Soil UMETA(DisplayName = "Soil"),
	GBT_Stony_Soil UMETA(DisplayName = "Stony soil"),
	GBT_Rich_Soil UMETA(DisplayName = "Rich soil"),
	GBT_Marsh UMETA(DisplayName = "Marsh"),
	GBT_Mud UMETA(DisplayName = "Mud"),
	GBT_Sand UMETA(DisplayName = "Sand"),
	GBT_Rough_Stone UMETA(DisplayName = "Rough Stone"),
	GBT_Shallow_Water UMETA(DisplayName = "Shallow Water"),
	GBT_Deep_Water UMETA(DisplayName = "Deep Water"),
	GBT_Lava UMETA(DisplayName = "Lava"),
};

UENUM(BlueprintType)
enum class ETerrainSupport : uint8
{
	TS_None UMETA(DisplayName = "None"),
	TS_Light UMETA(DisplayName = "Light"),
	TS_Heavy UMETA(DisplayName = "Heavy"),
};

USTRUCT(BlueprintType)
struct FGroundTerrainType : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	FGroundTerrainType() {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info")
	EGroundTerrainTypes Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info")
	int32 MinElevation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info")
	int32 MaxElevation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info", meta = (ClampMin = "0.1", ClampMax = "1"))
	float ZScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info", meta=(ClampMin = "0", ClampMax = "1"))
	float MoveSpeedModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info", meta = (ClampMin = "0", ClampMax = "1"))
	float Fertility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info")
	ETerrainSupport TerrainSupport;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info")
	FColor WorldMapColour;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info")
	UStaticMesh* BlockStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info")
	UMaterialInterface* BlockMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info")
	int32 MaterialIndex;

	FTransform InstanceTransform;
	int32 InstancedStaticMeshIndex;
};

/**
 * 
 */
UCLASS(Blueprintable)
class THESPELLPLANES_API AGroundTerrain : public ABlock
{
	GENERATED_BODY()
	
public:
	AGroundTerrain();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundBlocks")
	EGroundTerrainTypes m_Type;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundBlocks", meta = (ClampMin = "0", ClampMax = "1"))
	float m_MoveSpeedModifier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundBlocks", meta = (ClampMin = "0", ClampMax = "1"))
	float m_Fertility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundBlocks")
	int32 m_MaterialIndex;
};
