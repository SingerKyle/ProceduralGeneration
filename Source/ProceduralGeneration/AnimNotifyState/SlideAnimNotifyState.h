// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SlideAnimNotifyState.generated.h"

class UPAnimInstance;

/**
 * 
 */
UCLASS()
class PROCEDURALGENERATION_API USlideAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

	//UPROPERTY() UPAnimInstance* AnimInstance;
	//UPROPERTY() AParkourCharacter* Player;
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
