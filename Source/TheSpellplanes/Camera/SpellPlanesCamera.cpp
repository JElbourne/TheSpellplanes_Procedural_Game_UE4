// Fill out your copyright notice in the Description page of Project Settings.


#include "SpellPlanesCamera.h"
#include "Components/TimelineComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Player/SpellPlanesCharacter.h"

// Sets default values
ASpellPlanesCamera::ASpellPlanesCamera()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ActorRoot = CreateDefaultSubobject<USceneComponent>("ActorRoot");
	SetRootComponent(ActorRoot);
	
	CameraRoot = CreateDefaultSubobject<USceneComponent>("CameraRoot");
	CameraRoot->SetupAttachment(ActorRoot);
	CameraRoot->SetRelativeLocation(FVector(-500.f, -500.f, 0.f));

	SwingArmComponent = CreateDefaultSubobject<USpringArmComponent>("SwingArmComponent");
	SwingArmComponent->SetupAttachment(CameraRoot);
	SwingArmComponent->SetRelativeRotation(FRotator(-40.0f, 45.0f, 0.f));
	SwingArmComponent->bDoCollisionTest = 0;
	SwingArmComponent->TargetArmLength = 3000.f;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(SwingArmComponent);
	CameraComponent->SetFieldOfView(60.f);

	BackdropMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Backdrop Mesh");
	BackdropMeshComponent->SetupAttachment(SwingArmComponent);
	BackdropMeshComponent->SetRelativeLocation(FVector(10000.f, 0.f, 0.f));
	BackdropMeshComponent->SetRelativeRotation(FRotator(130.f, 0.f, 0.f));
	BackdropMeshComponent->SetRelativeScale3D(FVector(175.f, 175.f, 175.f));
	BackdropMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BackdropMeshComponent->SetCastShadow(false);
	BackdropMeshComponent->SetEnableGravity(false);
	auto BackdropMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/TheSpellplanes/Assets/SP_Plane_StaticMesh.SP_Plane_StaticMesh'"));
	if (BackdropMesh.Object != nullptr)
	{
		BackdropMeshComponent->SetStaticMesh(BackdropMesh.Object);
	}

	m_IsMoving = false;

	// Camera Rotate functionality
	IsRotating = false;
	RotationIncrement = 90.f;
	CameraYaw = NormalizeYawValue(GetActorRotation().Yaw);

	auto CameraMoveCurve = ConstructorHelpers::FObjectFinder<UCurveFloat>(TEXT("CurveFloat'/Game/TheSpellplanes/Blueprints/TimelineCurves/CameraMoveCurve.CameraMoveCurve'"));
	if (CameraMoveCurve.Object != nullptr)
	{
		m_CameraMoveCurve = CameraMoveCurve.Object;
	}
	auto CameraRotateCurve = ConstructorHelpers::FObjectFinder<UCurveFloat>(TEXT("CurveFloat'/Game/TheSpellplanes/Blueprints/TimelineCurves/CameraRotateCurve.CameraRotateCurve'"));
	if (CameraRotateCurve.Object != nullptr)
	{
		m_CameraRotateCurve = CameraRotateCurve.Object;
	}
}

// Called when the game starts or when spawned
void ASpellPlanesCamera::BeginPlay()
{
	Super::BeginPlay();

	if (m_CameraRotateCurve != NULL)
	{
		FOnTimelineFloat TimelineCallback;
		FOnTimelineEventStatic TimelineFinishedCallback;

		TimelineCallback.BindUFunction(this, FName("ControlCameraRotate"));
		TimelineFinishedCallback.BindUFunction(this, FName("FinishCameraRotate"));

		CameraRotateTimeline = NewObject<UTimelineComponent>(this, FName("Camera Rotate Animation"));
		CameraRotateTimeline->AddInterpFloat(m_CameraRotateCurve, TimelineCallback);
		CameraRotateTimeline->SetTimelineFinishedFunc(TimelineFinishedCallback);
		CameraRotateTimeline->RegisterComponent();
	}

	if (m_CameraMoveCurve != NULL)
	{
		FOnTimelineFloat TimelineMoveCallback;
		FOnTimelineEventStatic TimelineMoveFinishedCallback;

		TimelineMoveCallback.BindUFunction(this, FName("ControlCameraMove"));
		TimelineMoveFinishedCallback.BindUFunction(this, FName("FinishCameraMove"));

		m_CameraMoveTimeline = NewObject<UTimelineComponent>(this, FName("Camera Move Animation"));
		m_CameraMoveTimeline->AddInterpFloat(m_CameraMoveCurve, TimelineMoveCallback);
		m_CameraMoveTimeline->SetTimelineFinishedFunc(TimelineMoveFinishedCallback);
		m_CameraMoveTimeline->RegisterComponent();
	}
	
}

