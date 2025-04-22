// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralGeneration/Mechanics/BaseMechanic.h"
#include <GameFramework/CharacterMovementComponent.h>

#include "Components/TimelineComponent.h"
#include "ProceduralGeneration/AnimationInstance/PAnimInstance.h"
#include "ProceduralGeneration/PGameTags.h"
#include "WallRunMechanic.generated.h"

class AParkourCharacter;

/**
 * 
 */
UCLASS()
class PROCEDURALGENERATION_API UWallRunMechanic : public UBaseMechanic
{
	GENERATED_BODY()
	
public:
	UWallRunMechanic();
	
	virtual void OnMechanicAdded_Implementation(AActor* Actor) override;
	virtual void OnMechanicRemoved_Implementation(AActor* Actor) override;

	//virtual void Tick_Implementation
	UFUNCTION() void OnCapsuleComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void StartMechanic_Implementation(AActor* Actor) override;
	virtual void StopMechanic_Implementation(AActor* Actor) override;
	virtual void TickMechanic_Implementation(float DeltaTime) override;
	virtual bool CanStart_Implementation(AActor* Actor) override;
	
	void BeginWallrun();
	bool ContinueWallRun();
	void SetWallRunRotation();
	void SetWallRunOptions(float GravityScale, bool PlaneConstraintVal, FVector PlaneConstraintNormal, bool OrientToMovement, bool Wallrunning, float CapsuleRadius, bool UsePawnRotation);
	void EndWallrun();

	// Timeline Update Function
	UFUNCTION() void TimelineArcUpdate(float Value);
	UFUNCTION() void TimelineArcComplete(float Value);
	// Gravity
	void GravityUpdate(float Value);

	// Forward Trace
	UFUNCTION() void ShouldForwardTrace();

protected:
	
	UPROPERTY() AParkourCharacter* Player;
	UPROPERTY() UPAnimInstance* AnimInstance;
	UPROPERTY() UMovementComponent* MovementComp;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal") FGameplayTagContainer WallRunningTags;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "WallRun Options") float WallRunGravityScale;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "WallRun Options") bool bIsPlaneConstraintEnabled;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "WallRun Options") FVector WallRunConstrainNormal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "WallRun Options") bool bShouldPlayerOrientToMovement;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "WallRun Options") float WallRunCapsuleRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun Options") bool bIsWallRunning;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun Options") bool bApplyGravity;

	UPROPERTY() FVector WallRunNormal;
	bool bCanWallRun = true;

	// Variables for wall run speed, angles, forces (maybe move to custom character component
	UPROPERTY(EditDefaultsOnly) float MinWallRunSpeed;
	UPROPERTY(EditDefaultsOnly) float MaxWallRunSpeed;
	UPROPERTY(EditDefaultsOnly) float WallJumpOffForce = 350.f;

	// Normal of whichever wall we are running along
	FVector WallSurfaceNormal;
	
	// Animations for right wall run
	UPROPERTY(EditAnywhere, Category = "Animation") UAnimMontage* RightWallMontage;
	
	// Forward Trace
	bool bForwardTrace;
	// Timer for forward trace bool
	UPROPERTY() FTimerHandle ForwardTraceDelay;

	// Hand IK positions
	FVector WallRunHandIK_Location;
	float WallRunHandIK_LeftAlpha;
	FVector WallRunHandIK_RightLocation;
	float WallRunHandIK_RightAlpha;

	// Wallrun timeline
	UPROPERTY() FTimeline WallRunningTimeline;
	UPROPERTY(EditAnywhere, Category = "Timeline Float") UCurveFloat* WallRunArcCurve;
	// Gravity Scale Timeline
	UPROPERTY() FTimeline GravityScaleTimeline;
	UPROPERTY(EditAnywhere, Category = "Timeline Float") UCurveFloat* WallRunGravityCurve;

	// Direction Enum
	UPROPERTY() TEnumAsByte<EDirection> WallRunDirection;
};
