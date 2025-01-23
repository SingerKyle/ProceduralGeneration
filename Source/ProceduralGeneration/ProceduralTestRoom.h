// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralTestRoom.generated.h"

USTRUCT(BlueprintType)
struct FGrammarSymbol
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SymbolName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsTerminal; // True for final elements like jumps, vaults, etc.
};

// Define a struct for Grammar Rules
USTRUCT(BlueprintType)
struct FGrammarRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGrammarSymbol LHS; // Left-hand side of the rule

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGrammarSymbol> RHS; // Right-hand side, possible expansions
};

UCLASS()
class PROCEDURALGENERATION_API AProceduralTestRoom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProceduralTestRoom();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, category = "Mesh", meta = (AllowPrivateAccess = true)) UStaticMeshComponent* RoomFloor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, category = "Mesh", meta = (AllowPrivateAccess = true)) TSubclassOf<AActor> TestComponent;

	UPROPERTY(EditAnywhere, Category = "Traversal Elements")
	TSubclassOf<AActor> JumpActorClass;

	UPROPERTY(EditAnywhere, Category = "Traversal Elements")
	TSubclassOf<AActor> VaultActorClass;

	UPROPERTY(EditAnywhere, Category = "Traversal Elements")
	TSubclassOf<AActor> WallRunActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Settings", meta = (AllowPrivateAccess = true)) int SpawnNum;
	// grid size x and y
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Settings", meta = (AllowPrivateAccess = true)) FVector2D GridSize;
	
	// Squarewidth
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Settings", meta = (AllowPrivateAccess = true)) float SubGridWidth;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Settings", meta = (AllowPrivateAccess = true)) float GridHeight;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Settings", meta = (AllowPrivateAccess = true)) float SubGridLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Settings", meta = (AllowPrivateAccess = true)) float RoomLength;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Settings", meta = (AllowPrivateAccess = true)) float RoomWidth;
	
	FVector TopLeft;
	FVector BottomRight;

	float radius = 25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true)) TArray<FGrammarRule> GrammarRules;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SpawnTestItem(UClass* SpawnItem);

	void CreateGrid();

	// Use const references so they are cast in directly but can't be changed
	FVector GetRandomGridSquarePoint(const FVector& UpperLeft, const FVector& LowerRight);

	void PlacePointsOnGrid();

	bool IsLocationOccupied(const FVector& Location, float Radius);

	void ExpandSymbol(FGrammarSymbol Symbol, TArray<FGrammarSymbol>& OutSymbols);

	void PlaceTraversalElement(const FGrammarSymbol& Symbol, const FVector& Location);
};
