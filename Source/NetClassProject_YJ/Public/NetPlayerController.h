// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NetPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class NETCLASSPROJECT_YJ_API ANetPlayerController : public APlayerController
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	class ANetClassProject_YJGameMode* gm;
public:
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void ServerRPC_RespawnPlayer();

	// 사용할 위젯클래스
		// 컨트롤러에서 위젯만들면 한번만 만들수있음
	UPROPERTY(EditDefaultsOnly,Category=UI)
	TSubclassOf<class UMainWidget> WBPmainUIwidget;
	
	UPROPERTY(EditDefaultsOnly,Category=Pistol)
	class UMainWidget* MainWidget_UI;

	// 관전자 모드 5초동안 하게하고 그후 플레이어 스폰되게하기
	UFUNCTION(server, Reliable)
	void ServerRPC_ChangeToSpectator();
	
};
