// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Floor.h"
#include "Components/ActorComponent.h"
#include "HelperStructs.h"
#include "LevelGenerator.generated.h"

USTRUCT(BlueprintType)
struct FProceduralGenerationParams
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generator", meta = (AllowPrivateAccess = true)) FVector2D MapDimensions = FVector2D(5.f,5.f);

	UPROPERTY(EditAnywhere, Category = "Level Generator") float FloorTileSize = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Level Generator") float MinJumpDistance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Level Generator") float MaxJumpDistance = 650.0f;

	UPROPERTY(EditAnywhere, meta=(ClampMin=0,ClampMax=2, UIMin=0,UIMax=2), Category = "Level Generator") float SplitRate = 0.5f;

	// Minimum Grid Size
	UPROPERTY(EditAnywhere, meta=(ClampMin=0,ClampMax=50, UIMin=0,UIMax=50), Category = "Level Generator")FVector2D MinBounds = FVector2D(5,5);;

	// Should the generation use the max size value?
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Level Generator") bool bUseMaxSize = false;

	// Maximum Grid Size
	UPROPERTY(EditAnywhere, meta=(ClampMin=0,ClampMax=50, UIMin=0,UIMax=50), Category = "Level Generator") FVector2D MaxBounds = FVector2D(5,5);;

	// Minimum and Maximum Spawn Height
	UPROPERTY(EditAnywhere, Category = "Level Generator") FVector2f baseHeight = FVector2f(-500.f,500.f);

	// --------------------------------------------------------------- Spawn Parameters For Objects --------------------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Spawn Parameters") float WallRunMinHeight = 200.0f;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Spawn Parameters") float WallRunMaxHeight = 300.0f;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Spawn Parameters") float MantleMinHeight = 400.0f;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Spawn Parameters") float MantleMaxHeight = 800.0f;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Spawn Parameters") float MantleMaxDistance = 10000.0f;
};

USTRUCT(BlueprintType)
struct FProceduralGenerationMeshes
{
	GENERATED_BODY()

	// The mesh to use for this variation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roof Mesh")
	UStaticMesh* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roof Mesh")
	UStaticMesh* ClimbMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roof Mesh")
	UStaticMesh* WallRunMesh = nullptr;
};

USTRUCT(BlueprintType)
struct FPlatformData
{
	GENERATED_BODY()
	
	FVector Position;
	FVector Dimensions;
	// Store the 2D bounds of the platform for easier overlap checking
	FBox2D Bounds; 

	FPlatformData(): Position(FVector::ZeroVector), Dimensions(FVector::ZeroVector), Bounds(FVector2D::ZeroVector, FVector2D::ZeroVector)
	{}
	
	FPlatformData(const FVector& InPosition, const FVector& InDimensions)
		: Position(InPosition)
		, Dimensions(InDimensions)
	{
		// Calculate the 2D bounds of the platform
		FVector2D Center(Position.X, Position.Y);
		FVector2D Extents(Dimensions.X * 0.5f, Dimensions.Y * 0.5f);
		Bounds = FBox2D(Center - Extents, Center + Extents);
	}
    
	bool OverlapsWith(const FPlatformData& Other, float MinDistance) const
	{
		// Get the actual corners of both platforms
		FVector2D ThisMin = FVector2D(Position.X - Dimensions.X * 0.5f, Position.Y - Dimensions.Y * 0.5f);
		FVector2D ThisMax = FVector2D(Position.X + Dimensions.X * 0.5f, Position.Y + Dimensions.Y * 0.5f);
		FVector2D OtherMin = FVector2D(Other.Position.X - Other.Dimensions.X * 0.5f, Other.Position.Y - Other.Dimensions.Y * 0.5f);
		FVector2D OtherMax = FVector2D(Other.Position.X + Other.Dimensions.X * 0.5f, Other.Position.Y + Other.Dimensions.Y * 0.5f);

		// Add the minimum distance buffer
		ThisMin -= FVector2D(MinDistance);
		ThisMax += FVector2D(MinDistance);

		// Check for overlap on both axes
		bool XOverlap = (ThisMin.X <= OtherMax.X) && (ThisMax.X >= OtherMin.X);
		bool YOverlap = (ThisMin.Y <= OtherMax.Y) && (ThisMax.Y >= OtherMin.Y);

		// Check vertical overlap separately
		float ZDist = FMath::Abs(Position.Z - Other.Position.Z);
		float ZOverlap = (Dimensions.Z + Other.Dimensions.Z) * 0.5f + MinDistance;

		return XOverlap && YOverlap && ZDist < ZOverlap;
	}
    
