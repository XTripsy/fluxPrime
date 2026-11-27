#pragma once
#include "Components/ActorComponent.h"
#include "Framework/Configurations/FluxPrimeDebugConfiguration.h"
#include "Framework/Systems/Debug/FluxPrimeDebugPass.h"
#include "FluxPrimeDebugComponent.generated.h"
class UFluxPrimeBaseManager;
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FLUXPRIME_API UFluxPrimeDebugComponent : public UActorComponent
{
	GENERATED_BODY()
	
private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UFluxPrimeBaseManager> Manager;
	UPROPERTY(Transient)
	TArray<FInstancedStruct> RuntimeDebugSystems;
	TArray<FFluxPrimeDebugRuntimeEntry> RuntimeEntries;
	int32 InitializedArchetypeCount = INDEX_NONE;
	
public:
	UPROPERTY(EditAnywhere, Category="Debug")
	TObjectPtr<UFluxPrimeDebugConfiguration> DebugConfiguration;
	
	UPROPERTY(EditAnywhere, Category="Debug")
	bool bEnableDebug = false;
	
private:
	void RebuildRuntime();
	void RefreshTickEnabled();
	
public:
	UFluxPrimeDebugComponent();
	
	UFUNCTION(BlueprintCallable, Category="Debug")
	void SetDebugEnabled(bool bEnabled);
	
	UFUNCTION(BlueprintCallable, Category="Debug")
	bool SetSystemEnabled(int32 Index, bool bEnabled);
	
	void BindManager(UFluxPrimeBaseManager* InManager);
	int32 GetRuntimeSystemCount() const { return RuntimeEntries.Num(); }
	FString DescribeRuntimeSystems() const;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float Delta, ELevelTick TickType, FActorComponentTickFunction* TickFunction) override;
};
