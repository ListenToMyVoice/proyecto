// Fill out your copyright notice in the Description page of Project Settings.

#include "proyecto.h"
#include "EnemyCharacter.h"

#include "EnemyController.h"

AEnemyCharacter::AEnemyCharacter(const FObjectInitializer& OI) : Super(OI) {
    PrimaryActorTick.bCanEverTick = true;

    AIControllerClass = AEnemyController::StaticClass();
}

void AEnemyCharacter::BeginPlay() {
    Super::BeginPlay();
}

void AEnemyCharacter::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
}