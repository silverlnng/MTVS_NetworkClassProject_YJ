// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetClassProject_YJCharacter.h"

#include "AssetTypeCategories.h"
#include "EngineUtils.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HealrhBarWidget.h"
#include "InputActionValue.h"
#include "MainWidget.h"
#include "NetClassProject_YJ.h"
#include "NetPlayerController.h"
#include "NetPlayerState.h"
#include "NetTpsPlayerAnim.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ANetClassProject_YJCharacter

ANetClassProject_YJCharacter::ANetClassProject_YJCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 150.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->SetRelativeLocation(FVector(0.0f, 40.f, 60.f));

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	HandComp =CreateDefaultSubobject<USceneComponent>(TEXT("HandComp"));
	HandComp->SetupAttachment(GetMesh(),TEXT("PistolPosition"));
	HandComp->SetRelativeLocationAndRotation(FVector(-15,0,5),FRotator(0,90,0));
	// (X=-15.000000,Y=0.000000,Z=5.000000)
	// (Pitch=0.000000,Yaw=89.999999,Roll=0.000000)

	hpWidgetComp=CreateDefaultSubobject<UWidgetComponent>(TEXT("hpWidgetComp"));
	hpWidgetComp->SetupAttachment(GetMesh());

	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
}

void ANetClassProject_YJCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	PRINTLOG(TEXT("[%s] BeginPlay"),Controller?TEXT("player"):TEXT("Not player"));
	
	//UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(),AActor::StaticClass(),TEXT("Pistol"),PistolList);
	
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if(Actor->ActorHasTag(TEXT("Pistol")))
		{
			PistolList.Add(Actor);
			//PistolList.Push(Actor);
		}
	}
	// 컨트롤러를 가지고있고 서버가 아닐때 위젯생성
	if(IsLocallyControlled() && !HasAuthority())
	{
		InitMainWidget();
	}

	
}

void ANetClassProject_YJCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	PrintNetLog();

	// hp ui 빌보드로 만들기
		// GetVisibleFlag 보일때만 작동
	if(hpWidgetComp && hpWidgetComp->GetVisibleFlag())
	{
		// 카메라 위치
		// 카메라 와 체력바의 방향벡터
		FVector camLoc = UGameplayStatics::GetPlayerCameraManager(GetWorld(),0)->GetCameraLocation();
		FVector Dir = camLoc - hpWidgetComp->GetComponentLocation();
		Dir.Z=0;
		hpWidgetComp->SetWorldRotation(Dir.GetSafeNormal().ToOrientationRotator());
	}
	
}


//////////////////////////////////////////////////////////////////////////
// Input

void ANetClassProject_YJCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANetClassProject_YJCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANetClassProject_YJCharacter::Look);

		EnhancedInputComponent->BindAction(GrabPistolAction, ETriggerEvent::Started, this, &ANetClassProject_YJCharacter::OnGrabPistol);
		
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ANetClassProject_YJCharacter::OnFirePistol);
		
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ANetClassProject_YJCharacter::OnReloadPistol);

		EnhancedInputComponent->BindAction(VoiceChatAction, ETriggerEvent::Started, this, &ANetClassProject_YJCharacter::StartVoiceChat);
		EnhancedInputComponent->BindAction(VoiceChatAction, ETriggerEvent::Completed, this, &ANetClassProject_YJCharacter::CancelVoiceChat);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ANetClassProject_YJCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ANetClassProject_YJCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ANetClassProject_YJCharacter::InitMainWidget()
{
	//플레이어가 제어중일때만 처리
	PRINTLOG(TEXT("[%s] Begin"),Controller?TEXT("player"):TEXT("Not player"));

	// 플레이어가 제어중 아니면 처리하지 않는다
	if(GetController()==nullptr || !GetController()->IsLocalController()) return;

	auto Netpc = GetController<ANetPlayerController>();

	if(Netpc->WBPmainUIwidget)
	{
		if(Netpc->MainWidget_UI ==nullptr) // 중복생성을 안하고 최초 한번만 하게됨 
		{
			Netpc->MainWidget_UI = CreateWidget<UMainWidget>(GetWorld(),Netpc->WBPmainUIwidget);
		}

		HP = MaxHP;
		MainWidget_UI = Netpc->MainWidget_UI;
		
		if(MainWidget_UI)
		{
			MainWidget_UI->AddToViewport();

			MainWidget_UI->HP =1.f;
			MainWidget_UI->SetActivePistolUI(false);
			MainWidget_UI->InitBulletUI(MaxBullectCount);

			// 메인 ui 가 있으면 안보이도록 설정
			hpWidgetComp->SetVisibility(false);
		}
	}
}

