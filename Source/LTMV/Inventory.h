// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/StaticMeshComponent.h"
#include "Inventory.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LTMV_API UInventory : public UStaticMeshComponent {
    GENERATED_BODY()
private:
    TArray<AActor*> _items;

public:
    UInventory();
    virtual void BeginPlay() override;

    void AddItem(AActor* item);
    void RemoveItem(AActor* item);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    TArray<AActor*> GetItemsArray();

    FString GetFirstItem();

    //UFUNCTION(BlueprintNativeEvent, Category = "Inventory", BlueprintCallable)
    //void OnItemAdd();

    //void OnItemAdd_Implementation();

    void ShowAllItems();
    //virtual void TickComponent(float DeltaTime, ELevelTick TickType,
    //                           FActorComponentTickFunction* ThisTickFunction) override;

    AActor* PickItem(FString ItemName);
};