// Called every frame
void ASpellPlanesCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CameraRotateTimeline != NULL)
	{
		CameraRotateTimeline->TickComponent(DeltaTime, ELevelTick::LEVELTICK_TimeOnly, NULL);
	}
	if (m_CameraMoveTimeline != NULL)
	{
		m_CameraMoveTimeline->TickComponent(DeltaTime, ELevelTick::LEVELTICK_TimeOnly, NULL);
	}
}


void ASpellPlanesCamera::MoveCamera(FVector NextLocation)
{
	m_IsMoving = true;
	m_NewLocation = NextLocation;
	m_StartLocation = GetActorLocation();
	m_CameraMoveTimeline->PlayFromStart();
}

void ASpellPlanesCamera::ControlCameraMove()
{
	float TimelineValue = m_CameraMoveTimeline->GetPlaybackPosition();
	float CurveMovePercent = m_CameraMoveCurve->GetFloatValue(TimelineValue);
	FVector CurrentLocation = FMath::Lerp(m_StartLocation, m_NewLocation, CurveMovePercent);
	SetActorLocation(CurrentLocation);
}

void ASpellPlanesCamera::FinishCameraMove()
{
	m_IsMoving = false;
	// Nothing needs to happen when CameraMove is complete at this time.
}

void ASpellPlanesCamera::RotateCamera(float Dir)
{
	if (IsRotating) {
		CameraRotateTimeline->Stop();
		SetFinalCameraRotation(FRotator(0.0f, CameraYaw, 0.0f));
	}
	else {
		IsRotating = true;
		InitialRotation = GetActorRotation();
		CurrentRotationAmount = RotationIncrement * Dir;
		float GoalYaw = InitialRotation.Yaw + CurrentRotationAmount;
		CameraYaw = NormalizeYawValue(GoalYaw);
		CameraRotateTimeline->PlayFromStart();
	}
}

void ASpellPlanesCamera::ControlCameraRotate()
{
	float TimelineValue = CameraRotateTimeline->GetPlaybackPosition();
	float CurveRotateValue = CurrentRotationAmount * m_CameraRotateCurve->GetFloatValue(TimelineValue);

	FRotator NewRotation = FRotator(0.0f, CurveRotateValue, 0.0f);
	FRotator CombinedRotators = InitialRotation + NewRotation;

	this->SetActorRotation(CombinedRotators);
}

void ASpellPlanesCamera::FinishCameraRotate()
{
	float CurrentYaw = NormalizeYawValue(GetActorRotation().Yaw);
	if ((CurrentYaw - CameraYaw) < 5.f) {
		FRotator NewRotation = FRotator(0.0f, CameraYaw, 0.0f);
		SetFinalCameraRotation(NewRotation);
	}
}

float ASpellPlanesCamera::NormalizeYawValue(float RawYaw)
{
	if (RawYaw > 360.f) {
		return RestrictYawValue(abs(RawYaw) - 360.f);
	}
	else if (RawYaw == 360.f) {
		return 0.f;
	}
	else if (RawYaw < 0.f) {
		return RestrictYawValue(RawYaw + 360.f);
	}
	return RestrictYawValue(abs(RawYaw));
}

float ASpellPlanesCamera::RestrictYawValue(float NormalYaw)
{
	if (NormalYaw > 45.f && NormalYaw <= 135.f) {
		return 90.f;
	}
	else if (NormalYaw > 135.f && NormalYaw <= 225.f) {
		return 180.f;
	}
	else if (NormalYaw > 205.f && NormalYaw <= 315.f) {
		return 270.f;
	}
	return 0.f;
}

void ASpellPlanesCamera::SetFinalCameraRotation(FRotator NewRotation)
{
	this->SetActorRotation(NewRotation);
	IsRotating = false;
}

