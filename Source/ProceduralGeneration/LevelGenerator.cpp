// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelGenerator.h"

#include "Engine/StaticMeshActor.h"
#include "Field/FieldSystemNoiseAlgo.h"
#include "Kismet/KismetSystemLibrary.h"
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
    
    // Shuffle the floor nodes for random placement order
    TArray<TSharedPtr<FloorNode>> ShuffledFloors = Level->GetPartitionedFloor();
    for (int32 i = ShuffledFloors.Num() - 1; i >= 0; --i)
    {
        int32 SwapIndex = FMath::RandRange(0, i);
        ShuffledFloors.Swap(i, SwapIndex);
    }

    // Try to place each platform
    for (const auto& Floor : ShuffledFloors)
    {
        FCornerCoordinates Coords = Floor->GetCornerCoordinates();
        
        // Calculate random platform dimensions (in grid units)
        int32 GridWidth = FMath::RandRange(1, Coords.LowerRightX - Coords.UpperLeftX);
        int32 GridLength = FMath::RandRange(1, Coords.LowerRightY - Coords.UpperLeftY);
        
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

	AnalyseAndSpawnParkourConnections();
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

void ALevelGenerator::AnalyseAndSpawnParkourConnections()
{
	if (PlacedPlatforms.Num() < 2) return;

	for (int32 i = 0; i < PlacedPlatforms.Num(); i++)
	{
		for (int32 j = i + 1; j < PlacedPlatforms.Num(); j++)
		{
			const FPlatformData& Platform1 = PlacedPlatforms[i];
			const FPlatformData& Platform2 = PlacedPlatforms[j];

			// Calculate horizontal distance
			float Distance = FVector::Dist2D(
				FVector(Platform1.Position.X, Platform1.Position.Y, 0),
				FVector(Platform2.Position.X, Platform2.Position.Y, 0)
			);
            
			// Calculate height difference from the top of the lower platform to the bottom of the higher platform
			float Height1 = Platform1.Position.Z;
			float Height2 = Platform2.Position.Z;
			float HeightDiff = FMath::Abs(Height2 - Height1);

			EParkourType ParkourType = DetermineParkourType(Distance, HeightDiff);
            
			if (ParkourType != EParkourType::None)
			{
				SpawnParkourConnection(Platform1, Platform2, ParkourType);
			}
		}
	}
}

EParkourType ALevelGenerator::DetermineParkourType(float Distance, float HeightDiff)
{
    // These values should be tweaked based on your game's mechanics
	const float MantleMinHorizontalDistance = 500.0f;  
	const float MantleMinHeight = 800.0f;              
	const float MantleMaxHeight = 2000.0f;        
    
	const float WallRunMinDistance = 2000.0f;          
	const float WallRunMaxDistance = 5000.0f;
    
	const float LedgeGrabMaxDistance = 300.0f;
	const float LedgeGrabMinHeight = 500.0f;
    
	if (HeightDiff > MantleMinHeight
		&& HeightDiff < MantleMaxHeight
		&& Distance > MantleMinHorizontalDistance
		&& Distance < SpawnParams.MantleMaxDistance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Mantle Detected - Height: %f, Distance: %f"), HeightDiff, Distance);
		return EParkourType::Mantle;
	}

	// Check for Wall Run
	if (Distance > WallRunMinDistance &&
		Distance < WallRunMaxDistance &&
		FMath::Abs(HeightDiff) < SpawnParams.WallRunMaxHeight)
	{
		return EParkourType::WallRun;
	}
    
    // Check for Ledge Grab
    if (Distance < LedgeGrabMaxDistance && 
        HeightDiff > LedgeGrabMinHeight)
    {
        return EParkourType::LedgeGrab;
    }

    return EParkourType::None;
}

void ALevelGenerator::SpawnParkourConnection(const FPlatformData& Start, const FPlatformData& End, EParkourType Type)
{
    FVector StartPoint = Start.Position;
    FVector EndPoint = End.Position;
    
    switch(Type)
    {
        case EParkourType::Mantle:
            SpawnMantleIndicator(StartPoint, EndPoint);
    		UE_LOG(LogTemp, Warning, TEXT("Mantle"));
            break;
            
        case EParkourType::WallRun:
            SpawnWallRunSurface(Start, End);
        	UE_LOG(LogTemp, Warning, TEXT("WallRun"));
            break;
            
        case EParkourType::LedgeGrab:
            //SpawnLedgeGrabIndicator(StartPoint, EndPoint);
        	UE_LOG(LogTemp, Warning, TEXT("Ledge"));
            break;
    }
}

void ALevelGenerator::SpawnMantleIndicator(const FVector& Start, const FVector& End)
{
	if (!SpawnMeshes.ClimbMesh) return;
    
	// Find the corresponding platforms
	FPlatformData* StartPlatform = nullptr;
	FPlatformData* EndPlatform = nullptr;
    
	for (FPlatformData& Platform : PlacedPlatforms)
	{
		if (Platform.Position.Equals(Start, 1.0f))
			StartPlatform = &Platform;
		if (Platform.Position.Equals(End, 1.0f))
			EndPlatform = &Platform;
	}
    
	if (!StartPlatform || !EndPlatform) return;

	// Get all edges of both platforms
	TArray<FPlatformEdge> StartEdges = GetPlatformEdges(*StartPlatform);
	TArray<FPlatformEdge> EndEdges = GetPlatformEdges(*EndPlatform);
    
	// Find closest edges between platforms
	FPlatformEdge ClosestStartEdge;
	FPlatformEdge ClosestEndEdge;
	float MinDistance = MAX_FLT;
    
	for (const FPlatformEdge& StartEdge : StartEdges)
	{
		for (const FPlatformEdge& EndEdge : EndEdges)
		{
			float Dist = FVector::DistSquared(StartEdge.Start, EndEdge.Start);
			if (Dist < MinDistance)
			{
				MinDistance = Dist;
				ClosestStartEdge = StartEdge;
				ClosestEndEdge = EndEdge;
			}
		}
	}
    
	// Calculate path points along the edges
	TArray<FVector> PathPoints;
	GenerateEdgeFollowingPath(ClosestStartEdge, ClosestEndEdge, PathPoints);
    
	// Spawn mantle points along the path
	SpawnMantlePointsAlongPath(PathPoints);
}

TArray<FPlatformEdge> ALevelGenerator::GetPlatformEdges(const FPlatformData& Platform)
{
    TArray<FPlatformEdge> Edges;
    FVector Extents = Platform.Dimensions * 0.5f;
    FVector Center = Platform.Position;
    
    
    TArray<FVector> Corners;
    for (int32 i = 0; i < 8; ++i)
    {
        Corners.Add(FVector(
            Center.X + (i & 1 ? Extents.X : -Extents.X),
            Center.Y + (i & 2 ? Extents.Y : -Extents.Y),
            Center.Z + (i & 4 ? Extents.Z : -Extents.Z)
        ));
    }
    
    
    for (int32 i = 0; i < 4; ++i)
    {
        FVector Normal = FVector::CrossProduct(
            Corners[(i+1)%4] - Corners[i],
            Corners[i+4] - Corners[i]
        ).GetSafeNormal();
        
        Edges.Add(FPlatformEdge(Corners[i], Corners[i+4], Normal));
    }
    
    return Edges;
}

void ALevelGenerator::GenerateEdgeFollowingPath(const FPlatformEdge& StartEdge, const FPlatformEdge& EndEdge, TArray<FVector>& OutPoints)
{
	// Clear output array
	OutPoints.Empty();

	// Calculate total path length and height difference
	float TotalDist = FVector::Dist(StartEdge.Start, EndEdge.End);
	float HeightDiff = EndEdge.Start.Z - StartEdge.Start.Z;

	// Calculate number of points based on distance
	int32 NumPoints = FMath::Max(3, FMath::CeilToInt(TotalDist / 300.0f));

	for (int32 i = 0; i < NumPoints; ++i)
	{
		// Normalized interpolation factor
		float Alpha = static_cast<float>(i) / (NumPoints - 1);

		// Interpolate position along the main direction
		FVector Point = FMath::Lerp(StartEdge.Start, EndEdge.End, Alpha);

		// Ensure points move upwards in a smooth arc
		float HeightAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);
		Point.Z = StartEdge.Start.Z + HeightDiff * HeightAlpha;
		
		Point += FVector(
			FMath::RandRange(-5.0f, 5.0f),  
			FMath::RandRange(-5.0f, 5.0f),
			0.0f
		);

		OutPoints.Add(Point);
	}

	for (const FVector& Point : OutPoints)
	{
		FVector TraceStart = Point;
		FVector TraceEnd = Point;
        
		float SphereRadius = 50.0f;
		FHitResult HitResult;
		bool bHit = UKismetSystemLibrary::SphereTraceSingle(
			this,
			TraceStart,
			TraceEnd,
			SphereRadius,
			UEngineTypes::ConvertToTraceType(ECC_Visibility),
			false,  
			TArray<AActor*>(), 
			EDrawDebugTrace::Persistent, 
			HitResult,
			true, 
			FLinearColor::Red, 
			FLinearColor::Green, 
			-1.0f 
		);
		
		DrawDebugSphere(
			GetWorld(),
			Point,
			SphereRadius,
			12, 
			bHit ? FColor::Red : FColor::Green, 
			true, 
			0.0f 
		);
	}
}

