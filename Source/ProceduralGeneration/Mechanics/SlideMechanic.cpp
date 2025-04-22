// Fill out your copyright notice in the Description page of Project Settings.

#include "SlideMechanic.h"

#include "Components/CapsuleComponent.h"
#include "ProceduralGeneration/ParkourCharacter.h"
#include "ProceduralGeneration/AnimationInstance/PAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProceduralGeneration/MechanicComponents/MechanicsComponent.h"

USlideMechanic::USlideMechanic()
{
	SlideSpeed = 750.f;
}

void USlideMechanic::OnMechanicAdded_Implementation(AActor* Actor)
{
	Super::OnMechanicAdded_Implementation(Actor);

	Player = Cast<AParkourCharacter>(Actor);
	AnimInstance = Cast<UPAnimInstance>(Player->GetMesh()->GetAnimInstance());
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
	
	if (AnimInstance && MechanicMontage) // Ensure the anim instance and montage are valid
	{
		AnimInstance->Montage_Play(MechanicMontage); // Play the montage
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Sliding!"));
	}

	// Scale capsule component - timeline
	SlideCapsuleScaleTimeline.PlayFromStart();
	//Player->GetMesh()->SetRelativeLocation(FVector(0,0, -45));
	SlideMeshLocationTimeline.PlayFromStart();
	  
}

void USlideMechanic::StopSlide()
{
	if (AnimInstance && EndSlideMontage && MechanicMontage) // Ensure the anim instance and montage are valid
		{
			AnimInstance->Montage_StopWithBlendOut(0.2f ,MechanicMontage); // Play the montage
		if(bIsCrouching)
		{
			Player->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		}
		else
		{
			AnimInstance->Montage_Play(EndSlideMontage);
		}
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("NOT Sliding!"));
		}
	
	SlideCapsuleScaleTimeline.Reverse();
	Player->GetMesh()->SetRelativeRotation(FRotator(0,-90, 0));
	SlideMeshLocationTimeline.Reverse();
}

void USlideMechanic::UpdateCapsule(float Value) const
{
	// GetCapsuleFull.X returns the height of the full player capsule - GetFullCapsule.Y returns the Radius of the full player capsule

	if (UCapsuleComponent* CapsuleComponent = Player->GetCapsuleComponent())
	{
		CapsuleComponent->SetCapsuleHalfHeight(FMath::Lerp(Player->GetCapsuleFull().Y, Player->GetCapsuleHalf().Y, Value));

		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Changing Capsule Height?!"));
	}
}

void USlideMechanic::UpdateMesh(float Value) const
{
	if (USkeletalMeshComponent* Mesh = Player->GetMesh())
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Timeline Value: %f"), Value));
		
		// Get the current relative location of the mesh
		FVector CurrentLocation = Mesh->GetRelativeLocation();
		CurrentLocation.Z = Value;
        
		// Set the new location with the interpolated Z value
		Mesh->SetRelativeLocation(CurrentLocation);
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Can't Get Mesh"));
	}
}

void USlideMechanic::SetSlideVelocity(FVector NewSlideVelocity)
{
	SlideVelocity = NewSlideVelocity;
}

void USlideMechanic::CheckShouldContinueSlide()
{
	if(bIsSliding)
	{
		const FHitResult& Floor = Player->GetCharacterMovement()->CurrentFloor.HitResult;
		const float SlideAngle = GetSlideSlope(Floor.Normal);

		if(FMath::IsNearlyZero(SlideAngle))
		{
			Player->GetCharacterMovement()->AddImpulse(Player->GetCharacterMovement()->Velocity.GetSafeNormal() * SlideSpeed);
		}
		else if (Player->GetVelocity().Length() != 0)
		{
			const float ActualSlideForce = SlideAngle > 1.6f ? SlideSpeed / SlideAngle : SlideSpeed;
			
			Player->GetCharacterMovement()->AddImpulse(Player->GetCharacterMovement()->Velocity.GetSafeNormal() * ActualSlideForce);
		}
		else
		{
			StopSlide();
		}

		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Speed - %f"), Player->GetCharacterMovement()->Velocity.Length()));
	}
}

float USlideMechanic::GetSlideSlope(const FVector& FloorNormal)
{
	const float AngleRadians = FMath::Acos(FVector::DotProduct(FloorNormal, Player->GetVelocity()));
	return FMath::RadiansToDegrees(AngleRadians);
}

