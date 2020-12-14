// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Tools/LevelEditorUtilities.h"
#include "BlockUtilities.h"
#include "Block.generated.h"


UCLASS(Abstract, NotBlueprintable)
class THESPELLPLANES_API ABlock : public AActor
{
	GENERATED_BODY()
	
public:	
	ABlock();

	// The Block Id
	UPROPERTY(EditAnywhere, Category = "Block Data")
	int16 m_BlockId;

	// The Location (index) in the GridId
	UPROPERTY(EditAnywhere, Category = "Block Data")
	EBlockIdIndex BlockIdIndex;

	// Member Variables
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* BlockStaticMeshComponent;

	UPROPERTY()
	UStaticMesh* m_BlockStaticMesh;

	UPROPERTY()
	UMaterialInterface* m_BlockMaterial;

	/** The display name for this block */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText BlockDisplayName;

	/** The description for this block */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText BlockDisplayDescription;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