void ALevelGenerator::SpawnMantlePointsAlongPath(const TArray<FVector>& PathPoints)
{
    for (int32 i = 0; i < PathPoints.Num(); ++i)
    {
    	
        // Spawn the mantle point
        AStaticMeshActor* MantlePoint = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            PathPoints[i],
            FRotator(180,0,0)
        );
        
        if (MantlePoint)
        {
            UStaticMeshComponent* MeshComp = MantlePoint->GetStaticMeshComponent();
        	MeshComp->SetMobility(EComponentMobility::Movable);
            MeshComp->SetStaticMesh(SpawnMeshes.ClimbMesh);
            
            // Adjust scale based on position in sequence
            float ScaleMultiplier = (i == 0 || i == PathPoints.Num() - 1) ? 1.0f : 1.25f;
            MeshComp->SetWorldScale3D(FVector(4.0, 4.5, 2.0f) * ScaleMultiplier);
            
            MantlePoint->Tags.Add(FName("Mantle"));
            SpawnedActors.Add(MantlePoint);
        }
    }
}

void ALevelGenerator::SpawnWallRunSurface(const FPlatformData& Start, const FPlatformData& End)
{
    if (!SpawnMeshes.WallRunMesh)
        return;

    // Check if platforms are too far apart
    float Distance = GetPlatformEdgeDistance(Start, End);
	if (Distance < SpawnParams.MinJumpDistance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Wallrun doesn't meet minimum distance requirement"));
		return; // return, player should be able to jump between platforms
	}

    // Check height difference is appropriate for wall running
    float HeightDiff = FMath::Abs(Start.Position.Z - End.Position.Z);
    if (HeightDiff > SpawnParams.WallRunMaxHeight)
    {
    	UE_LOG(LogTemp, Warning, TEXT("Wallrun angle too steep"));
    	return;
    }
    	

	// Check if path is clear of obstacles
	if (!IsPathClear(Start, End))
	{
		UE_LOG(LogTemp, Warning, TEXT("Wallrun Path not clear"));
		return;
	}

	// Get actual edge points for wall placement
	FVector StartPoint, EndPoint;
	GetClosestPlatformPoints(Start, End, StartPoint, EndPoint);
	
    // Check for platforms between Start and End
    FVector Direction = (End.Position - Start.Position).GetSafeNormal();
    for (const FPlatformData& Platform : PlacedPlatforms)
    {
        if (&Platform == &Start || &Platform == &End)
            continue;

        // Project platform onto line between edge points
        FVector StartToPlat = Platform.Position - StartPoint;
        float Projection = FVector::DotProduct(StartToPlat, Direction);
        
        if (Projection > 0 && Projection < Distance)
        {
            FVector ClosestPoint = StartPoint + Direction * Projection;
            float DistToLine = FVector::Dist(Platform.Position, ClosestPoint);
            
            // Increased clearance check using platform dimensions
            float MinClearance = Platform.Dimensions.GetMax();// + SpawnParams.MinJumpDistance;
            if (DistToLine < MinClearance)
                return;
        }
    }

    // Calculate wall position using edge points
    FVector MidPoint = (StartPoint + EndPoint) * 0.5f;
    
    // Adjust height to be centered between platforms
    float AverageHeight = (Start.Position.Z + End.Position.Z) * 0.25f;
    MidPoint.Z = AverageHeight;
    
    FRotator Rotation = Direction.Rotation();

    // Spawn the wall run mesh
    AStaticMeshActor* WallActor = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        MidPoint,
        Rotation
    );

    if (WallActor)
    {
        UStaticMeshComponent* MeshComp = WallActor->GetStaticMeshComponent();
        MeshComp->SetMobility(EComponentMobility::Movable);
        MeshComp->SetStaticMesh(SpawnMeshes.WallRunMesh);

        // Adjust wall dimensions based on edge distance
        float Length = Distance * 0.8f; // Slightly shorter than full distance
        float Height = FMath::Min(350.0f, SpawnParams.WallRunMaxHeight); // Cap the height
        float Thickness = 50.0f;
        
        MeshComp->SetWorldScale3D(FVector(Length / 100.0f, Thickness / 100.0f, Height / 100.0f));
        
        WallActor->Tags.Add(FName("WallRun"));
        SpawnedActors.Add(WallActor);
    }
}

