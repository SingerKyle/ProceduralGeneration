// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseMechanic.h"
#include "Components/TimelineComponent.h"
#include "ProceduralGeneration/AnimationInstance/PAnimInstance.h"
#include "SlideMechanic.generated.h"

class AParkourCharacter;
class UMovementComponent;

/**
 * 
 */
UCLASS()
class PROCEDURALGENERATION_API USlideMechanic : public UBaseMechanic
{
	GENERATED_BODY()
	
public:
	USlideMechanic();

	virtual void OnMechanicAdded_Implementation(AActor* Actor) override;
	virtual void OnMechanicRemoved_Implementation(AActor* Actor) override;

	//virtual void Tick_Implementation

	virtual void StartMechanic_Implementation(AActor* Actor) override;
	virtual void StopMechanic_Implementation(AActor* Actor) override;
	virtual void TickMechanic_Implementation(float DeltaTime) override;
	virtual bool CanStart_Implementation(AActor* Actor) override;

	void StartSlide();
	void StopSlide();

	// Get Slope Angle
	FVector2f GetSlopeDegreeAngle(FVector SurfaceNorm, FVector RightVector, FVector UpVector);
	
	// Slide Timeline Function
	UFUNCTION() void UpdateCapsule(float Value) const;
	UFUNCTION() void UpdateMesh(float Value) const;

	FORCEINLINE FVector GetSlideVelocity() { return SlideVelocity; };
	void SetSlideVelocity(FVector NewSlideVelocity);

	void CheckShouldContinueSlide();
	float GetSlideSlope(const FVector& FloorNormal);

protected:

	UPROPERTY() AParkourCharacter* Player;
	UPROPERTY() UPAnimInstance* AnimInstance;
	UPROPERTY() UMovementComponent* MovementComp;

	UPROPERTY(EditAnywhere, Category = "Tags") FGameplayTagContainer CrouchTags;

	UPROPERTY(EditAnywhere, Category = "Tags") FGameplayTagContainer SlidingTags;

	// Slide Physics Values
	UPROPERTY(EditAnywhere, Category = "Physics") FVector SlideVelocity;
	UPROPERTY(EditAnywhere, Category = "Physics") float SlideSpeed;
	UPROPERTY(EditAnywhere, Category = "Physics") float SlideDuration;
	UPROPERTY(EditAnywhere, Category = "Physics") float MaxSlideSpeed;

	UPROPERTY(EditAnywhere, Category = "Animation") UAnimMontage* EndSlideMontage;

	float WalkSpeed;

	// Whether we are currently crouching
	bool bIsCrouching;
	// Whether we are currently sliding
	bool bIsSliding;

	// Timeline variables
	UPROPERTY() FTimeline SlideCapsuleScaleTimeline;
	UPROPERTY() FTimeline SlideMeshLocationTimeline;
	UPROPERTY(EditAnywhere, Category = "Timeline Float") UCurveFloat* TimelineCapsuleCurveFloat;
	UPROPERTY(EditAnywhere, Category = "Timeline Float") UCurveFloat* TimelineMeshCurveFloat;
};
