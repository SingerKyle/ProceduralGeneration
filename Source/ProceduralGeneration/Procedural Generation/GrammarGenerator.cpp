#include "GrammarGenerator.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"

AGrammarGenerator::AGrammarGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AGrammarGenerator::BeginPlay()
{
    Super::BeginPlay();
    GenerateLevel();
}

void AGrammarGenerator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AGrammarGenerator::ClearLevel()
{
    // Clear Debug Messages
    FlushPersistentDebugLines(GetWorld());
    FlushDebugStrings(GetWorld());

    // Clear all arrays of information
    PlacedLocations.Empty();
    PlacedScales.Empty();
    if (!PlacedPlatforms.IsEmpty())
    {
        for(auto Platform : PlacedPlatforms)
        {
            Platform->Destroy();
        }
        PlacedPlatforms.Empty();
    }
    
    if (!PlacedObstacles.IsEmpty())
    {
        for(auto Obstacle : PlacedObstacles)
        {
            Obstacle->Destroy();
        }
        PlacedObstacles.Empty();
    }

    if (!PlacedBuildings.IsEmpty())
    {
        for(auto Building : PlacedBuildings)
        {
            Building->Destroy();
        }
        PlacedBuildings.Empty();
    }
    
}

void AGrammarGenerator::GenerateLevel()
{
    ClearLevel();
    
    GeneratePlatformChain();

    PopulateWorld();
}

EPlacementDirection AGrammarGenerator::GetOppositeDirection(EPlacementDirection Dir)
{
    switch (Dir)
    {
    case EPlacementDirection::Forward:  return EPlacementDirection::Backward;
    case EPlacementDirection::Backward: return EPlacementDirection::Forward;
    case EPlacementDirection::Left:     return EPlacementDirection::Right;
    case EPlacementDirection::Right:    return EPlacementDirection::Left;
    default: return Dir;
    }
}

void AGrammarGenerator::GeneratePlatformChain()
{
    LastPlatformLocation = GetActorLocation();
    LastPlatformScale = FVector(FMath::RandRange(FSpawnParams.PlatformScale.X, FSpawnParams.PlatformScale.Y), FMath::RandRange(FSpawnParams.PlatformScale.X, FSpawnParams.PlatformScale.Y), FSpawnParams.PlatformScale.Z);
    FRotator InitialRotation = FRotator(180.f, 0.f, 0.f); // Use identity rotation for proper alignment
    LastPlatformRotation = InitialRotation;
    FPlatformEdges LastPlatformEdges = CalculatePlatformEdges(LastPlatformLocation, LastPlatformScale, InitialRotation);
    
    SpawnPlatform(LastPlatformLocation, LastPlatformScale, InitialRotation, TEXT("Platform: 1 : Start"));
    DrawDebugLabel(TEXT("Platform: 1 : Start"), LastPlatformLocation);

    // Debug spheres for corners
    DrawDebugSphere(GetWorld(), LastPlatformEdges.TopLeftCoord, 50.f, 12, FColor::Red, true, 30.f);
    DrawDebugSphere(GetWorld(), LastPlatformEdges.BottomLeftCoord, 50.f, 12, FColor::Orange, true, 30.f);
    DrawDebugSphere(GetWorld(), LastPlatformEdges.TopRightCoord, 50.f, 12, FColor::Yellow, true, 30.f);
    DrawDebugSphere(GetWorld(), LastPlatformEdges.BottomRightCoord, 50.f, 12, FColor::Blue, true, 30.f);

    ExpandRule("StartRule", FSpawnParams.NumPlatforms - 1);
}

void AGrammarGenerator::PopulateWorld()
{
    FBox ParkourBounds(ForceInit);

    for (const FVector& Loc : PlacedLocations)
    {
        ParkourBounds += Loc;
    }

    FVector Min = ParkourBounds.Min - FVector(FDecorateRules.BufferDistance, FDecorateRules.BufferDistance, 0.f);
    FVector Max = ParkourBounds.Max + FVector(FDecorateRules.BufferDistance, FDecorateRules.BufferDistance, 0.f);

    float BuildingSpacing = 2200.f;

    for (int32 Layer = 0; Layer < FDecorateRules.NumLayers; ++Layer)
    {
        float Offset = Layer * FDecorateRules.LayerSpacing;

        for (float X = Min.X; X <= Max.X; X += BuildingSpacing)
        {
            // Bottom edge
            SpawnBuilding(FVector(X, Min.Y - BuildingSpacing - Offset, 0.f));
            // Top edge
            SpawnBuilding(FVector(X, Max.Y + BuildingSpacing + Offset, 0.f));
        }

        for (float Y = Min.Y; Y <= Max.Y; Y += BuildingSpacing)
        {
            // Left edge
            SpawnBuilding(FVector(Min.X - BuildingSpacing - Offset, Y, 0.f));

            // Right edge
            SpawnBuilding(FVector(Max.X + BuildingSpacing + Offset, Y, 0.f));
        }
    }
}

