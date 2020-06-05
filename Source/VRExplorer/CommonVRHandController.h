// Copyright (c) 2020 Jeff Brown (jeffjjbrown@gmail.com)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CommonVRHandController.generated.h"

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
  void SetHand(EControllerHand Hand);
  void PairController(ACommonVRHandController *Controller);

  bool IsHandTeleporting() { return bIsTeleporting; }
  void SetHandTeleporting(bool TeleportStatus) { bIsTeleporting = TeleportStatus; }

protected:
  virtual void BeginPlay() override;

private:
  UPROPERTY(VisibleAnywhere)
  class UMotionControllerComponent *MotionController;

  UPROPERTY(VisibleAnywhere)
  class UChildActorComponent *ChildHand;

  ACommonVRHandController *OtherController;

  // State
  bool bIsTeleporting = false;
};