bool ALevelGenerator::IsPathClear(const FPlatformData& Start, const FPlatformData& End) const
{
	// Get points on platform edges that would be used for movement
	FVector StartPoint, EndPoint;
	GetClosestPlatformPoints(Start, End, StartPoint, EndPoint);
    
	// Setup collision query
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(SpawnedActors); // Ignore already spawned platforms
    
	FHitResult HitResult;
    
	// Check if there's anything blocking the path
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartPoint,
		EndPoint,
		ECC_Visibility,
		QueryParams
	);
    
	return !bHit; // Return true if path is clear (no hit)
}

void ALevelGenerator::GetClosestPlatformPoints(const FPlatformData& Start, const FPlatformData& End,
	FVector& OutStartPoint, FVector& OutEndPoint) const
{
	// Get direction vector between platforms
	FVector Direction = (End.Position - Start.Position).GetSafeNormal();
    
	// Calculate the half-dimensions of each platform
	FVector StartHalfDim = Start.Dimensions * 0.5f;
	FVector EndHalfDim = End.Dimensions * 0.5f;
    
	// Start platform edge point
	OutStartPoint = Start.Position + FVector(
		Direction.X * StartHalfDim.X,
		Direction.Y * StartHalfDim.Y,
		0.0f  // Keep Z at platform position
	);
    
	// End platform edge point
	OutEndPoint = End.Position - FVector(
		Direction.X * EndHalfDim.X,
		Direction.Y * EndHalfDim.Y,
		0.0f  // Keep Z at platform position
	);
}

float ALevelGenerator::GetPlatformEdgeDistance(const FPlatformData& Start, const FPlatformData& End) const
{
	FVector StartPoint, EndPoint;
	GetClosestPlatformPoints(Start, End, StartPoint, EndPoint);
    
	// Calculate actual edge-to-edge distance
	float HorizontalDist = FVector::Dist2D(StartPoint, EndPoint);
	float VerticalDist = FMath::Abs(Start.Position.Z - End.Position.Z);
    
	// Return true 3D distance between edges
	return FMath::Sqrt(HorizontalDist * HorizontalDist + VerticalDist * VerticalDist);
}

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
