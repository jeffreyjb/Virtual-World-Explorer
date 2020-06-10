// Copyright (c) 2020 Jeff Brown (jeffjjbrown@gmail.com)

#include "CommonVRHandController.h"

#include "MotionControllerComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/InputComponent.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#include "Engine/World.h"

/************************
Constructor and Overrides
************************/

ACommonVRHandController::ACommonVRHandController()
{
  PrimaryActorTick.bCanEverTick = true;

  MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController"));
  SetRootComponent(MotionController);

  ChildHand = CreateDefaultSubobject<UChildActorComponent>(TEXT("Hand"));
  ChildHand->SetupAttachment(MotionController);
}

void ACommonVRHandController::BeginPlay()
{
  Super::BeginPlay();

  OnActorBeginOverlap.AddDynamic(this, &ACommonVRHandController::ActorBeginOverlap);
  OnActorEndOverlap.AddDynamic(this, &ACommonVRHandController::ActorEndOverlap);
}

void ACommonVRHandController::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
}

/********************
 Public Class Methods
 *******************/

void ACommonVRHandController::SetHand(int Hand)
{
  if (Hand == LEFT_HAND)
  {
    MotionController->SetTrackingSource(EControllerHand::Left);
  }
  else if (Hand == RIGHT_HAND)
  {
    MotionController->SetTrackingSource(EControllerHand::Right);
  }
  else
  {
    return;
  }
  HandIdentity = Hand;
}

void ACommonVRHandController::PairController(ACommonVRHandController *Controller)
{
  OtherController = Controller;
  OtherController->OtherController = this;
}

void ACommonVRHandController::BindInputs()
{
  if (!ParentVRChar)
    return;

  UInputComponent *ParentPInComponent = ParentVRChar->GetPlayerInputComponent();
  if (!ParentPInComponent)
    return;

  if (HandIdentity == LEFT_HAND)
  {
    // Bind Axis events
    ParentPInComponent->BindAxis(TEXT("LThumbX"), this, &ACommonVRHandController::RotatePlayer);
    ParentPInComponent->BindAxis(TEXT("LThumbY"), this, &ACommonVRHandController::EnableTeleportation);

    // Bind Action events
    ParentPInComponent->BindAction(TEXT("GrabLeft"), IE_Pressed, this, &ACommonVRHandController::GrabObject);
  }
  else if (HandIdentity == RIGHT_HAND)
  {
    // Bind Axis events
    ParentPInComponent->BindAxis(TEXT("RThumbX"), this, &ACommonVRHandController::RotatePlayer);
    ParentPInComponent->BindAxis(TEXT("RThumbY"), this, &ACommonVRHandController::EnableTeleportation);

    // Bind Action events
    ParentPInComponent->BindAction(TEXT("GrabRight"), IE_Pressed, this, &ACommonVRHandController::GrabObject);
  }
  else
  {
    return;
  }
}

/*********************
 Private Class Methods
 ********************/

void ACommonVRHandController::EnableTeleportation(float throttle)
{
  if (!ParentVRChar)
    return;

  float ThumbX;

  // If we are already in teleportation selection
  if (bIsTeleporting)
  {
    ParentVRChar->UpdateTeleportationRotation();
    if (HandIdentity == LEFT_HAND)
    {
      ThumbX = ParentVRChar->GetInputAxisValue(TEXT("LThumbX"));
    }
    else if (HandIdentity == RIGHT_HAND)
    {
      ThumbX = ParentVRChar->GetInputAxisValue(TEXT("RThumbX"));
    }
    else
    {
      return;
    }

    float TotalThrottle = ThumbX * ThumbX + throttle * throttle;
    float ThrottleThreshSquare = TeleportThumbstickThreshold * TeleportThumbstickThreshold;
    if (TotalThrottle < ThrottleThreshSquare)
    {
      ParentVRChar->BeginTeleport();
      bIsTeleporting = false;
    }
  }

  // Start teleportation selection if we breach our threshold value
  if ((throttle < -TeleportThumbstickThreshold || throttle > TeleportThumbstickThreshold) && !(OtherController->IsHandTeleporting()))
  {
    if (HandIdentity == LEFT_HAND)
    {
      ParentVRChar->SetActiveTeleportHand(LEFT_HAND);
    }
    else if (HandIdentity == RIGHT_HAND)
    {
      ParentVRChar->SetActiveTeleportHand(RIGHT_HAND);
    }
    else
    {
      return;
    }

    bIsTeleporting = true;
  }
}

void ACommonVRHandController::RotatePlayer(float throttle)
{
  if (!ParentVRChar)
    return;

  if (bIsTeleporting || OtherController->IsHandRotating())
    return;

  if (!bRotateToLeftReady && !bRotateToRightReady)
  {
    if (throttle < -RotationThumbstickThreshold)
    {
      ParentVRChar->RotateThePlayer(-AngleToRotateBy);
      bRotateToLeftReady = true;
      bIsRotating = true;
      return;
    }
    if (throttle > RotationThumbstickThreshold)
    {
      ParentVRChar->RotateThePlayer(AngleToRotateBy);
      bRotateToRightReady = true;
      bIsRotating = true;
      return;
    }
  }

  if (bRotateToLeftReady && throttle >= -RotationThumbstickThreshold)
  {
    bRotateToLeftReady = false;
    bIsRotating = false;
    return;
  }

  if (bRotateToRightReady && throttle <= RotationThumbstickThreshold)
  {
    bRotateToRightReady = false;
    bIsRotating = false;
    return;
  }
}

void ACommonVRHandController::GrabObject()
{
  AActor *ObjectToGrab;
  if (CanGrab(ObjectToGrab))
  {
    ObjectToGrab->DisableComponentsSimulatePhysics();
    ObjectToGrab->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
  }
}

bool ACommonVRHandController::CanGrab() const
{
  TArray<AActor *> OverlappingActors;
  GetOverlappingActors(OverlappingActors);
  for (AActor *OverlappingActor : OverlappingActors)
  {
    if (OverlappingActor->ActorHasTag(TEXT("Grabbable")))
    {
      return true;
    }
  }
  return false;
}

bool ACommonVRHandController::CanGrab(AActor *&OutActor) const
{
  TArray<AActor *> OverlappingActors;
  GetOverlappingActors(OverlappingActors);
  for (AActor *OverlappingActor : OverlappingActors)
  {
    if (OverlappingActor->ActorHasTag(TEXT("Grabbable")))
    {
      OutActor = OverlappingActor;
      return true;
    }
  }
  return false;
}

/*****************
 Private Callbacks
 ****************/
void ACommonVRHandController::ActorBeginOverlap(AActor *OverlappedActor, AActor *OtherActor)
{
  if (CanGrab())
  {
    APawn *Pawn = Cast<APawn>(GetAttachParentActor());
    if (!Pawn)
      return;
    APlayerController *Controller = Cast<APlayerController>(Pawn->GetController());
    if (!Controller)
      return;

    Controller->PlayHapticEffect(HapticEffect, MotionController->GetTrackingSource());
  }
}

void ACommonVRHandController::ActorEndOverlap(AActor *OverlappedActor, AActor *OtherActor)
{
}
