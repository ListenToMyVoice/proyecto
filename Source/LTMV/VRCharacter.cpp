// Fill out your copyright notice in the Description page of Project Settings.

#include "LTMV.h"
#include "VRCharacter.h"

#include "ItfUsable.h"
#include "ItfUsableItem.h"
#include "HandPickItem.h"

/* VR Includes */
#include "HeadMountedDisplay.h"
#include "MotionControllerComponent.h"
#include "SteamVRChaperoneComponent.h"

AVRCharacter::AVRCharacter(const FObjectInitializer& OI) : Super(OI) {
    PrimaryActorTick.bCanEverTick = true;
    bPositionalHeadTracking = true;

    GetCharacterMovement()->MaxWalkSpeed = 240.0f;
    GetCharacterMovement()->MaxFlySpeed = 240.0f;
    GetCharacterMovement()->MaxCustomMovementSpeed = 240.0f;
    GetCharacterMovement()->MaxWalkSpeedCrouched = 120.0f;
    GetCharacterMovement()->MaxSwimSpeed = 120.0f;

    _VROriginComp = CreateDefaultSubobject<USceneComponent>(TEXT("_VROriginComp"));
    _VROriginComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
    _PlayerCamera->AttachToComponent(_VROriginComp, FAttachmentTransformRules::KeepRelativeTransform);
    _MenuInteractionComp->AttachToComponent(_PlayerCamera, FAttachmentTransformRules::KeepRelativeTransform);
    _ChaperoneComp = CreateDefaultSubobject<USteamVRChaperoneComponent>(TEXT("_ChaperoneComp"));

    HMD = nullptr;
    _GripStateLeft = EGripEnum::Open;
    _GripStateRight = EGripEnum::Open;

    BuildLeft();
    BuildRight();

    _RightSphere->OnComponentBeginOverlap.AddDynamic(this, &AVRCharacter::OnOverlap);
    _RightSphere->OnComponentEndOverlap.AddDynamic(this, &AVRCharacter::OnEndOverlap);
    _LeftSphere->OnComponentBeginOverlap.AddDynamic(this, &AVRCharacter::OnOverlap);
    _LeftSphere->OnComponentEndOverlap.AddDynamic(this, &AVRCharacter::OnEndOverlap);
}

void AVRCharacter::BuildLeft() {
    _LeftHandComp = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("_LeftHandComp"));
    _LeftHandComp->Hand = EControllerHand::Left;
    _LeftHandComp->AttachToComponent(_VROriginComp, FAttachmentTransformRules::KeepRelativeTransform);

    /* MESH */
    _SM_LeftHand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("_SM_LeftHand"));
    _SM_LeftHand->AttachToComponent(_LeftHandComp, FAttachmentTransformRules::KeepRelativeTransform);

    /* ADDITIONAL */
    _LeftSphere = CreateDefaultSubobject<USphereComponent>(TEXT("_LeftSphere"));
    _LeftSphere->AttachToComponent(_SM_LeftHand, FAttachmentTransformRules::KeepRelativeTransform);
}

void AVRCharacter::BuildRight() {
    _RightHandComp = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("_RightHandComp"));
    _RightHandComp->Hand = EControllerHand::Right;
    _RightHandComp->AttachToComponent(_VROriginComp, FAttachmentTransformRules::KeepRelativeTransform);

    /* MESH */
    _SM_RightHand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("_SM_RightHand"));
    _SM_RightHand->AttachToComponent(_RightHandComp, FAttachmentTransformRules::KeepRelativeTransform);

    /* ADDITIONAL */
    _RightSphere = CreateDefaultSubobject<USphereComponent>(TEXT("_RightSphere"));
    _RightSphere->AttachToComponent(_SM_RightHand, FAttachmentTransformRules::KeepRelativeTransform);
}

void AVRCharacter::BeginPlay() {
    Super::BeginPlay();

    HMD = (IHeadMountedDisplay*)(GEngine->HMDDevice.Get());
    //HMD->EnableHMD(true);
    //HMD->EnableStereo(true);
    SetupVROptions();
}

void AVRCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInput) {
    Super::SetupPlayerInputComponent(PlayerInput);

    /* ACTIONS */
    PlayerInput->BindAction("DropLeft", IE_Pressed, this, &AVRCharacter::ToggleTrackingSpace);
    PlayerInput->BindAction("DropRight", IE_Pressed, this, &AVRCharacter::ResetHMDOrigin);

    /* VR SPECIFIC */
    PlayerInput->BindAction("ToggleTrackingSpace", IE_Pressed, this, &AVRCharacter::ToggleTrackingSpace);
    PlayerInput->BindAction("ResetHMDOrigin", IE_Pressed, this, &AVRCharacter::ResetHMDOrigin);
}

