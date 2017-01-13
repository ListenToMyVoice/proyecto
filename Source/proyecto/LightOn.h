// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ItemOverlap.h"
#include "ItfUsable.h"

#include "LightOn.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROYECTO_API ULightOn : public UItemOverlap, public IItfUsable {
    GENERATED_BODY()

public:
    ULightOn();
    virtual void BeginPlay() override;

    virtual void activateItem(UPrimitiveComponent* OverlappedComp,
                              APlayerCharacter* player,
                              UPrimitiveComponent* OtherComp,
                              int32 OtherBodyIndex, bool bFromSweep,
                              const FHitResult& SweepResult);

    virtual void deactivateItem(UPrimitiveComponent* OverlappedComp, APlayerCharacter* player,
                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
    int Use();
    virtual int Use_Implementation() override;
};
