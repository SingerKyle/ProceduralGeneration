// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralTestRoom.generated.h"

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Settings", meta = (AllowPrivateAccess = true)) int SpawnNum;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Settings", meta = (AllowPrivateAccess = true)) FVector2D GridSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Settings", meta = (AllowPrivateAccess = true)) float GridWidth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Settings", meta = (AllowPrivateAccess = true)) float GridHeight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Settings", meta = (AllowPrivateAccess = true)) float GridLength;

	FVector TopLeft;
	FVector BottomRight;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SpawnTestItem(UClass* SpawnItem);

	void CreateGrid();
};
