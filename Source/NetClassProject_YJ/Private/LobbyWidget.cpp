// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyWidget.h"

#include "NetGameInstance.h"
#include "SessionSlotWidget.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"

class UNetGameInstance;

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 게임인스턴스를 가져오기
	gi =GetWorld()->GetGameInstance<UNetGameInstance>();
	gi->OnSearchSignatureCompleteDelegate.AddDynamic(this,&ULobbyWidget::AddSessionSlotWidget);

	MENU_Btn_GoCreate->OnClicked.AddDynamic(this,&ULobbyWidget::MENU_OnClickedCreateRoom);
	MENU_Btn_GoFind->OnClicked.AddDynamic(this,&ULobbyWidget::MENU_OnClickedGoFind);
	
	
	CR_Btn_Create->OnClicked.AddDynamic(this,&ULobbyWidget::CR_OnClickedCreateRoomName);
	CR_Slider_PlayerCount->OnValueChanged.AddDynamic(this,&ULobbyWidget::CR_OnSliderPlayerCountValueChanged);
	CR_Slider_PlayerCount->SetValue(2);
	// 슬라이더 값이 변할떄 마다 실행되는 델리게이트

	CR_Btn_GoMenu->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickedGoMenu);
	FS_Btn_GoMenu->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickedGoMenu);
	FS_Btn_Find->OnClicked.AddDynamic(this,&ULobbyWidget::FS_OnClickedFindSession);
	
}

void ULobbyWidget::OnClickedGoMenu()
{
	LobbyWidgetSwitcher->SetActiveWidgetIndex(0);
}

void ULobbyWidget::MENU_OnClickedCreateRoom()
{
	LobbyWidgetSwitcher->SetActiveWidgetIndex(1);
}

void ULobbyWidget::MENU_OnClickedGoFind()
{
	LobbyWidgetSwitcher->SetActiveWidgetIndex(2);
}

void ULobbyWidget::CR_OnClickedCreateRoomName()
{
	//UNetGameInstance* gi =GetWorld()->GetGameInstance<UNetGameInstance>();
	FString roomName = CR_EText_RoomName->GetText().ToString();
	int32 playerCount = (int32)CR_Slider_PlayerCount->GetValue();

	// roomName 체크하기 . 기재되지 않거나 공백이라면 방생성 하지 않기      
	 // 문자열에서 공백없애기 trim
	//roomName.RemoveSpacesInline(); // 공백을 제거한 뒤에도

	roomName =roomName.TrimStartAndEnd();
	
	if(roomName.IsEmpty()) {return;}; // 없으면 종료
	
	gi->CreateMySession(roomName,playerCount);
	
}

void ULobbyWidget::CR_OnSliderPlayerCountValueChanged(float Value)
{
	// 슬라이더의 값이 변경되면 텍스트에 그 값을	반영하고싶다
	CR_Text_PlayerCount->SetText(FText::AsNumber(Value));
}

void ULobbyWidget::FS_OnClickedFindSession()
{
	FS_Scrollbox->ClearChildren();
	// 기존의 목록은 삭제하고
	if(gi)
	{
		gi->FindSession();
	}
}

void ULobbyWidget::AddSessionSlotWidget(const struct FRoomInfo& info)
{
	//슬롯을 생성해서 스크롤 박스에 "추가 "하고 싶다
	auto* slot =CreateWidget<USessionSlotWidget>(this,SessionSlotWidgetFactory);
	
	slot->UpdateInfo(info);

	FS_Scrollbox->AddChild(slot);
}
