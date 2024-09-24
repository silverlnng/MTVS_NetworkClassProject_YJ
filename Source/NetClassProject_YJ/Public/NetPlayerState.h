// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NetPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class NETCLASSPROJECT_YJ_API ANetPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void ServerSetPlayerName(const FString& newName);
	
};