void ANetClassProject_YJCharacter::InitBullectWidget()
{
	//curBullectCount=MaxBullectCount;
	//isReloading=false;
	serverRPC_InitBullect();
	
}


void ANetClassProject_YJCharacter::OnGrabPistol(const FInputActionValue& Value)
{
	// 변수 bHasPistol 울 replicate해서 애니메이션을 동기화 시킬수있다
	if(bHasPistol) // 총을 이미 잡은 상태
	{
		MyReleasePistol();
	}
	else // 총을 이미 잡지 않은 상태
	{
		MyTakePistol();
	}
}

void ANetClassProject_YJCharacter::OnReloadPistol(const FInputActionValue& Value)
{
    if(!bHasPistol || isReloading ){return;}

	// 애니메이션을 재생
	/*auto anim =Cast<UNetTpsPlayerAnim>(GetMesh()->GetAnimInstance());
	// 재생하는 애니메이션 notify 에서 chrlghk wkrdjq
	anim->PlayReloadAnimMontage();
	isReloading=true;*/
	ServerRPC_Reload();
}

void ANetClassProject_YJCharacter::StartVoiceChat(const FInputActionValue& value)
{
	GetController<ANetPlayerController>()->StartTalking();
}

void ANetClassProject_YJCharacter::CancelVoiceChat(const FInputActionValue& value)
{
	GetController<ANetPlayerController>()->StopTalking();
}


void ANetClassProject_YJCharacter::MyTakePistol()
{
	// 잡지않은 상태에서는 잡아야함
	// 액터 붙이는것은  컴포넌트에 붙어야 원하는 위치에 붙일수있음
	// 잡은총은 기억을 해야되고 , 다른플레이어가 잡은 총은 못잡도록
	// 총의 owner를 설정하기 => 액터의 owner는  controller 으로 설정하기 . actor의 owner는 actor가 아니다
		
	/*for (AActor* pistol : PistolList)
	{
		// 총목록을 검사해서 => 나와 총과의 거리가 GrabPistolDistance 이하 => 소유자가 없는 총이면
		// 총을 기억하고 => 그총의 소유자를 나로 => 총을 나의 HandComp 에 부착하기
		float dist =FVector::Distance(GetActorLocation(),pistol->GetActorLocation());
		float tempdist = GetDistanceTo(pistol);

		if (tempdist > GrabPistolDistance) { continue; } // 조건해당안하니까 pass

		if (nullptr != pistol->GetOwner()) { continue; } // 조건해당안하니까 pass

		auto* pc = GetController<APlayerController>();
		pistol->SetOwner(this); //액터의 owner 액터로 가능. 
		AttachPistol(pistol);
			
		// 총검사는 클라이언트 쪽에서만 ! 총 부착은 서버에서 해야 모든 pc의 플레이어에게 동일하게 보일수있다

		bHasPistol=true;
		MainWidget_UI->SetActivePistolUI(true);	
		break; // 하나만 부착했음 다시 for문 반복할필요없음
	}*/
	ServeRPC_TakeGun();
}

void ANetClassProject_YJCharacter::MyReleasePistol()
{
	/*//재장전중이면 총을 버릴수없다
	if(!bHasPistol||isReloading){return;}
	
	// 이미 잡은상태에서는 놓아야함
	if (bHasPistol)
	{
		
		MainWidget_UI->SetActivePistolUI(false);
		bHasPistol = false;
	}
	
	
	if (GrabPistolActor) // 총을 오너를 지우고 ,잊고
	{
		DetachPistol(); // Detatch 하기
		GrabPistolActor->SetOwner(nullptr);
		GrabPistolActor = nullptr;
	}*/
	if(!bHasPistol||isReloading||!IsLocallyControlled()){return;}
	ServerRPC_releasePistol();
}


