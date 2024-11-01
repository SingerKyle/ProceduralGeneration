// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "ParkourCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UMechanicsComponent;
class UInputAction;
struct FInputActionValue;

UCLASS(config = game)
class PROCEDURALGENERATION_API AParkourCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	// True First Person Camera - Can See Third Person Mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = true)) UCameraComponent* TFPSCamera;

	// MappingContext
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	// Jump Input Action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	// Move Input Action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	// Look Input Action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	// Look Input Action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	// Sprint Input Action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	bool IsSprinting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	bool IsCrouching = false;

	FTimeline CrouchTimeline;

	UPROPERTY(EditAnywhere, Category = "TimeLine") UCurveFloat* CrouchCurveFloat;

	UPROPERTY(EditAnywhere, Category = "Capsule", meta = (AllowPrivateAccess = "true")) FVector2f FullCapsule; 
	UPROPERTY(EditAnywhere, Category = "Capsule", meta = (AllowPrivateAccess = "true")) FVector2f HalfCapsule;

	/** Action Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components") UMechanicsComponent* MechanicComponent;

	// Action Tags - could make this better
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true")) FGameplayTag SlideMechanicTag;

public:
	// Sets default values for this character's properties
	AParkourCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void StartSprint();
	void StopSprint();

	void ToggleCrouch();
	//void StopCrouching();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Returns camera object
	FORCEINLINE class UCameraComponent* GetTrueFirstPersonCamera() const { return TFPSCamera; }

	// Returns IsSprinting
	FORCEINLINE bool GetSprinting() const { return IsSprinting; }
	// Change capsule size depending on anim state.
	void ToggleCapsuleSize();

	FVector2f GetCapsuleFull() { return FullCapsule; }
	FVector2f GetCapsuleHalf() { return HalfCapsule; }

	void SetCrouching();
    void SetSprinting();
};