void AGrammarGenerator::SpawnBuilding(const FVector& Location)
{
    if (FSpawnParams.PlatformMesh.Num() == 0) return;

    int32 RandomIndex = FMath::RandRange(0, FSpawnParams.PlatformMesh.Num() - 1);
    UStaticMesh* MeshToSpawn = FSpawnParams.PlatformMesh[RandomIndex];

    FVector SpawnLocation = Location;
    SpawnLocation.Z = FMath::RandRange((-FDecorateRules.SpawnHeight * 2), (FDecorateRules.SpawnHeight * 2));

    AStaticMeshActor* Building = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnLocation, FRotator(180.f, FMath::FRandRange(0.f, 360.f), 0.f));
    if (Building)
    {
        Building->SetMobility(EComponentMobility::Movable);
        
        Building->GetStaticMeshComponent()->SetStaticMesh(MeshToSpawn);

        
        float RandomHeightScale = FMath::FRandRange(FDecorateRules.SpawnScale.X, FDecorateRules.SpawnScale.Y); // taller or shorter
        Building->GetStaticMeshComponent()->SetWorldScale3D(FVector(FMath::FRandRange(1.0f, 6.0f), FMath::FRandRange(1.0f, 6.0f), RandomHeightScale));
        
        Building->SetMobility(EComponentMobility::Static);

        PlacedBuildings.Add(Building);
    }
}

FString AGrammarGenerator::PickNextRule(EPlatformPlacementCategory Category)
{
    TArray<FString> PossibleNextRules;
        switch (Category)
        {
        case EPlatformPlacementCategory::HorizontalForward:
        case EPlatformPlacementCategory::HorizontalLeft:
        case EPlatformPlacementCategory::HorizontalRight:
        case EPlatformPlacementCategory::HorizontalBack:
            PossibleNextRules = { "BesideRule", "SmallJumpRule", "AboveRule", "BelowRule", "FarRule", "VeryHighRule", "VeryLowRule" };
            break;
        case EPlatformPlacementCategory::LongJumpForward:
        case EPlatformPlacementCategory::LongJumpBack:
        case EPlatformPlacementCategory::LongJumpLeft:
        case EPlatformPlacementCategory::LongJumpRight:
            PossibleNextRules = { "SmallJumpRule", "AboveRule", "BelowRule", "BesideRule" };
            break;
        case EPlatformPlacementCategory::AboveForward:
        case EPlatformPlacementCategory::AboveBack:
        case EPlatformPlacementCategory::AboveLeft:
        case EPlatformPlacementCategory::AboveRight:
            PossibleNextRules = { "FarRule", "SmallJumpRule", "BesideRule" };
            break;
        case EPlatformPlacementCategory::BelowForward:
        case EPlatformPlacementCategory::BelowBack:
        case EPlatformPlacementCategory::BelowLeft:
        case EPlatformPlacementCategory::BelowRight:
            PossibleNextRules = { "FarRule", "BesideRule", "SmallJumpRule"};
            break;
        case EPlatformPlacementCategory::SmallJumpForward:
        case EPlatformPlacementCategory::SmallJumpBack:
        case EPlatformPlacementCategory::SmallJumpLeft:
        case EPlatformPlacementCategory::SmallJumpRight:
            PossibleNextRules = { "VeryHighRule", "VeryLowRule", "BesideRule", "FarRule", "AboveRule", "BelowRule" };
            break;
        case EPlatformPlacementCategory::VeryHighPoint:
            PossibleNextRules = {"BesideRule" };
            break;
        case EPlatformPlacementCategory::VeryLowPoint:
            PossibleNextRules = {"BesideRule" };
            break;
        default:
            PossibleNextRules = { "BesideRule" };
            break;
        }
        FString NextRule = PossibleNextRules[FMath::RandRange(0, PossibleNextRules.Num() - 1)];

    return NextRule;
}

