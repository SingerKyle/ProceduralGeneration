// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelGenerator.h"

#include "Engine/StaticMeshActor.h"
#include "Field/FieldSystemNoiseAlgo.h"
#include "Procedural Generation/FloorNode.h"


// Sets default values
ALevelGenerator::ALevelGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ALevelGenerator::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

// Called when the game starts or when spawned
void ALevelGenerator::BeginPlay()
{
	Super::BeginPlay();
	
	Level = MakeShareable(new Floor(GetActorLocation(), SpawnParams.MapDimensions, SpawnParams.FloorTileSize, SpawnParams.SplitRate, SpawnParams.MinBounds, SpawnParams.bUseMaxSize));
	
	Level->Partition();
	
	Level->DrawFloorNodes(GetWorld());

	SpawnGrid();
	
	//DrawDebugLines();
	
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
	
	if(!PlacedPlatforms.IsEmpty())
	{
		PlacedPlatforms.Empty();
	}

	if(Level.IsValid())
	{
		Level->Reinitialise(GetActorLocation(), SpawnParams.MapDimensions, SpawnParams.FloorTileSize, SpawnParams.SplitRate, SpawnParams.MinBounds, SpawnParams.bUseMaxSize);
	
		Level->ClearPartitionedFloor();
	
		Level->Partition();
	
		Level->DrawFloorNodes(GetWorld());

		SpawnGrid();
	
		//DrawDebugLines();
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
    if (!GetWorld() || Level->GetPartitionedFloor().Num() == 0) return;

    //TArray<FPlatformData> PlacedPlatforms;
	
    //const float MinPlatformDistance = SpawnParams.FloorTileSize * 0.5f;
    
    // Shuffle the floor nodes for random placement order
    TArray<TSharedPtr<FloorNode>> ShuffledFloors = Level->GetPartitionedFloor();
    for (int32 i = ShuffledFloors.Num() - 1; i > 0; --i)
    {
        int32 SwapIndex = FMath::RandRange(0, i);
        ShuffledFloors.Swap(i, SwapIndex);
    }

    // Try to place each platform
    for (const auto& Floor : ShuffledFloors)
    {
        FCornerCoordinates Coords = Floor->GetCornerCoordinates();
        
        // Calculate random platform dimensions (in grid units)
        int32 GridWidth = FMath::RandRange(1, FMath::Min(99, Coords.LowerRightX - Coords.UpperLeftX));
        int32 GridLength = FMath::RandRange(1, FMath::Min(99, Coords.LowerRightY - Coords.UpperLeftY));
        
        // Convert to world units
        float Width = GridWidth * SpawnParams.FloorTileSize;
        float Length = GridLength * SpawnParams.FloorTileSize;
        float Height = FMath::RandRange(SpawnParams.baseHeight.X, SpawnParams.baseHeight.Y);

        // Try multiple positions for each platform
        const int32 MaxAttempts = 15;
        bool ValidPositionFound = false;

        for (int32 Attempt = 0; Attempt < MaxAttempts && !ValidPositionFound; ++Attempt)
        {
            FVector ProposedPosition = CalculatePlatformPosition(Coords, Height, GridWidth, GridLength);
            
            // Create platform data for validation
            FPlatformData NewPlatform(ProposedPosition, FVector(Width, Length, 50.0f));

            // Check if position is valid
            bool IsValidPosition = NewPlatform.IsWithinGrid(
                FVector2D(SpawnParams.MapDimensions.X * SpawnParams.FloorTileSize, SpawnParams.MapDimensions.Y * SpawnParams.FloorTileSize),
                SpawnParams.FloorTileSize
            );
            
            if (IsValidPosition)
            {
                for (const FPlatformData& ExistingPlatform : PlacedPlatforms)
                {
                    if (NewPlatform.OverlapsWith(ExistingPlatform, SpawnParams.MinJumpDistance))
                    {
                        IsValidPosition = false;
                        break;
                    }
                }
            }

            if (IsValidPosition)
            {
                // Spawn the platform
                FRotator Rotation(180, 0, 0);
                AStaticMeshActor* PlatformActor = GetWorld()->SpawnActor<AStaticMeshActor>(
                    AStaticMeshActor::StaticClass(), 
                    ProposedPosition, 
                    Rotation
                );

                if (PlatformActor && SpawnMeshes.Mesh)
                {
                    UStaticMeshComponent* MeshComp = PlatformActor->GetStaticMeshComponent();
                    MeshComp->SetMobility(EComponentMobility::Movable);
                    MeshComp->SetStaticMesh(SpawnMeshes.Mesh);
                    MeshComp->SetWorldScale3D(FVector(Width / 100.0f, Length / 100.0f, 50.0f));

                	SpawnedActors.Add(PlatformActor);
                    PlacedPlatforms.Add(NewPlatform);
                    ValidPositionFound = true;
                }
            }
        }
    }

	PlaceMantleObjects();
}

FVector ALevelGenerator::CalculatePlatformPosition(const FCornerCoordinates& Coords, float Height, int32 PlatformWidth, int32 PlatformLength) const
{
    // Calculate available space in the grid cell
    int32 GridWidth = Coords.LowerRightX - Coords.UpperLeftX;
    int32 GridHeight = Coords.LowerRightY - Coords.UpperLeftY;
    
    // Calculate maximum allowed offset based on platform size
    float MaxOffsetX = (GridWidth - PlatformWidth) * 0.5f * SpawnParams.FloorTileSize;
    float MaxOffsetY = (GridHeight - PlatformLength) * 0.5f * SpawnParams.FloorTileSize;
    
    // Add randomization within the available space
    float RandomOffsetX = FMath::RandRange(-MaxOffsetX, MaxOffsetX);
    float RandomOffsetY = FMath::RandRange(-MaxOffsetY, MaxOffsetY);
    
    return FVector(
        (Coords.UpperLeftX + (GridWidth/2.0f)) * SpawnParams.FloorTileSize + RandomOffsetX,
        (Coords.UpperLeftY + (GridHeight/2.0f)) * SpawnParams.FloorTileSize + RandomOffsetY,
        Height
    );
}

void ALevelGenerator::PlaceMantleObjects()
{
    
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
    float GridWidth = SpawnParams.MapDimensions.X * (SpawnParams.FloorTileSize);
    float GridHeight = SpawnParams.MapDimensions.Y * (SpawnParams.FloorTileSize);

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
