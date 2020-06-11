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

  CalculateVROffsets();
  UpdateDestinationMarker();
}

void ACommonVRCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
  Super::SetupPlayerInputComponent(PlayerInputComponent);

  PInComponent = PlayerInputComponent;
}

/********************
 Public Class Methods
 *******************/
void ACommonVRCharacter::BeginTeleport()
{
  if (!bHasDestination)
    return;

  StartFade(0, 1, FLinearColor::Gray);
  FTimerHandle TimerHandle;
  GetWorldTimerManager().SetTimer(TimerHandle, this, &ACommonVRCharacter::FinishTeleport, TeleportFadeTime);
}

void ACommonVRCharacter::RotateThePlayer(float YawIn)
{
  FRotator Rot = GetRootComponent()->GetRelativeRotation();
  Rot.Yaw += YawIn;
  GetRootComponent()->SetRelativeRotation(Rot);
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

  FVector ForwardVec = DestinationMarker->GetComponentLocation() - GetRootComponent()->GetComponentLocation();
  FVector RightVec = ForwardVec.RotateAngleAxis(90.f, FVector::UpVector);

  // NOTE:
  // Used to set above with two different attempted methods:
  // float MyForwardX = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetActorForwardVector().X;
  // float MyRightY = GetRootComponent()->GetRightVector().Y;

  float XVal = ThumbX * (RightVec.X) + ThumbY * (ForwardVec.X);
  float YVal = ThumbX * (RightVec.Y) + ThumbY * (ForwardVec.Y);
  FVector FinalDirection(XVal, YVal, 1);
  FinalDirection = FinalDirection.GetSafeNormal();

  TargetRotation.Yaw = FinalDirection.Rotation().Yaw;
}

/*********************
 Private Class Methods
 ********************/
void ACommonVRCharacter::SpawnHands()
{
  LeftHandController = GetWorld()->SpawnActor<ACommonVRHandController>(LeftHandControllerClass);
  if (LeftHandController != nullptr)
  {
    LeftHandController->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
    LeftHandController->SetHand(LEFT_HAND);
    LeftHandController->SetParentVRChar(this);
    LeftHandController->SetOwner(this);
  }

  RightHandController = GetWorld()->SpawnActor<ACommonVRHandController>(RightHandControllerClass);
  if (RightHandController != nullptr)
  {
    RightHandController->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
    RightHandController->SetHand(RIGHT_HAND);
    RightHandController->SetParentVRChar(this);
    RightHandController->SetOwner(this);
  }

  if (RightHandController != nullptr && LeftHandController != nullptr)
  {
    LeftHandController->PairController(RightHandController);

    LeftHandController->BindInputs();
    RightHandController->BindInputs();
  }
}

void ACommonVRCharacter::UpdateDestinationMarker()
{
  FVector NavLocation;
  TArray<FVector> Path;

  if (ActiveTeleportHand < 0)
    return;

  bHasDestination = FindTeleportDestination(Path, NavLocation);

  if (bHasDestination && (LeftHandController->IsHandTeleporting() || RightHandController->IsHandTeleporting()))
  {
    DestinationMarker->SetVisibility(true);
    DestinationMarker->SetWorldLocation(NavLocation);

    RotationIndication->SetVisibility(true);
    RotationIndication->SetWorldRotation(TargetRotation);

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

void ACommonVRCharacter::FinishTeleport()
{
  FVector Destination = DestinationMarker->GetComponentLocation();
  Destination += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * GetActorUpVector();
  SetActorLocation(Destination);
  GetRootComponent()->SetRelativeRotation(TargetRotation);

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

void ACommonVRCharacter::CalculateVROffsets()
{
  // Calculate offset for VRRoot and move the character based on offset
  FVector NewCameraOffset = Camera->GetComponentLocation() - GetActorLocation(); // Difference in pos between actor and camera
  NewCameraOffset.Z = 0;                                                         // Only move horizontal
  AddActorWorldOffset(NewCameraOffset);
  VRRoot->AddWorldOffset(-NewCameraOffset);

  // Calculate rotation offset for VRRoot and make sure to not alter the location of the character
  FVector RootLocation = VRRoot->GetComponentLocation();
  float NewCameraRotationYaw = Camera->GetComponentRotation().Yaw - GetActorRotation().Yaw;
  AddActorWorldRotation(FRotator(0, NewCameraRotationYaw, 0));
  VRRoot->AddWorldRotation(FRotator(0, -NewCameraRotationYaw, 0));
  VRRoot->SetWorldLocation(RootLocation);
}

// Add to tick to see look direction on Camera in blue, Root in red, and VRRoot in green
void ACommonVRCharacter::DebugVRCharacterLines()
{
  FVector Start = Camera->GetComponentLocation();
  Start.Z -= 30;
  FVector End = Start + Camera->GetForwardVector() * 10000;

  DrawDebugLine(
      GetWorld(),
      Start,
      End,
      FColor(0, 0, 255),
      false,
      0.f,
      0,
      5);

  FVector Start2 = GetRootComponent()->GetComponentLocation();
  FVector End2 = Start2 + GetRootComponent()->GetForwardVector() * 10000;

  DrawDebugLine(
      GetWorld(),
      Start2,
      End2,
      FColor(255, 0, 0),
      false,
      0.f,
      0,
      5);

  FVector Start3 = VRRoot->GetComponentLocation();
  FVector End3 = Start3 + VRRoot->GetForwardVector() * 10000;

  DrawDebugLine(
      GetWorld(),
      Start3,
      End3,
      FColor(0, 255, 0),
      false,
      0.f,
      0,
      5);
}