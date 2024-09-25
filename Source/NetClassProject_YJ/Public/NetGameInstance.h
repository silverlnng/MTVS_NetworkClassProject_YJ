// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "NetGameInstance.generated.h"

/**
 * 
 */



USTRUCT(BlueprintType)
struct FRoomInfo
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	FString roonName;
	UPROPERTY(BlueprintReadOnly)
	FString hostName;
	UPROPERTY(BlueprintReadOnly)
	int32 MaxplayerCount;
	UPROPERTY(BlueprintReadOnly)
	int32 CurplayerCount;
	UPROPERTY(BlueprintReadOnly)
	int32 PingMS;

	int32 index;

	FString ToString()
	{
		return  FString::Printf(TEXT("%d)[%s][%s] %d/%d - ping : %d"),index,*roonName,*hostName,CurplayerCount,MaxplayerCount,PingMS);
	}
	
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSearchSignature,const struct FRoomInfo&,Info);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFindSignature,bool,value);

// 파라미터의 타입, 파라미터 명
UCLASS()
class NETCLASSPROJECT_YJ_API UNetGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	virtual void Init() override;

	IOnlineSessionPtr SessionInterface;
	FString MySessionName = TEXT("FirstUser");
	void CreateMySession(FString roomName,int32 playerCount); // 방생성 요청
	void OnMyCreateSessionComplete(FName SessionName, bool bWasSuccessful); //방생성 응답

	//찾을 방의 목록
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	
	// 방찾기 요청 
	void FindSession();
	// 방찾기 응답
	void OnFindSessionComplete(bool bWasSuccessful);

	FSearchSignature OnSearchSignatureCompleteDelegate;
	FFindSignature OnFindSignatureCompleteDelegate;
	// 방입장 요청 , 방입장 응답
	void JoinSession(int32 index);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type OnJoinSessionCompleteResultType);

	// 방퇴장 , 방 퇴장 응답 ==> ui 에서 실행
	void ExitSession();

	UFUNCTION(Server,Reliable)
	void ServerRPCExitSession();

	UFUNCTION(NetMulticast,Reliable)
	void MulticastRPCExitSession();

	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	// 스팀으로 방생성시 한글 사용하면 한글이 깨지는 이슈 해결
	FString StringBase64Encode(const FString& str);
	
	FString StringBase64Decode(const FString& str);

	// 찾기를 위한 델리게이트
};

