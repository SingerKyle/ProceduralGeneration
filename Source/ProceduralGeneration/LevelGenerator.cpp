// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelGenerator.h"

#include "Field/FieldSystemNoiseAlgo.h"
#include "Procedural Generation/FloorNode.h"


// Sets default values
ALevelGenerator::ALevelGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	heightVariance = 5000.0f;
	baseHeight = 0.f;
	MinBounds = FVector2D(5,5);
}

void ALevelGenerator::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

// Called when the game starts or when spawned
void ALevelGenerator::BeginPlay()
{
	Super::BeginPlay();
	
	Level = MakeShareable(new Floor(GetActorLocation(), MapDimensions, FloorTileSize, SplitRate, MinBounds));
	
	Level->Partition();
	
	Level->DrawFloorNodes(GetWorld());

	/*for(int i = 0; i < Level->GetPartitionedFloor().Num(); i++)
	{
		UE_LOG(LogTemp, Warning, TEXT("X:Y Coordinate of each partitioned floor: %f : %f")
		, Level->GetPartitionedFloor()[i]->GetCornerCoordinates().LowerRightX * FloorTileSize, Level->GetPartitionedFloor()[i]->GetCornerCoordinates().LowerRightY * FloorTileSize);
	}*/

	SpawnGrid();
	
	DrawDebugLines();
	
}

void ALevelGenerator::InitialiseGrid()
{
	FlushPersistentDebugLines(GetWorld());

	if(!SpawnedActors.IsEmpty())
	{
		for (AActor* Actor : SpawnedActors)
		{
			if (Actor && Actor->IsValidLowLevel())
			{
				Actor->Destroy();
			}
		}
	
		SpawnedActors.Empty();
	}

	if(Level.IsValid())
	{
		Level->ClearPartitionedFloor();
	
		Level->Partition();
	
		Level->DrawFloorNodes(GetWorld());

		SpawnGrid();
	
		DrawDebugLines();
	}
}

void ALevelGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	//MapDimensions.X *= FloorTileSize;
	//MapDimensions.Y *= FloorTileSize;

	
}

// Called every frame
void ALevelGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}

void ALevelGenerator::SpawnGrid()
{
	FVector Origin = GetActorLocation();

	FVector lastLevelLocation = Origin; // Track the last level's location

	FVector lastPlatformCenter = FVector::ZeroVector;

	for (int i = 0; i < Level->GetPartitionedFloor().Num(); i++)
	{
		float noiseValue = FMath::PerlinNoise2D(FVector2D(i * FloorTileSize + 0.2, i * FloorTileSize + 0.2)) * heightVariance;
		//UE_LOG(LogTemp, Warning, TEXT("float for noise: %f"), noiseValue);
		float height = baseHeight + noiseValue;
		//UE_LOG(LogTemp, Warning, TEXT("Height Value: %f"), height);
		
		FCornerCoordinates CornerCoords = Level->GetPartitionedFloor()[i]->GetCornerCoordinates();

		int levelWidth = (CornerCoords.LowerRightX - CornerCoords.UpperLeftX) * FloorTileSize;
		int levelDepth = (CornerCoords.LowerRightY - CornerCoords.UpperLeftY) * FloorTileSize;
		
		FVector levelStartLocation = lastLevelLocation + FVector(FMath::RandRange(MinJumpDistance, MaxJumpDistance) + levelWidth,
			FMath::RandRange(MinJumpDistance, MaxJumpDistance) + levelDepth, 0);

		// Calculate the center of this platform
		 FVector platformCenter = FVector(
		 (CornerCoords.UpperLeftX + CornerCoords.LowerRightX) * 0.5f * FloorTileSize,
		(CornerCoords.UpperLeftY + CornerCoords.LowerRightY) * 0.5f * FloorTileSize,
		height);

		// If there is a previously spawned platform, calculate the distance
		if (!lastPlatformCenter.IsZero())
		{
			float distance = FVector::Dist(platformCenter, lastPlatformCenter);

			// Log or process the distance between the two platforms
			UE_LOG(LogTemp, Warning, TEXT("Distance between platforms: %f"), distance);
		}
		
		for (int x = CornerCoords.UpperLeftX; x < CornerCoords.LowerRightX; x++)
		{
			for (int y = CornerCoords.UpperLeftY; y < CornerCoords.LowerRightY; y++)
			{
				
				FVector SpawnLocation = levelStartLocation + 
					FVector(x * FloorTileSize, y * FloorTileSize, height);

				FActorSpawnParameters params;
				params.Owner = this;
				AActor* Actor = GetWorld()->SpawnActor<AActor>(FloorBPClass, SpawnLocation, FRotator::ZeroRotator);

				if(Actor)
				{
					Actor->GetComponentByClass<UStaticMeshComponent>()->SetWorldScale3D(FVector(2.5,2.5,100.f));
					SpawnedActors.Push(Actor);
				}
			}
		}

		lastPlatformCenter = platformCenter;
		lastLevelLocation = levelStartLocation;
	}
	
}

/*void ALevelGenerator::SpawnGrid()
{
		for (int32 Col = 0; Col < MapDimensions.X; Col++)
	{
		for (int32 Row = 0; Row < MapDimensions.Y; Row++)
		{
			//float Gap = FMath::RandRange(MinJumpDistance, MaxJumpDistance);
			FVector SpawnLocation = Origin + FVector(Col * (FloorTileSize), Row * (FloorTileSize), 0.f);

			if (FMath::RandRange(0,100) > 90) // Randomly decide to spawn floor tile
				{
				
				GetWorld()->SpawnActor<AActor>(FloorBPClass, SpawnLocation, FRotator::ZeroRotator);
				}
		}
	}
}*/

void ALevelGenerator::DrawDebugLines()
{
	
    FVector Origin = GetActorLocation();
    float GridWidth = MapDimensions.X * (FloorTileSize);
    float GridHeight = MapDimensions.Y * (FloorTileSize);

    // Bottom-left corner
    FVector BottomLeft = Origin;
    // Bottom-right corner
    FVector BottomRight = Origin + FVector(GridWidth, 0.0f, 0.0f);
    // Top-right corner
    FVector TopRight = Origin + FVector(GridWidth, GridHeight, 0.0f);
    // Top-left corner
    FVector TopLeft = Origin + FVector(0.0f, GridHeight, 0.0f);

    // Draw lines between the corners
    DrawDebugLine(GetWorld(), BottomLeft, BottomRight, FColor::Red, true, -1.0f, 0, 5.0f);
    DrawDebugLine(GetWorld(), BottomRight, TopRight, FColor::Red, true, -1.0f, 0, 5.0f);
    DrawDebugLine(GetWorld(), TopRight, TopLeft, FColor::Red, true, -1.0f, 0, 5.0f);
    DrawDebugLine(GetWorld(), TopLeft, BottomLeft, FColor::Red, true, -1.0f, 0, 5.0f);
}
