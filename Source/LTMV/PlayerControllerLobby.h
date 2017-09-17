// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NWGameInstance.h"

#include "GameFramework/PlayerController.h"
#include "PlayerControllerLobby.generated.h"


UCLASS()
class LTMV_API APlayerControllerLobby : public APlayerController {
    GENERATED_BODY()

public:
    APlayerControllerLobby(const FObjectInitializer& OI);

    UFUNCTION(Client, Reliable)
    void CLIENT_InitialSetup();
    UFUNCTION(Server, Reliable, WithValidation)
    void SERVER_CallUpdate(FPlayerInfo info, bool changedStatus);

    void AfterPossessed();
    //This override is because CLIENT_AfterPossessed does not work in host (Client-server)
    UFUNCTION()
    void OnRep_Pawn() override;

    UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Menu")
    void CLIENT_CreateMenu();

    UFUNCTION(Client, Reliable, BlueprintCallable, Category = "VR Inventory")
    void CLIENT_CreateVRInventory();

protected:
    virtual void SetupInputComponent() override;

    /********************************** ACTION MAPPINGS ******************************************/
    /******** USE ITEM LEFT *********/
    void UseLeftPressed();
    void UseLeftReleased();
    /******** USE ITEM RIGHT *********/
    void UseRightPressed();
    void UseRightReleased();

    /*************** TRIGGER MENU *************/
    void ToogleMenu();

private:
    bool _ClientPossesed;
    FString _MapMainMenu;

    /* MENU INTERFACE */
    class AMenu3D* _MenuActor;
    void CreateMenuActor(bool IsMainMenu);

    /* VR INVENTORY INTERFACE*/
    class AVRInventory* _VRInventoryActor;
    void CreateDestroyVRInventoryActor(bool IsMainMenu);
};
