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
  bool FindTeleportDestination(TArray<FVector> &OutPath, FVector &OutLocation);
  void UpdateTeleportationRotation();
  void DrawTeleportPath(const TArray<FVector> &Path);
  void UpdateSpline(const TArray<FVector> &Path);

  void EnableTeleportationLeft(float throttle);
  void EnableTeleportationRight(float throttle);

  void RotatePlayerLeftHand(float throttle);
  void RotatePlayerRightHand(float throttle);

  void BeginTeleport();
  void FinishTeleport();
  void StartFade(float FromAlpha, float ToAlpha, FLinearColor FadeColor);

  // Base Character Components
  UPROPERTY(VisibleAnywhere)
  class USceneComponent *VRRoot;

  UPROPERTY(VisibleAnywhere)
  class UCameraComponent *Camera;

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

  UPROPERTY(VisibleAnywhere)
  TArray<class USplineMeshComponent *> TeleportPathMeshPool;

  UPROPERTY(EditDefaultsOnly)
  class UStaticMesh *TeleportArcMesh;

  UPROPERTY(EditDefaultsOnly)
  class UMaterialInstance *TeleportArcMaterial;

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
  int32 ActiveTeleportHand = -1;

  UPROPERTY(EditAnywhere)
  float TeleportThumbstickThreshold = 0.75f;

  UPROPERTY(EditAnywhere)
  float TeleportFadeTime = 0.2f; // Half-Second fade

  UPROPERTY(EditAnywhere)
  FRotator TargetRotation;

  // Rotation States
  UPROPERTY(EditAnywhere)
  float RotationThumbstickThreshold = 0.9f;

  UPROPERTY(EditAnywhere)
  float AngleToRotateBy = 45.f;

  UPROPERTY(EditAnywhere)
  bool bRotateToLeftReady = false;

  UPROPERTY(EditAnywhere)
  bool bRotateToRightReady = false;
};
