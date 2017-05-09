// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PlayerCharacter.h"
#include "FPCharacter.generated.h"


UCLASS()
class LTMV_API AFPCharacter : public APlayerCharacter {
    GENERATED_BODY()

public:
    /* HUD */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
    TSubclassOf<class UUserWidget> _HUDClass;

    AFPCharacter(const FObjectInitializer& OI);
    void BeginPlay() override;
    void AfterPossessed(bool SetInventory) override;
    void Tick(float DeltaSeconds) override;

    /************* INVENTORY ************/
    // The class that will be used for the players Inventory UI
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
    TSubclassOf<class UInventoryWidget> _InventoryUIClass;

    UFUNCTION(BlueprintCallable, Category = "Player pool Items")
    UTexture2D* GetItemTextureAt(int itemIndex);

    /************** PICK ITEM *************/
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void PickItemInventory(AActor* ItemActor, FKey KeyStruct);
    UFUNCTION(Server, Reliable, WithValidation)
    void SERVER_PickItemInventoryLeft(AActor* ItemActor);
    UFUNCTION(NetMulticast, Reliable)
    void MULTI_PickItemInventoryLeft(AActor* ItemActor);
    UFUNCTION(Server, Reliable, WithValidation)
    void SERVER_PickItemInventoryRight(AActor* ItemActor);
    UFUNCTION(NetMulticast, Reliable)
    void MULTI_PickItemInventoryRight(AActor* ItemActor);

    /******** USE ITEM LEFT *********/
    void UseLeftPressed(bool IsMenuHidden) override;
    void UseLeftReleased(bool IsMenuHidden) override;

    /******* USE ITEM RIGHT *********/
    void UseRightPressed(bool IsMenuHidden) override;
    void UseRightReleased(bool IsMenuHidden) override;

    void ToggleMenuInteraction(bool Activate) override;

protected:
    UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UInventory* _Inventory;
	UPROPERTY(Category = Audio, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* _FirstPersonMesh;

    class UInventoryWidget* _InventoryWidget;
    bool _IsInventoryHidden;

    void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

    /*************************************** ACTION MAPPINGS *************************************/
    /************** USE *************/
    void UsePressed();
    void UseReleased();

    /********** TAKE ITEM ***********/
    void TakeDropRight();
    void TakeDropLeft();

    /********** INVENTORY ***********/
    void ToggleInventory();

    UFUNCTION(Server, Reliable, WithValidation)
    void SERVER_SaveItemInventory(AActor* ItemActor, int Hand);
    UFUNCTION(NetMulticast, Reliable)
    void MULTI_SaveItemInventory(AActor* ItemActor, int Hand);
    /* 
    Hand = 0 => void
    Hand = 1 => _ItemLeft
    Hand = 2 => _ItemRight
    */

    AActor* GetItemFocused();

    /* RAYCASTING  */
    UFUNCTION(BlueprintCallable, Category = "Raycasting")
    FHitResult Raycasting();

private:
    float _RayParameter;
    FHitResult _HitResult;
    
    UStaticMeshComponent* _LastMeshFocused = nullptr;
    bool bInventoryItemHit = false;

public:
    FORCEINLINE UInventory* AFPCharacter::GetInventory() const { return _Inventory; }
};
