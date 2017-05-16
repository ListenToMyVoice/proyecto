// Fill out your copyright notice in the Description page of Project Settings.

#include "LTMV.h"
#include "VRCharacter.h"

#include "ItfUsable.h"
#include "ItfUsableItem.h"
#include "GrabItem.h"

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
    _LeftHandComp->SetRelativeLocation(FVector(10.f, 0.f, 0.f));

    /* MESH */
    _SM_LeftHand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("_SM_LeftHand"));
    _SM_LeftHand->AttachToComponent(_LeftHandComp, FAttachmentTransformRules::KeepRelativeTransform);
    _SM_LeftHand->SetWorldScale3D(FVector(1.0f, 1.0f, -1.0f));
    _SM_LeftHand->SetRelativeRotation(FRotator(0.f, 0.0f, 90.f));
    _SM_LeftHand->SetRelativeLocation(FVector(-10.f, 0.f, 0.f));

    /* ADDITIONAL */
    _LeftSphere = CreateDefaultSubobject<USphereComponent>(TEXT("_LeftSphere"));
    _LeftSphere->AttachToComponent(_SM_LeftHand, FAttachmentTransformRules::KeepRelativeTransform);
    _LeftSphere->SetRelativeLocation(FVector(10.f, 0.f, 0.f));
    _LeftSphere->SetSphereRadius(10.f);
}

void AVRCharacter::BuildRight() {
    _RightHandComp = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("_RightHandComp"));
    _RightHandComp->Hand = EControllerHand::Right;
    _RightHandComp->AttachToComponent(_VROriginComp, FAttachmentTransformRules::KeepRelativeTransform);
    _RightHandComp->SetRelativeLocation(FVector(10.f, 0.f, 0.f));

    /* MESH */
    _SM_RightHand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("_SM_RightHand"));
    _SM_RightHand->AttachToComponent(_RightHandComp, FAttachmentTransformRules::KeepRelativeTransform);
    _SM_RightHand->SetRelativeRotation(FRotator(0.f, 0.0f, 90.f));
    _SM_RightHand->SetRelativeLocation(FVector(-10.f, 0.f, 0.f));

    /* ADDITIONAL */
    _RightSphere = CreateDefaultSubobject<USphereComponent>(TEXT("_RightSphere"));
    _RightSphere->AttachToComponent(_SM_RightHand, FAttachmentTransformRules::KeepRelativeTransform);
    _RightSphere->SetRelativeLocation(FVector(10.f, 0.f, 0.f));
    _RightSphere->SetSphereRadius(10.f);
}

void AVRCharacter::BeginPlay() {
    Super::BeginPlay();

    if (HMD == nullptr) {
        HMD = (IHeadMountedDisplay*)(GEngine->HMDDevice.Get());
        SetupVROptions();
    }

    _GrabDelegateLeft.BindUObject(this, &AVRCharacter::ItemGrabbedLeft);
    _GrabDelegateRight.BindUObject(this, &AVRCharacter::ItemGrabbedRight);

    UE_LOG(LogTemp, Warning, TEXT("Current player orientation: %s"), *this->GetActorRotation().ToString());
    UE_LOG(LogTemp, Warning, TEXT("Current player vector: %s"), *this->GetActorForwardVector().ToString());
    TargetOrientation = this->GetActorRotation();
}

void AVRCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInput) {
    Super::SetupPlayerInputComponent(PlayerInput);

    /* ACTIONS */
    PlayerInput->BindAction("DropLeft", IE_Pressed, this, &AVRCharacter::DropLeft);
    PlayerInput->BindAction("DropRight", IE_Pressed, this, &AVRCharacter::DropRight);

    /* VR SPECIFIC */
    PlayerInput->BindAction("ToggleTrackingSpace", IE_Pressed, this, &AVRCharacter::ToggleTrackingSpace);
    PlayerInput->BindAction("ResetHMDOrigin", IE_Pressed, this, &AVRCharacter::ResetHMDOrigin);

    /* MOVEMENT */
    PlayerInput->BindAction("TurnVRCharacter", IE_Pressed, this, &AVRCharacter::TurnVRCharacter);
}

void AVRCharacter::SetupVROptions() {
    if (HMD) {
        HMD->EnablePositionalTracking(bPositionalHeadTracking);
        /* Remove any translation when disabling positional head tracking */
        if (!bPositionalHeadTracking) _PlayerCamera->SetRelativeLocation(FVector(0, 0, 0));
    }
    ResetHMDOrigin();
}