void AGrammarGenerator::ExpandRule(const FString& Rule, int32 RemainingPlatforms)
{
    if (RemainingPlatforms <= 0)
    {
        return;
    }

    if (!GrammarRules.Contains(Rule))
    {
        return;
    }

    // Pick a random expansion from the rule set.
    const GrammarRule& SelectedRule = GrammarRules[Rule];

    // Filter valid expansions
    TArray<EPlatformPlacementCategory> ValidExpansions;
    for (EPlatformPlacementCategory Category : SelectedRule.Expansions)
    {
        EPlacementDirection CandidateDir = ConvertToPlacementDirection(Category);
        if (CandidateDir != GetOppositeDirection(LastPlacementDirection))
        {
            ValidExpansions.Add(Category);
        }
    }

    // Pick a random category
    EPlatformPlacementCategory NextCategory;
    if (ValidExpansions.Num() > 0)
    {
        NextCategory = ValidExpansions[FMath::RandRange(0, ValidExpansions.Num() - 1)];
    }
    else
    {
        // If no valid non-opposite moves, allow anything (safe fallback)
        NextCategory = SelectedRule.Expansions[FMath::RandRange(0, SelectedRule.Expansions.Num() - 1)];
    }

    EPlacementDirection Dir = ConvertToPlacementDirection(NextCategory);
    
    // Generate a random scale for the new platform.
    FVector NewScale(FMath::RandRange(FSpawnParams.PlatformScale.X, FSpawnParams.PlatformScale.Y), FMath::RandRange(FSpawnParams.PlatformScale.X, FSpawnParams.PlatformScale.Y), FSpawnParams.PlatformScale.Z);
    
    // Compute local offsets
    FVector LocalBaseOffset = CalculateOffsetForDirection(Dir, LastPlatformScale, NewScale);
    FVector LocalExtraOffset = FVector::ZeroVector;
    if (!(NextCategory == EPlatformPlacementCategory::HorizontalForward ||
          NextCategory == EPlatformPlacementCategory::HorizontalBack ||
          NextCategory == EPlatformPlacementCategory::HorizontalLeft ||
          NextCategory == EPlatformPlacementCategory::HorizontalRight))
    {
        LocalExtraOffset = CalculateOffsetForCategory(NextCategory);
    }

    // Rotate the local offsets by the parent's rotation
    FVector WorldOffset = LastPlatformRotation.RotateVector(LocalBaseOffset + LocalExtraOffset);
    FVector NewLocation = SnapToGrid(LastPlatformLocation + WorldOffset);
    
    // Change the way platform rotations work to fix mantle walls later :)
    FRotator RelativeRotation = FRotator(180,0,0);//ComputeRotationForDirection(Dir);
    FRotator NewRotation = FRotator(LastPlatformRotation.Pitch, LastPlatformRotation.Yaw + RelativeRotation.Yaw, LastPlatformRotation.Roll);

    if (IsLocationValid(NewLocation, NewScale))
    {
        FText EnumDisplayName = StaticEnum<EPlatformPlacementCategory>()->GetDisplayNameTextByValue((int64)NextCategory);
        SpawnPlatform(NewLocation, NewScale, NewRotation,FString::Printf(TEXT("Platform: %d : %s"), (FSpawnParams.NumPlatforms - RemainingPlatforms + 1), *EnumDisplayName.ToString()));
        
        DrawDebugLabel(FString::Printf(TEXT("Platform %d (%s)"), FSpawnParams.NumPlatforms - RemainingPlatforms + 1, *UEnum::GetValueAsString(NextCategory)), NewLocation);

        // Get edges of previous and current platform.
        FPlatformEdges OldEdges = CalculatePlatformEdges(LastPlatformLocation, LastPlatformScale, LastPlatformRotation);
        FPlatformEdges NewEdges = CalculatePlatformEdges(NewLocation, NewScale, NewRotation);
        
        SpawnObstaclesForCategory(NextCategory, OldEdges, NewEdges);
        
        FPlatformEdges NewPlatformEdges = CalculatePlatformEdges(NewLocation, NewScale, NewRotation);
        DrawDebugSphere(GetWorld(), NewPlatformEdges.TopLeftCoord, 50.f, 12, FColor::Red, true, 30.f);
        DrawDebugSphere(GetWorld(), NewPlatformEdges.BottomLeftCoord, 50.f, 12, FColor::Orange, true, 30.f);
        DrawDebugSphere(GetWorld(), NewPlatformEdges.TopRightCoord, 50.f, 12, FColor::Yellow, true, 30.f);
        DrawDebugSphere(GetWorld(), NewPlatformEdges.BottomRightCoord, 50.f, 12, FColor::Blue, true, 30.f);

        // Update state.
        LastPlatformLocation = NewLocation;
        LastPlatformScale = NewScale;
        LastPlatformRotation = NewRotation;
        PlacedLocations.Add(NewLocation);
        PlacedScales.Add(NewScale);
        
        // Determine the next rule to use.  
        FString NextRule = PickNextRule(NextCategory);

        LastPlacementDirection = Dir;
        
        // Recursively call next rule.
        ExpandRule(NextRule, RemainingPlatforms - 1);
    }
    else
    {
        // Determine the next rule to use.  
        FString NextRule = PickNextRule(NextCategory);
        
        // Recursive call.
        ExpandRule(NextRule, RemainingPlatforms);
        
        UE_LOG(LogTemp, Warning, TEXT("No valid position: %s - Position: %d"), *UEnum::GetValueAsString(EPlatformPlacementCategory::HorizontalRight), FSpawnParams.NumPlatforms - RemainingPlatforms + 1);
    }

}

void AGrammarGenerator::CalculateClosestEdges(const FPlatformEdges& OldEdges,const FPlatformEdges& NewEdges,EPlatformPlacementCategory Category,FVector& OldStart,FVector& OldEnd,FVector& NewStart,FVector& NewEnd)
{
    switch (Category)
    {
    case EPlatformPlacementCategory::LongJumpForward:
    case EPlatformPlacementCategory::BelowForward:
    case EPlatformPlacementCategory::VeryLowPoint:
        OldStart = OldEdges.BottomRightCoord;
        OldEnd = OldEdges.TopRightCoord;
        NewStart = NewEdges.BottomLeftCoord;
        NewEnd = NewEdges.TopLeftCoord;
        break;

    case EPlatformPlacementCategory::LongJumpLeft:
    case EPlatformPlacementCategory::BelowLeft:
        OldStart = OldEdges.TopRightCoord;
        OldEnd = OldEdges.TopLeftCoord;
        NewStart = NewEdges.BottomRightCoord;
        NewEnd = NewEdges.BottomLeftCoord;
        break;

    case EPlatformPlacementCategory::LongJumpRight:
    case EPlatformPlacementCategory::BelowRight:
        OldStart = OldEdges.BottomRightCoord;
        OldEnd = OldEdges.BottomLeftCoord;
        NewStart = NewEdges.TopRightCoord;
        NewEnd = NewEdges.TopLeftCoord;
        break;

    default:
        OldStart = FVector::ZeroVector;
        OldEnd = FVector::ZeroVector;
        NewStart = FVector::ZeroVector;
        NewEnd = FVector::ZeroVector;
        break;
    }
}

