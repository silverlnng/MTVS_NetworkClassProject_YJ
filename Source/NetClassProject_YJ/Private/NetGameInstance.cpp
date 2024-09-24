// Fill out your copyright notice in the Description page of Project Settings.


#include "NetGameInstance.h"

#include "NetClassProject_YJ.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"

void UNetGameInstance::Init()
{
	Super::Init();

	
	if (auto* subSystem =IOnlineSubsystem::Get()    )
	{
		SessionInterface = subSystem->GetSessionInterface();

		SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this,&UNetGameInstance::OnMyCreateSessionComplete);

		SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this,&UNetGameInstance::OnFindSessionComplete);

		SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this,&UNetGameInstance::OnJoinSessionComplete);

		SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this,&UNetGameInstance::OnDestroySessionComplete);
	}

	/*FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle,[&]()
	{
		CreateMySession(MySessionName,10);
		FindSession();
	},3.f,false);	*/
}

void UNetGameInstance::CreateMySession(FString roomName, int32 playerCount)
{
	FOnlineSessionSettings settings;

	settings.bIsDedicated = false;
	settings.bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL"? true : false;
	settings.bShouldAdvertise = true; // 공개방 , 비공개방(초대를 통해서) 세팅

	settings.bAllowJoinInProgress = true; // 중간에 join가능
	settings.bUsesPresence = true; // 유저의 Presence 상태정보 (온라인. 자리비움..) 사용
	settings.bAllowJoinViaPresence = true; //

	settings.NumPublicConnections = playerCount; // 최대명수

	//커스텀 정보
		// ROOM_NAME 을 매개변수인 roomName 으로 설정
	settings.Set(FName("ROOM_NAME"),roomName,EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	settings.Set(FName("HOST_NAME"),MySessionName,EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	//일반적으로 presence 는 상태 . 온라인 , 오프라인 등등...
		// bAllowJoinViaPresence
	FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();
	
	SessionInterface->CreateSession(*netID,FName(*MySessionName),settings);
	
	PRINTLOG(TEXT("Create Session Start ROOMNAME : %s | HOSTNAME :%s"),*roomName,*MySessionName);
}

void UNetGameInstance::OnMyCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		PRINTLOG(TEXT("OnMyCreateSessionComplete_Success"));

		// /Script/Engine.World'/Game/NetTPSS/Map/BattleMap.BattleMap'
		// 서버는 룸을 만들고나서 여행을 떠남
		// listen? 서버가 먼저들어가서 listen 하고있다 ( 수신받을준비 ??
		GetWorld()->ServerTravel("/Game/NetTPSS/Map/BattleMap?listen",true);
	}
	else
	{
		PRINTLOG(TEXT("OnMyCreateSessionComplete_Failed"));
	}
}

void UNetGameInstance::FindSession()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch);
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE,true
		,EOnlineComparisonOp::Equals); // 검색의 범위 를 결정
	
	SessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL"? true : false;
	SessionSearch->MaxSearchResults=40;
	
	SessionInterface->FindSessions(0,SessionSearch.ToSharedRef());
	
}

void UNetGameInstance::OnFindSessionComplete(bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		TArray<FOnlineSessionSearchResult> results =SessionSearch->SearchResults;
		for(int32 i=0;i<results.Num();i++)
		{
			FOnlineSessionSearchResult result_ = results[i];
			if(!result_.IsValid()){continue;}

			FRoomInfo roomInfo;
			// 방이름 , 호스트 이름 , 최대플레이어수 , 입장가능한 플레이어수 , ping 정보
			result_.Session.SessionSettings.Get(FName("ROOM_NAME"),roomInfo.roonName);

			result_.Session.SessionSettings.Get(FName("HOST_NAME"),roomInfo.hostName);
			
			roomInfo.MaxplayerCount =result_.Session.SessionSettings.NumPublicConnections;
			roomInfo.CurplayerCount = roomInfo.MaxplayerCount - result_.Session.NumOpenPublicConnections;
			roomInfo.PingMS = result_.PingInMs;
			roomInfo.index=i;
			
			if(OnSearchSignatureCompleteDelegate.IsBound()) // 
			{
				OnSearchSignatureCompleteDelegate.Broadcast(roomInfo);
			}
			PRINTLOG(TEXT("%s"),*roomInfo.ToString());
		}
	}
	else
	{
		PRINTLOG(TEXT("OnFindSessionComplete_Failed"));
	}
}

void UNetGameInstance::JoinSession(int32 index)
{
	auto result = SessionSearch->SearchResults[index];
	if(result.IsValid())
	SessionInterface->JoinSession(0,FName(MySessionName),result);
}

void UNetGameInstance::OnJoinSessionComplete(FName SessionName,
	EOnJoinSessionCompleteResult::Type OnJoinSessionCompleteResultType)
{
	if(OnJoinSessionCompleteResultType==EOnJoinSessionCompleteResult::Success)
	{
		auto* pc = GetWorld()->GetFirstPlayerController();
		FString url;
		SessionInterface->GetResolvedConnectString(SessionName,url);
		if(!url.IsEmpty())
		{
			pc->ClientTravel(url,TRAVEL_Absolute);
		}
	}
	/*switch (OnJoinSessionCompleteResultType)
	{
	case EOnJoinSessionCompleteResult::Success:
		//성공하면 서버가있는 레벨로 여행
		
		// TRAVEL_Absolute 설정에따라 다를수도있음 
	case EOnJoinSessionCompleteResult::SessionIsFull:
		PRINTLOG(TEXT("SessionIsFull"));
	case EOnJoinSessionCompleteResult::SessionDoesNotExist:
		PRINTLOG(TEXT("SessionDoesNotExist"));
	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
		PRINTLOG(TEXT("CouldNotRetrieveAddress"));
	case EOnJoinSessionCompleteResult::AlreadyInSession:
		PRINTLOG(TEXT("AlreadyInSession"));
	case EOnJoinSessionCompleteResult::UnknownError:
		PRINTLOG(TEXT("UnknownError"));
		; // Intentional fall-through
	}*/
}

void UNetGameInstance::ExitSession()
{
	ServerRPCExitSession();
}


void UNetGameInstance::ServerRPCExitSession_Implementation()
{
	MulticastRPCExitSession();
}

void UNetGameInstance::MulticastRPCExitSession_Implementation()
{
	SessionInterface->DestroySession(FName(*MySessionName));
	// 방퇴장하고싶다 방을 나가고 싶다.

	//호스트 입장에서는 방파괴 ==> 모두가 나가짐
	// 클라이언트 입장 ==> 혼자나감
}
void UNetGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		//클라이언트가 로비로
		auto* pc = GetWorld()->GetFirstPlayerController();
		// /Script/Engine.World'/Game/NetTPSS/Map/LobbyLevel.LobbyLevel'
		pc->ClientTravel(TEXT("/Game/NetTPSS/Map/LobbyLevel"),TRAVEL_Absolute);
		
		// 서버는 방을파괴하고 로비로
	}
}
