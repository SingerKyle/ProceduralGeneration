// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PAsyncActionBase.generated.h"

/**
 * 
 */
UCLASS()
class PROCEDURALGENERATION_API UPAsyncActionBase : public UBlueprintAsyncActionBase, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime) override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual bool IsTickableWhenPaused() const override;
	virtual TStatId GetStatId() const override;
protected:

};
