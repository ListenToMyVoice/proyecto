#pragma once

#include "ItfUsable.generated.h"

UINTERFACE(Blueprintable)
class PROYECTO_API UItfUsable : public UInterface {
    GENERATED_UINTERFACE_BODY()
};

class PROYECTO_API IItfUsable {
    GENERATED_IINTERFACE_BODY()

public:

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Usable")
    void Use();


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pressable")
		void Press();

};