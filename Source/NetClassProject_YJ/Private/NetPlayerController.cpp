// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerController.h"

#include "NetClassProject_YJGameMode.h"
#include "GameFramework/SpectatorPawn.h"

void ANetPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if(HasAuthority()) //게임모드 는 서버에만 있음
	{
		gm = GetWorld()->GetAuthGameMode<ANetClassProject_YJGameMode>();
		
	}
}

void ANetPlayerController::ServerRPC_ChangeToSpectator_Implementation()
{
	//관전자가 플레이어의 위치에서 생성될수있도록 하기 . => 플레이어의 정보
			// 이걸 호출한 플레이어 , 지금이 플레이어컨트롤의 getpawn 으로 가져오는것 !!
	APawn* player = GetPawn();
	
	if(player)
	{
		FActorSpawnParameters params;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	//	 관전자를 생성
		auto spectator = GetWorld()->SpawnActor<ASpectatorPawn>(gm->SpectatorClass,player->GetActorTransform(),params);
		
		Possess(spectator);
		
	// 이전 플레이어 제거
		player->Destroy();

		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle,this,&ANetPlayerController::ServerRPC_RespawnPlayer_Implementation,5.f,false);
		
	}
}

void ANetPlayerController::ServerRPC_RespawnPlayer_Implementation()
{
	auto player = GetPawn();
	UnPossess();
	player->Destroy();
	
	gm->RestartPlayer(this);
	// RestartPlayer 함수 안에서 possess 하고 있음
	// 체력값도 이걸로 알아서 초기화 됨
	// possess 은 서버에서만 호출되어야함
	
}