void AVRCharacter::SetupVROptions() {
    if (HMD && HMD->IsStereoEnabled()) {
        HMD->EnablePositionalTracking(bPositionalHeadTracking);
        /* Remove any translation when disabling positional head tracking */
        if (!bPositionalHeadTracking) _PlayerCamera->SetRelativeLocation(FVector(0, 0, 0));
    }
    ResetHMDOrigin();
}

void AVRCharacter::ResetHMDOrigin() {// R
    if (HMD && HMD->IsStereoEnabled()) HMD->ResetOrientationAndPosition();
}

void AVRCharacter::ToggleTrackingSpace() {// T
    // TODO: Fix module includes for SteamVR

    //@todo Make this safe once we can add something to the DeviceType enum.  For now, make the terrible assumption this is a SteamVR device.
    //FSteamVRHMD* SteamVRHMD = (FSteamVRHMD*)(GEngine->HMDDevice.Get());
    //if (SteamVRHMD && SteamVRHMD->IsStereoEnabled())
    //{
    // 	ESteamVRTrackingSpace TrackingSpace = SteamVRHMD->GetTrackingSpace();
    // 	SteamVRHMD->SetTrackingSpace(TrackingSpace == ESteamVRTrackingSpace::Seated ? ESteamVRTrackingSpace::Standing : ESteamVRTrackingSpace::Seated);
    //}
}

/************** OVERLAPPING *************/
void AVRCharacter::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                             bool bFromSweep, const FHitResult& SweepResult) {

    if (SweepResult.Actor.IsValid()) {
        TArray<UActorComponent*> Components;
        OtherActor->GetComponents(Components);

        UStaticMeshComponent* _StaticMesh;
        bool HitItem = false;
        for (UActorComponent* Component : Components) {
            //Highlight outline colors:
            //GREEN: 252 | BLUE: 253 | ORANGE: 254 | WHITE: 255
            if (Component->GetClass()->ImplementsInterface(UItfUsable::StaticClass())) {
                _StaticMesh = Cast<UStaticMeshComponent>(OtherActor->GetComponentByClass(
                    UStaticMeshComponent::StaticClass()));

                if (_StaticMesh) {
                    _StaticMesh->SetRenderCustomDepth(true);
                    _StaticMesh->SetCustomDepthStencilValue(252);
                    HitItem = true;
                }
            }
            else if (Component->GetClass() == UHandPickItem::StaticClass()) {
                _StaticMesh = Cast<UStaticMeshComponent>(OtherActor->GetComponentByClass(
                    UStaticMeshComponent::StaticClass()));

                if (_StaticMesh) {
                    _StaticMesh->SetRenderCustomDepth(true);
                    _StaticMesh->SetCustomDepthStencilValue(255);
                    HitItem = true;
                }
            }
        }
        if (HitItem) {
            if (OverlappedComponent == _LeftSphere) {
                ULibraryUtils::Log("OnOverlap Left");
                _ActorFocusedLeft = OtherActor;
                _GripStateLeft = EGripEnum::CanGrab;
            }
            else if (OverlappedComponent == _RightSphere) {
                ULibraryUtils::Log("OnOverlap Right");
                _ActorFocusedRight = OtherActor;
                _GripStateRight = EGripEnum::CanGrab;
            }
        }
    }
}

void AVRCharacter::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
    if (OtherActor == _ActorFocusedLeft) {
        ULibraryUtils::Log("OnEndOverlap Left");
        _ActorFocusedLeft = nullptr;
        _GripStateLeft = EGripEnum::Open;
    }
    else if (OtherActor == _ActorFocusedRight) {
        ULibraryUtils::Log("OnEndOverlap Right");
        _ActorFocusedRight = nullptr;
        _GripStateRight = EGripEnum::Open;
    }

    UStaticMeshComponent* _StaticMesh = Cast<UStaticMeshComponent>(OtherActor->GetComponentByClass(
        UStaticMeshComponent::StaticClass()));
    if (_StaticMesh) {
        _StaticMesh->SetCustomDepthStencilValue(0);
        _StaticMesh->SetRenderCustomDepth(false);
    }
}

/****************************************** ACTION MAPPINGS **************************************/
/******** USE ITEM LEFT *********/
void AVRCharacter::UseLeftPressed(bool IsMenuHidden) {
    if (IsMenuHidden) {
        if (_ItemLeft) {
            TArray<UActorComponent*> Components;
            _ItemLeft->GetComponents(Components);

            for (UActorComponent* Component : Components) {
                if (Component->GetClass()->ImplementsInterface(UItfUsableItem::StaticClass())) {
                    IItfUsableItem* ItfObject = Cast<IItfUsableItem>(Component);
                    if (ItfObject) ItfObject->Execute_UseItemPressed(Component);
                }
            }
        }
        else if (_ActorFocusedLeft) UseTriggerPressed(_ActorFocusedLeft);
    }
    else _MenuInteractionComp->PressPointer();

    /* ANIMATION */
    _GripStateLeft = EGripEnum::Grab;
}

