// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Procedural Generation/Floor.h"
#include "Components/ActorComponent.h"
#include "LevelGenerator.generated.h"

UENUM(BlueprintType)
enum class EPlatformConnectionType : uint8
{
	// Platforms right next to each other
	Direct,      
	// Platforms at jump distance
	Jump,        
	// Platforms at an angle for wall running
	WallRun,     
	// Platforms with climb obstacles
	Mantle       
};

struct FPlatformData
{
	FVector Position;
	
	float Width;
	
	float Length;
	
};

USTRUCT(BlueprintType)
struct FProceduralGenerationParams
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generator", meta = (AllowPrivateAccess = true)) FVector2D MapDimensions;

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
	UPROPERTY(EditAnywhere, Category = "Level Generator") FVector2f baseHeight = FVector2f(-500.f,500.f);   // Minimum building heigh
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

	FVector CalculatePlatformPosition(const FCornerCoordinates& Coords, float Height) const;

	FVector FindValidPlatformPosition(const FCornerCoordinates& Coords, float Width, float Length, float Height, const TArray<FPlatformData>& PlacedPlatforms);
	
private:
	
	//UPROPERTY(EditAnywhere, Category = "Level Generator")
	//TSubclassOf<AActor> FloorBPClass;

	TSharedPtr<Floor> Level;
	TArray<AActor*> SpawnedActors;

	TMap<int32, TArray<AActor*>> PartitionedFloorActors;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generator") FProceduralGenerationParams SpawnParams;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generator") FProceduralGenerationMeshes SpawnMeshes;
	
};
