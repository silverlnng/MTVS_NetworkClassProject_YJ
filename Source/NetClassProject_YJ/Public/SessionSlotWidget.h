// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class NETCLASSPROJECT_YJ_API USessionSlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	virtual void NativeConstruct() override;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* Text_RoomName;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* Text_HostName;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* Text_PlayerCount;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* Text_PingMS;

	int32 SessionSearchindex;

	UPROPERTY(meta=(BindWidget))
	class UButton* Btn_Join;

	UFUNCTION()
	void OnClickJoin();

	void UpdateInfo(const struct FRoomInfo& info); // 구조체 전방선언
	
};
