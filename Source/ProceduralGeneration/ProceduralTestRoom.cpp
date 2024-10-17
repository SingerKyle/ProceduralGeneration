// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralTestRoom.h"

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

	SpawnNum = 1;
	GridSize = FVector2d(1.f, 1.f);
}

// Called when the game starts or when spawned
void AProceduralTestRoom::BeginPlay()
{
	Super::BeginPlay();
	
	for (int i = 0; i < SpawnNum; i++)
	{
		SpawnTestItem(TestComponent);
	}
	
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
	for (int32 i = 0; i < GridSize.X; i++)
	{

	}
}