void USlideMechanic::StopMechanic_Implementation(AActor* Actor)
{
	Player = Cast<AParkourCharacter>(Actor);
	if(Player->GetCharacterMovement()->IsFalling())
	{
		return;
	}
	
	Super::StopMechanic_Implementation(Actor);

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("STOP!"));
	AnimInstance->SetCrouching(false);
	AnimInstance->SetSliding(false);

	StopSlide();
	
	bIsCrouching = false;
	bIsSliding = false;
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
		FVector Start = Player->GetMesh()->GetComponentTransform().GetLocation();
		FVector End = Player->GetMesh()->GetComponentTransform().GetLocation();
		FCollisionQueryParams CollisionParams;
		
		// Draw debug sphere for each trace in the loop
		//DrawDebugCapsule(GetWorld(), Start, 4, 18, FQuat::Identity, FColor::Yellow, false, 5.0f);
		
		// Check if On Floor
		if (GetWorld()->SweepSingleByChannel(SphereHitResult, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeCapsule(4, 18), CollisionParams))
		{
			// 
		}
		else
		{
			// set sliding to false
			StopMechanic_Implementation(Player);
			//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("Stop slide!"));
		}

		CheckShouldContinueSlide();
		
		// Align player to floor while sliding
		if (Player && bIsSliding)
		{
			
			// Get floor hit result
			const FHitResult HitResult = Player->GetCharacterMovement()->CurrentFloor.HitResult;
			
			if(Player->GetCharacterMovement()->GetMovementName() == "Walking")
			{
				GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("Walking!"));
			}
			
			// Ensure the normal is valid (i.e., the character is on the ground)
			if(HitResult.IsValidBlockingHit())
			{
				//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("Hit Floor!"));

				// Get Normal for the floor
				const FVector FloorNorm = HitResult.ImpactNormal;

				const FRotator TargetRotation = FRotationMatrix::MakeFromXZ(Player->GetMesh()->GetForwardVector(), FloorNorm).Rotator();
				const FRotator FinalTargetRotation = FRotator(TargetRotation.Pitch, Player->GetMesh()->GetRelativeRotation().Yaw, TargetRotation.Roll);
				//const FRotator FinalTargetRotation = FRotator(TargetRotation.Pitch, Player->GetMesh()->GetRelativeRotation().Yaw, TargetRotation.Roll);

				// Apply smooth rotation
				Player->GetMesh()->SetRelativeRotation(FMath::RInterpTo(Player->GetMesh()->GetRelativeRotation(), FinalTargetRotation, DeltaTime, 3.f));
				//GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, TEXT("Set Rotation!"));
				
				//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, FString::Printf(TEXT("ActorRotation - Pitch: %f  Roll: %f  Yaw: %f"), Player->GetActorRotation().Pitch, Player->GetActorRotation().Roll, Player->GetActorRotation().Yaw));
				//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, FString::Printf(TEXT("TargetRotation - Pitch: %f  Roll: %f  Yaw: %f"), TargetRotation.Pitch, TargetRotation.Roll, TargetRotation.Yaw));
				//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, FString::Printf(TEXT("FinalTargetRotation - Pitch: %f  Roll: %f  Yaw: %f"), FinalTargetRotation.Pitch, FinalTargetRotation.Roll, FinalTargetRotation.Yaw));
				
			}

			Start = Player->GetMesh()->GetSocketLocation(FName("foot_l"));
			// Turn offset into variable
			Start += FVector (0,0,0);
			End = Start;
			CollisionParams.AddIgnoredActor(Player->GetCharacterMovement()->CurrentFloor.HitResult.GetActor());
			
			if (GetWorld()->SweepSingleByChannel(SphereHitResult, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(20), CollisionParams))
			{
				FVector Normal = SphereHitResult.ImpactNormal;
				float DotProduct = Normal.Dot(FVector(0,0,1));
				float AngleRadians = FMath::Abs(FMath::Acos(FMath::Clamp(DotProduct, -1.f, 1.f)));
				if(AngleRadians > 80) // make variable
				{
					
				}
				else
				{
					
				}
			}
		}
		
	}
	
}

bool USlideMechanic::CanStart_Implementation(AActor* Actor)
{
	return Super::CanStart_Implementation(Actor);
}
