// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "NetTpsPlayerAnim.generated.h"

/**
 * 
 */
UCLASS()
class NETCLASSPROJECT_YJ_API UNetTpsPlayerAnim : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	bool bHasPistol;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	float Horizontal;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	float Vertical;

	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY()
	class ANetClassProject_YJCharacter* Me;

	UPROPERTY(EditDefaultsOnly)
	class UAnimMontage* FireMontage;

	void PlayFireAnimMontage();

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	float PitchAngle;
};
