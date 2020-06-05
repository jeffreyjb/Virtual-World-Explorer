// Copyright (c) 2020 Jeff Brown (jeffjjbrown@gmail.com)

#include "CommonVRHandController.h"

#include "MotionControllerComponent.h"
#include "Components/ChildActorComponent.h"

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
}

void ACommonVRHandController::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
}

/**************
 Class Methods
 *************/

void ACommonVRHandController::SetHand(EControllerHand Hand)
{
  MotionController->SetTrackingSource(Hand);
}

void ACommonVRHandController::PairController(ACommonVRHandController *Controller)
{
  OtherController = Controller;
  OtherController->OtherController = this;
}