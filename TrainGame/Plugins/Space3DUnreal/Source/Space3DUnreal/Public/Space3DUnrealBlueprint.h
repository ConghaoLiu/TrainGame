#pragma once

#define MAX_PERFSTRINGSIZE 2000

#include "CoreMinimal.h"
#include "Space3DUnreal.h"
#include "Space3DUnrealOutput.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Space3DUnrealBlueprint.generated.h"

UCLASS()
class SPACE3DUNREAL_API UpdateParams : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    public:
    UFUNCTION(BlueprintCallable, Category = "Custom", meta = (keywords = "UpdateOrder"))
        static void UpdateOrder(int order);
    UFUNCTION(BlueprintCallable, Category = "Custom", meta = (keywords = "UpdateRTRays"))
        static void UpdateRTRays(int rt_rays);
    UFUNCTION(BlueprintCallable, Category = "Custom", meta = (keywords = "UpdateRTBranches"))
        static void UpdateRTBranches(int rt_branches);
    UFUNCTION(BlueprintCallable, Category = "Custom", meta = (keywords = "UpdateDiffNAngles"))
        static void UpdateDiffNAngles(int n_angles);
    UFUNCTION(BlueprintCallable, Category = "Custom", meta = (keywords = "UpdateDefGamma"))
        static void UpdateDefGamma(float gamma);
    UFUNCTION(BlueprintCallable, Category = "Custom", meta = (keywords = "UpdateSSNRDEnabled"))
        static void UpdateSSNRDEnabled(bool enabled);
};

UCLASS()
class SPACE3DUNREAL_API UViewerBP : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    public:
    UFUNCTION(BlueprintCallable, Category = "Custom", meta = (keywords = "EnableViewer"))
        static void EnableViewer(bool enable);
    UFUNCTION(BlueprintCallable, Category = "Custom", meta = (keywords = "Space3DPerformance"))
        static FString GetPerformance();
};