FPlatformCalculations AGrammarGenerator::CalculatePlatformProperties(const FPlatformEdges& PlatformEdges)
{
    FPlatformCalculations Properties;
    
    // Get bounds from PlatformEdges
    Properties.MinBounds = PlatformEdges.BottomLeftCoord;
    Properties.MaxBounds = PlatformEdges.TopRightCoord;
    
    // Calculate center of the platform at its lowest Z (ground level)
    Properties.PlatformStart = FVector(
        (Properties.MinBounds.X + Properties.MaxBounds.X) * 0.5f,  // Center X
        (Properties.MinBounds.Y + Properties.MaxBounds.Y) * 0.5f,  // Center Y
        Properties.MinBounds.Z                              // Bottom-most Z
    );
    
    // Determine platform size and dimensions
    Properties.PlatformSize = (Properties.MaxBounds - Properties.MinBounds).GetAbs();
    Properties.PlatformWidth = Properties.PlatformSize.Y;
    Properties.PlatformDepth = Properties.PlatformSize.X;
    
    // Determine the dominant axis for spawning obstacles
    Properties.bSpawnAlongX = Properties.PlatformDepth > Properties.PlatformWidth;
    
    return Properties;
}

FVector AGrammarGenerator::CalculateWallRunLocation(const FPlatformEdges& OldEdges, const FPlatformEdges& NewEdges, EPlatformPlacementCategory Category)
{
    FVector OldStart, OldEnd, NewStart, NewEnd;
    
    CalculateClosestEdges(OldEdges, NewEdges, Category, OldStart, OldEnd, NewStart, NewEnd);

    if (OldStart.IsZero() || NewStart.IsZero())
    {
        // fallback incase something goes wrong
        return FVector::ZeroVector; 
    }

    // Get midpoint of both platform edges
    FVector OldMid = (OldStart + OldEnd) * 0.5f;
    FVector NewMid = (NewStart + NewEnd) * 0.5f;

    // Compute the final placement as the midpoint between the two edges
    FVector WallRunLocation = (OldMid + NewMid) * 0.5f;

    // Slight height adjustment to make it raised
    WallRunLocation.Z += 100.f; 

    return WallRunLocation;
    
}

FRotator AGrammarGenerator::CalculateWallRunRotation(const FPlatformEdges& OldEdges, const FPlatformEdges& NewEdges, EPlatformPlacementCategory Category)
{
    FVector OldStart, OldEnd, NewStart, NewEnd;
    
    CalculateClosestEdges(OldEdges, NewEdges, Category, OldStart, OldEnd, NewStart, NewEnd);

    if (OldStart.IsZero() || NewStart.IsZero())
    {
        // fallback incase something goes wrong
        return FRotator::ZeroRotator; 
    }
    
    // calculate direction vector between platforms
    FVector Direction = (((NewStart + NewEnd) / 2) - ((OldStart + OldEnd) / 2)).GetSafeNormal();

    DrawDebugSphere(GetWorld(), Direction, 100.f, 12, FColor::Purple, true, 30.f);
    
    return Direction.Rotation();
    
}

void AGrammarGenerator::SpawnWallRunObstacle(const FVector& Vector, const FRotator& Rotator, const float& Distance)
{
    // Spawn the wall run mesh
    AStaticMeshActor* WallActor = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Vector,
        Rotator
    );

    if (WallActor)
    {
        UStaticMeshComponent* MeshComp = WallActor->GetStaticMeshComponent();
        MeshComp->SetMobility(EComponentMobility::Movable);
        MeshComp->SetStaticMesh(FSpawnParams.WallRunMesh);
        
        float Length = Distance * 0.8f;
        float Height = FSpawnParams.WallRunHeight;
        float Thickness = 50.0f;
        
        MeshComp->SetWorldScale3D(FVector(Length / 100.0f, Thickness / 100.0f, Height / 100.0f));

        MeshComp->SetMaterial(0, FSpawnParams.ObstacleMaterial);
        
        WallActor->Tags.Add(FName("WallRun"));
        PlacedObstacles.Add(WallActor);
    }
}

float AGrammarGenerator::CalculateWallRunDistance(const FPlatformEdges& OldEdges, const FPlatformEdges& NewEdges, EPlatformPlacementCategory Category)
{
    FVector OldStart, OldEnd, NewStart, NewEnd;
    
    CalculateClosestEdges(OldEdges, NewEdges, Category, OldStart, OldEnd, NewStart, NewEnd);

    if (OldStart.IsZero() || NewStart.IsZero())
    {
        // fallback incase something goes wrong
        return 0; 
    }
    
    return FVector::Dist(OldStart, NewStart);
}

void AGrammarGenerator::SpawnMantleObstacle(const FVector& Vector, const FRotator& Rotator)
{
    AStaticMeshActor* MantlePoint = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            Vector,
            Rotator
        );
        
    if (MantlePoint)
    {
        UStaticMeshComponent* MeshComp = MantlePoint->GetStaticMeshComponent();
        MeshComp->SetMobility(EComponentMobility::Movable);
        MeshComp->SetStaticMesh(FSpawnParams.MantleMesh);

        MeshComp->SetWorldScale3D(FVector(.25, 20, 2.5f));

        MeshComp->SetMaterial(0, FSpawnParams.ObstacleMaterial);
        
        MantlePoint->Tags.Add(FName("Mantle"));
        PlacedObstacles.Add(MantlePoint);
    }
}

