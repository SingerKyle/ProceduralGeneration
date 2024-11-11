// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROCEDURALGENERATION_API UPAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
	UPAnimInstance();

public:
	FORCEINLINE UFUNCTION() bool GetCrouching() { return bIsCrouching; }
	void SetCrouching(bool Val);
	FORCEINLINE UFUNCTION() bool GetSliding() { return bIsSliding; }
	void SetSliding(bool Val);
	FORCEINLINE UFUNCTION() bool GetIsOnZipline() { return bIsOnZipline; }
	void SetIsOnZipline(bool Val);
protected:
	UPROPERTY(BlueprintReadWrite, Category = "Animation") bool bIsCrouching;
	UPROPERTY(BlueprintReadWrite, Category = "Animation") bool bIsSliding;
	UPROPERTY(BlueprintReadWrite, Category = "Animation") bool bIsOnZipline;
};
