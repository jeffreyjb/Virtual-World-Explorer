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

  bool IsHandTeleporting() const { return bIsTeleporting; }
  void SetHandTeleporting(bool TeleportStatus) { bIsTeleporting = TeleportStatus; }

  bool IsHandRotating() const { return bIsRotating; }
  void SetHandRotating(bool RotateStatus) { bIsRotating = RotateStatus; }

  AActor *GetGrabbedObject() const { return GrabbedObject; }
  void SetGrabbedObject(AActor *NewGrabbedObject) { GrabbedObject = NewGrabbedObject; }

protected:
  virtual void BeginPlay() override;

private:
  // Private class methods
  void EnableTeleportation(float throttle);
  void RotatePlayer(float throttle);
  void GrabObject();
  void ReleaseObject();

  bool CanGrab() const;
  bool CanGrab(AActor *&OutActor) const;

  // Callbacks
  UFUNCTION()
  void ActorBeginOverlap(AActor *OverlappedActor, AActor *OtherActor);

  UFUNCTION()
  void ActorEndOverlap(AActor *OverlappedActor, AActor *OtherActor);

  // Components
  UPROPERTY(VisibleAnywhere)
  class UMotionControllerComponent *MotionController;

  UPROPERTY(VisibleAnywhere)
  class UChildActorComponent *ChildHand;

  UPROPERTY(VisibleAnywhere)
  class ACommonVRCharacter *ParentVRChar;

  UPROPERTY(VisibleAnywhere)
  ACommonVRHandController *OtherController;

  UPROPERTY(VisibleAnywhere)
  AActor *GrabbedObject = nullptr;

  // Configuration parameters
  UPROPERTY(EditDefaultsOnly)
  class UHapticFeedbackEffect_Base *HapticEffect;

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
