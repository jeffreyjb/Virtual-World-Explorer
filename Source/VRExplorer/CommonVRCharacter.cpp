// Copyright (c) 2020 Jeff Brown (jeffjjbrown@gmail.com)

#include "CommonVRCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"

#include "NavigationSystem.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DrawDebugHelpers.h"

#include "CommonVRHandController.h"

/************************
Constructor and Overrides
************************/

ACommonVRCharacter::ACommonVRCharacter()
{
  PrimaryActorTick.bCanEverTick = true;

  VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
  VRRoot->SetupAttachment(GetRootComponent());

  Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
  Camera->SetupAttachment(VRRoot);

  TeleportPath = CreateDefaultSubobject<USplineComponent>(TEXT("TeleportPath"));
  TeleportPath->SetupAttachment(VRRoot);

  DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
  DestinationMarker->SetupAttachment(GetRootComponent());
  DestinationMarker->SetVisibility(false);

  RotationIndication = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RotationIndication"));
  RotationIndication->SetupAttachment(DestinationMarker);
  RotationIndication->SetVisibility(false);
}

void ACommonVRCharacter::BeginPlay()
{
  Super::BeginPlay();

  SpawnHands();
}

void ACommonVRCharacter::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  FVector NewCameraOffset = Camera->GetComponentLocation() - GetActorLocation(); // Difference in pos between actor and camera
  NewCameraOffset.Z = 0;                                                         // Only move horizontal
  AddActorWorldOffset(NewCameraOffset);                                          // Sets up the offset
  VRRoot->AddWorldOffset(-NewCameraOffset);

  UpdateDestinationMarker();
}

void ACommonVRCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
  Super::SetupPlayerInputComponent(PlayerInputComponent);

  PlayerInputComponent->BindAxis(TEXT("LThumbX"), this, &ACommonVRCharacter::RotatePlayerLeftHand);
  PlayerInputComponent->BindAxis(TEXT("LThumbY"), this, &ACommonVRCharacter::EnableTeleportationLeft);

  PlayerInputComponent->BindAxis(TEXT("RThumbX"), this, &ACommonVRCharacter::RotatePlayerRightHand);
  PlayerInputComponent->BindAxis(TEXT("RThumbY"), this, &ACommonVRCharacter::EnableTeleportationRight);

  //PlayerInputComponent->BindAction(TEXT("LTeleport"), IE_Released, this, &ACommonVRCharacter::BeginTeleportLeft);
  //PlayerInputComponent->BindAction(TEXT("RTeleport"), IE_Released, this, &ACommonVRCharacter::BeginTeleportRight);
}

/**************
 Class Methods
 *************/
void ACommonVRCharacter::SpawnHands()
{
  LeftHandController = GetWorld()->SpawnActor<ACommonVRHandController>(LeftHandControllerClass);
  if (LeftHandController != nullptr)
  {
    LeftHandController->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
    LeftHandController->SetHand(EControllerHand::Left);
    LeftHandController->SetOwner(this); // FIX FOR 4.22
  }

  RightHandController = GetWorld()->SpawnActor<ACommonVRHandController>(RightHandControllerClass);
  if (RightHandController != nullptr)
  {
    RightHandController->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
    RightHandController->SetHand(EControllerHand::Right);
    RightHandController->SetOwner(this); // FIX FOR 4.22
  }

  if (RightHandController != nullptr && LeftHandController != nullptr)
  {
    LeftHandController->PairController(RightHandController);
  }
}

void ACommonVRCharacter::UpdateDestinationMarker()
{
  FVector NavLocation;
  TArray<FVector> Path;

  if (ActiveTeleportHand < 0)
    return;

  bool bHasDestination = FindTeleportDestination(Path, NavLocation);

  if (bHasDestination && (LeftHandController->IsHandTeleporting() || RightHandController->IsHandTeleporting()))
  {
    DestinationMarker->SetVisibility(true);
    RotationIndication->SetVisibility(true);
    DestinationMarker->SetWorldLocation(NavLocation);
    DrawTeleportPath(Path);
  }
  else
  {
    DestinationMarker->SetVisibility(false);
    RotationIndication->SetVisibility(false);

    TArray<FVector> EmptyPath;
    DrawTeleportPath(EmptyPath);
  }
}