void ANetClassProject_YJCharacter::AttachPistol(AActor* pistolActor)
{
	GrabPistolActor =pistolActor;
	
	auto* mesh =GrabPistolActor->GetComponentByClass<UStaticMeshComponent>();
	mesh->SetSimulatePhysics(false);
	// AttachToComponent 를 컴포넌트 단위 인지 액터단위인지...???
	mesh->AttachToComponent(HandComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	//pistolActor->AttachToComponent(HandComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	// 멀티캐스트로 실행하지만 IsLocallyControlled 으로 묶어서 로컬창에만 보임 
	if (IsLocallyControlled() && MainWidget_UI)
	{
		MainWidget_UI->SetActivePistolUI(true);
	}	
}

void ANetClassProject_YJCharacter::DetachPistol(AActor* pistolActor)
{
	// 총을 가져와서 detach하기
	GrabPistolActor = pistolActor;
	auto* mesh =GrabPistolActor->GetComponentByClass<UStaticMeshComponent>();
	check(mesh);  //==> 디버깅을 위해 일부러 오류발생시키기
	if(mesh)
	{
		// 다시 물리를 켜주기
		mesh->SetSimulatePhysics(true);
		mesh->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	}
	if (IsLocallyControlled() && MainWidget_UI)
	{
		MainWidget_UI->SetActivePistolUI(false);
	}	
}



void ANetClassProject_YJCharacter::OnFirePistol(const FInputActionValue& value)
{
 
	if(!bHasPistol ||isReloading || !GrabPistolActor) {return;}
	ServerRPC_Fire();
}

void ANetClassProject_YJCharacter::OnRep_CurHp() // 클라이언트에서만 작동하는 함수
{

	// 여기서 죽는지 체크
	if(CurHP<=0)
	{
		isDead = true;
		if(bHasPistol)
		{
			OnGrabPistol(FInputActionValue());
		}
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		GetCharacterMovement()->DisableMovement();
	}
	
	float hpPercent = CurHP / MaxHP;

	// MainWidget_UI create를 로컬에서만 하도록 만들었음
	
	if(MainWidget_UI) // 피격을 당하는 플레이어 로컬에서만 가짐 
	{
		MainWidget_UI->HP=hpPercent;
		MainWidget_UI->PlayDamageAnimation();

		if(damageCameraShake)
		{
			auto pc =GetController<APlayerController>();
			if(pc)
			{
				pc->ClientStartCameraShake(damageCameraShake);
			}
		}
		
	}
	// MainWidget_UI 없으면 다른 방에서의 피격을 당하는 플레이어
	auto hpUI = Cast<UHealrhBarWidget>(hpWidgetComp->GetWidget());
	if(hpUI)
	{
		hpUI->HP = hpPercent;
	}
}

float ANetClassProject_YJCharacter::GetHP()
{
	return CurHP;
}

void ANetClassProject_YJCharacter::SetHP(float HP)
{
	CurHP=HP;
	OnRep_CurHp(); // 서버쪽에서는 안불려서 따로 넣어줌
}

void ANetClassProject_YJCharacter::DamageProcess() // 이거를 서버에서만 작동함
{
	// 
	//Server_SetHP();
	//체력 감소
	//CurHP--;
	HP--;
	if(HP<=0)
	{
		isDead=true;
	}
	// 맞은 대상이 상대방일 경우 데미지 처리
	//Multicast_SetHP();
}

void ANetClassProject_YJCharacter::PrintNetLog()
{
	const FString constStr =GetNetConnection()!=nullptr ? TEXT("Valid NetConnection") :TEXT("InValid NetConnection");

	const FString ownerName = GetOwner()!=nullptr ? GetOwner()->GetName() :TEXT("No Owner");
	//const FString RemoteRoleTest =
	const FString logStr = FString::Printf(TEXT("NetConnection :%s \n  OwnerName : %s \n Local Role :%s \n RemoteRole:%s"),*constStr,*ownerName,*LOCALROLE,*REMOTEROLE);
	
	DrawDebugString(GetWorld(),GetActorLocation()+FVector::UpVector*100,logStr,nullptr,FColor::Red,0,true,1);
	
}



void ANetClassProject_YJCharacter::ClientRPC_TakeGun_Implementation(bool value)
{
	if (IsLocallyControlled() && MainWidget_UI)
	{
		MainWidget_UI->SetActivePistolUI(value);
	}	
}

void ANetClassProject_YJCharacter::MulticastRPC_TakeGun_Implementation(AActor* pistolActor)
{
	AttachPistol(pistolActor);
}

void ANetClassProject_YJCharacter::ServeRPC_TakeGun_Implementation()
{
	for (AActor* pistol : PistolList)
	{
		// 총목록을 검사해서 => 나와 총과의 거리가 GrabPistolDistance 이하 => 소유자가 없는 총이면
		// 총을 기억하고 => 그총의 소유자를 나로 => 총을 나의 HandComp 에 부착하기
		float dist =FVector::Distance(GetActorLocation(),pistol->GetActorLocation());
		float tempdist = GetDistanceTo(pistol);

		if (tempdist > GrabPistolDistance) { continue; } // 조건해당안하니까 pass

		if (nullptr != pistol->GetOwner()) { continue; } // 조건해당안하니까 pass

		auto* pc = GetController<APlayerController>();
		pistol->SetOwner(this); //액터의 owner 설정 액터로 가능.

		
		bHasPistol=true;
		
		/*if(IsLocallyControlled() && MainWidget_UI)
		{
			MainWidget_UI->SetActivePistolUI(true);	
		}*/
		//ClientRPC_TakeGun(true);
		MulticastRPC_TakeGun(pistol);	
		// 총검사는 클라이언트 쪽에서만 ! 총 부착은 서버에서 해야 모든 pc의 플레이어에게 동일하게 보일수있다

		
		break; // 하나만 부착했음 다시 for문 반복할필요없음
	}
}






void ANetClassProject_YJCharacter::ServerRPC_releasePistol_Implementation()
{
	//재장전중이면 총을 버릴수없다
	if(!bHasPistol||isReloading){return;}
	
	// 이미 잡은상태에서는 놓아야함
	if (bHasPistol)
	{
		bHasPistol = false;
		//MainWidget_UI->SetActivePistolUI(false);
	}
	//ClientRPC_TakeGun(false);
	
	if (GrabPistolActor) // 총을 오너를 지우고 ,잊고
	{
		//DetachPistol();
		MulticastRPC_releasePistol(GrabPistolActor);// Detatch 하기
		GrabPistolActor->SetOwner(nullptr);
		GrabPistolActor = nullptr;
	}
}

void ANetClassProject_YJCharacter::MulticastRPC_releasePistol_Implementation(AActor* pistolActor)
{
	DetachPistol(pistolActor);
}

void ANetClassProject_YJCharacter::ServerRPC_Fire_Implementation()
{
	if(curBullectCount<0){return;}
	
	curBullectCount--;
	
	FVector start = FollowCamera->GetComponentLocation();
	FVector end = start+FollowCamera->GetForwardVector()*10000.f;
	FHitResult hitResult;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	
	bool hit =GetWorld()->LineTraceSingleByChannel(hitResult,start,end,ECC_Visibility,params);

	MulticastRPC_Fire(hit,hitResult);
	
	if(hit)
	{
		//FVector hitVec = hitResult.ImpactPoint;
		//UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),BullectFX,hitVec);
		
		auto otherPlayer =Cast<ANetClassProject_YJCharacter>(hitResult.GetActor());
		
		if(otherPlayer)
		{
			//Server_SetHP(otherPlayer);
			otherPlayer->DamageProcess();

			// 점수증가 시킴 
			auto* ps =GetPlayerState<ANetPlayerState>();
			if (ps)
			{
				ps->SetScore(ps->GetScore()+1);
			}
		}
	}
}

void ANetClassProject_YJCharacter::MulticastRPC_Fire_Implementation(bool bHit,const FHitResult& hitInfo)
{
	if(IsLocallyControlled() && MainWidget_UI) MainWidget_UI->RemoveBulletUI();
	
	UNetTpsPlayerAnim* anim = CastChecked<UNetTpsPlayerAnim>(GetMesh()->GetAnimInstance());
	check(anim);

	//캐스트하면서 동시에 체크도 !
	
	if(anim)
	{
		anim->PlayFireAnimMontage();
	}
	if(bHit)
	{
		FVector hitVec = hitInfo.ImpactPoint;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),BullectFX,hitVec);
		
	}
	
}