void AGrammarGenerator::SpawnMantleWalls(const FPlatformEdges& PlatformEdges)
{
    FVector MinBounds = PlatformEdges.BottomLeftCoord;
    FVector MaxBounds = PlatformEdges.TopRightCoord;

    // Calculate center of platform at its lowest Z (ground level)
    FVector PlatformStart = FVector(
        (MinBounds.X + MaxBounds.X) * 0.5f,
        (MinBounds.Y + MaxBounds.Y) * 0.5f, 
        MinBounds.Z                        
    );

    // Determine platform size
    FVector PlatformSize = (MaxBounds - MinBounds).GetAbs();
    float PlatformWidth = PlatformSize.Y;
    float PlatformDepth = PlatformSize.X;
    
    FVector WallLocation = PlatformStart;

    bool bSpawnAlongX = PlatformDepth > PlatformWidth;
    FVector WallDirection = bSpawnAlongX ? FVector(1, 0, 0) : FVector(0, 1, 0);

    FRotator WallRotation = FRotationMatrix::MakeFromXZ(WallDirection, FVector::UpVector).Rotator();

    float Distance = bSpawnAlongX ? PlatformWidth : PlatformDepth;

    WallLocation += bSpawnAlongX ? FVector(FMath::RandRange(-PlatformDepth * 0.4, PlatformDepth * 0.4), 0, 0) : FVector(0, FMath::RandRange(-PlatformWidth * 0.4, PlatformWidth * 0.4), 0);
    
    // Spawn the wall run mesh
    AStaticMeshActor* MantleWallActor = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        WallLocation,
        WallRotation
    );

    if (MantleWallActor)
    {
        UStaticMeshComponent* MeshComp = MantleWallActor->GetStaticMeshComponent();
        MeshComp->SetMobility(EComponentMobility::Movable);
        MeshComp->SetStaticMesh(FSpawnParams.WallRunMesh);
        
        MeshComp->SetWorldScale3D(FVector(.25, Distance / 100.f, 2.5));

        MeshComp->SetMaterial(0, FSpawnParams.ObstacleMaterial);
        
        MantleWallActor->Tags.Add(FName("Mantle Wall"));
        PlacedObstacles.Add(MantleWallActor);
    }
    
}

void AGrammarGenerator::SpawnVaultObstacle(const FVector& Vector, const FRotator& Rotator, const float& Distance)
{
    AStaticMeshActor* Vault = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            Vector,
            Rotator
        );
        
    if (Vault)
    {
        UStaticMeshComponent* MeshComp = Vault->GetStaticMeshComponent();
        MeshComp->SetMobility(EComponentMobility::Movable);
        MeshComp->SetStaticMesh(FSpawnParams.MantleMesh);

        float Length = Distance * 0.9f;
        MeshComp->SetWorldScale3D(FVector(FMath::RandRange(0.15, 1.2), Length / 100, 1));

        MeshComp->SetMaterial(0, FSpawnParams.ObstacleMaterial);

        Vault->Tags.Add(FName("Vault"));
        PlacedObstacles.Add(Vault);

    }
}

void AGrammarGenerator::SpawnVaultObstacles(const FPlatformEdges& PlatformEdges)
{
    FPlatformCalculations Properties = CalculatePlatformProperties(PlatformEdges);
    
    FVector VaultStart = Properties.bSpawnAlongX ?
        FVector(
            (Properties.MinBounds.X - Properties.PlatformDepth),
            (Properties.MinBounds.Y + Properties.MaxBounds.Y) / 2,
            Properties.MinBounds.Z
            ) :
    FVector(
        (Properties.MinBounds.X + Properties.MaxBounds.X) / 2,
        (Properties.MaxBounds.Y - Properties.PlatformWidth),
        Properties.MinBounds.Z
        );
    
    FVector VaultDirection = Properties.bSpawnAlongX ? FVector(1, 0, 0) : FVector(0, 1, 0);
    
    VaultStart += Properties.bSpawnAlongX ? FVector(FMath::RandRange(100.f, (Properties.PlatformWidth / 3)), 0, 0)
        : FVector(0, FMath::RandRange(100.f, Properties.PlatformDepth / 3), 0);

    float Spacing = FMath::RandRange(550.f, 1050.f);
    
    float Distance = Properties.bSpawnAlongX ? Properties.PlatformWidth : Properties.PlatformDepth;
    
    int NumVaults = std::min(static_cast<int>(Distance / Spacing), 6);
    
    UE_LOG(LogTemp, Warning, TEXT("HELP: %i"), NumVaults);

    // figure out a way to spawn vaults depending on how big the platform space is. 
    for (int i = 0; i < NumVaults; i++)
    {
        
        FVector Offset = VaultDirection * (i * Spacing);
        FVector VaultLocation = VaultStart + Offset;

        // Ensure correct obstacle rotation
        FRotator VaultRotation = FRotationMatrix::MakeFromXZ(VaultDirection, FVector::UpVector).Rotator();

        SpawnVaultObstacle(VaultLocation, VaultRotation, Distance);
        }
}

