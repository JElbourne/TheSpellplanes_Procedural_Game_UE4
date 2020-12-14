// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blocks/GroundTerrain.h"
#include "Blocks/ResourceBlock.h"
#include "Components/SceneComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Zone.generated.h"

UENUM(BlueprintType)
enum class EZoneVisibilityLevel : uint8
{
	ZVL_Hidden,
	ZVL_FOG,
	ZVL_Visible,
};

UCLASS()
class THESPELLPLANES_API AZone : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AZone();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Components")
	class USceneComponent* ActorRoot;
	UPROPERTY(EditAnywhere, Category = "Components")
	class UInstancedStaticMeshComponent* TerrainBlocksComponent;
	UPROPERTY(EditAnywhere, Category = "Components")
	class UInstancedStaticMeshComponent* MountainBlocksComponent;
	UPROPERTY(EditAnywhere, Category = "Components")
	class UInstancedStaticMeshComponent* TreeBlocksComponent;

	UFUNCTION()
	void AddTerrainInstance(FVector LocalLocation, int32 GroundId, int32 ModifierId);

	UFUNCTION()
	void AddResourceInstance(FVector LocalLocation, int32 ResourceId, int32 ModifierId);

private:
	TMap<FVector, FGroundTerrainType> GroundTerrainTileMap;
	TMap<FVector, FResourceBlockType> MountainResourceTileMap;
	TMap<FVector, FResourceBlockType> TreeResourceTileMap;


};
