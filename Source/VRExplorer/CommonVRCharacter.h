// Copyright (c) 2020 Jeff Brown (jeffjjbrown@gmail.com)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CommonVRCharacter.generated.h"

#define LEFT_HAND 0
#define RIGHT_HAND 1

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
  // Private Class Methods
  void SpawnHands();

  void UpdateDestinationMarker();
  bool FindTeleportDestination(bool bHand, TArray<FVector> &OutPath, FVector &OutLocation);
  //void DrawTeleportPath(const TArray<FVector> &Path);
  //void UpdateSpline(const TArray<FVector> &Path);

  // Base Character Components
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

  // Teleportation Components
  UPROPERTY(VisibleAnywhere)
  class USplineComponent *TeleportPath;

  UPROPERTY(VisibleAnywhere)
  class UStaticMeshComponent *DestinationMarker;

  UPROPERTY(VisibleAnywhere)
  UStaticMeshComponent *RotationIndication;

  // Teleportation Configuration
  UPROPERTY(EditAnywhere)
  float TeleportProjectileRadius = 10.f;

  UPROPERTY(EditAnywhere)
  float TeleportProjectileSpeed = 1000.f; // 10 cm/s

  UPROPERTY(EditAnywhere)
  float TeleportSimulationTime = 3.f; // 10 s

  UPROPERTY(EditAnywhere)
  FVector TeleportProjectionExtent = FVector(100, 100, 100);

  UPROPERTY(EditAnywhere)
  bool bTeleportEnabled = false;

  UPROPERTY(EditAnywhere)
  int32 ActiveTeleportHand = -1;
};