	bool IsWithinGrid(const FVector2D& GridSize, float TileSize) const
	{
		// Check if the platform (including its full dimensions) stays within the grid
		FVector2D MinPoint(Position.X - Dimensions.X * 0.5f, Position.Y - Dimensions.Y * 0.5f);
		FVector2D MaxPoint(Position.X + Dimensions.X * 0.5f, Position.Y + Dimensions.Y * 0.5f);
        
		return MinPoint.X >= 0 && MinPoint.Y >= 0 && 
			   MaxPoint.X <= GridSize.X * TileSize && 
			   MaxPoint.Y <= GridSize.Y * TileSize;
	}
};

UENUM(BlueprintType)
enum class EParkourType : uint8
{
	None            UMETA(DisplayName = "None"),
	Mantle          UMETA(DisplayName = "Mantle"),
	WallRun        UMETA(DisplayName = "Wall Run"),
	Jump		   UMETA(DisplayName = "Jump")
};

USTRUCT(BlueprintType)
struct FPlatformEdge
{
GENERATED_BODY()
	
	FVector Start;
	FVector End;
	FVector Normal;
	
	FPlatformEdge()
		: Start(FVector::ZeroVector)
		, End(FVector::ZeroVector)
		, Normal(FVector::ZeroVector)
	{}
	
	FPlatformEdge(const FVector& InStart, const FVector& InEnd, const FVector& InNormal)
		: Start(InStart), End(InEnd), Normal(InNormal) {}
};

UCLASS()
class PROCEDURALGENERATION_API ALevelGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALevelGenerator();
	// before begin play but after contructor
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Level Generator") void InitialiseGrid(); // Seperate function to allow for button in editor

	void OnConstruction(const FTransform& Transform) override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;
	
	void DrawDebugLines();

	/*
	 *
	 *	   Generation Functions
	 *
	 */
	
	void SpawnGrid();

	FVector CalculatePlatformPosition(const FCornerCoordinates& Coords, float Height, int32 PlatformWidth, int32 PlatformLength) const;

	void AnalyseAndSpawnParkourConnections();
	EParkourType DetermineParkourType(float Distance, float HeightDiff);
	void SpawnParkourConnection(const FPlatformData& Start, const FPlatformData& End, EParkourType Type);

	// Mantle
	void SpawnMantleIndicator(const FVector& Start, const FVector& End);
	
	void SpawnMantlePointsAlongPath(const TArray<FVector>& PathPoints);
	void GenerateEdgeFollowingPath(const FPlatformEdge& StartEdge, const FPlatformEdge& EndEdge, TArray<FVector>& OutPoints);
	TArray<FPlatformEdge> GetPlatformEdges(const FPlatformData& Platform);

	// Wallrun
	void SpawnWallRunSurface(const FPlatformData& Start, const FPlatformData& End);

	/*
	 *
	 *		Helper Functions
	 *
	 */

	// Check if a path between platforms is clear
	bool IsPathClear(const FPlatformData& Start, const FPlatformData& End) const;

	// Get the closest points between two platforms
	void GetClosestPlatformPoints(const FPlatformData& Start, const FPlatformData& End, 
								FVector& OutStartPoint, FVector& OutEndPoint) const;

	// Calculate edge-to-edge distance between platforms
	float GetPlatformEdgeDistance(const FPlatformData& Start, const FPlatformData& End) const;
	
private:

	TSharedPtr<Floor> Level;
	UPROPERTY() TArray<AActor*> SpawnedActors;

	TMap<int32, TArray<AActor*>> PartitionedFloorActors;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generator") FProceduralGenerationParams SpawnParams;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generator") FProceduralGenerationMeshes SpawnMeshes;
	UPROPERTY() TArray<FPlatformData> PlacedPlatforms;
	
};
