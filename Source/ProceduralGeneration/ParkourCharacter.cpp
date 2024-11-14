// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourCharacter.h"
#include <Camera/CameraComponent.h>
#include <Components/CapsuleComponent.h>
#include <GameFramework/CharacterMovementComponent.h>
#include <GameFramework/Controller.h>
#include "GameplayTagContainer.h"

#include "ProceduralGeneration/PGameTags.h"
#include "ProceduralGeneration/MechanicComponents/MechanicsComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "AnimationInstance/PAnimInstance.h"

#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "MotionWarpingComponent.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "GameFramework/SpringArmComponent.h"
#include "PhysicsEngine/PhysicsSpringComponent.h"

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
	GetCharacterMovement()->AirControl = 0.55f;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(GetMesh(), "head");
	
	TFPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPSCamera"));
	TFPSCamera->SetupAttachment(SpringArm);
	TFPSCamera->SetRelativeLocation(FVector(0.0f, 10.0f, 0.0f));
	TFPSCamera->SetRelativeRotation(FRotator(0.0f, 90.0f, -90.0f));
	TFPSCamera->bUsePawnControlRotation = true;

	MechanicComponent = CreateDefaultSubobject<UMechanicsComponent>(TEXT("Mechanic Component"));

	// speed setting
	WalkSpeed = 300.f;
	RunSpeed = 600.f;
	CrouchSpeed = 200.f;

	// VAULTING
	InitialTraceLength = 380.f;
	SecondaryTraceZOffset = 100.f;
	SecondaryTraceGap = 20.f;
	LandingPositionForwardOffset = 10.f;

	// MANTLE
	MantleInitialTraceLength = 120.f;
	MantleSecondaryTraceZOffset = 200.f;
	MantleFallHeightMultiplier = 0.5;
}

// Called when the game starts or when spawned
void AParkourCharacter::BeginPlay()
{
	Super::BeginPlay();

	AnimInstance = Cast<UPAnimInstance>(GetMesh()->GetAnimInstance());
	
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

void AParkourCharacter::CheckForFall()
{
	// get velocity at landing
	float PlayerVelocity = GetVelocity().Z;
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("PlayerVelocity: %f"), PlayerVelocity));
	//PlayerVelocity *= 1;

	// Ensure the delegate is only added once
	if (!AnimInstance->OnMontageEnded.IsBound())
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &AParkourCharacter::OnFallMontageEnded);
	}
	
	// if fallheight larger than 750 then roll
	if (PlayerVelocity < -850 && PlayerVelocity > -1250) // make variable
	{
		if(AnimInstance)
		{
			bIsPerformingAction = true;
			TFPSCamera->bUsePawnControlRotation = false;
			AnimInstance->Montage_Play(FallMontages[0]);
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Play 0"));
			if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
			{
				DisableInput(PlayerController); // find better way
			}
		}
	}
	else if (PlayerVelocity < -1251)
	{
		if(AnimInstance)
		{
			bIsPerformingAction = true;
			TFPSCamera->bUsePawnControlRotation = false;
			AnimInstance->Montage_Play(FallMontages[1]);
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Play 1"));
			if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
			{
				DisableInput(PlayerController);
			}
		}
	}

	// stop sprinting if we are sprinting
	IsSprinting = false;
}

void AParkourCharacter::Landed(const FHitResult& Hit)
{
	CheckForFall();
	
	Super::Landed(Hit);
}

void AParkourCharacter::OnFallMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("End Fall Montage"));
	
	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &AParkourCharacter::OnFallMontageEnded);
		AnimInstance->OnMontageEnded.Clear();
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		EnableInput(PlayerController);
	}
	TFPSCamera->bUsePawnControlRotation = true;
	bIsPerformingAction = false;
}

void AParkourCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	CurrentMoveVector = Value.Get<FVector2D>();

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
		AddMovementInput(ForwardDirection, CurrentMoveVector.Y);

		// Log the tags to the screen (on top of the game view)
		FString ActiveTagsString = MechanicComponent->GetActiveTags().ToString();

		// Display the active tags on screen for 5 seconds
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Active Tags: %s"), *ActiveTagsString));
		
		if (MechanicComponent->GetActiveTags().HasTag(WallRunMechanicTag))
		{
			AddMovementInput(GetActorForwardVector(), CurrentMoveVector.X);
		}
		else
		{
			AddMovementInput(RightDirection, CurrentMoveVector.X);
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Move with Mesh Forward"));
		}
		
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
	IsSprinting = true;
	SwitchMovementState(EMovementState::RUNNING);
}

void AParkourCharacter::StopSprint()
{
	IsSprinting = false;
	SwitchMovementState(EMovementState::WALKING);
}

