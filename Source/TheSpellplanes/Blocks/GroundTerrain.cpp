// Fill out your copyright notice in the Description page of Project Settings.


#include "GroundTerrain.h"

AGroundTerrain::AGroundTerrain()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BlockIdIndex = EBlockIdIndex::BII_GroundType;
	//SetReplicates(true);
}
