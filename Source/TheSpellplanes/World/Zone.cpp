// Fill out your copyright notice in the Description page of Project Settings.


#include "Zone.h"
#include "WorldUtilities.h"

// Sets default values
AZone::AZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);
	
	ActorRoot = CreateDefaultSubobject<USceneComponent>("ActorRoot");
	SetRootComponent(ActorRoot);

	TerrainBlocksComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Terrain Block Instances"));
	TerrainBlocksComponent->SetupAttachment(ActorRoot);

	MountainBlocksComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Mountain Block Instances"));
	MountainBlocksComponent->SetupAttachment(ActorRoot);

	TreeBlocksComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Tree Block Instances"));
	TreeBlocksComponent->SetupAttachment(ActorRoot);

	// We need to init the Custom Data array with a length before we can use it.
	TerrainBlocksComponent->NumCustomDataFloats = 3;
	MountainBlocksComponent->NumCustomDataFloats = 3;
	TreeBlocksComponent->NumCustomDataFloats = 3;

	auto TerrainMeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/TheSpellplanes/Assets/BaseBlock_StaticMesh.BaseBlock_StaticMesh'"));
	// Check if the MeshAsset is valid before setting it
	if (TerrainMeshAsset.Object != nullptr)
	{
		TerrainBlocksComponent->SetStaticMesh(TerrainMeshAsset.Object);
	}

	auto MountainMeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/TheSpellplanes/Assets/Meshes/Mountain_001.Mountain_001'"));
	// Check if the MeshAsset is valid before setting it
	if (MountainMeshAsset.Object != nullptr)
	{
		MountainBlocksComponent->SetStaticMesh(MountainMeshAsset.Object);
	}

	auto TreeMeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/TheSpellplanes/Assets/Meshes/Tree_003.Tree_003'"));
	// Check if the MeshAsset is valid before setting it
	if (TreeMeshAsset.Object != nullptr)
	{
		TreeBlocksComponent->SetStaticMesh(TreeMeshAsset.Object);
	}
}

// Called when the game starts or when spawned
void AZone::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AZone::AddTerrainInstance(FVector LocalLocation, int32 GroundId, int32 ModifierId)
{
	// TODO utilize the Modifier ID in the future.
	UDataTable* GroundTerrainTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), NULL, TEXT("DataTable'/Game/TheSpellplanes/Data/GroundTerrain.GroundTerrain'")));
	if (GroundTerrainTable)
	{
		static const FString ContextString(TEXT("GroundTerrainTypes"));
		TArray<FName> RowNames = GroundTerrainTable->GetRowNames();
		if (RowNames.IsValidIndex(GroundId))
		{
			if (FGroundTerrainType* GroundTerrainType = GroundTerrainTable->FindRow<FGroundTerrainType>(RowNames[GroundId], ContextString, true))
			{
				FTransform InstanceTransform;
				InstanceTransform.SetLocation(LocalLocation);
				InstanceTransform.SetScale3D(FVector(1.f, 1.f, GroundTerrainType->ZScale));
				GroundTerrainType->InstanceTransform = InstanceTransform;

				int32 InstanceIndex = TerrainBlocksComponent->AddInstance(InstanceTransform);
				TArray<float> CustomInstanceData;
				CustomInstanceData.Add(((GroundTerrainType->MaterialIndex % 16) * 0.0625f)); // Texture Tile Location X
				CustomInstanceData.Add(((GroundTerrainType->MaterialIndex / 16) * 0.0625f)); // Texture Tile location Y
				CustomInstanceData.Add(1.0f); // Opacity

				TerrainBlocksComponent->SetCustomData(InstanceIndex, CustomInstanceData);

				GroundTerrainType->InstancedStaticMeshIndex = InstanceIndex;
				GroundTerrainTileMap.Add(LocalLocation, *GroundTerrainType);
			}
		}
	}
}

void AZone::AddResourceInstance(FVector LocalLocation, int32 ResourceId, int32 ModifierId)
{
	UDataTable* ResourceBlockTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), NULL, TEXT("DataTable'/Game/TheSpellplanes/Data/ResourceBlocks.ResourceBlocks'")));
	if (ResourceBlockTable)
	{
		static const FString ContextString(TEXT("ResourceBlockTypes"));
		TArray<FName> RowNames = ResourceBlockTable->GetRowNames();
		if (RowNames.IsValidIndex(ResourceId))
		{
			if (FResourceBlockType* ResourceBlockType = ResourceBlockTable->FindRow<FResourceBlockType>(RowNames[ResourceId], ContextString, true))
			{
				TArray<float> CustomInstanceData;
				CustomInstanceData.Add(((ResourceBlockType->MaterialIndex % 16) * 0.0625f));
				CustomInstanceData.Add(((ResourceBlockType->MaterialIndex / 16) * 0.0625f));
				CustomInstanceData.Add(1.0f); // Opacity

				FTransform InstanceTransform;
				InstanceTransform.SetLocation(LocalLocation);

				if ((int8)(ResourceBlockType->BlockType) <= 10) // Rock Resource
				{
					int32 InstanceIndex = MountainBlocksComponent->AddInstance(InstanceTransform);
					MountainBlocksComponent->SetCustomData(InstanceIndex, CustomInstanceData);
					ResourceBlockType->InstancedStaticMeshIndex = InstanceIndex;
					MountainResourceTileMap.Add(LocalLocation, *ResourceBlockType);
				}
				else if ((int8)(ResourceBlockType->BlockType) <= 20) // Tree Resource
				{
					int32 InstanceIndex = TreeBlocksComponent->AddInstance(InstanceTransform);
					TreeBlocksComponent->SetCustomData(InstanceIndex, CustomInstanceData);
					ResourceBlockType->InstancedStaticMeshIndex = InstanceIndex;
					TreeResourceTileMap.Add(LocalLocation, *ResourceBlockType);
				}
			}
		}
	}
}

