// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralGeneration/AnimNotifyState/Slide_CalculateVelocity.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "ProceduralGeneration/PGameTags.h"
#include "ProceduralGeneration/MechanicComponents/MechanicsComponent.h"

void USlide_CalculateVelocity::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation);

	if (Player == Cast<AParkourCharacter>(MeshComp->GetOwner()))
	{
		if(UMechanicsComponent* MechanicsComponent = Player->FindComponentByClass<UMechanicsComponent>())
		{
			if (MechanicsComponent)
			{
				if(UBaseMechanic* SlideMechanic = MechanicsComponent->GetMechanic(USlideMechanic::StaticClass()))
				{
					if(USlideMechanic* Slide = Cast<USlideMechanic>(SlideMechanic))
					{
						Slide->SetSlideVelocity(Player->GetCharacterMovement()->Velocity);
						Slide->CheckShouldContinueSlide();
					}
				}
			}
		}
	}
}
