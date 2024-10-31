// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "MechanicsComponent.generated.h"

// player class
class AParkourCharacter;
class UBaseMechanic;

// component which will contain all UObjects related to game mechanics
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROCEDURALGENERATION_API UMechanicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMechanicsComponent();

	// Get component from Actor
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mechanics") static UMechanicsComponent* GetMechanicsComponent(AActor* GetActor);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayTags") FGameplayTagContainer GetActiveTags();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayTags") FGameplayTagContainer GetBlockedTags();
	FGameplayTag& GetMasterBlockedTag();

	// Get a mechanic from given mechanic class
	UFUNCTION(BlueprintCallable, Category = "Mechanics") UBaseMechanic* GetMechanic(TSubclassOf<UBaseMechanic> Mechanic);

	// Adds an action to the component
	UFUNCTION(BlueprintCallable, Category = "GameplayTags") void AddMechanic(AActor* Actor, TSubclassOf<UBaseMechanic> Mechanic, bool bAutoAdded = false);

	// Remove an action from component
	UFUNCTION(BlueprintCallable, Category = "GameplayTags") void RemoveMechanic(UBaseMechanic* MechanicToRemove);

	UFUNCTION(BlueprintCallable, Category = "Mechanics") bool StartMechanic(AActor* Actor, FGameplayTag MechanicTag);

	UFUNCTION(BlueprintCallable, Category = "Mechanics") bool StopMechanic(AActor* Actor, FGameplayTag MechanicTag);

	bool DoesTagExist(FGameplayTag Tag);

protected:

	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Array of actions available from the start of play
	UPROPERTY(EditAnywhere, Category = "Mechanics") TArray<TSubclassOf<UBaseMechanic>> StartingMechanics;

	// add array of all mechanics
	UPROPERTY(BlueprintReadWrite, Category = "Mechanics") TArray<UBaseMechanic*> Mechanics;

	/* ---------------------------------  Gameplay Tags --------------------------------- */

	// gameplay tags to determine when actions are blocked.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Tags") FGameplayTagContainer BlockedGameplayTags;

	// current active gameplay tags
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Tags") FGameplayTagContainer ActiveGameplayTags;

	// list of tags to determine if a mechanic can play
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Tags")
	FGameplayTagContainer BlockTags;

	// Tag that blocks all mechanics - if stunned for example
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Tags")
	FGameplayTag MasterBlockTag;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	
};