void AVRCharacter::ResetHMDOrigin() {// R
    if (HMD) HMD->ResetOrientationAndPosition();
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

void AVRCharacter::MoveForward(float Value) {
    if (Value != 0.0f) {
        AddMovementInput(this->GetActorForwardVector(), Value);
        UE_LOG(LogTemp, Warning, TEXT("Forward vector: %s"), *this->GetActorForwardVector().ToString());
    }
}

void AVRCharacter::TurnVRCharacter() {
    float _YawValue = _PlayerCamera->GetForwardVector().Rotation().Yaw - this->GetActorForwardVector().Rotation().Yaw;
    // TargetOrientation.Yaw = VRCameraForwardVector.Rotation().Yaw;
    // this->SetActorRotation(TargetOrientation);
    AddControllerYawInput(_YawValue);
    HMD->ResetOrientation(_YawValue);
    UE_LOG(LogTemp, Warning, TEXT("Current player orientation: %s"), *this->GetActorRotation().ToString());
    UE_LOG(LogTemp, Warning, TEXT("Current player vector: %s"), *this->GetActorForwardVector().ToString());
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
            else if (Component->GetClass() == UGrabItem::StaticClass()) {
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
        else if (_ActorFocusedLeft) UseTriggerPressed(_ActorFocusedLeft, _SM_LeftHand, 1);
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
        else if (_ActorFocusedLeft || _ActorGrabbing)
            UseTriggerReleased(_ActorFocusedLeft, _SM_LeftHand, 1);
    }
    else _MenuInteractionComp->ReleasePointer();

    /* ANIMATION */
    _GripStateLeft = EGripEnum::Open;
}

/******* USE ITEM RIGHT *** ******/
void AVRCharacter::UseRightPressed(bool IsMenuHidden) {
    if (IsMenuHidden) {
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
        else if (_ActorFocusedRight) UseTriggerPressed(_ActorFocusedRight, _SM_RightHand, 2);
    }
    else _MenuInteractionComp->PressPointer();

    /* ANIMATION */
    _GripStateRight = EGripEnum::Grab;
}

void AVRCharacter::UseRightReleased(bool IsMenuHidden) {
    if (IsMenuHidden) {
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
        else if (_ActorFocusedRight || _ActorGrabbing)
            UseTriggerReleased(_ActorFocusedRight, _SM_RightHand, 2);
    }
    else _MenuInteractionComp->ReleasePointer();

    /* ANIMATION */
    _GripStateRight = EGripEnum::Open;
}

/*************** USE TRIGGER *************/
void AVRCharacter::UseTriggerPressed(AActor* ActorFocused, USceneComponent* InParent, int Hand) {
    if (ActorFocused) {
        /* CAN BE GRABBED */
        UGrabItem* GrabItemComp = Cast<UGrabItem>(ActorFocused->GetComponentByClass(
            UGrabItem::StaticClass()));
        if (GrabItemComp) {
            /* FOCUS */
            UStaticMeshComponent* _StaticMesh = Cast<UStaticMeshComponent>(ActorFocused->GetComponentByClass(
                UStaticMeshComponent::StaticClass()));
            if (_StaticMesh) {
                _StaticMesh->SetCustomDepthStencilValue(0);
                _StaticMesh->SetRenderCustomDepth(false);
            }
            SERVER_GrabPress(ActorFocused, InParent, FName("TakeSocket"), Hand);
        }
        else {
            /* CAN BE USED */
            TArray<UActorComponent*> Components;
            ActorFocused->GetComponents(Components);
            for (UActorComponent* Component : Components) {
                if (Component->GetClass()->ImplementsInterface(UItfUsable::StaticClass())) {
                    SERVER_UsePressed(Component);
                }
            }
        }
    }
}

void AVRCharacter::UseTriggerReleased(AActor* ActorFocused, USceneComponent* InParent, int Hand) {
    if (_ActorGrabbing) {
        /* DROP GRABBING */
        UGrabItem* GrabItemComp = Cast<UGrabItem>(_ActorGrabbing->GetComponentByClass(
            UGrabItem::StaticClass()));
        if (GrabItemComp) {
            /* FOCUS */
            UStaticMeshComponent* _StaticMesh = Cast<UStaticMeshComponent>(_ActorGrabbing->GetComponentByClass(
                UStaticMeshComponent::StaticClass()));
            if (_StaticMesh) {
                _StaticMesh->SetCustomDepthStencilValue(0);
                _StaticMesh->SetRenderCustomDepth(false);
            }
            /* Drop item */
            SERVER_GrabRelease(Hand);
        }
    }
    else if (ActorFocused) {
        /* CAN BE USED */
        TArray<UActorComponent*> Components;
        ActorFocused->GetComponents(Components);
        for (UActorComponent* Component : Components) {
            if (Component->GetClass()->ImplementsInterface(UItfUsable::StaticClass())) {
                SERVER_UseReleased(Component);
            }
        }
    }
}

/********** TAKE ITEM ***********/
bool AVRCharacter::SERVER_GrabPress_Validate(AActor* Actor, USceneComponent* InParent,
                                             FName SocketName, int Hand) {
    return true;
}
void AVRCharacter::SERVER_GrabPress_Implementation(AActor* Actor, USceneComponent* InParent,
                                                   FName SocketName, int Hand) {
    MULTI_GrabPress(Actor, InParent, SocketName, Hand);
}
void AVRCharacter::MULTI_GrabPress_Implementation(AActor* Actor, USceneComponent* InParent,
                                                  FName SocketName, int Hand) {
    if (Actor) {
        _ActorGrabbing = Actor;
        UGrabItem* GrabItemComp = Cast<UGrabItem>(_ActorGrabbing->FindComponentByClass(
            UGrabItem::StaticClass()));
        if (GrabItemComp && InParent) {
            GrabItemComp->BeginGrab(InParent, SocketName);
            _ActorGrabbing->SetActorEnableCollision(false);

            if (Hand == 1) GrabItemComp->AddOnGrabDelegate(_GrabDelegateLeft);
            else if (Hand == 2) GrabItemComp->AddOnGrabDelegate(_GrabDelegateRight);
        }
    }
}

bool AVRCharacter::SERVER_GrabRelease_Validate(int Hand) {
    return true;
}
void AVRCharacter::SERVER_GrabRelease_Implementation(int Hand) {
    MULTI_GrabRelease(Hand);
}
void AVRCharacter::MULTI_GrabRelease_Implementation(int Hand) {
    if (_ActorGrabbing) {
        UGrabItem* GrabItemComp = Cast<UGrabItem>(_ActorGrabbing->FindComponentByClass(
            UGrabItem::StaticClass()));
        if (GrabItemComp) {
            GrabItemComp->EndGrab(true);
            _ActorGrabbing->SetActorEnableCollision(true);
            _ActorGrabbing = nullptr;
        }
    }
}

void AVRCharacter::ItemGrabbedLeft() {
    if (_ActorGrabbing) {
        UStaticMeshComponent* ItemMesh = Cast<UStaticMeshComponent>(_ActorGrabbing->GetComponentByClass(
            UStaticMeshComponent::StaticClass()));
        UGrabItem* GrabItemComp = Cast<UGrabItem>(_ActorGrabbing->FindComponentByClass(
            UGrabItem::StaticClass()));
        if (ItemMesh && GrabItemComp) {
            ItemMesh->SetSimulatePhysics(false);
            ItemMesh->AttachToComponent(_SM_LeftHand,
                                        FAttachmentTransformRules::KeepRelativeTransform,
                                        TEXT("TakeSocket"));

            ItemMesh->RelativeLocation = GrabItemComp->_locationAttach_L;
            ItemMesh->RelativeRotation = GrabItemComp->_rotationAttach_L;

            _ActorGrabbing->SetActorEnableCollision(false);
            _ItemLeft = _ActorGrabbing;
            _ActorGrabbing = nullptr;
        }
    }
}

void AVRCharacter::ItemGrabbedRight() {
    if (_ActorGrabbing) {
        UStaticMeshComponent* ItemMesh = Cast<UStaticMeshComponent>(_ActorGrabbing->GetComponentByClass(
            UStaticMeshComponent::StaticClass()));
        UGrabItem* GrabItemComp = Cast<UGrabItem>(_ActorGrabbing->FindComponentByClass(
            UGrabItem::StaticClass()));
        if (ItemMesh && GrabItemComp) {
            ItemMesh->SetSimulatePhysics(false);
            ItemMesh->AttachToComponent(_SM_RightHand,
                                        FAttachmentTransformRules::KeepRelativeTransform,
                                        TEXT("TakeSocket"));

            ItemMesh->RelativeLocation = GrabItemComp->_locationAttach_R;
            ItemMesh->RelativeRotation = GrabItemComp->_rotationAttach_R;

            _ActorGrabbing->SetActorEnableCollision(false);
            _ItemRight = _ActorGrabbing;
            _ActorGrabbing = nullptr;
        }
    }
}

void AVRCharacter::DropLeft() {
    if (_ItemLeft && _ItemLeft->GetComponentByClass(UGrabItem::StaticClass())) {
        /* Drop item */
        SERVER_Drop(_ItemLeft, 1);
    }
}

void AVRCharacter::DropRight() {
    if (_ItemRight && _ItemRight->GetComponentByClass(UGrabItem::StaticClass())) {
        /* Drop item */
        SERVER_Drop(_ItemRight, 2);
    }
}