// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "InteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginInteract, class ASpellPlanesCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndInteract, class ASpellPlanesCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginFocus, class ASpellPlanesCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndFocus, class ASpellPlanesCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteract, class ASpellPlanesCharacter*, Character);

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THESPELLPLANES_API UInteractionComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FText InteractableNameText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FText InteractableActionText;

	// Whether we allow multiple players to interact with the item, or just one at any given time.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bAllowMultipleInteractors;

	// Call this to change the name of the interactable. Will also refresh the interaction widget.
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractableNameText(const FText& NewNameText);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractableActionText(const FText& NewActionText);

	//Delegates
	//
	// [local + server]
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginInteract OnBeginInteract;
	// [local + server]
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnEndInteract OnEndInteract;
	// [local + server]
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginFocus OnBeginFocus;
	// [local + server]
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnEndFocus OnEndFocus;
	// [local + server]
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnInteract OnInteract;

protected:

	// Called when the game starts
	virtual void Deactivate() override;

	bool CanInteract(class ASpellPlanesCharacter* Character) const;

	// ON the server, this will hold all interactors. On the local player, this will just hold the local player (Provided they are an interactor)
	UPROPERTY()
	TArray<class ASpellPlanesCharacter*> Interactors;

public:

	/***Refresh the interaction widget and it's custom widgets.
	An example of when we would use this is when we take 3 items out of a stack of 10, and we need to update the widget
	so it shows the stack having 7 items left. */
	void RefreshWidget();

	// Called on the client when the players interaction check trace begins/ends hitting this item.
	void BeginFocus(class ASpellPlanesCharacter* Character);
	void EndFocus(class ASpellPlanesCharacter* Character);

	// Called on the client when the player begins/ends interaction with the item
	void BeginInteract(class ASpellPlanesCharacter* Character);
	void EndInteract(class ASpellPlanesCharacter* Character);

	void Interact(class ASpellPlanesCharacter* Character);

	// Return a value from 0-1 denoting how far through the interact we are.
	// On Server this is the first interactor's percentage, on client this is the local interactor's percentage.
	UFUNCTION(BlueprintPure, Category = "Interaction")
	float GetInteractionPercentage();


	
};
