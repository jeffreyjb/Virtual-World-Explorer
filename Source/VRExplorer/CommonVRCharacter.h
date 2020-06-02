// Copyright (c) 2020 Jeff Brown (jeffjjbrown@gmail.com)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CommonVRCharacter.generated.h"

class ACommonVRHandController;

UCLASS()
class VREXPLORER_API ACommonVRCharacter : public ACharacter
{
  GENERATED_BODY()

public:
  // Constructor and Overrides
  ACommonVRCharacter();
  virtual void Tick(float DeltaTime) override;
  virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

protected:
  virtual void BeginPlay() override;

private:
  // Components
  UPROPERTY(VisibleAnywhere)
  class USceneComponent *VRRoot;

  UPROPERTY(VisibleAnywhere)
  ACommonVRHandController *LeftHandController;

  UPROPERTY(VisibleAnywhere)
  ACommonVRHandController *RightHandController;

  UPROPERTY(EditDefaultsOnly)
  TSubclassOf<ACommonVRHandController> LeftHandControllerClass;

  UPROPERTY(EditDefaultsOnly)
  TSubclassOf<ACommonVRHandController> RightHandControllerClass;

  // Private Class Methods
  void SpawnHands();
};