void AGrammarGenerator::SpawnMantleStaircase(const FPlatformEdges& OldEdges, const FPlatformEdges& NewEdges, EPlatformPlacementCategory Category, int32 StepCount)
{
    if (StepCount <= 1)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("1 Mantle or less: %d"), StepCount));
        return;
    }
    
    FVector OldStart, OldEnd, NewStart, NewEnd;
    
    CalculateClosestEdges(OldEdges, NewEdges, Category, OldStart, OldEnd, NewStart, NewEnd);

    if (OldStart.IsZero() || NewStart.IsZero())
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("IsZero")));
        return;
    }

    // Get midpoint of both platform edges
    FVector OldMid = (OldStart + OldEnd) * 0.5f;
    FVector NewMid = (NewStart + NewEnd) * 0.5f;
    
    // Large horizontal movement and subtle vertical drop
    /*for (int32 i = 0; i < StepCount; i++)
    {
        
        AStaticMeshActor* MantleCube = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            Start,
            ForwardRotation
        );

        if (MantleCube)
        {
            UStaticMeshComponent* MeshComp = MantleCube->GetStaticMeshComponent();
            MeshComp->SetMobility(EComponentMobility::Movable);
            MeshComp->SetStaticMesh(FSpawnParams.WallRunMesh);
            MeshComp->SetWorldScale3D(FVector(5, 6, 2.5));
            MeshComp->SetMaterial(0, FSpawnParams.ObstacleMaterial);
            MantleCube->Tags.Add(FName("Mantle Wall"));
            PlacedObstacles.Add(MantleCube);
        }
    }*/

    AStaticMeshActor* MantleCube = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            NewMid,
            FRotator::ZeroRotator
        );

    /*AStaticMeshActor* MantleCube2 = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            OldMid,
            FRotator::ZeroRotator
        );*/

    if (MantleCube)
    {
        UStaticMeshComponent* MeshComp = MantleCube->GetStaticMeshComponent();
        MeshComp->SetMobility(EComponentMobility::Movable);
        MeshComp->SetStaticMesh(FSpawnParams.WallRunMesh);
        MeshComp->SetWorldScale3D(FVector(5, 6, 2.5));
        MeshComp->SetMaterial(0, FSpawnParams.ObstacleMaterial);
        MantleCube->Tags.Add(FName("Mantle Wall"));
        PlacedObstacles.Add(MantleCube);

        /*UStaticMeshComponent* MeshComp2 = MantleCube2->GetStaticMeshComponent();
        MeshComp2->SetMobility(EComponentMobility::Movable);
        MeshComp2->SetStaticMesh(FSpawnParams.WallRunMesh);
        MeshComp2->SetWorldScale3D(FVector(5, 6, 2.5));
        MeshComp2->SetMaterial(0, FSpawnParams.StartPlatformMaterial);
        MantleCube2->Tags.Add(FName("Mantle Wall"));
        PlacedObstacles.Add(MantleCube2);*/
    }
}

int32 AGrammarGenerator::HandleMantleSpawns(const FPlatformEdges& OldEdges, const FPlatformEdges& NewEdges)
{
    // Calculate how many mantle steps to spawn
    float HeightDifference = abs(NewEdges.Centre.Z - OldEdges.Centre.Z);

    // Convert difference to steps, clamped to a safe range
    int32 StepCount = FMath::Clamp(
        FMath::CeilToInt(HeightDifference / FSpawnParams.GridUnit),
        1,
        10
    );

    return StepCount;
}

void AGrammarGenerator::SpawnObstaclesForCategory(EPlatformPlacementCategory Category,const FPlatformEdges& OldEdges,const FPlatformEdges& NewEdges)
{
    switch (Category)
    {
        case EPlatformPlacementCategory::LongJumpForward:
        case EPlatformPlacementCategory::LongJumpBack:
        case EPlatformPlacementCategory::LongJumpLeft:
        case EPlatformPlacementCategory::LongJumpRight:
            {
                FVector ObstacleLocation = CalculateWallRunLocation(OldEdges, NewEdges, Category);
                FRotator ObstacleRotation = CalculateWallRunRotation(OldEdges, NewEdges, Category);

                float Distance = CalculateWallRunDistance(OldEdges, NewEdges, Category);
                
                SpawnWallRunObstacle(ObstacleLocation, ObstacleRotation, Distance);
                

                SpawnMantleWalls(NewEdges);
                break;
            }
        case EPlatformPlacementCategory::BelowForward:
        case EPlatformPlacementCategory::BelowLeft:
        case EPlatformPlacementCategory::BelowRight:
        case EPlatformPlacementCategory::VeryLowPoint:
            {
                int32 StepCount = HandleMantleSpawns(OldEdges, NewEdges);

                SpawnMantleStaircase(OldEdges, NewEdges,Category, StepCount);
                
                break;
            }
        default:
            {
                SpawnVaultObstacles(NewEdges);
                SpawnMantleWalls(NewEdges);
                break;
            }
    }
}