void ANetClassProject_YJCharacter::Server_SetHP_Implementation(ANetClassProject_YJCharacter* otherPlayer)
{
	otherPlayer->CurHP--;
	if(otherPlayer->CurHP<=0)
	{
		otherPlayer->isDead=true;
	}
	Multicast_SetHP();
}

void ANetClassProject_YJCharacter::Multicast_SetHP_Implementation()
{
	float hpPercent = CurHP / MaxHP;
	//MainWidget_UI 은 로컬만
	
	auto hpUI = Cast<UHealrhBarWidget>(hpWidgetComp->GetWidget());
	hpUI->HP = hpPercent;
	
}

void ANetClassProject_YJCharacter::DieProcess()
{
	// ui 클릭하려면 마우스 보이도럭 . 이건 로컬에서만 실행하도록 설계됨
	auto pc = GetController<APlayerController>();
	if(pc)
	{
		pc->SetShowMouseCursor(true);
	}
	GetFollowCamera()->PostProcessSettings.ColorSaturation =FVector4(0,0,0,1);
	// FVector4 = RGBA(0,0,0,1) !! 검정색이지만 내부적으로 회색처리 되어서 반투명으로 보이게 된다 !

	MainWidget_UI->GameOverUI->SetVisibility(ESlateVisibility::Visible);
}

