// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetClassProject_YJGameMode.h"
#include "NetClassProject_YJCharacter.h"
#include "UObject/ConstructorHelpers.h"

ANetClassProject_YJGameMode::ANetClassProject_YJGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