bool ACommonVRCharacter::FindTeleportDestination(TArray<FVector> &OutPath, FVector &OutLocation)
{
  FVector Start, Look;

  if (ActiveTeleportHand == LEFT_HAND)
  {
    Start = LeftHandController->GetActorLocation();
    Look = LeftHandController->GetActorForwardVector();
  }
  else if (ActiveTeleportHand == RIGHT_HAND)
  {
    Start = RightHandController->GetActorLocation();
    Look = RightHandController->GetActorForwardVector();
  }
  else
  {
    return false;
  }

  FPredictProjectilePathParams Params(
      TeleportProjectileRadius,
      Start,
      Look * TeleportProjectileSpeed,
      TeleportSimulationTime,
      ECollisionChannel::ECC_Visibility,
      this);

  Params.bTraceComplex = true;
  //Params.DrawDebugType = EDrawDebugTrace::ForOneFrame;

  FPredictProjectilePathResult Result;
  bool bHit = UGameplayStatics::PredictProjectilePath(this, Params, Result);

  if (!bHit)
    return false;

  FNavLocation NavLocation;
  bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->ProjectPointToNavigation(Result.HitResult.Location, NavLocation, TeleportProjectionExtent);

  if (!bOnNavMesh)
    return false;

  for (FPredictProjectilePathPointData PointData : Result.PathData)
  {
    OutPath.Add(PointData.Location);
  }

  OutLocation = NavLocation.Location;
  return true;
}

void ACommonVRCharacter::UpdateTeleportationRotation()
{

  float ThumbX, ThumbY;

  if (ActiveTeleportHand == LEFT_HAND)
  {
    ThumbX = GetInputAxisValue(TEXT("LThumbX"));
    ThumbY = GetInputAxisValue(TEXT("LThumbY"));
  }
  else if (ActiveTeleportHand == RIGHT_HAND)
  {
    ThumbX = GetInputAxisValue(TEXT("RThumbX"));
    ThumbY = GetInputAxisValue(TEXT("RThumbY"));
  }
  else
  {
    return;
  }

  float MyForwardX = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetActorForwardVector().X;
  float MyForwardY = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetActorForwardVector().Y;
  float MyRightX = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetActorRightVector().X;
  float MyRightY = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetActorRightVector().Y;

  float XVal = ThumbX * (MyRightX) + ThumbY * (MyForwardX);
  float YVal = ThumbX * (MyRightY) + ThumbY * (MyForwardY);
  FVector FinalDirection(XVal, YVal, 1);
  FinalDirection = FinalDirection.GetSafeNormal();

  TargetRotation.Yaw = FinalDirection.Rotation().Yaw;

  if (!RotationIndication)
    return;

  RotationIndication->SetRelativeRotation(TargetRotation);
}

void ACommonVRCharacter::DrawTeleportPath(const TArray<FVector> &Path)
{
  UpdateSpline(Path);

  for (USplineMeshComponent *SplineMesh : TeleportPathMeshPool)
  {
    SplineMesh->SetVisibility(false);
  }

  int32 SegmentNum = Path.Num() - 1;
  for (int32 i = 0; i < SegmentNum; ++i)
  {
    if (TeleportPathMeshPool.Num() <= i)
    {
      USplineMeshComponent *SplineMesh = NewObject<USplineMeshComponent>(this);
      SplineMesh->SetMobility(EComponentMobility::Movable);
      SplineMesh->AttachToComponent(TeleportPath, FAttachmentTransformRules::KeepRelativeTransform);
      SplineMesh->SetStaticMesh(TeleportArcMesh);
      SplineMesh->SetMaterial(0, TeleportArcMaterial);
      SplineMesh->RegisterComponent(); // Need this to make sure component exists

      TeleportPathMeshPool.Add(SplineMesh);
    }

    USplineMeshComponent *SplineMesh = TeleportPathMeshPool[i];
    SplineMesh->SetVisibility(true);

    FVector StartPos, StartTangent, EndPos, EndTangent;
    TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i, StartPos, StartTangent);
    TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i + 1, EndPos, EndTangent);
    SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
  }
}

void ACommonVRCharacter::UpdateSpline(const TArray<FVector> &Path)
{
  TeleportPath->ClearSplinePoints(false); // false so we don't update spline until we add all points and update once at end
  for (int32 i = 0; i < Path.Num(); ++i)
  {
    FVector LocalPosition = TeleportPath->GetComponentTransform().InverseTransformPosition(Path[i]);
    FSplinePoint Point(i, LocalPosition, ESplinePointType::Curve);
    TeleportPath->AddPoint(Point, false);
  }
  TeleportPath->UpdateSpline(); // Update here
}

void ACommonVRCharacter::EnableTeleportationLeft(float throttle)
{
  // If we are already in teleportation selection
  if (LeftHandController->IsHandTeleporting())
  {
    UpdateTeleportationRotation();
    float ThumbX = GetInputAxisValue(TEXT("LThumbX"));
    float TotalThrottle = ThumbX * ThumbX + throttle * throttle;
    float ThrottleThreshSquare = TeleportThumbstickThreshold * TeleportThumbstickThreshold;
    if (TotalThrottle < ThrottleThreshSquare)
    {
      BeginTeleport();
      LeftHandController->SetHandTeleporting(false);
    }
  }

  // Start teleportation selection if we breach our threshold value
  if ((throttle < -TeleportThumbstickThreshold || throttle > TeleportThumbstickThreshold) && !(RightHandController->IsHandTeleporting()))
  {
    ActiveTeleportHand = LEFT_HAND;
    LeftHandController->SetHandTeleporting(true);
  }
}

