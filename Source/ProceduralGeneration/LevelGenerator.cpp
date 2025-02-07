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
	if (!GetWorld()) return;

    TArray<FVector> PlatformCenters;
    TArray<float> PlatformHeights;

	TArray<FPlatformData> PlacedPlatforms;  // Store already placed platforms
    
    for (const auto& Floor : Level->GetPartitionedFloor())
        {
            FCornerCoordinates Coords = Floor->GetCornerCoordinates();
            
            float RoofHeight = FMath::RandRange(SpawnParams.baseHeight.X, SpawnParams.baseHeight.Y);



    	// Calculate platform dimensions
    	float Width = (Coords.LowerRightX - Coords.UpperLeftX) * SpawnParams.FloorTileSize;
    	float Length = (Coords.LowerRightY - Coords.UpperLeftY) * SpawnParams.FloorTileSize;
        
    	// Try to find a valid position
    	FVector ValidPosition = FindValidPlatformPosition(
			Coords, 
			Width, 
			Length, 
			RoofHeight, 
			PlacedPlatforms
		);


    	


    	
            // Calculate platform position with offset
            FVector PlatformCenter = CalculatePlatformPosition(Coords, RoofHeight);
            
            // Store platform data
            PlatformCenters.Add(PlatformCenter);
            PlatformHeights.Add(RoofHeight);

    		FVector Position;
    	
    		if(bool CoinFlip = FMath::RandBool())
    		{
    			// Add random offset within reasonable bounds
    			float RandomOffsetX = FMath::RandRange(-Width * 1.2f, Width * 1.2f);
    			
    			Position = FVector(Coords.UpperLeftX * SpawnParams.FloorTileSize + Width / 2 + RandomOffsetX, Coords.UpperLeftY * SpawnParams.FloorTileSize + Length / 2, RoofHeight);
    		}
		    else
		    {
		    	float RandomOffsetY = FMath::RandRange(-Length * 1.2f, Length * 1.2f);
		    	
		    	Position = FVector( Coords.UpperLeftX * SpawnParams.FloorTileSize + Width / 2, Coords.UpperLeftY * SpawnParams.FloorTileSize + Length / 2 + RandomOffsetY,RoofHeight);
		    }
    	
            
            
            

            FRotator Rotation(180, 0, 0);

            // Spawn the roof actor
            FActorSpawnParameters ActorParams;
            ActorParams.Owner = GetOwner();
            
            AStaticMeshActor* RoofActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Position, Rotation, ActorParams);

            if (RoofActor)
            {
                UStaticMesh* SelectedMesh = SpawnMeshes.Mesh;
                
                if (SelectedMesh)
                {
                    UStaticMeshComponent* MeshComp = RoofActor->GetStaticMeshComponent();
                    MeshComp->SetMobility(EComponentMobility::Movable);
                    MeshComp->SetStaticMesh(SelectedMesh);
                    MeshComp->SetWorldScale3D(FVector(Width / 100.0f, Length / 100.0f, 50.0f));
                    SpawnedActors.Add(RoofActor);

                	// Store the platform data
                	FPlatformData NewPlatform;
                	NewPlatform.Position = ValidPosition;
                	NewPlatform.Width = Width;
                	NewPlatform.Length = Length;
                	PlacedPlatforms.Add(NewPlatform);
                }
            }
        }
}

FVector ALevelGenerator::CalculatePlatformPosition(const FCornerCoordinates& Coords, float Height) const
{
	// Calculate grid dimensions
	int32 GridWidth = Coords.LowerRightX - Coords.UpperLeftX;
	int32 GridHeight = Coords.LowerRightY - Coords.UpperLeftY;
    
	// Add randomization to platform positioning within the grid cell
	float RandomOffsetX = FMath::RandRange(-GridWidth * 0.25f, GridWidth * 0.25f) * SpawnParams.FloorTileSize;
	float RandomOffsetY = FMath::RandRange(-GridHeight * 0.25f, GridHeight * 0.25f) * SpawnParams.FloorTileSize;
    
	// Calculate center position with random offset
	return FVector(
		(Coords.UpperLeftX + (GridWidth/2.0f)) * SpawnParams.FloorTileSize + RandomOffsetX,
		(Coords.UpperLeftY + (GridHeight/2.0f)) * SpawnParams.FloorTileSize + RandomOffsetY,
		Height
	);
}

FVector ALevelGenerator::FindValidPlatformPosition(const FCornerCoordinates& Coords, float Width, float Length, float Height, const TArray<FPlatformData>& PlacedPlatforms)
{
    const int32 MaxAttempts = 10;
    const float MinimumSpacing = 50.0f; // Ensure spacing between platforms

    for (int32 Attempt = 0; Attempt < MaxAttempts; Attempt++)
    {
        float RandomOffsetX = FMath::RandRange(-Width * 1.35f, Width * 1.35f);
        float RandomOffsetY = FMath::RandRange(-Length * 1.35f, Length * 1.35f);

        FVector TestPosition(
            Coords.UpperLeftX * SpawnParams.FloorTileSize + Width / 2 + RandomOffsetX,
            Coords.UpperLeftY * SpawnParams.FloorTileSize + Length / 2 + RandomOffsetY,
            Height
        );

        FVector AdjustedPosition = TestPosition;
        bool NeedsAdjustment = false;

        for (const FPlatformData& PlacedPlatform : PlacedPlatforms)
        {
            float XDistance = FMath::Abs(TestPosition.X - PlacedPlatform.Position.X);
            float YDistance = FMath::Abs(TestPosition.Y - PlacedPlatform.Position.Y);

            float RequiredXDistance = (Width + PlacedPlatform.Width) / 2 + MinimumSpacing;
            float RequiredYDistance = (Length + PlacedPlatform.Length) / 2 + MinimumSpacing;

            if (XDistance < RequiredXDistance && YDistance < RequiredYDistance)
            {
                NeedsAdjustment = true;

                // Push out fully in both X and Y directions
                if (XDistance < RequiredXDistance)
                {
                    if (TestPosition.X > PlacedPlatform.Position.X)
                        AdjustedPosition.X = PlacedPlatform.Position.X + (PlacedPlatform.Width / 2) + (Width / 2) + MinimumSpacing;
                    else
                        AdjustedPosition.X = PlacedPlatform.Position.X - (PlacedPlatform.Width / 2) - (Width / 2) - MinimumSpacing;
                }

                if (YDistance < RequiredYDistance)
                {
                    if (TestPosition.Y > PlacedPlatform.Position.Y)
                        AdjustedPosition.Y = PlacedPlatform.Position.Y + (PlacedPlatform.Length / 2) + (Length / 2) + MinimumSpacing;
                    else
                        AdjustedPosition.Y = PlacedPlatform.Position.Y - (PlacedPlatform.Length / 2) - (Length / 2) - MinimumSpacing;
                }
            }
        }

        if (!NeedsAdjustment)
        {
            return TestPosition; // Valid position found
        }
        else
        {
            return AdjustedPosition; // Return fully adjusted position
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("No valid position found after max attempts!"));

    return FVector(
        Coords.UpperLeftX * SpawnParams.FloorTileSize + Width / 2,
        Coords.UpperLeftY * SpawnParams.FloorTileSize + Length / 2,
        Height
    );
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
