// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralGeneration/Mechanics/WallRunMechanic.h"
#include "ProceduralGeneration/ParkourCharacter.h"
#include "ProceduralGeneration/AnimationInstance/PAnimInstance.h"
#include <GameFramework/CharacterMovementComponent.h>

#include "InputActionValue.h"
#include "ProceduralGeneration/PGameTags.h"

#include "Components/CapsuleComponent.h"
#include "ProceduralGeneration/MechanicComponents/MechanicsComponent.h"

UWallRunMechanic::UWallRunMechanic()
{
	WallRunGravityScale = 0.0f;
	bIsPlaneConstraintEnabled = true;
	WallRunConstrainNormal = FVector(0,0,1);
	bShouldPlayerOrientToMovement = false;
	WallRunCapsuleRadius = 45;

	bIsWallRunning = false;
}

void UWallRunMechanic::OnMechanicAdded_Implementation(AActor* Actor)
{
	Super::OnMechanicAdded_Implementation(Actor);

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Wall Run Added"));
	
	Player = Cast<AParkourCharacter>(Actor);
	AnimInstance = Cast<UPAnimInstance>(Player->GetMesh()->GetAnimInstance());
	MovementComp = Player->GetCharacterMovement<UMovementComponent>();

	if (WallRunArcCurve)
	{
		// Bind the timeline curve for Capsule to the UpdateCapsule function
		FOnTimelineFloat WallRunTimelineProgress;
		FOnTimelineEventStatic onTimelineFinishedCallback;
		WallRunTimelineProgress.BindUFunction(this, FName("TimelineArcUpdate"));
		onTimelineFinishedCallback.BindUFunction(this, FName("TimelineArcComplete"));
		WallRunningTimeline.AddInterpFloat(WallRunArcCurve, WallRunTimelineProgress);
		WallRunningTimeline.SetTimelineFinishedFunc(onTimelineFinishedCallback);
	}

	// Gravity Timeline
	GravityScaleTimeline.SetLooping(true);
	FOnTimelineFloat GravityTimelineProgress;
	GravityTimelineProgress.BindUFunction(this, FName("GravityUpdate"));

	// Setup function to detect if player should wallrun
	Player->GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &UWallRunMechanic::OnCapsuleComponentHit);
}

void UWallRunMechanic::OnMechanicRemoved_Implementation(AActor* Actor)
{
	Super::OnMechanicRemoved_Implementation(Actor);
	
}

void UWallRunMechanic::OnCapsuleComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Black, TEXT("Capsule Hit"));
	
	if(!bIsWallRunning && !Player->GetCharacterMovement()->IsFalling())
	{
		// Cross product to receive a vector which is perpendicular to two given vectors - player and wall.
		float ReName = FVector::DotProduct(Hit.ImpactNormal, Player->GetActorRightVector());
		
		FVector WallRunNormal;
		
		if (ReName > 0)
		{
			WallRunNormal = FVector::CrossProduct(Hit.ImpactNormal, FVector(0,0,1));
			WallRunDirection = EDirection::Right;
		}
		else
		{
			WallRunNormal = FVector::CrossProduct(Hit.ImpactNormal, FVector(0,0,-1));
			WallRunDirection = EDirection::Left;
		}

		if (ContinueWallRun())
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("True!"));
		}

		if (!Player->GetCharacterMovement()->IsMovingOnGround() && !bIsWallRunning)
		{
			BeginWallrun();
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Wall Run Logic!"));
		}
	}
}

void UWallRunMechanic::StartMechanic_Implementation(AActor* Actor)
{
	Super::StartMechanic_Implementation(Actor);

	GetOwningComponent()->GetActiveTags().AppendTags(WallRunningTags);
	
	if (bIsWallRunning)
	{
		// Stop wallrun and launch in camera forward direction
		
	}
	else
	{
		Player->Jump();
	}
}

void UWallRunMechanic::StopMechanic_Implementation(AActor* Actor)
{
	Super::StopMechanic_Implementation(Actor);

	Player->StopJumping();
}

void UWallRunMechanic::TickMechanic_Implementation(float DeltaTime)
{
	Super::TickMechanic_Implementation(DeltaTime);

	if (bIsWallRunning)
	{
		if (bApplyGravity)
		{
			GravityScaleTimeline.TickTimeline(DeltaTime);
		}
		
		WallRunningTimeline.TickTimeline(DeltaTime);
		
		// Trace Forward
		
	}
}

bool UWallRunMechanic::CanStart_Implementation(AActor* Actor)
{
	Super::CanStart_Implementation(Actor);

	return Player->CanJump();
}

void UWallRunMechanic::BeginWallrun()
{
	// Wall Run bool Enable
	bIsWallRunning = true;

	// Increase capsule size
	Player->GetCapsuleComponent()->SetCapsuleRadius(WallRunCapsuleRadius);

	// Set Trace Forward
	bForwardTrace = true;

	// Start Timeline
	WallRunningTimeline.PlayFromStart();
}

bool UWallRunMechanic::ContinueWallRun() // very shoddy, redo completely.
{
	float DirectionFloat;
	if (WallRunDirection == EDirection::Left)
	{
		DirectionFloat = 1.f;
	}
	else
	{
		DirectionFloat = -1.f;
	}
	
	if (FMath::IsNearlyEqual(Player->GetPlayerMoveValue().X, DirectionFloat) || Player->GetPlayerMoveValue().Y > 0)
	{
		return true;
	}
	return false;
}

void UWallRunMechanic::EndWallrun()
{
	// Wall Run bool Disable
	bIsWallRunning = false;

	// Revert Capsule Size
	Player->GetCapsuleComponent()->SetCapsuleRadius(Player->GetCapsuleFull().X);

	bForwardTrace = false;

	// Reset Variables - Maybe not needed
	WallRunHandIK_Location = FVector(0,0,0);
	WallRunHandIK_LeftAlpha = 0.f;
	WallRunHandIK_RightLocation = FVector(0,0,0);;
	WallRunHandIK_RightAlpha = 0.f;

	// Stop Arcing Timeline
	WallRunningTimeline.Stop();
}

// Maybe not needed - can move to tick or something?
void UWallRunMechanic::TimelineArcUpdate(float Value)
{
	// Add actor world offset
	Player->AddActorWorldOffset(FVector(0,0,Value));

	// Lerp speed to slow down over time.
	Player->GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(600, 500, Value); // make variable for speeds (in player perhaps).
}

void UWallRunMechanic::TimelineArcComplete(float Value)
{
	if (bApplyGravity) // if gravity is to be applied
	{
		return;
	}
}

void UWallRunMechanic::GravityUpdate(float Value)
{
	Player->AddActorWorldOffset(FVector(0,0,-1));
}
