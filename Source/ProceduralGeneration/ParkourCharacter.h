// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AnimationInstance/PAnimInstance.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "ParkourCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UMechanicsComponent;
class UInputAction;
struct FInputActionValue;

UENUM(BlueprintType)
enum EMovementState
{
	WALKING UMETA(DisplayName = "Walking"),
	RUNNING UMETA(DisplayName = "Running"),
	CROUCHING UMETA(DisplayName = "Crouching"),
	SLIDING UMETA(DisplayName = "Sliding")
};

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

	// Vault / Mantle Input Action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	bool IsSprinting = false;

//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
//	bool IsCrouching = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true")) bool bCanVault = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true")) bool bCanMantle = false;
	
	FTimeline CrouchTimeline;

	UPROPERTY(EditAnywhere, Category = "TimeLine") UCurveFloat* CrouchCurveFloat;

	UPROPERTY(EditAnywhere, Category = "Capsule", meta = (AllowPrivateAccess = "true")) FVector2f FullCapsule; 
	UPROPERTY(EditAnywhere, Category = "Capsule", meta = (AllowPrivateAccess = "true")) FVector2f HalfCapsule;

	/** Action Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components") UMechanicsComponent* MechanicComponent;

	// Action Tags - could make this better
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true")) FGameplayTag SlideMechanicTag;

	UPROPERTY(BlueprintReadWrite, Category = "MovementState", meta = (AllowPrivateAccess = "true"))  TEnumAsByte<EMovementState> MovementState;

	// Speed Variables
	UPROPERTY(BlueprintReadOnly, Category = "Speeds", meta = (AllowPrivateAccess = "true")) float WalkSpeed;
	UPROPERTY(BlueprintReadOnly, Category = "Speeds", meta = (AllowPrivateAccess = "true")) float RunSpeed;
	UPROPERTY(BlueprintReadOnly, Category = "Speeds", meta = (AllowPrivateAccess = "true")) float CrouchSpeed;

	// VAULTING
	FVector VaultStart;
	FVector VaultMiddle;
	FVector VaultLand;
	int VaultDistance;

	UPROPERTY(EditAnywhere, Category = "Animation") TArray<UAnimMontage*> VaultMontages;

	// Motion Warping Component

	UPROPERTY(EditAnywhere, Category = "Vaulting", meta = (AllowPrivateAccess = "true")) float InitialTraceLength;
	UPROPERTY(EditAnywhere, Category = "Vaulting", meta = (AllowPrivateAccess = "true")) float SecondaryTraceZOffset;
	UPROPERTY(EditAnywhere, Category = "Vaulting", meta = (AllowPrivateAccess = "true")) float SecondaryTraceGap;
	UPROPERTY(EditAnywhere, Category = "Vaulting", meta = (AllowPrivateAccess = "true")) float LandingPositionForwardOffset;

	// MANTLING
	UPROPERTY(EditAnywhere, Category = "Mantling", meta = (AllowPrivateAccess = "true")) UAnimMontage* MantleMontage;
	
	FVector MantlePosition1;
	FVector MantlePosition2;
	
	UPROPERTY(EditAnywhere, Category = "Vaulting", meta = (AllowPrivateAccess = "true")) float MantleInitialTraceLength;
	UPROPERTY(EditAnywhere, Category = "Vaulting", meta = (AllowPrivateAccess = "true")) float MantleSecondaryTraceZOffset;
	UPROPERTY(EditAnywhere, Category = "Vaulting", meta = (AllowPrivateAccess = "true")) float MantleFallHeightMultiplier;

	bool bIsPerformingAction = false;

	// FALL DAMAGE AND LANDING
	UPAnimInstance* AnimInstance;
	UPROPERTY(EditAnywhere, Category = "Mantling", meta = (AllowPrivateAccess = "true")) TArray<UAnimMontage*> FallMontages;
public:
	// Sets default values for this character's properties
	AParkourCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Check for fall animations
	void CheckForFall();
	virtual void Landed(const FHitResult& Hit) override;

	UFUNCTION() void OnFallMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void StartSprint();
	void StopSprint();

	virtual void Jump() override;
	
	void StartCrouch();
	void StopCrouch();

	// VAULTING
	void Interact();
	void VaultTrace();
	void ApplyMotionWarping(FName WarpName, FVector WarpLocation);

	UFUNCTION()
	void OnVaultMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// MANTLING
	void MantleTrace();
	void ApplyMantleMotionWarping(FName WarpName, FVector WarpLocation, float Offset);
	UFUNCTION()
	void OnMantleMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Returns camera object
	FORCEINLINE class UCameraComponent* GetTrueFirstPersonCamera() const { return TFPSCamera; }

	void SwitchMovementState(EMovementState State);
	FORCEINLINE EMovementState GetMovementState() { return MovementState; }

	// Returns IsSprinting
	FORCEINLINE bool GetSprinting() const { return IsSprinting; }
	// Change capsule size depending on anim state.
	void ToggleCapsuleSize();

	FVector2f GetCapsuleFull() { return FullCapsule; }
	FVector2f GetCapsuleHalf() { return HalfCapsule; }

	// Maybe change - used to pass walk speed to components
	FORCEINLINE float GetPlayerWalkSpeed() const { return WalkSpeed; }

	//void SetCrouching();
    void SetSprinting();
};
