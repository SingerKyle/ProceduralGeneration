// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ProceduralGeneration/ParkourCharacter.h"
#include "ProceduralGeneration/Mechanics/SlideMechanic.h"
#include "ProceduralGeneration/AnimationInstance/PAnimInstance.h"
#include "Slide_CalculateVelocity.generated.h"


/**
 * 
 */
UCLASS()
class PROCEDURALGENERATION_API USlide_CalculateVelocity : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	AParkourCharacter* Player;

	UPAnimInstance* Instance;
};
