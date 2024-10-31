// Fill out your copyright notice in the Description page of Project Settings.

#include "SlideMechanic.h"
#include "ProceduralGeneration/ParkourCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProceduralGeneration/PGameTags.h"

USlideMechanic::USlideMechanic()
{
	MechanicTag = PGameTags::Mechanics_Slide.GetTag();
}

void USlideMechanic::OnMechanicAdded_Implementation(AActor* Actor)
{
	Super::OnMechanicAdded_Implementation(Actor);

	Player = Cast<AParkourCharacter>(Actor);
	MovementComp = Player->GetCharacterMovement<UMovementComponent>();

	IsCrouching = false;
	IsSliding = false;
	IsRunning = false;

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Added!"));
}

void USlideMechanic::OnMechanicRemoved_Implementation(AActor* Actor)
{
	Super::OnMechanicRemoved_Implementation(Actor);
}

void USlideMechanic::StartMechanic_Implementation(AActor* Actor)
{
	Super::StartMechanic_Implementation(Actor);

	IsCrouching = true;
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Crouching and Sliding!"));
}

void USlideMechanic::StopMechanic_Implementation(AActor* Actor)
{
	Super::StopMechanic_Implementation(Actor);


}

void USlideMechanic::TickMechanic_Implementation(float DeltaTime)
{
	Super::TickMechanic_Implementation(DeltaTime);


}

bool USlideMechanic::CanStart_Implementation(AActor* Actor)
{
	return false;
}
