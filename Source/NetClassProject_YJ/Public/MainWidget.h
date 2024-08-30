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
	UPROPERTY(EditDefaultsOnly,meta=(BindWidget))
	class UImage* Img_CrossHair;

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

	int32 MaxCol =5;
	
};
