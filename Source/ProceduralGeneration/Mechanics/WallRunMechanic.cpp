// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralGeneration/Mechanics/WallRunMechanic.h"
#include "ProceduralGeneration/ParkourCharacter.h"
#include "ProceduralGeneration/AnimationInstance/PAnimInstance.h"
#include <GameFramework/CharacterMovementComponent.h>

#include "InputActionValue.h"
#include "VectorTypes.h"
#include "Camera/CameraComponent.h"
#include "ProceduralGeneration/PGameTags.h"

#include "Components/CapsuleComponent.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "ProceduralGeneration/MechanicComponents/MechanicsComponent.h"

UWallRunMechanic::UWallRunMechanic()
{
	//SetWallRunOptions()
	WallRunGravityScale = 0.0f;
	bIsPlaneConstraintEnabled = true;
	WallRunConstrainNormal = FVector(0,0,1);
	bShouldPlayerOrientToMovement = false;
	WallRunCapsuleRadius = 45;

	bApplyGravity = false;
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
	//GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, TEXT("Capsule Overlap!"));
	
	if(!bIsWallRunning && !Player->GetCharacterMovement()->IsFalling())
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Wall Run is false && Not Falling"));
	}
	else
	{
		// Cross product to receive a vector which is perpendicular to two given vectors - player and wall.
		float ReName = FVector::DotProduct(Hit.ImpactNormal, Player->GetActorRightVector());

		//GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Red, FString::Printf(TEXT("Starting prep for wall run - ReName: %f"), ReName));
		
		if (ReName > 0)
		{
			WallRunNormal = FVector::CrossProduct(Hit.ImpactNormal, FVector(0,0,1));
			WallRunDirection = EDirection::Right;
			AnimInstance->SetWallrunDirection(EDirection::Right);
		}
		else
		{
			WallRunNormal = FVector::CrossProduct(Hit.ImpactNormal, FVector(0,0,-1));
			WallRunDirection = EDirection::Left;
			AnimInstance->SetWallrunDirection(EDirection::Left);
		}

		FRotator PlayerDirection = Player->GetInputDirection();
		// Dot product will be 0 if player is perpendicular to wall and will increase the closer their forward is to the wall.
		float DotResult = FVector::DotProduct(Player->GetActorForwardVector(), WallRunNormal);
		//PlayerDirection.Vector()
		
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, FString::Printf(TEXT("DotResult = %f"), DotResult));
		
		if (ContinueWallRun() && DotResult > 0.4) // make variable
		{
			// Sphere trace to detect if player is far enough from the ground?
			
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Wall Run Logic!"));
			BeginWallrun();
		}
	}
}

void UWallRunMechanic::StartMechanic_Implementation(AActor* Actor)
{
	Super::StartMechanic_Implementation(Actor);

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Star WallRun!"));

	GetWorld()->GetTimerManager().SetTimer(ForwardTraceDelay, this, &UWallRunMechanic::ShouldForwardTrace, 0.2f, false);
	
	if (bIsWallRunning)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("LAUNCH CHARACTER"));
		// Stop wallrun and launch in camera forward direction
		const FVector JumpOffDirection = Player->GetTrueFirstPersonCamera()->GetForwardVector();
		const FVector JumpOffVector = Player->GetCharacterMovement()->GetCurrentAcceleration().GetSafeNormal() + FVector::UpVector + JumpOffDirection;
		
		Player->LaunchCharacter(JumpOffVector * WallJumpOffForce, true, true);

		EndWallrun();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Jumping!"));
		Player->Jump();
	}
}

void UWallRunMechanic::StopMechanic_Implementation(AActor* Actor)
{
	Super::StopMechanic_Implementation(Actor);

	GetWorld()->GetTimerManager().ClearTimer(ForwardTraceDelay);
	EndWallrun();
	Player->StopJumping();
}