void AParkourCharacter::Jump()
{
	if(!MechanicComponent->GetActiveTags().HasTag(SlideMechanicTag))
	{
		if(AnimInstance->GetIsOnZipline())
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			const FVector LaunchVelocity = (GetActorForwardVector() * 250.f) + (GetActorUpVector() * 1000.f);
			LaunchCharacter(LaunchVelocity, false, false);
			Super::Jump();
		}
		else
		{
			Super::Jump();
		}
	}
}

FRotator AParkourCharacter::GetInputDirection()
{
	FVector MoveVector = FVector(GetPlayerMoveValue().X, GetPlayerMoveValue().Y, 0);
	FRotator MoveRotation = FRotator(0, MoveVector.Rotation().Yaw,0);

	FRotator DeltaRotation = MoveRotation - GetControlRotation();
	DeltaRotation += FRotator(90, 90, 90);

	return FRotator(0,DeltaRotation.Yaw,0);
}

void AParkourCharacter::StartJumpCheck()
{
	MechanicComponent->StartMechanic(this, WallRunMechanicTag);
}

void AParkourCharacter::StopJumpCheck()
{
	MechanicComponent->StopMechanic(this, WallRunMechanicTag);
}

void AParkourCharacter::StartCrouch()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("StartCrouch called"));

	if (MechanicComponent)
	{
		if (!MechanicComponent->StartMechanic(this, SlideMechanicTag))
		{
			UE_LOG(LogTemp, Warning, TEXT("Can't Start Crouching/Sliding"));
		}
	}
}

void AParkourCharacter::StopCrouch()
{
	UE_LOG(LogTemp, Warning, TEXT("InStopCrouch"));

	if (MechanicComponent)
	{
		if (!MechanicComponent->StopMechanic(this, SlideMechanicTag))
		{
			UE_LOG(LogTemp, Warning, TEXT("Can't Start Crouching/Sliding"));
		}
		else
		{
			SwitchMovementState(EMovementState::WALKING);
		}
	}
}

void AParkourCharacter::Interact()
{
	if (bIsPerformingAction)
	{
		return;
	}
	
	// Check if player is moving fast enough to vault AND not falling.
	if (GetCharacterMovement()->Velocity.Length() > WalkSpeed - 150.f && !GetCharacterMovement()->IsFalling())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Interacting"));
		VaultTrace();

		if (bCanVault)
		{
			bIsPerformingAction = true;
			bCanVault = false;
			GetCharacterMovement()->SetMovementMode(MOVE_Flying);
			//GetCharacterMovement()->Velocity = FVector::ZeroVector;

			// Set Warping
			ApplyMotionWarping(FName("VaultStart"), VaultStart);
			ApplyMotionWarping(FName("VaultHeight"), VaultMiddle);
			ApplyMotionWarping(FName("VaultLand"), VaultLand);

			// Log the warping locations
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("VaultStart: %s"), *VaultStart.ToString()));
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("VaultMiddle: %s"), *VaultMiddle.ToString()));
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("VaultLand: %s"), *VaultLand.ToString()));
			
			if (AnimInstance && VaultMontages.Num() > 0) // Ensure the anim instance and montage are valid
				{
				//TFPSCamera->bUsePawnControlRotation = false;
				
				AnimInstance->OnMontageEnded.AddDynamic(this, &AParkourCharacter::OnVaultMontageEnded);
				
				AnimInstance->Montage_Play(VaultMontages[FMath::RandRange(0, VaultMontages.Num() - 1)]); // Play the montage
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Sliding!"));
				}
		}
		else // Jump OR MANTLE
		{
			MantleTrace();
			if(bCanMantle)
			{
				bIsPerformingAction = true;
			
				// Set Actor Collision
				SetActorEnableCollision(false);
				GetCharacterMovement()->SetMovementMode(MOVE_Flying);

				ApplyMantleMotionWarping(FName("MantlePoint1"), MantlePosition1, -100);
				ApplyMantleMotionWarping(FName("MantlePoint2"), MantlePosition2, 30);
				
				if (AnimInstance && MantleMontage) // Ensure the anim instance and montage are valid
					{
					TFPSCamera->bUsePawnControlRotation = false;
					AnimInstance->OnMontageEnded.AddDynamic(this, &AParkourCharacter::OnMantleMontageEnded);
				
					AnimInstance->Montage_Play(MantleMontage); // Play the montage
					//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Sliding!"));
					}
			}
			else // Jump
				{
			
				}
		}
	}
	else // Try to Mantle
	{
		MantleTrace();
		if(bCanMantle)
		{
			bIsPerformingAction = true;
			
			// Set Actor Collision
			SetActorEnableCollision(false);
			GetCharacterMovement()->SetMovementMode(MOVE_Flying);

			ApplyMantleMotionWarping(FName("MantlePoint1"), MantlePosition1, -100);
			ApplyMantleMotionWarping(FName("MantlePoint2"), MantlePosition2, 30);
			
			if (AnimInstance && MantleMontage) // Ensure the anim instance and montage are valid
				{
				TFPSCamera->bUsePawnControlRotation = false;
				AnimInstance->OnMontageEnded.AddDynamic(this, &AParkourCharacter::OnMantleMontageEnded);
				
				AnimInstance->Montage_Play(MantleMontage); // Play the montage
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Sliding!"));
				}
		}
		else // Jump
		{
			
		}
	}
	{
		
	}
}

