// Fill out your copyright notice in the Description page of Project Settings.


#include "SlideAnimNotifyState.h"
#include "ProceduralGeneration/ParkourCharacter.h"
#include "ProceduralGeneration/AnimationInstance/PAnimInstance.h"

void USlideAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);
	//Player = Cast<AParkourCharacter>(MeshComp->GetOwner());
	//AnimInstance = Cast<UPAnimInstance>(Player->GetMesh()->GetAnimInstance());
}

void USlideAnimNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	//if(!AnimInstance->GetSliding())
	//{
		
	//}
}

void USlideAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);
}