void ANetClassProject_YJCharacter::GetLifetimeReplicatedProps(
	TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANetClassProject_YJCharacter,CurHP);
	DOREPLIFETIME(ANetClassProject_YJCharacter,isDead);
	DOREPLIFETIME(ANetClassProject_YJCharacter,bHasPistol);
	DOREPLIFETIME(ANetClassProject_YJCharacter,curBullectCount);
	DOREPLIFETIME(ANetClassProject_YJCharacter,isReloading);
}




void ANetClassProject_YJCharacter::ServerRPC_Reload_Implementation()
{
	isReloading=true;
	MulticastRPC_Reload();
}

void ANetClassProject_YJCharacter::MulticastRPC_Reload_Implementation()
{
	auto anim =Cast<UNetTpsPlayerAnim>(GetMesh()->GetAnimInstance());
	// 재생하는 애니메이션 notify 에서 
	anim->PlayReloadAnimMontage(); // InitBullect 을 실행하게 됨 
}
void ANetClassProject_YJCharacter::serverRPC_InitBullect_Implementation()
{
	curBullectCount=MaxBullectCount;
	isReloading=false;
	ClientRPC_Reload();
}

void ANetClassProject_YJCharacter::ClientRPC_Reload_Implementation()
{
	if(IsLocallyControlled()&&MainWidget_UI)
	{
		MainWidget_UI->RemoveAllBulletUI();
	}
    
	for(int i=0;i<MaxBullectCount;i++)  
	{
		if(IsLocallyControlled()&&MainWidget_UI)
		{
			MainWidget_UI->AddBulletUI();
		}
	}
}


void ANetClassProject_YJCharacter::PossessedBy(AController* NewController)
{
	PRINTLOG(TEXT("Begin"));
	Super::PossessedBy(NewController);

	// 이함수자체가 서버에서 실행 , 그리고 이후로 컨트롤러를 무조건 가지게됨

	if(IsLocallyControlled())
	{
		InitMainWidget();
	}
	
	PRINTLOG(TEXT("End"));
}

void ANetClassProject_YJCharacter::ServerRPC_Chat_Implementation(const FString& msg)
{
	MulticastRPC_Chat(msg);
}

void ANetClassProject_YJCharacter::MulticastRPC_Chat_Implementation(const FString& msg)
{
	auto* pc= GetWorld()->GetFirstPlayerController<ANetPlayerController>();
	// 메인 ui
	if (pc->MainWidget_UI)
	{
		pc->MainWidget_UI->OnAddChatMessage(msg);
	}
}