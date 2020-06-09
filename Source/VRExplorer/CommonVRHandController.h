// Copyright (c) 2020 Jeff Brown (jeffjjbrown@gmail.com)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "CommonVRCharacter.h"

#include "CommonVRHandController.generated.h"

#define LEFT_HAND 0
#define RIGHT_HAND 1

class UMotionControllerComponent;

UCLASS()
class VREXPLORER_API ACommonVRHandController : public AActor
{
  GENERATED_BODY()

public:
  // Constructor and Overrides
  ACommonVRHandController();
  virtual void Tick(float DeltaTime) override;

  // Public Class Methods
  void SetHand(int Hand);
  void PairController(ACommonVRHandController *Controller);
  void SetParentVRChar(ACommonVRCharacter *NewParentVRChar) { ParentVRChar = NewParentVRChar; }
  void BindInputs();

  bool IsHandTeleporting() { return bIsTeleporting; }
  void SetHandTeleporting(bool TeleportStatus) { bIsTeleporting = TeleportStatus; }

  bool IsHandRotating() { return bIsRotating; }
  void SetHandRotating(bool RotateStatus) { bIsRotating = RotateStatus; }

protected:
  virtual void BeginPlay() override;

private:
  // Private class methods
  void EnableTeleportation(float throttle);
  void RotatePlayer(float throttle);

  // Components
  UPROPERTY(VisibleAnywhere)
  class UMotionControllerComponent *MotionController;

  UPROPERTY(VisibleAnywhere)
  class UChildActorComponent *ChildHand;

  UPROPERTY(VisibleAnywhere)
  class ACommonVRCharacter *ParentVRChar;

  UPROPERTY(VisibleAnywhere)
  ACommonVRHandController *OtherController;

  // Configuration parameters
  UPROPERTY(EditAnywhere)
  float TeleportThumbstickThreshold = 0.75f;

  UPROPERTY(EditAnywhere)
  float RotationThumbstickThreshold = 0.75f;

  UPROPERTY(EditAnywhere)
  float AngleToRotateBy = 45.f;

  // State
  int HandIdentity = -1;
  bool bIsTeleporting = false;
  bool bIsRotating = false;

  bool bRotateToLeftReady = false;
  bool bRotateToRightReady = false;
};