void UWallRunMechanic::TickMechanic_Implementation(float DeltaTime)
{
	Super::TickMechanic_Implementation(DeltaTime);

	if (bIsWallRunning)
	{
		SetWallRunRotation();
		
		if(!ContinueWallRun())
		{
			EndWallrun();
		}
		else
		{
			FHitResult SphereHitResult;
			FVector Start = Player->GetMesh()->GetComponentLocation();
			float Direction = (WallRunDirection == EDirection::Left) ? 1.f : -1.f;
			FVector End = Start + (Player->GetActorRightVector() * Direction * 60);

			// Foot Location Trace
			if (GetWorld()->SweepSingleByChannel(SphereHitResult, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(5)))
			{
				
				DrawDebugSphere(GetWorld(), Start, 5.0f, 12, FColor::Green, false, 5.0f);
				
				Start = Player->GetMesh()->GetComponentLocation() + (Player->GetActorRightVector() * Direction * 5);
				// ground check
				if (GetWorld()->SweepSingleByChannel(SphereHitResult, Start, Start, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(10)))
				{
					EndWallrun();
				}
				else
				{
					
					// Capsule Trace for checking if a wall lies ahead.
					float CapsuleRadius = bForwardTrace ? 10.f : 0.f;
					Start = Player->GetActorLocation() + (Player->GetActorForwardVector() * 50); // make variable?
				
					if (GetWorld()->SweepSingleByChannel(SphereHitResult, Start, Start, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeCapsule(CapsuleRadius, 50)))
					{
						EndWallrun();
					}
					else
					{
						DrawDebugCapsule(GetWorld(), Start, 50.f, CapsuleRadius, FQuat::Identity, FColor::Purple, false, 5.0f);
						FHitResult LineHitResult;
						Start = Player->GetActorLocation();
						FVector WallRunDirectionVector = (WallRunDirection == EDirection::Left) ? FVector(0,0,-1) : FVector(0,0,1);
						End = Start + FVector(FVector::CrossProduct(WallRunNormal, WallRunDirectionVector) * 200 );
						if(GetWorld()->LineTraceSingleByChannel(LineHitResult, Start, End, ECC_Visibility))
						{
							WallRunNormal = FVector::CrossProduct(LineHitResult.ImpactNormal, WallRunDirectionVector);
							FVector NewVelocity = FVector(WallRunNormal * Player->GetCharacterMovement()->MaxWalkSpeed);
							Player->GetCharacterMovement()->Velocity = FVector(NewVelocity.X, NewVelocity.Y, 0);
						}
						else
						{
							
						}

						for (int i = 0; i < 5; i++)
						{
							Start = Player->GetActorLocation() + FVector(0,0, 50 - (i * 10));
							End = Player->GetActorLocation() + FVector(FVector::CrossProduct(WallRunNormal, WallRunDirectionVector) * 200 ) + FVector(0,0,(50 - (i * 10)));
					
							if(GetWorld()->LineTraceSingleByChannel(LineHitResult, Start, End, ECC_Visibility))
							{
								WallSurfaceNormal = LineHitResult.Normal;
						
								// Offset for hand on wall location.
								switch (WallRunDirection)
								{
								case EDirection::Left:
									WallRunHandIK_RightLocation = FVector((LineHitResult.Normal * 5.f) + LineHitResult.ImpactPoint);
									AnimInstance->SetHandRightLocation(WallRunHandIK_RightLocation);
									WallRunHandIK_LeftAlpha = 0.f;
									AnimInstance->SetHandLeftAlpha(WallRunHandIK_LeftAlpha);
									WallRunHandIK_RightAlpha = 1.f;
									AnimInstance->SetHandLeftAlpha(WallRunHandIK_RightAlpha);
									break;
								case EDirection::Right:
									WallRunHandIK_Location = FVector((LineHitResult.Normal * 5.f) + LineHitResult.ImpactPoint);
									AnimInstance->SetHandLeftLocation(WallRunHandIK_Location);
									WallRunHandIK_LeftAlpha = 1.f;
									AnimInstance->SetHandLeftAlpha(WallRunHandIK_LeftAlpha);
									WallRunHandIK_RightAlpha = 0.f;
									AnimInstance->SetHandLeftAlpha(WallRunHandIK_RightAlpha);
									break;
								default:
									break;
								}
							
							
								// finally break
								break;
							}
						}
					
						if (bApplyGravity)
						{
							GravityScaleTimeline.TickTimeline(DeltaTime);
						}
		
						WallRunningTimeline.TickTimeline(DeltaTime);
					}
				}
			}
			else
			{
				DrawDebugSphere(GetWorld(), Start, 5.0f, 12, FColor::Red, false, 5.0f);
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Fuck you!"));
				EndWallrun();
			}
		}
	}
}

