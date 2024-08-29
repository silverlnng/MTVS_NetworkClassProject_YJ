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
#include "InputActionValue.h"
#include "NetTpsPlayerAnim.h"
#include "Kismet/GameplayStatics.h"

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
}

void ANetClassProject_YJCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

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

void ANetClassProject_YJCharacter::MyTakePistol()
{
	// 잡지않은 상태에서는 잡아야함
	// 액터 붙이는것은  컴포넌트에 붙어야 원하는 위치에 붙일수있음
	// 잡은총은 기억을 해야되고 , 다른플레이어가 잡은 총은 못잡도록
	// 총의 owner를 설정하기 => 액터의 owner는  controller 으로 설정하기 . actor의 owner는 actor가 아니다
		
	for (AActor* pistol : PistolList)
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
			
		break; // 하나만 부착했음 다시 for문 반복할필요없음
	}
}

void ANetClassProject_YJCharacter::MyReleasePistol()
{
	// 이미 잡은상태에서는 놓아야함
	if (bHasPistol)
	{
		
		
		bHasPistol = false;
	}
	
	
	if (GrabPistolActor) // 총을 오너를 지우고 ,잊고
	{
		DetachPistol(); // Detatch 하기
		GrabPistolActor->SetOwner(nullptr);
		GrabPistolActor = nullptr;
	}
}


void ANetClassProject_YJCharacter::AttachPistol(AActor* pistolActor)
{
	GrabPistolActor =pistolActor;
	
	auto* mesh =GrabPistolActor->GetComponentByClass<UStaticMeshComponent>();
	mesh->SetSimulatePhysics(false);
	// AttachToComponent 를 컴포넌트 단위 인지 액터단위인지...???
	mesh->AttachToComponent(HandComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	//pistolActor->AttachToComponent(HandComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void ANetClassProject_YJCharacter::DetachPistol()
{
	// 총을 가져와서 detach하기
	auto* mesh =GrabPistolActor->GetComponentByClass<UStaticMeshComponent>();
	check(mesh);  //==> 디버깅을 위해 일부러 오류발생시키기
	if(mesh)
	{
		// 다시 물리를 켜주기
		mesh->SetSimulatePhysics(true);
		mesh->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	}
}

void ANetClassProject_YJCharacter::OnFirePistol(const FInputActionValue& value)
{
 
	if(!bHasPistol) {return;}

	UNetTpsPlayerAnim* anim = CastChecked<UNetTpsPlayerAnim>(GetMesh()->GetAnimInstance());
	check(anim);

	//캐스트하면서 동시에 체크도 !
	
	if(anim)
	{
		anim->PlayFireAnimMontage();
	}
	
	FVector start = FollowCamera->GetComponentLocation();
	FVector end = start+FollowCamera->GetForwardVector()*10000.f;
	FHitResult hitResult;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	
	bool hit =GetWorld()->LineTraceSingleByChannel(hitResult,start,end,ECC_Visibility,params);
	if(hit)
	{
		FVector hitVec = hitResult.ImpactPoint;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),BullectFX,hitVec);
	}
	
}
