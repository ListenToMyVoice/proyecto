// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"


UCLASS()
class LTMV_API APlayerCharacter : public ACharacter {
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    float _baseTurnRate;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    float _baseLookUpRate;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Life")
    int _Health;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Life")
	float _DamageDisappearVelocity;

	void Tick(float DeltaSeconds) override;

    /* Radio Delegate */
    FRadioDelegate _OnRadioPressedDelegate;
    FRadioDelegate _OnRadioReleasedDelegate;


    APlayerCharacter(const FObjectInitializer& OI);
    virtual void AfterPossessed(bool SetInventory , bool respawning);

    /******** USE ITEM LEFT *********/
    virtual void UseLeftPressed(bool IsMenuHidden);
    virtual void UseLeftReleased(bool IsMenuHidden);

    /******* USE ITEM RIGHT *********/
    virtual void UseRightPressed(bool IsMenuHidden);
    virtual void UseRightReleased(bool IsMenuHidden);

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                             class AController* EventInstigator, class AActor* DamageCauser) override;

	/********** TAKE ITEM ***********/
	virtual void TakeDropRight_Respawn(AActor* actor);

    /*********** MOVEMENT ***********/
    virtual void TurnAtRate(float Rate);
    virtual void LookUpAtRate(float Rate);

    AActor* GetWalkieActor();
    bool IsWalkieInHand();
    class UFMODEvent* GetWalkieEvent();

    virtual void ToggleMenuInteraction(bool Activate);

	UFUNCTION(BlueprintCallable, Category = "Hands")
	FORCEINLINE AActor* GetItemLeft() { return _ItemLeft; }
	UFUNCTION(BlueprintCallable, Category = "Hands")
	FORCEINLINE AActor* GetItemRight() { return _ItemRight; }

	/********** DROP ITEM ***********/
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void SERVER_Drop(AActor* ItemActor, int Hand);
	UFUNCTION(NetMulticast, Reliable)
		virtual void MULTI_Drop(AActor* ItemActor, int Hand);
protected:
    UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* _PlayerCamera;
    UPROPERTY(Category = Audio, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UFMODAudioComponent* _StepsAudioComp;
	UPROPERTY(Category = Audio, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UFMODAudioComponent* _BreathAudioComp;
    UPROPERTY(Category = Audio, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UMenuInteraction* _MenuInteractionComp;

    AActor* _ItemLeft;
    AActor* _ItemRight;

    void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

    /*************************************** ACTION MAPPINGS *************************************/
    /*********** MOVEMENT ***********/
    virtual void MoveForward(float Val);
    virtual void MoveRight(float Val);

    /************** USE *************/
    UFUNCTION(Server, Reliable, WithValidation)
    void SERVER_UsePressed(UActorComponent* component);
    UFUNCTION(NetMulticast, Reliable)
    void MULTI_UsePressed(UActorComponent* component);

    virtual void UseReleased();
    UFUNCTION(Server, Reliable, WithValidation)
    void SERVER_UseReleased(UActorComponent* component);
    UFUNCTION(NetMulticast, Reliable)
    void MULTI_UseReleased(UActorComponent* component);


    /********** TAKE ITEM ***********/
    UFUNCTION(Server, Reliable, WithValidation)
    virtual void SERVER_TakeRight(AActor* Actor);
    UFUNCTION(NetMulticast, Reliable)
    virtual void MULTI_TakeRight(AActor* Actor);
    UFUNCTION(Server, Reliable, WithValidation)
    virtual void SERVER_TakeLeft(AActor* Actor);
    UFUNCTION(NetMulticast, Reliable)
    virtual void MULTI_TakeLeft(AActor* Actor);

    
    /*
    Hand = 0 => void
    Hand = 1 => _ItemLeft
    Hand = 2 => _ItemRight
    */

    UFUNCTION(NetMulticast, Reliable)
    void MULTI_CharacterDead();
    UFUNCTION(Client, Reliable)
    void CLIENT_AddRadioDelegates(AActor* Actor);
    UFUNCTION(Client, Reliable)
    void CLIENT_ClearRadioDelegates(AActor* Actor);

	UFUNCTION()
	void OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	/***********POST PROCESS***********/
	bool _Damaged;

private:
    AActor* _WalkieActor;

    /* Radio Delegate */
    FDelegateHandle _OnRadioPressedDelegateHandle;
    FDelegateHandle _OnRadioReleasedDelegateHandle;

	/*Physic Materials*/
	const USkeletalMeshSocket* _FootSocket;
	FHitResult _FootHitResult;
public:
	void CheckFloorMaterial();
	
    FORCEINLINE UCameraComponent* APlayerCharacter::GetPlayerCamera() const { return _PlayerCamera; }
    FORCEINLINE UFMODAudioComponent* APlayerCharacter::GetStepsAudioComp() const { return _StepsAudioComp; }
	FORCEINLINE UFMODAudioComponent* APlayerCharacter::GetBreathAudioComp() const { return _BreathAudioComp; }
    FORCEINLINE UMenuInteraction* APlayerCharacter::GetMenuInteractionComp() const { return _MenuInteractionComp; }
};