// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralTestRoom.h"
#include "DrawDebugHelpers.h"
#include "Procedural Generation/Floor.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"

// Sets default values
AProceduralTestRoom::AProceduralTestRoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (!RootComponent)
	{
		RoomFloor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Floor Mesh"));
		SetRootComponent(RoomFloor);
	}


	// setup proc gen details
	// Number of items to spawn in grid
	SpawnNum = 1;
	// number of squares in grid
	GridSize = FVector2d(5.f, 5.f); // 240 x 5 = 1200 (overall size of floor)
	// height of grid square
	GridHeight = 1.f;
	// length and width of each square in the grid
	SubGridLength = 240.f;
	SubGridWidth = 240.f;

	RoomLength = 1200;
	RoomWidth = 1200;
	
	TopLeft = RoomFloor->GetComponentLocation();
	BottomRight = FVector(1200.f, 1200.f, RoomFloor->GetComponentLocation().Z);
}

// Called when the game starts or when spawned
void AProceduralTestRoom::BeginPlay()
{
	Super::BeginPlay();

	//TSharedPtr<Floor> TheFloor(new Floor());

	//TheFloor->Partition();

	//UE_LOG(LogTemp, Warning, TEXT("Nodes in partitioned floor stack: %d"), TheFloor->GetPartitionedFloor().Num());
	
	/*for (int i = 0; i < SpawnNum; i++)
	{
		SpawnTestItem(TestComponent);
	}*/

	//CreateGrid();


	FGrammarSymbol PathSymbol = { TEXT("Path"), false };
	FGrammarSymbol JumpSymbol = { TEXT("Jump"), true };
	FGrammarSymbol VaultSymbol = { TEXT("Vault"), true };
	FGrammarSymbol WallRunSymbol = { TEXT("WallRun"), true };
	FGrammarSymbol GapSymbol = { TEXT("Gap"), true };

	FGrammarRule PathToTraversalElements = { PathSymbol, { JumpSymbol, VaultSymbol, WallRunSymbol, GapSymbol } };

	//GrammarRules.Add(PathToTraversalElements);
	//PlacePointsOnGrid();
	
	//UE_LOG(LogTemp, Warning, TEXT("Nodes in Existance: %d"), FloorNode::GetNodeCount());
	//TUniquePtr<FloorNode> UniqueNodePtr(new FloorNode());
	//UE_LOG(LogTemp, Warning, TEXT("Nodes in Existance: %d"), FloorNode::GetNodeCount());
}

// Called every frame
void AProceduralTestRoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProceduralTestRoom::SpawnTestItem(UClass* SpawnItem)
{

	FVector SpawnLocation(FMath::FRandRange(0.f, RoomFloor->GetStaticMesh()->GetBounds().BoxExtent.X), FMath::FRandRange(0.f, RoomFloor->GetStaticMesh()->GetBounds().BoxExtent.Y), 0.0f);
	FRotator SpawnRotation(0.0f, FMath::FRandRange(0.f, 360.f), 0.0f);

	GetWorld()->SpawnActor<AActor>(SpawnItem, SpawnLocation, SpawnRotation);
}

void AProceduralTestRoom::CreateGrid()
{
	for (int32 i = 0; i < GridSize.X + 1; i++)
	{
		FVector Start = TopLeft + FVector(i * SubGridWidth, 0.0f, GridHeight);
		FVector End = Start + FVector(0.f, RoomLength, GridHeight);

		DrawDebugLine(GetWorld(), Start, End, FColor::Blue, true);
	}

	for (int32 i = 0; i < GridSize.X + 1; i++)
	{
		FVector Start = TopLeft + FVector(0, i * SubGridWidth, GridHeight);
		FVector End = Start + FVector(RoomWidth, 0, GridHeight);

		DrawDebugLine(GetWorld(), Start, End, FColor::Blue, true);
	}
}

FVector AProceduralTestRoom::GetRandomGridSquarePoint(const FVector& UpperLeft, const FVector& LowerRight)
{
	FVector2d RandomPoint = FVector2D(FMath::FRandRange(UpperLeft.X, LowerRight.X), FMath::FRandRange(UpperLeft.Y, LowerRight.Y));

	return FVector(RandomPoint.X, RandomPoint.Y, GridHeight);
}

void AProceduralTestRoom::PlacePointsOnGrid()
{
	for (int i = 0; i < GridSize.X; i++)
	{
		for (int j = 0; j < GridSize.Y; j++)
		{
			FGrammarSymbol CurrentSymbol = { TEXT("Path"), false };
			TArray<FGrammarSymbol> ExpandedSymbols;
			ExpandSymbol(CurrentSymbol, ExpandedSymbols);

			for (const FGrammarSymbol& Symbol : ExpandedSymbols)
			{
				FVector UpperLeft(i * SubGridWidth + radius, j * SubGridWidth + radius, GridHeight);
				FVector LowerRight(i * SubGridWidth + SubGridWidth - radius, j * SubGridWidth + SubGridWidth - radius, GridHeight);
				FVector RandomPointInSquare = GetRandomGridSquarePoint(UpperLeft, LowerRight);

				PlaceTraversalElement(Symbol, RandomPointInSquare);
			}
		}
	}
}

bool AProceduralTestRoom::IsLocationOccupied(const FVector& Location, float Radius)
{
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(Radius);

	// Query the world for overlaps at the given location using the Visibility channel.
	bool bIsOverlapping = GetWorld()->OverlapAnyTestByChannel(
		Location, 
		FQuat::Identity, 
		ECC_Visibility,  // Make sure the channel you're using is correct
		CollisionShape
	);

	// Return if any overlaps occurred
	return bIsOverlapping;
}

void AProceduralTestRoom::ExpandSymbol(FGrammarSymbol Symbol, TArray<FGrammarSymbol>& OutSymbols)
{
	if (Symbol.bIsTerminal)
	{
		OutSymbols.Add(Symbol);
		return;
	}

	for (const FGrammarRule& Rule : GrammarRules)
	{
		if (Rule.LHS.SymbolName == Symbol.SymbolName)
		{
			for (const FGrammarSymbol& RHS : Rule.RHS)
			{
				OutSymbols.Add(RHS);
			}
		}
	}
}

void AProceduralTestRoom::PlaceTraversalElement(const FGrammarSymbol& Symbol, const FVector& Location)
{
	FVector NewLocation = Location;
	float SpawnRadius = 25.f; // Adjust as needed for your elements

	// Check and adjust location until it is not overlapping
	//while (IsLocationOccupied(NewLocation, SpawnRadius))
	//{
	//	NewLocation = GetRandomGridSquarePoint(TopLeft, BottomRight);
	//}

	FRotator Rotation = FRotator::ZeroRotator;

	if (Symbol.SymbolName == "Jump" && JumpActorClass)
	{
		GetWorld()->SpawnActor<AActor>(JumpActorClass, NewLocation, Rotation);
	}
	else if (Symbol.SymbolName == "Vault" && VaultActorClass)
	{
		GetWorld()->SpawnActor<AActor>(VaultActorClass, NewLocation, Rotation);
	}
	else if (Symbol.SymbolName == "WallRun" && WallRunActorClass)
	{
		// Ensure no floor beneath the WallRun element
		FVector WallRunLocation = NewLocation + FVector(0.f, 0.f, 100.f); // Adjust height as needed
		GetWorld()->SpawnActor<AActor>(WallRunActorClass, WallRunLocation, Rotation);
	}
}