// Copyright (c) 2020 Jeff Brown (jeffjjbrown@gmail.com)

#include "CommonVRCharacter.h"

#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"

#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"

#include "CommonVRHandController.h"

/************************
Constructor and Overrides
************************/

ACommonVRCharacter::ACommonVRCharacter()
{
  PrimaryActorTick.bCanEverTick = true;

  VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
  VRRoot->SetupAttachment(GetRootComponent());

  TeleportPath = CreateDefaultSubobject<USplineComponent>(TEXT("TeleportPath"));
  TeleportPath->SetupAttachment(VRRoot);

  DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
  DestinationMarker->SetupAttachment(GetRootComponent());
  DestinationMarker->SetVisibility(false);

  RotationIndication = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RotationIndication"));
  RotationIndication->SetupAttachment(DestinationMarker);
}

void ACommonVRCharacter::BeginPlay()
{
  Super::BeginPlay();

  SpawnHands();
}

void ACommonVRCharacter::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  UpdateDestinationMarker();
}

void ACommonVRCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
  Super::SetupPlayerInputComponent(PlayerInputComponent);
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
}

void ACommonVRCharacter::UpdateDestinationMarker()
{
  FVector NavLocation;
  TArray<FVector> Path;

  if (ActiveTeleportHand < 0)
    return;

  bool bHasDestination = FindTeleportDestination((bool)ActiveTeleportHand, Path, NavLocation);

  if (bHasDestination && bTeleportEnabled)
  {
    DestinationMarker->SetVisibility(true);
    DestinationMarker->SetWorldLocation(NavLocation);
    //DrawTeleportPath(Path);
  }
  else
  {
    DestinationMarker->SetVisibility(false);

    TArray<FVector> EmptyPath;
    //DrawTeleportPath(EmptyPath);
  }
}

bool ACommonVRCharacter::FindTeleportDestination(bool bHand, TArray<FVector> &OutPath, FVector &OutLocation)
{
  FVector Start, Look;

  if (bHand == LEFT_HAND)
  {
    Start = LeftHandController->GetActorLocation();
    Look = LeftHandController->GetActorForwardVector();
  }
  else if (bHand == RIGHT_HAND)
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
  Params.DrawDebugType = EDrawDebugTrace::ForOneFrame;

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
