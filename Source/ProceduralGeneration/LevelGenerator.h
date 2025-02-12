// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Procedural Generation/Floor.h"
#include "Components/ActorComponent.h"
#include "LevelGenerator.generated.h"

USTRUCT(BlueprintType)
struct FPlatformData
{
	GENERATED_BODY()
	
	FVector Position;
	FVector Dimensions;
	FBox2D Bounds; // Store the 2D bounds of the platform for easier overlap checking

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
		// First check if the expanded bounds overlap
		FBox2D ExpandedBounds = Bounds;
		ExpandedBounds.Min -= FVector2D(MinDistance, MinDistance);
		ExpandedBounds.Max += FVector2D(MinDistance, MinDistance);
        
		if (!ExpandedBounds.IsInside(Other.Bounds))
			return false;
            
		// Check vertical overlap
		float ZDist = FMath::Abs(Position.Z - Other.Position.Z);
		float ZOverlap = (Dimensions.Z + Other.Dimensions.Z) * 0.5f;
        
		return ZDist < ZOverlap;
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

USTRUCT(BlueprintType)
struct FProceduralGenerationParams
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generator", meta = (AllowPrivateAccess = true)) FVector2D MapDimensions = FVector2D(5.f,5.f);

	UPROPERTY(EditAnywhere, Category = "Level Generator") float FloorTileSize = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Level Generator") float MinJumpDistance = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Level Generator") float MaxJumpDistance = 600.0f;

	UPROPERTY(EditAnywhere, meta=(ClampMin=0,ClampMax=2, UIMin=0,UIMax=2), Category = "Level Generator") float SplitRate = 0.5f;

	// Minimum Grid Size
	UPROPERTY(EditAnywhere, meta=(ClampMin=0,ClampMax=50, UIMin=0,UIMax=50), Category = "Level Generator")FVector2D MinBounds = FVector2D(5,5);;

	// Should the generation use the max size value?
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Level Generator") bool bUseMaxSize = false;

	// Maximum Grid Size
	UPROPERTY(EditAnywhere, meta=(ClampMin=0,ClampMax=50, UIMin=0,UIMax=50), Category = "Level Generator") FVector2D MaxBounds = FVector2D(5,5);;

	// Minimum and Maximum Spawn Height
	UPROPERTY(EditAnywhere, Category = "Level Generator") FVector2f baseHeight = FVector2f(-500.f,500.f);  
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

	void PlaceMantleObjects();

private:

	TSharedPtr<Floor> Level;
	UPROPERTY() TArray<AActor*> SpawnedActors;

	TMap<int32, TArray<AActor*>> PartitionedFloorActors;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generator") FProceduralGenerationParams SpawnParams;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generator") FProceduralGenerationMeshes SpawnMeshes;
	UPROPERTY() TArray<FPlatformData> PlacedPlatforms;
	
};
