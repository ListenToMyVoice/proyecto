// Fill out your copyright notice in the Description page of Project Settings.

#include "LTMV.h"
#include "Menu3D.h"

#include "MenuPanel.h"
#include "InputMenu.h"


AMenu3D::AMenu3D(const class FObjectInitializer& OI) : Super(OI) {
    PrimaryActorTick.bCanEverTick = true;
    //SetActorRotation(FRotator(0, 180, 0));
    AddActorWorldRotation(FRotator(0, 180, 0));
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root Component")));

    /*** DECORATORS ***/
    _TopDecorator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("_TopDecorator"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(
        TEXT("StaticMesh'/Game/Art/Common/Menu/Meshes/menu2_parte_superior.menu2_parte_superior'"));
    _TopDecorator->SetStaticMesh(Finder.Object);

    //_BottomDecorator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("_BottomDecorator"));
    //static ConstructorHelpers::FObjectFinder<UStaticMesh> Finder2(
    //    TEXT("StaticMesh'/Game/Art/Common/Menu/Meshes/menu2_parte_abajo.menu2_parte_abajo'"));
    //_BottomDecorator->SetStaticMesh(Finder2.Object);

    //_MiddleDecorator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("_MiddleDecorator"));
    //static ConstructorHelpers::FObjectFinder<UStaticMesh> Finder3(
    //    TEXT("StaticMesh'/Game/Art/Common/Menu/Meshes/menu2_parte_intermedia.menu2_parte_intermedia'"));
    //_MiddleDecorator->SetStaticMesh(Finder3.Object);
    
    _BackSubmenu = CreateDefaultSubobject<UInputMenu>(TEXT("BACK"));
    _BackSubmenu->_InputMenuReleasedDelegate.BindUObject(this, &AMenu3D::OnButtonBack);
    _BackSubmenu->AddOnInputMenuDelegate();

    _BackSubmenu->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    _TopDecorator->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    //_BottomDecorator->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    //_MiddleDecorator->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

    _IsMenuHidden = true;
    _Submenus = {};
    _Breadcrumb = {};
}

void AMenu3D::AddSubmenu(UMenuPanel* Submenu) {
    Submenu->RegisterComponent();
    Submenu->AttachToComponent(_TopDecorator, FAttachmentTransformRules::KeepRelativeTransform);
    _Submenus.Add(Submenu);
}

void AMenu3D::ToogleMenu(FVector Location, FRotator Rotation) {
    if (_IsMenuHidden) {
        Rotation.Yaw = 180;
        Rotation.Roll = 0;
        Location.X += 200;

        SetSubmenuByIndex(0);
        ULibraryUtils::SetActorEnable(this);

        SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::TeleportPhysics);
    }
    else {
        ULibraryUtils::SetActorEnable(this, false);
        SetSubmenuByIndex(-1);
    }
    _IsMenuHidden = !_IsMenuHidden;
}

void AMenu3D::SetSubmenuByIndex(const int& Index) {
    for (int i = 0; i < _Submenus.Num(); i++) {
        if (i == Index) {
            _Submenus[i]->EnablePanel(true);

            PlaceBackButton(!(Index == 0), _Submenus[i]->_PanelHeight);
            _Breadcrumb.Add(Index);
        }
        else {
            _Submenus[i]->EnablePanel(false);
        }
    }

    if (Index < 0) _Breadcrumb.Reset();
}

void AMenu3D::SetInputMenuLoading(int IndexPanel, int IndexInputMenu, bool IsLoading,
                                  FString Text) {
    if (IndexPanel < _Submenus.Num()) {
        UInputMenu* InputMenu = _Submenus[IndexPanel]->GetInputMenuAt(IndexInputMenu);
        if(InputMenu) InputMenu->SetLoading(IsLoading, Text);
    }
}

/*********************************** BINDINGS ****************************************************/
void AMenu3D::OnButtonBack(UInputMenu* InputMenu) {
    _Breadcrumb.RemoveAt(_Breadcrumb.Num() - 1);
    int Aux = _Breadcrumb.Top();
    _Breadcrumb.RemoveAt(_Breadcrumb.Num() - 1);

    SetSubmenuByIndex(Aux);
}

/*********************************** AUXILIAR ****************************************************/
void AMenu3D::PlaceBackButton(bool Place, float PanelHeight) {
    if (Place) {
        _BackSubmenu->Enable(true);
        _BackSubmenu->SetRelativeLocation(FVector(0, 0, -PanelHeight));
    }
    else {
        _BackSubmenu->Enable(false);
    }
}