void ACommonVRCharacter::EnableTeleportationRight(float throttle)
{
  // If we are already in teleportation selection
  if (RightHandController->IsHandTeleporting())
  {
    UpdateTeleportationRotation();
    float ThumbX = GetInputAxisValue(TEXT("RThumbX"));
    float TotalThrottle = ThumbX * ThumbX + throttle * throttle;
    float ThrottleThreshSquare = TeleportThumbstickThreshold * TeleportThumbstickThreshold;
    if (TotalThrottle < ThrottleThreshSquare)
    {
      BeginTeleport();
      RightHandController->SetHandTeleporting(false);
    }
  }

  // Start teleportation selection if we breach our threshold value
  if ((throttle < -TeleportThumbstickThreshold || throttle > TeleportThumbstickThreshold) && !(LeftHandController->IsHandTeleporting()))
  {
    ActiveTeleportHand = RIGHT_HAND;
    RightHandController->SetHandTeleporting(true);
  }
}

void ACommonVRCharacter::RotatePlayerLeftHand(float throttle)
{
  if (LeftHandController->IsHandTeleporting() || RightHandController->IsHandRotating())
    return;

  if (!bRotateToLeftReady && !bRotateToRightReady)
  {
    if (throttle < -RotationThumbstickThreshold)
    {
      bRotateToLeftReady = true;
      LeftHandController->SetHandRotating(true);
      return;
    }
    if (throttle > RotationThumbstickThreshold)
    {
      bRotateToRightReady = true;
      LeftHandController->SetHandRotating(true);
      return;
    }
  }

  if (bRotateToLeftReady && throttle >= -RotationThumbstickThreshold)
  {
    FRotator Rot = VRRoot->GetRelativeRotation();
    Rot.Yaw -= 30;
    VRRoot->SetWorldRotation(Rot);
    bRotateToLeftReady = false;
    LeftHandController->SetHandRotating(false);
  }

  if (bRotateToRightReady && throttle <= RotationThumbstickThreshold)
  {
    FRotator Rot = VRRoot->GetRelativeRotation();
    Rot.Yaw += 30;
    VRRoot->SetWorldRotation(Rot);
    bRotateToRightReady = false;
    LeftHandController->SetHandRotating(false);
  }
}

void ACommonVRCharacter::RotatePlayerRightHand(float throttle)
{
  if (RightHandController->IsHandTeleporting() || LeftHandController->IsHandRotating())
    return;

  if (!bRotateToLeftReady && !bRotateToRightReady)
  {
    if (throttle < -RotationThumbstickThreshold)
    {
      bRotateToLeftReady = true;
      RightHandController->SetHandRotating(true);
      return;
    }
    if (throttle > RotationThumbstickThreshold)
    {
      bRotateToRightReady = true;
      RightHandController->SetHandRotating(true);
      return;
    }
  }

  if (bRotateToLeftReady && throttle >= -RotationThumbstickThreshold)
  {
    FRotator Rot = VRRoot->GetRelativeRotation();
    Rot.Yaw -= AngleToRotateBy;
    VRRoot->SetWorldRotation(Rot);
    bRotateToLeftReady = false;
    RightHandController->SetHandRotating(false);
  }

  if (bRotateToRightReady && throttle <= RotationThumbstickThreshold)
  {
    FRotator Rot = VRRoot->GetRelativeRotation();
    Rot.Yaw += AngleToRotateBy;
    VRRoot->SetWorldRotation(Rot);
    bRotateToRightReady = false;
    RightHandController->SetHandRotating(false);
  }
}

void ACommonVRCharacter::BeginTeleport()
{
  StartFade(0, 1, FLinearColor::Gray);

  FTimerHandle TimerHandle;
  GetWorldTimerManager().SetTimer(TimerHandle, this, &ACommonVRCharacter::FinishTeleport, TeleportFadeTime);
}

void ACommonVRCharacter::FinishTeleport()
{
  FVector Destination = DestinationMarker->GetComponentLocation();
  Destination += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * GetActorUpVector();
  SetActorLocation(Destination);
  VRRoot->SetWorldRotation(TargetRotation);

  StartFade(1, 0, FLinearColor::Gray);
}

void ACommonVRCharacter::StartFade(float FromAlpha, float ToAlpha, FLinearColor FadeColor)
{
  APlayerController *PC = Cast<APlayerController>(GetController());
  if (PC != nullptr)
  {
    PC->PlayerCameraManager->StartCameraFade(FromAlpha, ToAlpha, TeleportFadeTime, FadeColor);
  }
}