// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralGeneration/AnimNotifyState/IgnoreRootMotion.h"
#include "ProceduralGeneration/ParkourCharacter.h"

void UIgnoreRootMotion::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                               const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	Player = Cast<AParkourCharacter>(MeshComp->GetOwner());

	if (Player)
	{
		UPAnimInstance* AnimInstance = Cast<UPAnimInstance>(Player->GetMesh()->GetAnimInstance());

		if(AnimInstance)
		{
			// Allows for velocity to be kept after vaulting
			AnimInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		}
	}
}