void AVRCharacter::UseLeftReleased(bool IsMenuHidden) {
    if (IsMenuHidden) {
        if (_ItemLeft) {
            TArray<UActorComponent*> Components;
            _ItemLeft->GetComponents(Components);

            for (UActorComponent* Component : Components) {
                if (Component->GetClass()->ImplementsInterface(UItfUsableItem::StaticClass())) {
                    IItfUsableItem* ItfObject = Cast<IItfUsableItem>(Component);
                    if (ItfObject) ItfObject->Execute_UseItemReleased(Component);
                }
            }
        }
        else if (_ActorFocusedLeft) UseTriggerReleased(_ActorFocusedLeft);
    }
    else _MenuInteractionComp->ReleasePointer();

    /* ANIMATION */
    _GripStateLeft = EGripEnum::Open;
}

/******* USE ITEM RIGHT *********/
void AVRCharacter::UseRightPressed(bool IsMenuHidden) {
    if (_ItemRight) {
        TArray<UActorComponent*> Components;
        _ItemRight->GetComponents(Components);

        for (UActorComponent* Component : Components) {
            if (Component->GetClass()->ImplementsInterface(UItfUsableItem::StaticClass())) {
                IItfUsableItem* ItfObject = Cast<IItfUsableItem>(Component);
                if (ItfObject) ItfObject->Execute_UseItemPressed(Component);
            }
        }
    }
    else if (_ActorFocusedRight) UseTriggerPressed(_ActorFocusedRight);

    /* ANIMATION */
    _GripStateRight = EGripEnum::Grab;
}

void AVRCharacter::UseRightReleased(bool IsMenuHidden) {
    if (_ItemRight) {
        TArray<UActorComponent*> Components;
        _ItemRight->GetComponents(Components);

        for (UActorComponent* Component : Components) {
            if (Component->GetClass()->ImplementsInterface(UItfUsableItem::StaticClass())) {
                IItfUsableItem* ItfObject = Cast<IItfUsableItem>(Component);
                if (ItfObject) ItfObject->Execute_UseItemReleased(Component);
            }
        }
    }
    else if (_ActorFocusedRight) UseTriggerReleased(_ActorFocusedRight);

    /* ANIMATION */
    _GripStateRight = EGripEnum::Open;
}

/*************** USE TRIGGER *************/
void AVRCharacter::UseTriggerPressed(AActor* ActorFocused) {
    /* CAN BE USED */
    TArray<UActorComponent*> Components;
    ActorFocused->GetComponents(Components);
    for (UActorComponent* Component : Components) {
        if (Component->GetClass()->ImplementsInterface(UItfUsable::StaticClass())) {
            SERVER_UsePressed(Component);
        }
    }
}

void AVRCharacter::UseTriggerReleased(AActor* ActorFocused) {
    /* CAN BE GRABBED */
    UHandPickItem* HandPickItemComp = Cast<UHandPickItem>(ActorFocused->GetComponentByClass(
        UHandPickItem::StaticClass()));
    if (HandPickItemComp) {
        if (ActorFocused == _ActorFocusedLeft) {
            UStaticMeshComponent* _StaticMesh = Cast<UStaticMeshComponent>(ActorFocused->GetComponentByClass(
                UStaticMeshComponent::StaticClass()));
            if (_StaticMesh) {
                _StaticMesh->SetCustomDepthStencilValue(0);
                _StaticMesh->SetRenderCustomDepth(false);
            }

            _ActorFocusedLeft = nullptr;
        }
        else if (ActorFocused == _ActorFocusedRight) {
            UStaticMeshComponent* _StaticMesh = Cast<UStaticMeshComponent>(ActorFocused->GetComponentByClass(
                UStaticMeshComponent::StaticClass()));
            if (_StaticMesh) {
                _StaticMesh->SetCustomDepthStencilValue(0);
                _StaticMesh->SetRenderCustomDepth(false);
            }

            _ActorFocusedRight = nullptr;
        }
    }

    /* CAN BE USED */
    TArray<UActorComponent*> Components;
    ActorFocused->GetComponents(Components);
    for (UActorComponent* Component : Components) {
        if (Component->GetClass()->ImplementsInterface(UItfUsable::StaticClass())) {
            SERVER_UseReleased(Component);
        }
    }
}

void AVRCharacter::DropLeft() {
    if (_ItemLeft && _ItemLeft->GetComponentByClass(UHandPickItem::StaticClass())) {
        /* Drop item */
        SERVER_Drop(_ItemLeft, 1);
    }
}

void AVRCharacter::DropRight() {
    if (_ItemRight && _ItemRight->GetComponentByClass(UHandPickItem::StaticClass())) {
        /* Drop item */
        SERVER_Drop(_ItemRight, 2);
    }
}