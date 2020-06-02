// Copyright (c) 2020 Jeff Brown (jeffjjbrown@gmail.com)

#include "CommonVRCharacter.h"

#include "CommonVRHandController.h"

/************************
Constructor and Overrides
************************/

ACommonVRCharacter::ACommonVRCharacter()
{
  PrimaryActorTick.bCanEverTick = true;

  VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
  VRRoot->SetupAttachment(GetRootComponent());
}

void ACommonVRCharacter::BeginPlay()
{
  Super::BeginPlay();

  SpawnHands();
}

void ACommonVRCharacter::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
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
