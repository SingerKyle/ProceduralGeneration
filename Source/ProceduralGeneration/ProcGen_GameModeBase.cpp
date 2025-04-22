// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcGen_GameModeBase.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

void AProcGen_GameModeBase::RestartPlayerAtPlatform(AController* PlayerController)
{
	if (!PlayerController || LevelGenerator->GetPlacedPlatforms().Num() == 0) return;

	AActor* FirstPlatform = LevelGenerator->GetPlacedPlatforms()[0]; // Get the first platform
	if (!FirstPlatform) return;

	FVector SpawnLocation = FirstPlatform->GetActorLocation() + FVector(0, 0, 100); // Offset so player is above platform
	FRotator SpawnRotation = FRotator::ZeroRotator;

	// Move player to the new location
	if (ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerController->GetPawn()))
	{
		PlayerCharacter->SetActorLocation(SpawnLocation);
		PlayerCharacter->SetActorRotation(SpawnRotation);
	}
	else
	{
		// If no pawn, spawn a new one
		APawn* NewPawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, SpawnLocation, SpawnRotation);
		PlayerController->Possess(NewPawn);
	}
}

AProcGen_GameModeBase::AProcGen_GameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AProcGen_GameModeBase::BeginPlay()
{
	Super::BeginPlay();

	
	if(AGrammarGenerator* LevelGen = Cast<AGrammarGenerator>(UGameplayStatics::GetActorOfClass(this, AGrammarGenerator::StaticClass())))
	{
		LevelGenerator = LevelGen;
	}

}

void AProcGen_GameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	for (AController* PlayerController : TActorRange<APlayerController>(GetWorld()))
	{
		if (ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerController->GetPawn()))
		{
			if(PlayerCharacter->GetActorLocation().Z < FallThresholdZ && bHumanLevel)
			{
				PlayerCharacter->SetActorLocation(HumanStartPoint);
			}
			else if (PlayerCharacter->GetActorLocation().Z < FallThresholdZ && LevelGenerator)
			{
				RestartPlayerAtPlatform(PlayerController);
			}

			if (PlayerCharacter && LevelGenerator && LevelGenerator->GetPlacedPlatforms().Num() > 0)
			{
				AActor* LastPlatform = LevelGenerator->GetPlacedPlatforms().Last();
				if (LastPlatform)
				{
					float Distance = FVector::Dist(PlayerCharacter->GetActorLocation(), LastPlatform->GetActorLocation());
					if (Distance <= 400.f)
					{
						UGameplayStatics::OpenLevel(this, FName("MainMenuLevel"), true);
						return;
					}
				}
			}
			else if (PlayerCharacter && bHumanLevel)
			{
				float Distance = FVector::Dist(PlayerCharacter->GetActorLocation(), HumanEndPoint);
				if (Distance <= 400.f)
				{
					UGameplayStatics::OpenLevel(this, FName("MainMenuLevel"), true);
					bHumanLevel = false;
					return;
				}
			}
		}
	}
	
}
