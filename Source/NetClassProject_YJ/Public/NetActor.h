// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetActor.generated.h"

UCLASS()
class NETCLASSPROJECT_YJ_API ANetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANetActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* StaticMeshComp;

	void PrintNetLog();

	UPROPERTY(EditAnywhere)
	float searchDistance = 200;

	// Owner 설정
	void FindOwner();

	//UPROPERTY(Replicated)

	// UPROPERTY(ReplicatedUsing= OnRep_RotYaw)
	// ==> 변수값이 변할때 마다 OnRep_RotYaw 함수실행 (클라이언트에서만 실행) !!
	
	UPROPERTY(ReplicatedUsing= OnRep_RotYaw)
	float RotYaw =0;

	UFUNCTION()
	void OnRep_RotYaw();

	UPROPERTY()
	class UMaterialInstanceDynamic* Mat;

	UPROPERTY(ReplicatedUsing=OnRep_ChangeMatColor)
	FLinearColor MatColor;

	UFUNCTION()
	void OnRep_ChangeMatColor();
	
	UFUNCTION(Server,Reliable)
	void ServerRPC_ChangeMatColor(const FLinearColor newColor);

	UFUNCTION(Client,Unreliable)
	void ClientRPC_ChangeMatColor(const FLinearColor newColor);
	
	UFUNCTION(NetMulticast,Reliable)
	void MulticastRPC_ChangeMatColor(const FLinearColor newColor);
};
