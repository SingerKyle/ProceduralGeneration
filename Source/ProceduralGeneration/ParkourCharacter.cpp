// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourCharacter.h"
#include <Camera/CameraComponent.h>
#include <Components/CapsuleComponent.h>
#include <GameFramework/CharacterMovementComponent.h>
#include <GameFramework/SpringArmComponent.h>
#include <GameFramework/Controller.h>

#include "ProceduralGeneration/PGameTags.h"
#include "ProceduralGeneration/MechanicComponents/MechanicsComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

// Sets default values
AParkourCharacter::AParkourCharacter()
{
	// assign capsule variables
	FullCapsule = FVector2f(42.f, 96.0f);
	HalfCapsule = FVector2f(FullCapsule.X / 2, FullCapsule.Y / 2);

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	TFPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPSCamera"));
	TFPSCamera->SetupAttachment(GetMesh(), "head");
	TFPSCamera->SetRelativeLocation(FVector(0.0f, 10.0f, 0.0f));
	TFPSCamera->SetRelativeRotation(FRotator(0.0f, 90.0f, -90.0f));
	TFPSCamera->bUsePawnControlRotation = true;

	MechanicComponent = CreateDefaultSubobject<UMechanicsComponent>(TEXT("Mechanic Component"));

}

// Called when the game starts or when spawned
void AParkourCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (CrouchCurveFloat)
	{
		FOnTimelineFloat timelineProgress;
		//timelineProgress.BindUFunction(this, FName("CrouchTimer"));
		CrouchTimeline.AddInterpFloat(CrouchCurveFloat, timelineProgress);
	}
}

void AParkourCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Movement!"));
}

void AParkourCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AParkourCharacter::StartSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = 600;
	SetSprinting();
}

void AParkourCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = 300;
	SetSprinting();
}

void AParkourCharacter::StartCrouching()
{
	if (MechanicComponent)
	{
		// probably a better way to do this, look into later
		if (!GetCrouching()) // if player is already crouching / sliding
		{
			if (MechanicComponent->StartMechanic(this, SlideMechanicTag))
			{
				SetCrouching();
			}
		}
		else // if player is not crouching
		{
			if (MechanicComponent->StopMechanic(this, SlideMechanicTag))
			{
				SetCrouching();
			}
		}

	}
}

// Called every frame
void AParkourCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CrouchTimeline.TickTimeline(DeltaTime);
}

// Called to bind functionality to input
void AParkourCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) 
	{

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AParkourCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AParkourCharacter::Look);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AParkourCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AParkourCharacter::StopSprint);

		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AParkourCharacter::StartCrouching);

	}
}

void AParkourCharacter::SetCrouching()
{
	IsCrouching = !IsCrouching;
}

void AParkourCharacter::SetSprinting()
{
	IsSprinting = !IsSprinting;
}