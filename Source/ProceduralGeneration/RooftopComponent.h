// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RooftopComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROCEDURALGENERATION_API URooftopComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URooftopComponent();
	
	UPROPERTY(EditAnywhere, Category = "Static Room Mesh")
	TSubclassOf<class AActor> FloorMesh;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// size of each grid space.
	UPROPERTY(EditAnywhere, Category = "Room Settings", meta = (AllowPrivateAccess = true))
	int Width = 5;
	UPROPERTY(EditAnywhere, Category = "Room Settings", meta = (AllowPrivateAccess = true))
	int Length = 5;
	UPROPERTY(EditAnywhere, Category = "Room Properties")
	int MaxRooms = 20;
	UPROPERTY(EditAnywhere, Category = "Room Properties")
	int X = 0;
	UPROPERTY(EditAnywhere, Category = "Room Properties")
	int Y = 0;
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