void AParkourCharacter::ApplyMotionWarping(FName WarpName, FVector WarpLocation)
{
	// get motion component
	if(UMotionWarpingComponent* WarpComponent = FindComponentByClass<UMotionWarpingComponent>())
	{
		FMotionWarpingTarget WarpTarget;

		WarpTarget.Name = WarpName;
		//WarpTarget.GetTargetTrasform().SetLocation(WarpLocation);
		WarpTarget.Location = WarpLocation;
		WarpTarget.Rotation = GetActorRotation();
		//WarpTarget.GetTargetTrasform().SetRotation(GetActorRotation().Quaternion());

		// Log the warping locations
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("VaultStart: %s"), *WarpLocation.ToString()));
				
		WarpComponent->AddOrUpdateWarpTarget(WarpTarget);
	}
}

void AParkourCharacter::ApplyMantleMotionWarping(FName WarpName, FVector WarpLocation, float Offset)
{
	// get motion component
	if(UMotionWarpingComponent* WarpComponent = FindComponentByClass<UMotionWarpingComponent>())
	{
		FMotionWarpingTarget WarpTarget;

		WarpTarget.Name = WarpName;
		//WarpTarget.GetTargetTrasform().SetLocation(WarpLocation);
		WarpTarget.Location = WarpLocation + FVector(0,0, Offset);
		WarpTarget.Rotation = GetActorRotation();
		//WarpTarget.GetTargetTrasform().SetRotation(GetActorRotation().Quaternion());

		// Log the warping locations
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("MantleStart: %s"), *WarpLocation.ToString()));
				
		WarpComponent->AddOrUpdateWarpTarget(WarpTarget);
	}
}

void AParkourCharacter::OnMantleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// Optionally, clear the delegate if you don't want it to trigger for other montages
	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &AParkourCharacter::OnMantleMontageEnded);
		AnimInstance->OnMontageEnded.Clear();
	}

	SetActorEnableCollision(true);
	bIsPerformingAction = false;

	TFPSCamera->bUsePawnControlRotation = true;

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Mantle Montage Ended, Movement Mode Set to Walking"));

	GetCharacterMovement()->SetMovementMode(MOVE_Falling);
}

void AParkourCharacter::OnVaultMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	
	// Optionally, clear the delegate if you don't want it to trigger for other montages
	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &AParkourCharacter::OnVaultMontageEnded);
		AnimInstance->OnMontageEnded.Clear();
	}

	bIsPerformingAction = false;

	//TFPSCamera->bUsePawnControlRotation = true;
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Vault Montage Ended, Movement Mode Set to Walking"));
	
	// Reset movement mode back to walking
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	// Reset root motion mode to default
	AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Root Motion Mode: %d"), AnimInstance->RootMotionMode.GetValue()));
	//GetCharacterMovement()->Velocity = FVector::ZeroVector;
	//SetActorLocation(VaultLand);
}

