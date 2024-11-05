// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "BaseMechanic.generated.h"

class UMechanicsComponent;

/**
 * 
 */
UCLASS(Blueprintable)
class PROCEDURALGENERATION_API UBaseMechanic : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UMechanicsComponent* NewMechanic);

	UFUNCTION(BlueprintNativeEvent, Category = "Mechanics") bool CanStartMechanic(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Mechanics") bool GetIsRunning() const;

	// Check if this mechanic can be started
	UFUNCTION(BlueprintNativeEvent, Category = "Mechanics")
	bool CanStart(AActor* Actor);

	bool StartByDefault = false;

	// Start Mechanic
	UFUNCTION(BlueprintNativeEvent, Category = "Mechanics")
	void StartMechanic(AActor* Actor);

	// Stop Mechanic
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Mechanics")
	void StopMechanic(AActor* Actor);

	UFUNCTION(BlueprintNativeEvent, Category = "Mechanics") void TickMechanic(float DeltaTime);

	// Event called when action is added
	UFUNCTION(BlueprintNativeEvent, Category = "Mechanics") void OnMechanicAdded(AActor* Actor);

	// Event called when action is removed
	UFUNCTION(BlueprintNativeEvent, Category = "Mechanics") void OnMechanicRemoved(AActor* Actor);

	// the gameplay tag associated with this mechanic
	UPROPERTY(EditAnywhere, Category = "Mechanics") FGameplayTag MechanicTag;

	//UFUNCTION(BlueprintNativeEvent, Category = "Mechanics") void PlayMontage(UAnimMontage* MontageToPlay);

protected:
	UPROPERTY() UMechanicsComponent* MechanicComponent;
	UPROPERTY(EditAnywhere, Category = "Animation") UAnimMontage* MechanicMontage;

	UFUNCTION(BlueprintCallable, Category = "Action") UMechanicsComponent* GetOwningComponent() const;

	// Tags to be given to MechanicComponent
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Tags") FGameplayTagContainer GivenTags;
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Tags") FGameplayTagContainer BlockedTags;

	//UPROPERTY(BlueprintReadWrite, Category = "Animation") UPAnimInstance* AnimInstance;

	UPROPERTY() bool bIsRunning;
	
	// Actor that owns mechanic
	UPROPERTY(BlueprintReadOnly) AActor* OwningActor;
};