bool UWallRunMechanic::CanStart_Implementation(AActor* Actor)
{
	if (!Super::CanStart_Implementation(Actor))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("False!"));
		return false;
	}

	return Player->CanJump() || Player->GetMovementComponent()->IsJumpAllowed() && bIsWallRunning;
}

void UWallRunMechanic::BeginWallrun()
{
	//if(!bCanWallRun)
	//{
	//	return;
	//}

	SetWallRunOptions(WallRunGravityScale, bIsPlaneConstraintEnabled, WallRunConstrainNormal, false, true, WallRunCapsuleRadius, false);

	bCanWallRun = false;
	
	GetOwningComponent()->GetActiveTags().AppendTags(WallRunningTags);

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

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("DirectionFloat = %f"), DirectionFloat));
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("MoveValue = %f"), Player->GetPlayerMoveValue().X));
	
	if (FMath::IsNearlyEqual(Player->GetPlayerMoveValue().X, DirectionFloat) || Player->GetPlayerMoveValue().Y > 0)
	{
		return true;
	}
	return false;
}

void UWallRunMechanic::SetWallRunRotation()
{
	// make interpspeed a variable
	Player->SetActorRotation(FMath::RInterpTo(Player->GetActorRotation(), FRotationMatrix::MakeFromX(WallRunNormal).Rotator(), GetWorld()->DeltaTimeSeconds, 15.f));
}

void UWallRunMechanic::SetWallRunOptions(float GravityScale, bool PlaneConstraintVal, FVector PlaneConstraintNormal,
	bool OrientToMovement, bool Wallrunning, float CapsuleRadius, bool UsePawnRotation)
{
	Player->GetCharacterMovement()->GravityScale = GravityScale;
	Player->GetCharacterMovement()->SetPlaneConstraintEnabled(PlaneConstraintVal);
	Player->GetCharacterMovement()->SetPlaneConstraintNormal(PlaneConstraintNormal);
	Player->GetCharacterMovement()->bOrientRotationToMovement = OrientToMovement;
	Player->bUseControllerRotationYaw = UsePawnRotation;
	bIsWallRunning = Wallrunning;
	AnimInstance->SetIsWallrunning(Wallrunning);
	Player->GetCapsuleComponent()->SetCapsuleRadius(CapsuleRadius);
}

void UWallRunMechanic::EndWallrun()
{
	//if (bCanWallRun)
	//{
	//	return;
	//}

	SetWallRunOptions(1.25f, false, FVector(0,0,0), true, false, Player->GetCapsuleFull().X, true); // make grav scale a variable in player
	
	bCanWallRun = true;
	
	
	bForwardTrace = false;

	// Reset Variables - Maybe not needed
	WallRunHandIK_Location = FVector(0,0,0);
	WallRunHandIK_LeftAlpha = 0.f;
	WallRunHandIK_RightLocation = FVector(0,0,0);
	WallRunHandIK_RightAlpha = 0.f;
	AnimInstance->ResetIK();

	// Stop Arcing Timeline
	WallRunningTimeline.Stop();

	GetOwningComponent()->GetActiveTags().RemoveTags(WallRunningTags);
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

void UWallRunMechanic::ShouldForwardTrace()
{
	bForwardTrace = true;
}