void AParkourCharacter::MantleTrace()
{
	bCanMantle = false;

	//Re-initialize hit info
	FHitResult OutHit;

	FVector Start = GetActorLocation();
	FVector End = Start + (GetActorForwardVector() * MantleInitialTraceLength);

	DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 5.0f, 0, 2.0f);

	// Trace to check for a mantle-able object
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Found Mantle Object"));
		
		float SphereRadius = 10.f;

		float offset = MantleSecondaryTraceZOffset * (GetCharacterMovement()->IsFalling() ? MantleFallHeightMultiplier : 1.0f);
		Start = OutHit.Location + FVector(0,0,offset);
		End = OutHit.Location;

		FHitResult SphereHitResult;
		FCollisionQueryParams CollisionParams;

		// Draw debug sphere for each trace in the loop
		DrawDebugSphere(GetWorld(), Start, SphereRadius, 12, FColor::Yellow, false, 5.0f);
		DrawDebugSphere(GetWorld(), End, SphereRadius, 12, FColor::Yellow, false, 5.0f);
			
		// Sphere trace for object height, if player is falling them max height is reduced.
		if (GetWorld()->SweepSingleByChannel(SphereHitResult, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(SphereRadius), CollisionParams))
		{
			
			// Set motion warp positions if there is a collision
			MantlePosition1 = SphereHitResult.ImpactPoint + (GetActorForwardVector() * -50.f);
			MantlePosition2 = SphereHitResult.ImpactPoint + (GetActorForwardVector() * 120.f);

			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("MantlePosition1: %s"), *MantlePosition1.ToString()));
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("MantlePosition2: %s"), *MantlePosition2.ToString()));
			
			DrawDebugSphere(GetWorld(), MantlePosition1, SphereRadius, 12, FColor::Orange, false, 5.0f);
			DrawDebugSphere(GetWorld(), MantlePosition2, SphereRadius, 12, FColor::Red, false, 5.0f);

			bCanMantle = true;
			
			FHitResult InnerSphereHitResult;
			Start = MantlePosition2 + FVector(0,0,20);
			DrawDebugSphere(GetWorld(), Start, SphereRadius, 12, FColor::Blue, false, 5.0f);
			// Sphere trace to check if player has enough space to climb up
			if (GetWorld()->SweepSingleByChannel(InnerSphereHitResult, Start, Start, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(SphereRadius), CollisionParams))
			{
				bCanMantle = false;
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Not enough space for player"));
			}
			else
			{
				MantlePosition2 = SphereHitResult.ImpactPoint + (GetActorForwardVector() * 50);
				if(MantlePosition1.IsZero() || MantlePosition2.IsZero())
				{
					bCanMantle = false;
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Position 1 and 2 are 0,0,0"));
				}
				else
				{
					float ZOffset = MantlePosition2.Z + 100;
					Start = FVector(MantlePosition1.X, MantlePosition1.Y, ZOffset);
					End = MantlePosition2 + FVector(0,0, 120);

					DrawDebugLine(GetWorld(), Start, End, FColor::Purple, false, 5.0f, 0, 2.0f);
					DrawDebugSphere(GetWorld(), MantlePosition1, SphereRadius, 12, FColor::Black, false, 5.0f);
					DrawDebugSphere(GetWorld(), MantlePosition2, SphereRadius, 12, FColor::White, false, 5.0f);
					
					// Final check to see if mantle spot is clear
					if (GetWorld()->SweepSingleByChannel(SphereHitResult, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(SphereRadius), CollisionParams))
					{
						bCanMantle = false;
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Can't Mantle at End"));
					}
					else
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Can Mantle"));
					}
				}
			}
		}
	}
	else
	{
		bCanMantle = false;
	}
}

