// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RWTxt.generated.h"

/**
 * 
 */
UCLASS()
class TRAINGAME_API URWTxt : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	public:
		UFUNCTION(BlueprintPure, Category = "Custom", meta = (keywords = "ReadTxt"))
			static bool ReadTxt(FString FilePath, FString& Text);
		UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "WriteTxt"))
			static bool WriteTxt(FString FilePath, FString Text);
};
