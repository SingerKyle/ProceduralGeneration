// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseMechanic.h"
#include "ProceduralGeneration/MechanicComponents/MechanicsComponent.h"
#include "ProceduralGeneration/PGameTags.h"

void UBaseMechanic::Initialize(UMechanicsComponent* NewMechanic)
{
	MechanicComponent = NewMechanic;

	bIsRunning = false;

}

bool UBaseMechanic::GetIsRunning() const
{
	return bIsRunning;
}

bool UBaseMechanic::CanStart_Implementation(AActor* Actor)
{
	if (GetIsRunning())
	{
		return false;
	}

	UMechanicsComponent* OwnerComponent = GetOwningComponent();

	if (OwnerComponent->GetActiveTags().HasAnyExact(BlockedTags))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Canstart is false"));
		return false;
	}

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Canstart is true"));
	return true;
}

void UBaseMechanic::OnMechanicAdded_Implementation(AActor* Actor)
{

}

void UBaseMechanic::OnMechanicRemoved_Implementation(AActor* Actor)
{

}

void UBaseMechanic::TickMechanic_Implementation(float DeltaTime)
{

}

UMechanicsComponent* UBaseMechanic::GetOwningComponent() const
{
	return MechanicComponent;
}

void UBaseMechanic::StartMechanic_Implementation(AActor* Actor)
{
	UMechanicsComponent* OwnerComponent = GetOwningComponent();

	OwnerComponent->GetActiveTags().AppendTags(GivenTags);

	bIsRunning = true;
	OwningActor = Actor;

	

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("BaseMechanic StartMechanic Function"));
}

void UBaseMechanic::StopMechanic_Implementation(AActor* Actor)
{
	UMechanicsComponent* OwnerComponent = GetOwningComponent();
	OwnerComponent->GetActiveTags().RemoveTags(GivenTags);

	bIsRunning = false;
	OwningActor = Actor;


}

bool UBaseMechanic::CanStartMechanic_Implementation(AActor* InInstigator)
{
	if (GetIsRunning())
	{
		return false;
	}
	
	UMechanicsComponent* OwnerComponent = GetOwningComponent();

	if (OwnerComponent->GetActiveTags().HasAnyExact(BlockedTags))
	{
		return false;
	}

	return true;
}
