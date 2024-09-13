// Fill out your copyright notice in the Description page of Project Settings.


#include "MainWidget.h"

#include "NetGameInstance.h"
#include "NetPlayerController.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"

class UNetGameInstance;

void UMainWidget::NativeConstruct()
{
	Super::NativeConstruct();
	btn_Retry->OnClicked.AddDynamic(this,&UMainWidget::OnRetry);
	btn_Exit->OnClicked.AddDynamic(this,&UMainWidget::OnExit);
}

void UMainWidget::SetActivePistolUI(bool value)
{
	if (value)
	{
		Img_CrossHair->SetVisibility(ESlateVisibility::Visible);

		GridPanel_Bullect->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		Img_CrossHair->SetVisibility(ESlateVisibility::Hidden);

		GridPanel_Bullect->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMainWidget::InitBulletUI(int32 maxBullectCount)
{
	RemoveAllBulletUI();
	
	for (int32 i = 0; i < maxBullectCount; i++)
	{
		auto* bullectUI=CreateWidget(GetWorld(),BullectUIFactory);
		
		int col = i%MaxCol;
		int row = i/MaxCol;
		GridPanel_Bullect->AddChildToUniformGrid(bullectUI,row,col);
		
	}
}

void UMainWidget::AddBulletUI()
{
	// 일단 먼저 만들기
	auto* bullectUI=CreateWidget(GetWorld(),BullectUIFactory);

	int32 now = GridPanel_Bullect->GetChildrenCount();
	int col = now%MaxCol;
	int row = now/MaxCol;
	GridPanel_Bullect->AddChildToUniformGrid(bullectUI,row,col);
	
}

void UMainWidget::RemoveBulletUI()
{
	if(GridPanel_Bullect->GetChildrenCount()<=0) return; 
	GridPanel_Bullect->RemoveChildAt(GridPanel_Bullect->GetChildrenCount()-1);
}

void UMainWidget::RemoveAllBulletUI()
{
	// 일단 먼저 만들기

	int32 now = GridPanel_Bullect->GetChildrenCount();
    for(auto bullectWidget : GridPanel_Bullect->GetAllChildren())
    {
        GridPanel_Bullect->RemoveChild(bullectWidget);
    }
	
}

void UMainWidget::PlayDamageAnimation()
{
	if(DamageUIAni)
	{
		PlayAnimation(DamageUIAni);
	}
}

void UMainWidget::OnRetry()
{
	// 메인위젯에서 컨트롤러 가져오기
	auto pc =GetWorld()->GetFirstPlayerController<ANetPlayerController>();

	if(pc)
	{
		pc->SetShowMouseCursor(false);

		// 여기서 관전자 모드 로 실행되게
		pc->ServerRPC_ChangeToSpectator();
		//pc->ServerRPC_RespawnPlayer();
	}
	
	//게임종료 안보이도록 
	GameOverUI->SetVisibility(ESlateVisibility::Hidden);
}

void UMainWidget::OnExit()
{
	auto* gi = GetWorld()->GetGameInstance<UNetGameInstance>();
	if(gi)
	{
		gi->ExitSession();
	}
}
