// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "NetClassProject_YJCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ANetClassProject_YJCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* GrabPistolAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* VoiceChatAction;
	
	//  meta = (AllowPrivateAccess = "true") : private 멤버를 에디터에 보이도록 하는 설정 
	
public:
	ANetClassProject_YJCharacter();
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void OnGrabPistol(const FInputActionValue& Value);
	void MyTakePistol();	
	void MyReleasePistol();
	void OnFirePistol(const FInputActionValue& value);
	void OnReloadPistol(const FInputActionValue& value);
	void StartVoiceChat(const FInputActionValue& value);
	void CancelVoiceChat(const FInputActionValue& value);
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();
	virtual void Tick(float DeltaTime) override;
public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UPROPERTY(Replicated,EditDefaultsOnly,BlueprintReadWrite)
	bool bHasPistol;

	// 태어날때 부터 총의목록을 
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TArray<AActor*> PistolList;

	UPROPERTY(EditDefaultsOnly,Category=Pistol)
	class USceneComponent* HandComp; // 이걸 손에 부착하고 위치조절을 함

	UPROPERTY()
	class AActor* GrabPistolActor;

	UPROPERTY(EditDefaultsOnly,Category=Pistol)
	float GrabPistolDistance =300;

	void AttachPistol(AActor* pistolActor);
	void DetachPistol(AActor* pistolActor);

	// 마우스왼버튼으로 총을 쏘기
	// 총알이 부딪힌 곳에 총알자국을 표현하기
	UPROPERTY(EditDefaultsOnly,Category=Pistol)
	class UParticleSystem* BullectFX;

	/*UPROPERTY(EditDefaultsOnly,Category=Pistol)
	TSubclassOf<class UUserWidget> WBP_mainWidget;*/
	
	UPROPERTY(EditDefaultsOnly,Category=Pistol)
	class UMainWidget* MainWidget_UI;
	
	void InitMainWidget();

	void InitBullectWidget();

	// 재장전 중인지를 기억하기
	UPROPERTY(Replicated)
	bool isReloading=false;
	
	UPROPERTY(EditDefaultsOnly,Category=Pistol)
	int32 MaxBullectCount=10;
	UPROPERTY(Replicated)
	int32 curBullectCount=MaxBullectCount;

	// 체력
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category=HP)
	float MaxHP =3;
	
	UPROPERTY(ReplicatedUsing=OnRep_CurHp,VisibleAnywhere,BlueprintReadOnly,Category=HP)
	float CurHP = MaxHP;

	UFUNCTION()
	void OnRep_CurHp();

	__declspec(property(get = GetHP,put = SetHP)) float HP;
	float GetHP();
	void SetHP(float HP);

	// __declspec 으로 선언으로 자동으로 HP = HP-1; 이런식으로 값을 set 하면 자동으로 선언한 SetHP 함수를 불러옴 
	
	UPROPERTY(VisibleAnywhere)
	class UWidgetComponent* hpWidgetComp;

	void DamageProcess();
	
	// 네트워크 상태로그를 출력을 할 함수
	void PrintNetLog();
	UPROPERTY(Replicated,EditDefaultsOnly,BlueprintReadOnly,Category=HP)
	bool isDead=false;


	UPROPERTY(EditDefaultsOnly,Category=UI)
	TSubclassOf<class UCameraShakeBase> damageCameraShake;

	// 죽음처리 - 화면을 회색으로 , 다시하기 ui
	void DieProcess();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	
	UFUNCTION(Server,Unreliable)
	void Server_SetHP(ANetClassProject_YJCharacter* otherPlayer);
	UFUNCTION(NetMulticast,Unreliable)
	void Multicast_SetHP();
	
	///////////////////////////////////////////////
	UFUNCTION(Server,Reliable)
	void ServeRPC_TakeGun(); // 총잡았다는걸 서버에 알리기

	UFUNCTION(Client,Reliable)
	void ClientRPC_TakeGun(bool value);
	
	UFUNCTION(NetMulticast,Reliable)
	void MulticastRPC_TakeGun(AActor* pistolActor);

	UFUNCTION(Server,Unreliable)
	void ServerRPC_releasePistol();

	UFUNCTION(NetMulticast,Unreliable)
	void MulticastRPC_releasePistol(AActor* pistolActor);


	UFUNCTION(Server,Unreliable)
	void ServerRPC_Fire();
	UFUNCTION(NetMulticast,Unreliable)
	void MulticastRPC_Fire(bool bHit,const FHitResult& hitInfo);

	UFUNCTION(Server,Reliable)
	void ServerRPC_Reload();
	UFUNCTION(Client,Reliable)
	void ClientRPC_Reload();	
	UFUNCTION(NetMulticast,Unreliable)
	void MulticastRPC_Reload();
	UFUNCTION(Server,Unreliable)
	void serverRPC_InitBullect();

	virtual void PossessedBy(AController* NewController) override;
	// PossessedBy : 서버에서만 호출되는 함수 

	// 채팅을 요청
	// 서버가 멀티캐스트
	UFUNCTION(Server,Reliable)
	void ServerRPC_Chat(const FString& msg);

	UFUNCTION(NetMulticast,Reliable)
	void MulticastRPC_Chat(const FString& msg);
	
};



