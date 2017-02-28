// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "VRCharacter.generated.h"

class UGrabItem;

UCLASS()
class PROYECTO_API AVRCharacter : public ACharacter {
    GENERATED_BODY()

public:
    AVRCharacter(const FObjectInitializer& OI);
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

    void SetupVROptions();
    void ResetHMDOrigin();
    /* Toggle between Seated and Standing VR Tracking */
    void ToggleTrackingSpace();

protected:
    UPROPERTY(EditDefaultsOnly, Category = "VR")
    bool bPositionalHeadTracking;

    /************************************* MAIN COMPONENTS ***************************************/
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* VROriginComp;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UCameraComponent* CameraComp;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class USteamVRChaperoneComponent* ChaperoneComp;

    /*********************************** MOTION CONTROLLERS **************************************/
    /************ LEFT ***********/
    /* Motion Controllers */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UMotionControllerComponent* LeftHandComp;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* SM_LeftHand;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UArrowComponent* LeftArrow;
    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    //class USPlineComponent* LeftSPline;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USphereComponent* LeftSphere;
    /*********** RIGHT ***********/
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UMotionControllerComponent* RightHandComp;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* SM_RightHand;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UArrowComponent* RightArrow;
    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    //class USPlineComponent* RightSPline;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USphereComponent* RightSphere;

    /********************************** ACTION MAPPINGS ******************************************/
    /* MOVEMENT */
    void MoveForward(float Val);
    void MoveRight(float Val);
    //void TurnAtRate(float Rate);
    //void LookUpAtRate(float Rate);

    /************** TRIGGER LEFT *************/
    void TriggerLeft();
    /************** TRIGGER RIGHT *************/
    void TriggerRight();
    /*************** TRIGGER MENU *************/
    void ToogleMenu();

private:
    IHeadMountedDisplay* HMD;
    AStaticMeshActor* _itemLeft;
    AStaticMeshActor* _itemRight;

    void BuildLeft();
    void BuildRight();

    /*************** USE TRIGGER *************/
    UFUNCTION(Server, Reliable, WithValidation)
    void SERVER_Use(UActorComponent* component);
    UFUNCTION(NetMulticast, Reliable)
    void MULTI_Use(UActorComponent* component);

    /************** GRAB TRIGGER *************/
    UFUNCTION(Server, Reliable, WithValidation)
    void SERVER_GrabItem(bool isLeft, UGrabItem* grabComp);
    UFUNCTION(NetMulticast, Reliable)
    void MULTI_GrabItem(bool isLeft, UGrabItem* grabComp);

    /************** DROP TRIGGER *************/
    UFUNCTION(Server, Reliable, WithValidation)
    void SERVER_DropItem(bool isLeft);
    UFUNCTION(NetMulticast, Reliable)
    void MULTI_DropItem(bool isLeft);
};
