// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatrWidget.generated.h"

/**
 * 
 */
UCLASS()
class NETCLASSPROJECT_YJ_API UChatrWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* txt_msg;
	
};
