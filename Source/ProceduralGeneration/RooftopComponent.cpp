// Fill out your copyright notice in the Description page of Project Settings.


#include "RooftopComponent.h"

// Sets default values for this component's properties
URooftopComponent::URooftopComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
}

// Called when the game starts
void URooftopComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	//Rooms.SetNum(NumOfRoomMesh, true); // do this in code

}


// Called every frame
void URooftopComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

