// Fill out your copyright notice in the Description page of Project Settings.

#include "SlideMechanic.h"
#include "ProceduralGeneration/ParkourCharacter.h"
#include "ProceduralGeneration/AnimationInstance/PAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProceduralGeneration/MechanicComponents/MechanicsComponent.h"
#include "ProceduralGeneration/PGameTags.h"

USlideMechanic::USlideMechanic()
{
	//MechanicTag = PGameTags::Mechanics_Slide.GetTag();
}

void USlideMechanic::OnMechanicAdded_Implementation(AActor* Actor)
{
	Super::OnMechanicAdded_Implementation(Actor);

	Player = Cast<AParkourCharacter>(Actor);
	MovementComp = Player->GetCharacterMovement<UMovementComponent>();

	bIsRunning = false;
	bIsCrouching = false;
	bIsSliding = false;

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Added!"));
}

void USlideMechanic::OnMechanicRemoved_Implementation(AActor* Actor)
{
	Super::OnMechanicRemoved_Implementation(Actor);
}

void USlideMechanic::StartMechanic_Implementation(AActor* Actor)
{

	Super::StartMechanic_Implementation(Actor);

	Player = Cast<AParkourCharacter>(Actor);
	UPAnimInstance* AnimInstance = Cast<UPAnimInstance>(Player->GetMesh()->GetAnimInstance());

	// Check velocity on start - move these to own variables later
	if (Player->GetSprinting())
	{
		AnimInstance->SetSliding(true);
		Player->SwitchMovementState(EMovementState::SLIDING);
		StartSlide();
	}
	// If velocity > RUNSPEED value then set slide bool
	else
	{
		AnimInstance->SetCrouching(true);
		Player->SwitchMovementState(EMovementState::CROUCHING);
		bIsCrouching = true;
	}

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Crouching and Sliding!"));
}

void USlideMechanic::StartSlide()
{
	bIsSliding = true;

	GetOwningComponent()->GetActiveTags().AppendTags(SlidingTags);

	UPAnimInstance* AnimInstance = Cast<UPAnimInstance>(Player->GetMesh()->GetAnimInstance());
	if (AnimInstance && MechanicMontage) // Ensure the anim instance and montage are valid
	{
		AnimInstance->Montage_Play(MechanicMontage); // Play the montage
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Sliding!"));
	}
}

void USlideMechanic::StopMechanic_Implementation(AActor* Actor)
{
	Super::StopMechanic_Implementation(Actor);

	Player = Cast<AParkourCharacter>(Actor);
	UPAnimInstance* AnimInstance = Cast<UPAnimInstance>(Player->GetMesh()->GetAnimInstance());

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("STOP!"));
	AnimInstance->SetCrouching(false);
	AnimInstance->SetSliding(false);
	bIsCrouching = false;
	bIsSliding = false;
}

void USlideMechanic::TickMechanic_Implementation(float DeltaTime) // Replace with Async tick
{
	Super::TickMechanic_Implementation(DeltaTime);

	// if the player velocity goes to below the and the player is sliding then set crouching

}

bool USlideMechanic::CanStart_Implementation(AActor* Actor)
{
	return Super::CanStart_Implementation(Actor);
}
