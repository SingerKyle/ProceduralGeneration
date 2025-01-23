// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Procedural Generation/Floor.h"
#include "LevelGenerator.generated.h"


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

	void SpawnGrid();

	void DrawDebugLines();
	
	//void NewQuadCell();
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generator", meta = (AllowPrivateAccess = true)) FVector2D MapDimensions;

	UPROPERTY(EditAnywhere, Category = "Level Generator")
	float FloorTileSize = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Level Generator")
	float MinJumpDistance = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Level Generator")
	float MaxJumpDistance = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Level Generator")
	TSubclassOf<AActor> FloorBPClass;

	UPROPERTY(EditAnywhere, meta=(ClampMin=0,ClampMax=2, UIMin=0,UIMax=2), Category = "Level Generator")
	float SplitRate = 0.5f;

	UPROPERTY(EditAnywhere, meta=(ClampMin=0,ClampMax=50, UIMin=0,UIMax=50), Category = "Level Generator")
	FVector2D MinBounds;

	TSharedPtr<Floor> Level;
	TArray<AActor*> SpawnedActors;

	UPROPERTY(EditAnywhere, Category = "Level Generator") float baseHeight;   // Minimum building height
	UPROPERTY(EditAnywhere, Category = "Level Generator") float heightVariance; // Height can vary by ±250 units
};
