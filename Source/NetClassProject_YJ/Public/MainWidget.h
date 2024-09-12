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
	UFUNCTION()
	void OnRetry();
	UFUNCTION()
	void OnExit();
};
