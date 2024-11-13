// Fill out your copyright notice in the Description page of Project Settings.


#include "PAnimInstance.h"

UPAnimInstance::UPAnimInstance()
{
	bIsCrouching = false;
	bIsSliding = false;
	bIsWallrunning = false;
}

void UPAnimInstance::SetCrouching(bool Val)
{
	bIsCrouching = Val;
}

void UPAnimInstance::SetSliding(bool Val)
{
	bIsSliding = Val;
}

void UPAnimInstance::SetIsWallrunning(bool Val)
{
	bIsWallrunning = Val;
}

void UPAnimInstance::SetWallrunDirection(EDirection Direction)
{
	WallrunDirection = Direction;
}

void UPAnimInstance::SetHandRightAlpha(float Alpha)
{
	WallrunHandIK_RightAlpha = FMath::FInterpTo(WallrunHandIK_RightAlpha, Alpha, GetWorld()->DeltaTimeSeconds, 10.f);
}

void UPAnimInstance::SetHandLeftAlpha(float Alpha)
{
	WallrunHandIK_LeftAlpha = FMath::FInterpTo(WallrunHandIK_LeftAlpha, Alpha, GetWorld()->DeltaTimeSeconds, 10.f);
}

void UPAnimInstance::SetHandRightLocation(FVector Location)
{
	WallrunHandIK_RightLocation = Location;
}

void UPAnimInstance::SetHandLeftLocation(FVector Location)
{
	WallrunHandIK_LeftLocation = Location;
}

void UPAnimInstance::ResetIK()
{
	WallrunHandIK_LeftLocation = FVector(0,0,0);
	WallrunHandIK_RightLocation = FVector(0,0,0);
	WallrunHandIK_LeftAlpha = 0;
	WallrunHandIK_RightAlpha = 0;
}