EPlacementDirection AGrammarGenerator::ConvertToPlacementDirection(EPlatformPlacementCategory Category)
{
    switch (Category)
    {
    case EPlatformPlacementCategory::HorizontalForward:
    case EPlatformPlacementCategory::SmallJumpForward:
    case EPlatformPlacementCategory::LongJumpForward:
    case EPlatformPlacementCategory::AboveForward:
    case EPlatformPlacementCategory::BelowForward: 
        return EPlacementDirection::Forward;
    case EPlatformPlacementCategory::HorizontalLeft:
    case EPlatformPlacementCategory::SmallJumpLeft:
    case EPlatformPlacementCategory::LongJumpLeft:
    case EPlatformPlacementCategory::AboveLeft:
    case EPlatformPlacementCategory::BelowLeft: 
        return EPlacementDirection::Left;
    case EPlatformPlacementCategory::HorizontalRight:
    case EPlatformPlacementCategory::SmallJumpRight:
    case EPlatformPlacementCategory::LongJumpRight:
    case EPlatformPlacementCategory::AboveRight:
    case EPlatformPlacementCategory::BelowRight:
        return EPlacementDirection::Right;
    default:
        return EPlacementDirection::Forward; // Default to Forward if invalid
    }
}

FVector AGrammarGenerator::CalculateOffsetForDirection(EPlacementDirection Direction, const FVector& CurrentScale, const FVector& NewScale) const
{
    // Calculate half-extents
    FVector CurrentHalfExtents = CurrentScale * FSpawnParams.GridUnit;
    FVector NewHalfExtents = NewScale * FSpawnParams.GridUnit;
    FVector Offset = FVector::ZeroVector;

    switch (Direction)
    {
    case EPlacementDirection::Forward:
        Offset.X = CurrentHalfExtents.X + NewHalfExtents.X;
        break;
    case EPlacementDirection::Backward:
        Offset.X = -(CurrentHalfExtents.X + NewHalfExtents.X);
        break;
    case EPlacementDirection::Left:
        Offset.Y = CurrentHalfExtents.Y + NewHalfExtents.Y;
        break;
    case EPlacementDirection::Right:
        Offset.Y = -(CurrentHalfExtents.Y + NewHalfExtents.Y);
        break;
    }
    return Offset;
}

FRotator AGrammarGenerator::CalculateRotationForDirection(EPlacementDirection Direction) const
{
    switch (Direction)
    {
    case EPlacementDirection::Forward:
        return FRotator(180.f, 0.f, 0.f);
    case EPlacementDirection::Backward:
        return FRotator(180.f, 180.f, 0.f);
    case EPlacementDirection::Left:
        return FRotator(180.f, -90.f, 0.f);
    case EPlacementDirection::Right:
        return FRotator(180.f, 90.f, 0.f);
    default:
        return FRotator::ZeroRotator;
    }
}

FVector AGrammarGenerator::CalculateOffsetForCategory(EPlatformPlacementCategory Category) const
{
    // Pre-calculate the Z offsets for Above and Below cases.
    const float AboveZ = FMath::RandRange(FSpawnParams.AboveHeightMin, FSpawnParams.AboveHeightMax);
    const float BelowZ = FMath::RandRange(FSpawnParams.BelowHeightMax, FSpawnParams.BelowHeightMin);

    switch (Category)
    {
        // SMALL JUMPS
        case EPlatformPlacementCategory::SmallJumpForward:
            return FVector(FMath::RandRange(FSpawnParams.SmallJumpMinimum, FSpawnParams.SmallJumpMaximum), 
                           0.f, 
                           FMath::RandRange(-FSpawnParams.SmallJumpHeight, FSpawnParams.SmallJumpHeight));

        case EPlatformPlacementCategory::SmallJumpLeft:
            return FVector(0.f, 
                           FMath::RandRange(FSpawnParams.SmallJumpMinimum, FSpawnParams.SmallJumpMaximum), 
                           FMath::RandRange(-FSpawnParams.SmallJumpHeight, FSpawnParams.SmallJumpHeight));

        case EPlatformPlacementCategory::SmallJumpRight:
            return FVector(0.f, 
                           -FMath::RandRange(FSpawnParams.SmallJumpMinimum, FSpawnParams.SmallJumpMaximum), 
                           FMath::RandRange(-FSpawnParams.SmallJumpHeight, FSpawnParams.SmallJumpHeight));

        // LONG JUMPS
        case EPlatformPlacementCategory::LongJumpForward:
            return FVector(FMath::RandRange(FSpawnParams.LongJumpMinimum, FSpawnParams.LongJumpMaximum), 
                           0.f, 
                           FMath::RandRange(-FSpawnParams.LongJumpHeight, FSpawnParams.LongJumpHeight));

        case EPlatformPlacementCategory::LongJumpLeft:
            return FVector(0.f, 
                           FMath::RandRange(FSpawnParams.LongJumpMinimum, FSpawnParams.LongJumpMaximum), 
                           FMath::RandRange(-FSpawnParams.LongJumpHeight, FSpawnParams.LongJumpHeight));

        case EPlatformPlacementCategory::LongJumpRight:
            return FVector(0.f, 
                           -FMath::RandRange(FSpawnParams.LongJumpMinimum, FSpawnParams.LongJumpMaximum), 
                           FMath::RandRange(-FSpawnParams.LongJumpHeight, FSpawnParams.LongJumpHeight));

        // ABOVE placements
        case EPlatformPlacementCategory::AboveForward:
        case EPlatformPlacementCategory::AboveBack:
        case EPlatformPlacementCategory::AboveLeft:
        case EPlatformPlacementCategory::AboveRight:
            return FVector(0.f, 0.f, AboveZ);

        // BELOW placements
        case EPlatformPlacementCategory::BelowForward:
        case EPlatformPlacementCategory::BelowBack:
        case EPlatformPlacementCategory::BelowLeft:
        case EPlatformPlacementCategory::BelowRight:
            return FVector(0.f, 0.f, BelowZ);

        // High up / down points
        case EPlatformPlacementCategory::VeryHighPoint:
            return FVector(0.f, 0.f, AboveZ * 2.5f);

        case EPlatformPlacementCategory::VeryLowPoint:
            return FVector(0.f, 0.f, BelowZ * 2.5f);

        default:
            return FVector::ZeroVector;
    }
}

