// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CableComponent.h"
#include "Zipline.generated.h"

class USplineComponent;

UCLASS()
class PROCEDURALGENERATION_API AZipline : public AActor
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Zipline Parts") UStaticMeshComponent* StartMesh;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) USceneComponent* StartPoint;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Zipline Parts") UStaticMeshComponent* EndMesh;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) USceneComponent* EndPoint;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Zipline Parts") USplineComponent* ZipLine;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Zipline Parts") UCableComponent* ZiplineCable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Zipline Parts") UStaticMesh* CubeMesh;
	
	float ZipSpeed = 800.f;
public:	
	// Sets default values for this actor's properties
	AZipline();

	FORCEINLINE float GetZipSpeed() { return ZipSpeed; }
	FORCEINLINE FVector GetEndLocation() { return EndLocation; }
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY() FVector EndLocation;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
