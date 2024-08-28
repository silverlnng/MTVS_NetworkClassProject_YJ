// Fill out your copyright notice in the Description page of Project Settings.


#include "NetTpsPlayerAnim.h"

#include "NetClassProject_YJCharacter.h"

void UNetTpsPlayerAnim::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Me =Cast<ANetClassProject_YJCharacter>(TryGetPawnOwner());
}

void UNetTpsPlayerAnim::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if(!Me)
	{
		return;
	}
	FVector veloc = Me->GetVelocity();
	FVector forward = Me->GetActorForwardVector();
	FVector right = Me->GetActorRightVector();
	
	Horizontal = FVector::DotProduct(right,veloc);
	Vertical = FVector::DotProduct(forward,veloc);

	bHasPistol = Me->bHasPistol;
}
