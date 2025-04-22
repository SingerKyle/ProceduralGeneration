// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ProceduralGeneration/PGameTags.h"
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
	// Wall Running
	FORCEINLINE UFUNCTION() bool GetIsWallrunning() { return bIsWallrunning; }
	void SetIsWallrunning(bool Val);
	FORCEINLINE UFUNCTION() EDirection GetWallrunDirection() { return WallrunDirection; }
	void SetWallrunDirection(EDirection Direction);
	// Wall Running - Hand IK
	void SetHandRightAlpha(float Alpha);
	void SetHandLeftAlpha(float Alpha);
	void SetHandRightLocation(FVector Location);
	void SetHandLeftLocation(FVector Location);
	void ResetIK();
protected:
	UPROPERTY(BlueprintReadWrite, Category = "Animation") bool bIsCrouching;
	UPROPERTY(BlueprintReadWrite, Category = "Animation") bool bIsSliding;
	UPROPERTY(BlueprintReadWrite, Category = "Animation") bool bIsOnZipline;
	// Wall Running
	UPROPERTY(BlueprintReadWrite, Category = "Animation") bool bIsWallrunning;
	UPROPERTY(BlueprintReadWrite, Category = "Animation") TEnumAsByte<EDirection> WallrunDirection;
	// Wall Running - Hand IK - Alpha
	UPROPERTY(BlueprintReadWrite, Category = "Animation") float WallrunHandIK_LeftAlpha;
	UPROPERTY(BlueprintReadWrite, Category = "Animation") float WallrunHandIK_RightAlpha;
	// Wall Running - Hand IK - Location
	UPROPERTY(BlueprintReadWrite, Category = "Animation") FVector WallrunHandIK_LeftLocation;
	UPROPERTY(BlueprintReadWrite, Category = "Animation") FVector WallrunHandIK_RightLocation;
};
