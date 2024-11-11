// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

// Enum for cover 
UENUM(BlueprintType)
enum ECoverState
{
	LeftEdge UMETA(DisplayName = "Left Edge"),
	LeftMoving UMETA(DisplayName = "Left Moving"),
	LeftIdle UMETA(DisplayName = "Left Idle"),
	Idle UMETA(DisplayName = "Idle"),
	RightIdle UMETA(DisplayName = "Right Idle"),
	RightMoving UMETA(DisplayName = "Right Moving"),
	RightEdge UMETA(DisplayName = "Right Edge")
};

// Enum for Direction 
UENUM(BlueprintType)
enum EDirection
{
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

namespace PGameTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Mechanics_Slide);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Mechanics_Vault);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Mechanics_Wallrun);
}
