// Fill out your copyright notice in the Description page of Project Settings.

#include "LTMV.h"
#include "VRInventory.h"


// Sets default values
AVRInventory::AVRInventory()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    bIsVRInventoryHidden = true;

}

// Called when the game starts or when spawned
void AVRInventory::BeginPlay()
{
	Super::BeginPlay();

    SetActorHiddenInGame(bIsVRInventoryHidden);
}

// Called every frame
void AVRInventory::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AVRInventory::ToggleVRInventory(FVector Location, FRotator Rotation) {
    if (!bIsVRInventoryHidden) {
        SetActorHiddenInGame(!bIsVRInventoryHidden);
    }
    else {
        SetActorHiddenInGame(bIsVRInventoryHidden);
        SetActorLocationAndRotation(Location, Rotation);
    }
    bIsVRInventoryHidden = !bIsVRInventoryHidden;
}