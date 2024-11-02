// Fill out your copyright notice in the Description page of Project Settings.


#include "PAnimInstance.h"

UPAnimInstance::UPAnimInstance()
{
	bIsCrouching = false;
	bIsSliding = false;
}

void UPAnimInstance::SetCrouching(bool Val)
{
	bIsCrouching = Val;
}

void UPAnimInstance::SetSliding(bool Val)
{
	bIsSliding = Val;
}
