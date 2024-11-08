// Fill out your copyright notice in the Description page of Project Settings.

#include "MechanicsComponent.h"

#include "ProceduralGeneration/Mechanics/BaseMechanic.h"

// Sets default values for this component's properties
UMechanicsComponent::UMechanicsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

UMechanicsComponent* UMechanicsComponent::GetMechanicsComponent(AActor* GetActor)
{
	if (GetActor)
	{
		return Cast<UMechanicsComponent>(GetActor->GetComponentByClass(StaticClass()));
	}

	return nullptr;
}

FGameplayTagContainer UMechanicsComponent::GetActiveTags()
{
	return ActiveGameplayTags;
}

FGameplayTagContainer UMechanicsComponent::GetBlockedTags()
{
	return BlockedGameplayTags;
}

FGameplayTag& UMechanicsComponent::GetMasterBlockedTag()
{
	return MasterBlockTag;
}

UBaseMechanic* UMechanicsComponent::GetMechanic(TSubclassOf<UBaseMechanic> Mechanic)
{
	for (int i = 0; i < Mechanics.Num(); i++)
	{
		if (Mechanics[i] && Mechanics[i]->IsA(Mechanic))
		{
			return Mechanics[i];
		}
	}

	return nullptr;
}

void UMechanicsComponent::AddMechanic(AActor* TargetActor, TSubclassOf<UBaseMechanic> MechanicClass, bool bAutoAdded)
{
	if (!MechanicClass)
	{
		return;
	}

	UBaseMechanic* NewMechanic = NewObject<UBaseMechanic>(GetOwner(), MechanicClass);

	// Check to see if tag already exists
	if (DoesTagExist(NewMechanic->MechanicTag))
	{
		return;
	}

	if (NewMechanic)
	{
		NewMechanic->Initialize(this);
		NewMechanic->OnMechanicAdded(GetOwner());
		Mechanics.Add(NewMechanic);

		if (NewMechanic->StartByDefault && NewMechanic->CanStart(TargetActor))
		{
			NewMechanic->StartMechanic(TargetActor);
		}
	}
}

void UMechanicsComponent::RemoveMechanic(UBaseMechanic* MechanicToRemove)
{
	if (!MechanicToRemove)
	{
		return;
	}

	if (MechanicToRemove->GetIsRunning())
	{
		MechanicToRemove->StopMechanic(GetOwner());
	}

	Mechanics.Remove(MechanicToRemove);
}

bool UMechanicsComponent::StartMechanic(AActor* Actor, FGameplayTag MechanicTag)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Entering StartMechanic Function"));

	if (GetActiveTags().HasAnyExact(GetBlockedTags()))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Matches Blocked Tags"));
		return false;
	}

	for (int i = 0; i < Mechanics.Num(); i++)
	{
		if (Mechanics[i] && Mechanics[i]->MechanicTag.MatchesTagExact(MechanicTag))
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Found Mechanic from tag"));
			if (!Mechanics[i]->CanStart(Actor))
			{
				// log no start
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Mechanic Won't start"));
				continue;
			}

			Mechanics[i]->StartMechanic_Implementation(Actor);
			return true;
		}
	}

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("No matching tags"));

	return false;
}

bool UMechanicsComponent::StopMechanic(AActor* Actor, FGameplayTag MechanicTag)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Entering StopMechanic Function"));

	for (int i = 0; i < Mechanics.Num(); i++)
	{
		if (Mechanics[i] && Mechanics[i]->MechanicTag.MatchesTagExact(MechanicTag))
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Found Mechanic from tag"));
			if (Mechanics[i]->GetIsRunning())
			{
				// if running stop action
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Mechanic Stopped"));
				Mechanics[i]->StopMechanic_Implementation(Actor);
				return true;
			}
		}
	}

	return false;
}

bool UMechanicsComponent::DoesTagExist(FGameplayTag Tag)
{
	for (int i = 0; i < Mechanics.Num(); i++)
	{
		if (Mechanics[i]->MechanicTag.MatchesTagExact(Tag))
		{
			return true;
		}
	}

	return false;
}

// Called when the game starts
void UMechanicsComponent::BeginPlay()
{
	Super::BeginPlay();

	for (int i = 0; i < StartingMechanics.Num(); i++)
	{
		AddMechanic(GetOwner(), StartingMechanics[i], true);
	}
	
}

void UMechanicsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TArray<UBaseMechanic*> TempMechanics = Mechanics;
	for (int i = 0; i < TempMechanics.Num(); i++)
	{
		if (Mechanics[i] && Mechanics[i]->GetIsRunning())
		{
			Mechanics[i]->StopMechanic(GetOwner());
		}
	}

	Super::EndPlay(EndPlayReason);
}


// Called every frame
void UMechanicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (int i = 0; i < Mechanics.Num(); i++)
	{
		Mechanics[i]->TickMechanic_Implementation(DeltaTime);
	}
}

