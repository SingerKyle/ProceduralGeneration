// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ParkourCharacter.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerStart.h"
#include "Procedural Generation/GrammarGenerator.h"
#include "ProcGen_GameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class PROCEDURALGENERATION_API AProcGen_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	void RestartPlayerAtPlatform(AController* PlayerController);
	
public: 
	AProcGen_GameModeBase();

	UPROPERTY(EditDefaultsOnly, Category = "Game Rules")
	float FallThresholdZ = -4000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Rules") bool bHumanLevel;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	AGrammarGenerator* LevelGenerator;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector HumanStartPoint = FVector(8185.000000,7184.000062,-1397.999707);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector HumanEndPoint = FVector(8185.000000,7184.000062,-1397.999707);
};