void AGrammarGenerator::SpawnPlatform(const FVector& Location, const FVector& Scale, const FRotator& Rotation, const FString& PlatformName)
{
    AStaticMeshActor* PlatformActor = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(), 
        Location, 
        Rotation
    );

    if (!FSpawnParams.PlatformMesh.IsEmpty() && PlatformActor)
    {
        UStaticMeshComponent* MeshComp = PlatformActor->GetStaticMeshComponent();
        PlatformActor->SetMobility(EComponentMobility::Movable);
        MeshComp->SetStaticMesh(FSpawnParams.PlatformMesh[FMath::RandRange(0, FSpawnParams.PlatformMesh.Num() - 1)]);
        MeshComp->SetWorldScale3D(Scale);
        //PlatformActor->SetActorLabel(PlatformName);

        if(PlacedPlatforms.IsEmpty())
        {
            // if array is empty give this start platform material
            MeshComp->SetMaterial(0, FSpawnParams.StartPlatformMaterial);
        }
        else if (PlacedPlatforms.Num() + 1 == FSpawnParams.NumPlatforms)
        {
            MeshComp->SetMaterial(0, FSpawnParams.FinishPlatformMaterial);
            MeshComp->SetMaterial(1, FSpawnParams.FinishPlatformMaterial);
        }
    }
    
    // Optionally draw a debug sphere at the platform location.
    DrawDebugSphere(GetWorld(), Location, 100.f, 12, FColor::Cyan, true, 30.f);

    // add to array
    PlacedPlatforms.Add(PlatformActor);
}

FVector AGrammarGenerator::SnapToGrid(const FVector& Location) const
{
    return FVector(
        FMath::RoundToInt(Location.X / FSpawnParams.GridUnit) * FSpawnParams.GridUnit,
        FMath::RoundToInt(Location.Y / FSpawnParams.GridUnit) * FSpawnParams.GridUnit,
        Location.Z
    );
}

void AGrammarGenerator::DrawDebugLabel(const FString& Text, const FVector& Location) const
{
    if (GetWorld())
    {
        DrawDebugString(GetWorld(), Location + FVector(0, 0, 150.f), Text, nullptr, FColor::White, 30.f);
    }
}

FPlatformEdges AGrammarGenerator::CalculatePlatformEdges(const FVector& Location, const FVector& Scale, const FRotator& Rotation) const
{
    FVector HalfExtents = Scale * FSpawnParams.GridUnit;
    
    FVector LocalTopLeftFront  = FVector(HalfExtents.X, -HalfExtents.Y, 0.f);
    FVector LocalTopRightFront = FVector(HalfExtents.X, HalfExtents.Y, 0.f);
    FVector LocalTopLeftBack   = FVector(-HalfExtents.X, -HalfExtents.Y, 0.f);
    FVector LocalTopRightBack  = FVector(-HalfExtents.X, HalfExtents.Y, 0.f);

    FVector TopLeftFront  = Location + Rotation.RotateVector(LocalTopLeftFront);
    FVector TopRightFront = Location + Rotation.RotateVector(LocalTopRightFront);
    FVector TopLeftBack   = Location + Rotation.RotateVector(LocalTopLeftBack);
    FVector TopRightBack  = Location + Rotation.RotateVector(LocalTopRightBack);

    return FPlatformEdges(TopLeftFront, TopRightFront, TopLeftBack, TopRightBack, Location);
}

bool AGrammarGenerator::IsLocationValid(const FVector& Location, const FVector& Scale) const
{
    // calculate bounding box for the new platform
    FBox NewBox = CalculatePlatformBoundingBox(Location, Scale);

    // Check against each placed platform
    for (AActor* ExistingPlatform : PlacedPlatforms)
    {
        if (!IsValid(ExistingPlatform))
        {
            continue;
        }

        // Get the existing platform’s bounding box
        FVector ExistingLocation = ExistingPlatform->GetActorLocation();
        FVector ExistingScale    = ExistingPlatform->GetActorScale3D();
        FBox ExistingBox         = CalculatePlatformBoundingBox(ExistingLocation, ExistingScale);
        ExistingBox = ExistingBox.ExpandBy(-1.0f); // add some tolerance to allow for side by side spawns
        
        if (NewBox.Intersect(ExistingBox))
        {
            UE_LOG(LogTemp, Warning, TEXT("Intersection"));
            
            return false;
        }
    }
    return true;
}

FBox AGrammarGenerator::CalculatePlatformBoundingBox(const FVector& Location, const FVector& Scale) const
{
    FVector BaseHalfExtents(FSpawnParams.GridUnit, FSpawnParams.GridUnit, FSpawnParams.PlatformScale.Z / 2); 
    FVector AdjustedHalfExtents = BaseHalfExtents * Scale;  

    FVector Min = Location - AdjustedHalfExtents;
    FVector Max = Location + AdjustedHalfExtents;
    return FBox(Min, Max);
}
