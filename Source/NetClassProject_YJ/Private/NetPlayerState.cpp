// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerState.h"

#include "NetGameInstance.h"

void ANetPlayerState::BeginPlay()
{
	Super::BeginPlay();
	auto* gi =GetWorld()->GetGameInstance<UNetGameInstance>();
	auto* pc =GetWorld()->GetFirstPlayerController();
	if (gi && pc && pc->IsLocalController())
	{
		ServerSetPlayerName(gi->MySessionName);
	}
}

void ANetPlayerState::ServerSetPlayerName_Implementation(const FString& newName)
{
	SetPlayerName(newName);
}
