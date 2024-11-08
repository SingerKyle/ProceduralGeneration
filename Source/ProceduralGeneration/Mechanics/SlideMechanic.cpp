// Fill out your copyright notice in the Description page of Project Settings.

#include "SlideMechanic.h"

#include "Components/CapsuleComponent.h"
#include "ProceduralGeneration/ParkourCharacter.h"
#include "ProceduralGeneration/AnimationInstance/PAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProceduralGeneration/MechanicComponents/MechanicsComponent.h"

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

	if (TimelineCapsuleCurveFloat && TimelineMeshCurveFloat)
	{
		// Bind the timeline curve for Capsule to the UpdateCapsule function
		FOnTimelineFloat CapsuleTimelineProgress;
		CapsuleTimelineProgress.BindUFunction(this, FName("UpdateCapsule"));
		SlideCapsuleScaleTimeline.AddInterpFloat(TimelineCapsuleCurveFloat, CapsuleTimelineProgress);

		// Bind the timeline curve for Mesh to the UpdateMesh function
		FOnTimelineFloat MeshTimelineProgress;
		MeshTimelineProgress.BindUFunction(this, FName("UpdateMesh"));
		SlideMeshLocationTimeline.AddInterpFloat(TimelineMeshCurveFloat, MeshTimelineProgress);
	}

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

	// Check velocity of player to determine whether to slide or crouch.
	if (Player->GetMovementComponent()->Velocity.Length() > Player->GetPlayerWalkSpeed() && !Player->GetMovementComponent()->IsFalling())
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

	// Scale capsule component - timeline
	SlideCapsuleScaleTimeline.PlayFromStart();
	//Player->GetMesh()->SetRelativeLocation(FVector(0,0, -45));
	SlideMeshLocationTimeline.PlayFromStart();
	  
}

void USlideMechanic::StopSlide()
{
	if (bIsCrouching || bIsSliding)
	{
		UPAnimInstance* AnimInstance = Cast<UPAnimInstance>(Player->GetMesh()->GetAnimInstance());
		if (AnimInstance && MechanicMontage) // Ensure the anim instance and montage are valid
			{
			AnimInstance->Montage_StopWithBlendOut(0.2f ,MechanicMontage); // Play the montage
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("NOT Sliding!"));
			}
		SlideCapsuleScaleTimeline.Reverse();
		//Player->GetMesh()->SetRelativeLocation(FVector(0,0, -90));
		SlideMeshLocationTimeline.Reverse();
	}
}

void USlideMechanic::UpdateCapsule(float Value) const
{
	// GetCapsuleFull.X returns the height of the full player capsule - GetFullCapsule.Y returns the Radius of the full player capsule

	if (UCapsuleComponent* CapsuleComponent = Player->GetCapsuleComponent())
	{
		CapsuleComponent->SetCapsuleHalfHeight(FMath::Lerp(Player->GetCapsuleFull().Y, Player->GetCapsuleHalf().Y, Value));

		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Changing Capsule Height?!"));
	}
}

void USlideMechanic::UpdateMesh(float Value) const
{
	if (USkeletalMeshComponent* Mesh = Player->GetMesh())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Timeline Value: %f"), Value));
		
		// Get the current relative location of the mesh
		FVector CurrentLocation = Mesh->GetRelativeLocation();
		CurrentLocation.Z = Value;
        
		// Set the new location with the interpolated Z value
		Mesh->SetRelativeLocation(CurrentLocation);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Can't Get Mesh"));
	}
}

void USlideMechanic::StopMechanic_Implementation(AActor* Actor)
{
	Super::StopMechanic_Implementation(Actor);

	Player = Cast<AParkourCharacter>(Actor);
	UPAnimInstance* AnimInstance = Cast<UPAnimInstance>(Player->GetMesh()->GetAnimInstance());

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("STOP!"));
	AnimInstance->SetCrouching(false);
	AnimInstance->SetSliding(false);
	bIsCrouching = false;
	bIsSliding = false;

	StopSlide();
}

void USlideMechanic::TickMechanic_Implementation(float DeltaTime) // Replace with Async tick
{
	Super::TickMechanic_Implementation(DeltaTime);

	// if the timeline is active then tick:
	if (SlideCapsuleScaleTimeline.IsPlaying())
	{
		SlideCapsuleScaleTimeline.TickTimeline(DeltaTime);
	}
	if (SlideMeshLocationTimeline.IsPlaying())
	{
		SlideMeshLocationTimeline.TickTimeline(DeltaTime);
	}
	
	if (bIsSliding)
	{
		FHitResult SphereHitResult;
		FVector Start = Player->GetActorLocation();
		FVector End = Player->GetActorLocation();
		FCollisionQueryParams CollisionParams;

		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Tick?"));
		
		// Draw debug sphere for each trace in the loop
		DrawDebugCapsule(GetWorld(), Start, 4, 18, FQuat::Identity, FColor::Yellow, false, 5.0f);
		
		// Check if On Floor
		if (GetWorld()->SweepSingleByChannel(SphereHitResult, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeCapsule(4, 18), CollisionParams))
		{
			// 
		}
		else
		{
			// set sliding to false
			StopSlide();
		}
	}
	
}

bool USlideMechanic::CanStart_Implementation(AActor* Actor)
{
	return Super::CanStart_Implementation(Actor);
}
