// Fill out your copyright notice in the Description page of Project Settings.


#include "Zipline.h"
#include "Components/SplineComponent.h"

#include "NavigationSystemTypes.h"
#include "Components/SplineMeshComponent.h"

void AZipline::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	EndMesh->SetRelativeLocation(EndLocation);
}

// Sets default values
AZipline::AZipline()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialise Meshes
	StartMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Zipline Start Mesh"));
	StartMesh->SetupAttachment(RootComponent);
	EndMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Zipline End Mesh"));
	EndMesh->SetupAttachment(RootComponent);
	
	// Initialise Scenes for location
	StartPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Zipline Spline Start Point"));
	StartPoint->SetupAttachment(StartMesh);
	EndPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Zipline Spline End Point"));
	EndPoint->SetupAttachment(EndMesh);
	
	// Initialise Spline
	ZipLine = CreateDefaultSubobject<USplineComponent>(TEXT("Zipline Spline"), true);
	EndMesh->SetupAttachment(RootComponent);
	
	// Initialise Cable
	ZiplineCable = CreateDefaultSubobject<UCableComponent>(TEXT("Zipline Cable"));
	ZiplineCable->SetupAttachment(RootComponent);

	EndLocation = FVector(500.f,0,0);
	
}

// Called when the game starts or when spawned
void AZipline::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AZipline::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

