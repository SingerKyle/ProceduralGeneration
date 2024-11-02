// Fill out your copyright notice in the Description page of Project Settings.


#include "PAsyncActionBase.h"

void UPAsyncActionBase::Tick(float DeltaTime)
{

}

UWorld* UPAsyncActionBase::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

bool UPAsyncActionBase::IsTickableWhenPaused() const
{
	return false;
}

TStatId UPAsyncActionBase::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCBMoveActorToAction, STATGROUP_Tickables);
}
