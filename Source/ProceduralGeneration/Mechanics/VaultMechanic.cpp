// Fill out your copyright notice in the Description page of Project Settings.


#include "VaultMechanic.h"
#include "ProceduralGeneration/ParkourCharacter.h"
#include "ProceduralGeneration/AnimationInstance/PAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProceduralGeneration/MechanicComponents/MechanicsComponent.h"

void UVaultMechanic::OnMechanicAdded_Implementation(AActor* Actor)
{
	Super::OnMechanicAdded_Implementation(Actor);

	Player = Cast<AParkourCharacter>(Actor);
	MovementComp = Player->GetCharacterMovement<UMovementComponent>();

	bIsRunning = false;
}

void UVaultMechanic::OnMechanicRemoved_Implementation(AActor* Actor)
{
	Super::OnMechanicRemoved_Implementation(Actor);
}

void UVaultMechanic::StartMechanic_Implementation(AActor* Actor)
{
	Super::StartMechanic_Implementation(Actor);

	


	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Crouching and Sliding!"));
}

void UVaultMechanic::StopMechanic_Implementation(AActor* Actor)
{
	Super::StopMechanic_Implementation(Actor);

	
}

void UVaultMechanic::TickMechanic_Implementation(float DeltaTime)
{

}

bool UVaultMechanic::CanStart_Implementation(AActor* Actor)
{
	return Super::CanStart_Implementation(Actor);
}
