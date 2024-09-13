// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

/**
 * 
 */
UCLASS()
class NETCLASSPROJECT_YJ_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION()
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidget))
	class UWidgetSwitcher* LobbyWidgetSwitcher;

	UFUNCTION()
	void OnClickedGoMenu();
	
	//메뉴 ////////////////////////////////
	UPROPERTY(meta=(BindWidget))
	class UButton* MENU_Btn_GoCreate;

	UPROPERTY(meta=(BindWidget))
	class UButton* MENU_Btn_GoFind;


	UFUNCTION()
	void MENU_OnClickedCreateRoom();

	UFUNCTION()
	void MENU_OnClickedGoFind();
	
	// Create Room////////////////////////////////////////
	UPROPERTY(meta=(BindWidget))
	class UEditableText* CR_EText_RoomName;
	UPROPERTY(meta=(BindWidget))
	class USlider* CR_Slider_PlayerCount;
	UPROPERTY(meta=(BindWidget))
	class UButton* CR_Btn_Create;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* CR_Text_PlayerCount;

	UPROPERTY(meta=(BindWidget))
	class UButton* CR_Btn_GoMenu;

	UFUNCTION()
	void CR_OnClickedCreateRoomName();
	UFUNCTION()
	void CR_OnSliderPlayerCountValueChanged(float Value);

	//Find Session /////////////////////////////////////////

	UPROPERTY(meta=(BindWidget))
	class UButton* FS_Btn_Find;

	UPROPERTY(meta=(BindWidget))
	class UButton* FS_Btn_GoMenu;

	UPROPERTY(meta=(BindWidget))
	class UScrollBox* FS_Scrollbox;
	
	UFUNCTION()
	void FS_OnClickedFindSession();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class USessionSlotWidget> SessionSlotWidgetFactory;

	UFUNCTION()
	void AddSessionSlotWidget(const struct FRoomInfo& info);
	
	UPROPERTY()
	class UNetGameInstance* gi;
};
