// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainWidget.generated.h"

/**
 * 
 */
UCLASS()
class NETCLASSPROJECT_YJ_API UMainWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;
	
	UPROPERTY(EditDefaultsOnly,meta=(BindWidget))
	class UImage* Img_CrossHair;

	UPROPERTY(EditDefaultsOnly,meta=(BindWidget))
	class UProgressBar* PB_HealthBar;
	
	void SetActivePistolUI(bool value);

	UPROPERTY(meta=(BindWidget))
	class UUniformGridPanel* GridPanel_Bullect;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UUserWidget> BullectUIFactory;

	// 총알 ui 초기설정 (10개)
		// 총알 추가 , 제거 기능
	void InitBulletUI(int32 maxBullectCount);

	void AddBulletUI();

	void RemoveBulletUI();
	
	void RemoveAllBulletUI();

	int32 MaxCol =5;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category=HP)
	float HP =1.f;

	//damage ui 애니메이션
	UPROPERTY(EditDefaultsOnly,meta=(BindWidgetAnim),Transient,Category=MySettings)
	UWidgetAnimation* DamageUIAni;

	// 피격퍼리 ui 애니 실행시키는 함수
	void PlayDamageAnimation();

	UPROPERTY(BlueprintReadWrite,meta=(BindWidget))
	class UHorizontalBox* GameOverUI;
	UPROPERTY(BlueprintReadWrite,meta=(BindWidget))
	class UButton* btn_Retry;
	UPROPERTY(BlueprintReadWrite,meta=(BindWidget))
	class UButton* btn_Exit;
	
	UPROPERTY(meta=(BindWidget)) // 상대방 플레이어의 이름을 우측에 나열하고싶다
	class UTextBlock* text_users;

	////////////// 채팅창//////////////////
	UPROPERTY(meta=(BindWidget))
	class UScrollBox* Scroll_msgList;
	UPROPERTY(meta=(BindWidget))
	class UEditableText* EditTxt_Input;
	UPROPERTY(meta=(BindWidget))
	class UButton* btn_Send;

	// send 버튼을 누르면 서버로 EditTxt_Input 으로 전송  serverRPC ==>플레이어컨트롤러
	// serverRPC를 하면 multicast 으로 내용을 전달받음 ==> 플레이어컨트롤러
	// 전달받은 메세지를 Scroll_msgList 에 맨위에 추가
	// 전달받은 메세지를 chatWidget을 만들어서 Scroll_msgList에 추가하기
	
	/////////////////////////////////////////
	
	UFUNCTION()
	void OnRetry();
	UFUNCTION()
	void OnExit();
	UFUNCTION()
	void OnClickChatSend();

	void OnAddChatMessage(const FString& msg);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UChatrWidget> ChatrWidgetFactory;
	

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
