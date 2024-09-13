// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionSlotWidget.h"

#include "NetGameInstance.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void USessionSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Btn_Join->OnClicked.AddDynamic(this,&USessionSlotWidget::OnClickJoin);
}

void USessionSlotWidget::OnClickJoin()
{
	auto* gi =GetWorld()->GetGameInstance<UNetGameInstance>();
	// SessionSearchindex 으로 정보를 받아서 join 하기
	if(gi)
	{
		gi->JoinSession(SessionSearchindex);
	}
}

void USessionSlotWidget::UpdateInfo(const struct FRoomInfo& info)
{
	Text_RoomName->SetText(FText::FromString(info.roonName));
	Text_HostName->SetText(FText::FromString(info.hostName));
	FString count = FString::Printf((TEXT("%d/%d")),info.CurplayerCount,info.MaxplayerCount);
	Text_PlayerCount->SetText(FText::FromString(count)); 
	Text_PingMS->SetText(FText::AsNumber(info.PingMS));

	SessionSearchindex=info.index;
}
