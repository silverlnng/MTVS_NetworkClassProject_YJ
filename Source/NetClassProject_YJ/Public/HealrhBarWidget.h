// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealrhBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class NETCLASSPROJECT_YJ_API UHealrhBarWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category=HP)
	float HP =1.f;

	
};
