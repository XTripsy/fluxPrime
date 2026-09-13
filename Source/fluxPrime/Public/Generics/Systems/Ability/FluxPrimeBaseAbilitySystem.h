#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "StructUtils/InstancedStruct.h"
#include "FluxPrimeBaseAbilitySystem.generated.h"

using FAbilityExecute = void(*)(FInstancedStruct&, FVector, FRotator, float);

USTRUCT()
struct FAbilityExecutor
{
	GENERATED_BODY()
	
	FInstancedStruct* Data = nullptr;
	FAbilityExecute Execute = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeBaseAbilitySystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, meta = (BaseStruct="FluxPrimeBaseAbility", ExcludeBaseStruct, AllowPrivateAccess = true))
	TArray<FInstancedStruct> AbilityFragments;
	
	UPROPERTY()
	TArray<FAbilityExecutor> AbilityExecutors;
	
protected:
	template<typename T>
	int32 RegisterAbility(FInstancedStruct& source)
	{
		FAbilityExecutor executor;
		executor.Data = &source;
		executor.Execute = &ExecuteAbility<T>;

		return AbilityExecutors.Add(MoveTemp(executor));
	}
	
	int32 RegisterEmptyAbility()
	{
		FAbilityExecutor Executor;
		Executor.Execute = &ExecuteEmptyAbility;

		return AbilityExecutors.Add(MoveTemp(Executor));
	}
	
	template<typename T>
	static void ExecuteAbility(FInstancedStruct& data, FVector location, FRotator rotation, float damage)
	{
		T& ability = data.GetMutable<T>();

		ability.Execute(location, rotation, damage);
	}
	
	static void ExecuteEmptyAbility(FInstancedStruct& Data, FVector Location, FRotator Rotation, float Damage)
	{
		UE_LOG(LogTemp, Log, TEXT("ABILITY SYSTEMS:: EMPTY ABILITY"));
	}
	
};
