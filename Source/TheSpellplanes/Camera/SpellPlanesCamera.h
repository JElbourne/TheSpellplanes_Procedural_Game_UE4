// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "SpellPlanesCamera.generated.h"

UCLASS()
class THESPELLPLANES_API ASpellPlanesCamera : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpellPlanesCamera();

	class USceneComponent* ActorRoot;

	UPROPERTY(EditAnywhere, Category = "Components")
	class USceneComponent* CameraRoot;
	
	UPROPERTY(EditAnywhere, Category="Components")
	class USpringArmComponent* SwingArmComponent;
	
	UPROPERTY(EditAnywhere, Category = "Components")
	class UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, Category = "Components")
	class UStaticMeshComponent* BackdropMeshComponent;

	// Camera Move Functionality
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RotateCamera")
	bool m_IsMoving;
	UPROPERTY(EditAnywhere, Category = "MoveCamera")
	UCurveFloat* m_CameraMoveCurve;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MoveCamera")
	FVector m_NewLocation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MoveCamera")
	FVector m_StartLocation;

	UTimelineComponent* m_CameraMoveTimeline;

	UFUNCTION(BlueprintCallable, Category = "MoveCamera")
	void MoveCamera(FVector NextLocation);

	UFUNCTION()
	void ControlCameraMove();

	UFUNCTION()
	void FinishCameraMove();


	// Camera Rotate Functionality
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RotateCamera")
	bool IsRotating;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RotateCamera")
	float CameraYaw;
	FRotator InitialRotation;
	float RotationIncrement;
	float CurrentRotationAmount;
	UTimelineComponent* CameraRotateTimeline;

	UPROPERTY(EditAnywhere, Category = "RotateCamera")
	UCurveFloat* m_CameraRotateCurve;

	UFUNCTION()
	void RotateCamera(float Dir);

	UFUNCTION()
	void ControlCameraRotate();

	UFUNCTION()
	void FinishCameraRotate();

	UFUNCTION()
	float NormalizeYawValue(float RawYaw);

	UFUNCTION()
	float RestrictYawValue(float NormalYaw);

	UFUNCTION()
	void SetFinalCameraRotation(FRotator NewRotation);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//void OnHitBackdrop(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


};
