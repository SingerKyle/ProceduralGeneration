// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseMechanic.h"
#include "VaultMechanic.generated.h"

class AParkourCharacter;
class UMovementComponent;

/**
 * 
 */
UCLASS()
class PROCEDURALGENERATION_API UVaultMechanic : public UBaseMechanic
{
	GENERATED_BODY()
	
public:
	virtual void OnMechanicAdded_Implementation(AActor* Actor) override;
	virtual void OnMechanicRemoved_Implementation(AActor* Actor) override;

	//virtual void Tick_Implementation

	virtual void StartMechanic_Implementation(AActor* Actor) override;
	virtual void StopMechanic_Implementation(AActor* Actor) override;
	virtual void TickMechanic_Implementation(float DeltaTime) override;
	virtual bool CanStart_Implementation(AActor* Actor) override;

protected:
	UPROPERTY() AParkourCharacter* Player;

	UPROPERTY() UMovementComponent* MovementComp;

	UPROPERTY(EditAnywhere, Category = "Tags") FGameplayTagContainer VaultTags;
};
