// Fill out your copyright notice in the Description page of Project Settings.

#include "LTMV.h"
#include "Inventory.h"

#include "InventoryItem.h"

UInventory::UInventory() : Super() {
    _items = {};
    bReplicates = true;
    SetIsReplicated(true);
}

void UInventory::BeginPlay() {
    Super::BeginPlay();
}

void UInventory::AddItem(AActor* item) {
    ULibraryUtils::SetActorEnable(item, false);

    UStaticMeshComponent* itemMesh = Cast<UStaticMeshComponent>(item->GetComponentByClass(
        UStaticMeshComponent::StaticClass()));

    itemMesh->AttachToComponent(this,
        FAttachmentTransformRules::KeepRelativeTransform);

    itemMesh->RelativeLocation = FVector(0.0f, 0.0f, 0.0f);
    itemMesh->RelativeRotation = FRotator(0.0f, 0.0f, 0.0f);

    _items.Add(item);

}

void UInventory::RemoveItem(AActor* itemToRemove) {

    for (AActor* item : _items) {
        if (itemToRemove == item) {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("REMOVED ITEM: %s"), *item->GetName()));
            UE_LOG(LogTemp, Warning, TEXT("REMOVED ITEM: %s"), *item->GetName());

            _items.Remove(item);
        }
    }

}

FString UInventory::GetFirstItem() {
    //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("FIRST ITEM: %s"), *_items[0]->GetName()));
    if(_items.Num() > 0)
        return _items[0]->GetName();
    else return "NO ITEM";
}

TArray<AActor*> UInventory::GetItemsArray() {
    return _items;
}

void UInventory::ShowAllItems() {
    for (AActor* item:_items){
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("ITEM: %s"), *item->GetName()));
        UE_LOG(LogTemp, Warning, TEXT("ITEM: %s"), *item->GetName());

    }
}

AActor* UInventory::PickItem(FString ItemName) {
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("KEY PRESSED!!2")));


    AActor *ReturnItem = nullptr;
    UInventoryItem* ItemComp = nullptr;
    
    for (AActor* item : _items) {

        //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("ITEM: %s"), *ItemName));
        ItemComp = Cast<UInventoryItem>(item->FindComponentByClass(UInventoryItem::StaticClass()));
        //UInventoryItem* InventoryItemComp = item->FindComponentByClass(UInventoryItem::StaticClass());

        if (item->GetName().Equals(ItemName) && ItemComp->IsEquipped() == false) {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Picking item: %s from inventory"), *item->GetName()));
            UE_LOG(LogTemp, Warning, TEXT("Picking item: %s from inventory"), *item->GetName());

            item->SetReplicates(true);
            item->SetReplicateMovement(true);

            ReturnItem = item;

            ReturnItem->SetReplicates(true);
            ReturnItem->SetReplicateMovement(true);

            return ReturnItem;
        }
    }

    return ReturnItem;
}

