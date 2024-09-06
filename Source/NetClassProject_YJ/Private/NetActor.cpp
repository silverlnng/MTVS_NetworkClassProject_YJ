// Fill out your copyright notice in the Description page of Project Settings.


#include "NetActor.h"

#include "EngineUtils.h"
#include "NetClassProject_YJ.h"
#include "NetClassProject_YJCharacter.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ANetActor::ANetActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMeshComp);
	StaticMeshComp->SetRelativeScale3D(FVector(0.5f));

	bReplicates = true;

	NetUpdateFrequency = 100.0f;
	// NetUpdateFrequency = 100.0f; 100이면 1초에 100번 검사하겠다는 의미
	// How often (per second) this actor will be considered for replication, used to determine NetUpdateTime
}

// Called when the game starts or when spawned
void ANetActor::BeginPlay()
{
	Super::BeginPlay();

	Mat = StaticMeshComp->CreateDynamicMaterialInstance(0);
	if(HasAuthority())
	{
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle,[&]()
		{
			MatColor = FLinearColor(FMath::RandRange(0.f, 1.f), FMath::RandRange(0.f, 1.f), FMath::RandRange(0.f, 1.f), 1.f);
			Mat->SetVectorParameterValue(TEXT("ColorSeats"),MatColor);
		},1.f,true);
	}
}

// Called every frame
void ANetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	PrintNetLog();

	FindOwner();

	if(HasAuthority())
	{
		AddActorLocalRotation(FRotator(0, 50*DeltaTime, 0));
		RotYaw =GetActorRotation().Yaw;
		
	}
	/*else
	{
		FRotator newrot =GetActorRotation();
		newrot.Yaw=RotYaw;
		SetActorRotation(newrot);
		//AddActorLocalRotation(FRotator(0, RotYaw*DeltaTime, 0));
	}*/
	
}

void ANetActor::PrintNetLog()
{
	const FString constStr =GetNetConnection()!=nullptr ? TEXT("Valid NetConnection") :TEXT("InValid NetConnection");

	const FString ownerName = GetOwner()!=nullptr ? GetOwner()->GetName() :TEXT("No Owner");
	//const FString RemoteRoleTest =  UEnum::GetValueAsString<ENetRole>(RemoteRole);
	const FString logStr = FString::Printf(TEXT("NetConnection :%s \n  OwnerName : %s \n Local Role :%s \n RemoteRole:%s"),*constStr,*ownerName,*LOCALROLE,*REMOTEROLE);
	
	DrawDebugString(GetWorld(),GetActorLocation()+FVector::UpVector*100,logStr,nullptr,FColor::Red,0,true,1);
}

void ANetActor::FindOwner()
{
	if(HasAuthority()) // 서버에서만 체크
	{
		
		AActor* newOwner =nullptr;
		float minDist =searchDistance;
		
		for(TActorIterator<ANetClassProject_YJCharacter> it(GetWorld()) ; it ; ++it)
		{
			AActor* otherActor = *it;
			float curdist = this->GetDistanceTo(otherActor);
			
			if(curdist<minDist)
			{
				minDist = curdist;
				newOwner = otherActor;
			}
		}
		if(GetOwner()!=newOwner)// 새로운 오너가  이미 기존의 오너와 동일하면 또 set을 해줄필요가 없어서  
		{
			SetOwner(newOwner);
		}
	}

	DrawDebugSphere(GetWorld(),GetActorLocation(),searchDistance,30,FColor::Yellow,false,0,0,1);
}

void ANetActor::OnRep_RotYaw()
{
	FRotator newrot =GetActorRotation();
	newrot.Yaw=RotYaw;
	SetActorRotation(newrot);
}

//색상동기화
void ANetActor::OnRep_ChangeMatColor()
{
	if(Mat)
	{
		Mat->SetVectorParameterValue(TEXT("ColorSeats"),MatColor);
	}
}

void ANetActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANetActor,RotYaw);
	DOREPLIFETIME(ANetActor,MatColor);
}