/// <summary>
/// InitialTraceLength - How close do you need to be to vault
/// SecondaryTraceOffset - Check for how tall the object you want to vault is 
/// SecondaryTraceGap - Determines distance in between traces
/// LnadingPositionForwardOffset - Landing position offset to make it look more accurate
/// </summary>
void AParkourCharacter::VaultTrace()
{
	// local variables

	//Re-initialize hit info
	FHitResult OutHit;

	FVector Start = GetActorLocation() + FVector(0,0,-15);
	FVector End = (Start + GetActorForwardVector() * InitialTraceLength) + FVector(0,0,-15);

	DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 5.0f, 0, 2.0f);
	
	// Line trace for an object to vault over, if object is found it will find vault target locations
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("HIT LINE TRACE"));
		
		VaultDistance = 0;

		float SphereRadius = 10.f;

		for (int i = 0; i < 10; i++)
		{
			VaultDistance++;

			FVector Middle = GetActorForwardVector() * i * SecondaryTraceGap;

			Start = OutHit.Location + Middle;
			End = Start;
			Start.Z += SecondaryTraceZOffset;
			

			FHitResult SphereHitResult;
			FCollisionQueryParams CollisionParams;

			// Draw debug sphere for each trace in the loop
			DrawDebugSphere(GetWorld(), Start, SphereRadius, 12, FColor::Yellow, false, 5.0f);
			//DrawDebugSphere(GetWorld(), End, SphereRadius, 12, FColor::Green, false, 5.0f);
			
			// Sphere trace to determine length and height of object
			if (GetWorld()->SweepSingleByChannel(SphereHitResult, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(SphereRadius), CollisionParams))
			{
				if (i == 0) // set start location of vault if it is the first trace. Sphere trace checks for any blocking that prevents vaulting
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("VAULT START LOCATION"));
					VaultStart = SphereHitResult.ImpactPoint;

					DrawDebugLine(GetWorld(), Start, VaultStart, FColor::Green, false, 5.0f, 0, 2.0f);
					
					// another sphere trace for end
					if (GetWorld()->SweepSingleByChannel(SphereHitResult, FVector(VaultStart.X, VaultStart.Y, VaultStart.Z + 50), FVector(VaultStart.X, VaultStart.Y, VaultStart.Z + 20), FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(SphereRadius), CollisionParams))
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("VAULT START SPHERE TRIGGER"));
						bCanVault = false;
						DrawDebugLine(GetWorld(), VaultStart, FVector(VaultStart.X, VaultStart.Y, VaultStart.Z + 20), FColor::Red, false, 5.0f, 0, 2.0f);
						break;
					}
				}
				else // if the trace is not the first then this will determine the middle / height location
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("VAULT MIDDLE LOCATION"));
					VaultMiddle = SphereHitResult.ImpactPoint;

					DrawDebugLine(GetWorld(), Start, VaultMiddle, FColor::Green, false, 5.0f, 0, 2.0f);
					
					// Another trace for end
					if (GetWorld()->SweepSingleByChannel(SphereHitResult, SphereHitResult.TraceStart, SphereHitResult.TraceStart, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(SphereRadius), CollisionParams))
					{
						bCanVault = false;
						DrawDebugLine(GetWorld(), SphereHitResult.TraceStart, SphereHitResult.TraceEnd, FColor::Red, false, 5.0f, 0, 2.0f);
					}
				}
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("HIT SPHERE TRACE"));
			}
			else // find landing location by doing a line trace down from an offset
			{
				bCanVault = true;

				Middle = GetActorForwardVector() * LandingPositionForwardOffset;
				Start = OutHit.TraceEnd + Middle;
				End = Start + FVector(0, 0, -100);

				DrawDebugLine(GetWorld(), Start, End, FColor::Purple, false, 5.0f, 0, 2.0f);
				
				if(GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility))
				{
					//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("LAND LINE TRACE HIT"));
					DrawDebugSphere(GetWorld(), End, 20, 12, FColor::Red, false, 5.0f);
					if(GetWorld()->SweepSingleByChannel(SphereHitResult, Start, Start, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(SphereRadius), CollisionParams))
					{
						bCanVault = false;
						DrawDebugLine(GetWorld(), Start, Start, FColor::Purple, false, 5.0f, 0, 2.0f);
						//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("CAN'T VAULT"));
					}
					else
					{
						VaultLand = OutHit.Location +FVector(0,0,-20.f);
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("VAULT LAND LOCATION"));
						DrawDebugSphere(GetWorld(), VaultLand, SphereRadius, 12, FColor::Red, false, 5.0f);
					}
				}

				break;
			}
		}
	}
}

// Called every frame
void AParkourCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check if AnimInstance is crouching
}

// Called to bind functionality to input
void AParkourCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) 
	{

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AParkourCharacter::StartJumpCheck);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AParkourCharacter::StopJumpCheck);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AParkourCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AParkourCharacter::Look);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AParkourCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AParkourCharacter::StopSprint);

		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AParkourCharacter::StartCrouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AParkourCharacter::StopCrouch);

		// Parkour Mantle & Vault
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AParkourCharacter::Interact);
	}
}

void AParkourCharacter::SwitchMovementState(EMovementState State)
{
	if (MovementState == State)
	{
		return;
	}

	switch (State)
	{
	case EMovementState::WALKING:
		MovementState = EMovementState::WALKING;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		break;
	case EMovementState::RUNNING:
		MovementState = EMovementState::RUNNING;
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		break;
	case EMovementState::CROUCHING:
		MovementState = EMovementState::CROUCHING;
		GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
		break;
	case EMovementState::SLIDING:
		MovementState = EMovementState::SLIDING;
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed + 150.f;
		break;
	case EMovementState::WALLRUN: // maybe remove
		MovementState = EMovementState::WALLRUN;
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed + 150.f;
		break;
	default:

		break;
	}
}

void AParkourCharacter::ToggleCapsuleSize()
{
	if (AnimInstance->GetCrouching())
	{
		GetCapsuleComponent()->SetCapsuleSize(HalfCapsule.X, HalfCapsule.Y);
	}
	else
	{
		GetCapsuleComponent()->SetCapsuleSize(FullCapsule.X, FullCapsule.Y);
	}
}

/*void AParkourCharacter::SetCrouching()
{
	IsCrouching = !IsCrouching;
}*/

FVector2D AParkourCharacter::GetPlayerMoveValue() const
{
	return CurrentMoveVector;
}
