// Fill out your copyright notice in the Description page of Project Settings.

#include "proyecto.h"
#include "itemAttachable.h"

UItemAttachable::UItemAttachable() : Super(), _locationAttach(0.f, 0.f, 0.f),
                                              _rotationAttach(0.f, 0.f, 0.f) {}

void UItemAttachable::BeginPlay() {
    Super::BeginPlay();
}

void UItemAttachable::activateItem(UPrimitiveComponent* OverlappedComp,
                                   APlayerCharacter* OtherActor,
                                   UPrimitiveComponent* OtherComp,
                                   int32 OtherBodyIndex, bool bFromSweep,
                                   const FHitResult& SweepResult) {

    Super::activateItem(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex,
                        bFromSweep, SweepResult);
    
    ULibraryUtils::Log(TEXT("Attachable active"));
    _binding = &OtherActor->InputComponent->BindAction("Take", IE_Released, this,
                                                       &UItemAttachable::inputCB);
}

void UItemAttachable::deactivateItem(UPrimitiveComponent* OverlappedComp,
                                     APlayerCharacter* OtherActor, UPrimitiveComponent* OtherComp,
                                     int32 OtherBodyIndex) {

    Super::deactivateItem(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex);

    ULibraryUtils::Log(TEXT("Attachable deactive"));
    _binding->ActionDelegate.Unbind();
}

void UItemAttachable::inputCB() {
    AStaticMeshActor* owner = Cast<AStaticMeshActor>(GetOwner());
    if (owner != nullptr) {
        _actor->TakeItem(owner, _locationAttach, _rotationAttach);
    }
}