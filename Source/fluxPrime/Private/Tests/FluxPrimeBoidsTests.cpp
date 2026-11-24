#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Generics/Systems/FluxPrimeBoidsSystem.h"
#include "HAL/PlatformTime.h"
#include "NativeGameplayTags.h"
#include "UObject/UnrealType.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_BoidsLocation, "FluxPrime.Tests.Boids.Location");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_BoidsAcceleration, "FluxPrime.Tests.Boids.Acceleration");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_BoidsSize, "FluxPrime.Tests.Boids.Size");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_BoidsCell, "FluxPrime.Tests.Boids.Cell");

namespace
{
struct FBoidsFixture
{
	TArray<FFluxPrimeArchetypeDataStore> Stores;
	FFluxPrimeBoidsSystem System;

	bool Initialize(int32 CountPerStore, float Weight = 2.0f)
	{
		for (int32 StoreIndex = 0; StoreIndex < 2; ++StoreIndex)
		{
			auto& Store = Stores.AddDefaulted_GetRef();
			if (!Store.AddBuffer(TAG_BoidsLocation, TBaseStructure<FVector>::Get(), sizeof(FVector), CountPerStore)
				|| !Store.AddBuffer(TAG_BoidsAcceleration, TBaseStructure<FVector>::Get(), sizeof(FVector), CountPerStore)
				|| !Store.AddBuffer(TAG_BoidsSize, FFluxPrimeWrapperInt16::StaticStruct(), sizeof(FFluxPrimeWrapperInt16), CountPerStore)
				|| !Store.AddBuffer(TAG_BoidsCell, FFluxPrimeWrapperInt32::StaticStruct(), sizeof(FFluxPrimeWrapperInt32), CountPerStore)) return false;
			Store.SetTotalEntityCount(CountPerStore);
			Store.SetActiveEntityCount(CountPerStore);
		}
		auto* Config = FindFProperty<FStructProperty>(FFluxPrimeBoidsSystem::StaticStruct(), TEXT("DataTagConfig"))
			->ContainerPtrToValuePtr<FBoidsSystemDataTagConfig>(&System);
		Config->EntityDataLocationConfig.EntityDataTag = TAG_BoidsLocation;
		Config->EntityDataLocationConfig.ExpectedStructType = TBaseStructure<FVector>::Get();
		Config->EntityDataAccelerationConfig.EntityDataTag = TAG_BoidsAcceleration;
		Config->EntityDataAccelerationConfig.ExpectedStructType = TBaseStructure<FVector>::Get();
		Config->EntityDataSizeConfig.EntityDataTag = TAG_BoidsSize;
		Config->EntityDataSizeConfig.ExpectedStructType = FFluxPrimeWrapperInt16::StaticStruct();
		FindFProperty<FFloatProperty>(FFluxPrimeBoidsSystem::StaticStruct(), TEXT("SeparationWeight"))
			->SetPropertyValue_InContainer(&System, Weight);
		auto* Grid = FindFProperty<FStructProperty>(FFluxPrimeBoidsSystem::StaticStruct(), TEXT("SpatialGridSystem"))
			->ContainerPtrToValuePtr<FFluxPrimeSpatialGridSystem>(&System);
		auto* GridConfig = FindFProperty<FStructProperty>(FFluxPrimeSpatialGridSystem::StaticStruct(), TEXT("DataTagConfig"))
			->ContainerPtrToValuePtr<FSpatialGridSystemDataTagConfig>(Grid);
		GridConfig->EntityDataLocationConfig.EntityDataTag = TAG_BoidsLocation;
		GridConfig->EntityDataLocationConfig.ExpectedStructType = TBaseStructure<FVector>::Get();
		GridConfig->EntityDataCellIdConfig.EntityDataTag = TAG_BoidsCell;
		GridConfig->EntityDataCellIdConfig.ExpectedStructType = FFluxPrimeWrapperInt32::StaticStruct();
		System.Initialized(Stores);
		return System.IsAllDataValid;
	}

	template <typename T> T* Buffer(int32 Store, FGameplayTag Tag)
	{
		return reinterpret_cast<T*>(Stores[Store].GetRawBufferData(Stores[Store].GetDataID(Tag)));
	}

	void ResetAcceleration()
	{
		for (int32 S = 0; S < Stores.Num(); ++S)
			for (int32 I = 0; I < Stores[S].GetTotalEntityCount(); ++I)
				Buffer<FVector>(S, TAG_BoidsAcceleration)[I] = FVector(3.0, -7.0, 11.0);
	}
};

struct FReferenceMember
{
	int32 Store, Row, X, Y;
	FVector Position;
	float Diameter, RadiusSquared;
};

// Enumerates pairs without using the production grid, then reproduces the original
// cell/row accumulation order and its double-source, float-neighbor position arithmetic.
TArray<TArray<FVector>> Reference(FBoidsFixture& Fixture, float Weight)
{
	TArray<TArray<FVector>> Result;
	TArray<FReferenceMember> Members;
	for (int32 S = 0; S < Fixture.Stores.Num(); ++S)
	{
		auto& Values = Result.AddDefaulted_GetRef();
		Values.Append(Fixture.Buffer<FVector>(S, TAG_BoidsAcceleration), Fixture.Stores[S].GetTotalEntityCount());
		for (int32 I = 0; I < Fixture.Stores[S].GetActiveEntityCount(); ++I)
		{
			const FVector P = Fixture.Buffer<FVector>(S, TAG_BoidsLocation)[I];
			const float D = Fixture.Buffer<FFluxPrimeWrapperInt16>(S, TAG_BoidsSize)[I].ValueInt16 * 1.2f;
			Members.Add({S, I, static_cast<int32>(FMath::Clamp(FMath::FloorToInt((P.X + 10000.0) / 100.0), 0, 199)),
				static_cast<int32>(FMath::Clamp(FMath::FloorToInt((P.Y + 10000.0) / 100.0), 0, 199)), P, D, FMath::Max(1.0f, D * D)});
		}
	}
	for (int32 A = 0; A < Members.Num(); ++A)
	{
		const auto& Self = Members[A];
		TArray<int32> Candidates;
		for (int32 B = 0; B < Members.Num(); ++B)
		{
			const int32 DX = Members[B].X - Self.X, DY = Members[B].Y - Self.Y;
			if ((DX == 0 && DY == 0 && B > A) || (DY == 0 && DX == 1)
				|| (DY == 1 && FMath::Abs(DX) <= 1)) Candidates.Add(B);
		}
		Candidates.Sort([&](int32 AIndex, int32 BIndex)
		{
			const int32 ACell = Members[AIndex].Y * 200 + Members[AIndex].X;
			const int32 BCell = Members[BIndex].Y * 200 + Members[BIndex].X;
			return ACell == BCell ? AIndex < BIndex : ACell < BCell;
		});
		FVector Force = FVector::ZeroVector;
		for (int32 B : Candidates)
		{
			const auto& Other = Members[B];
			const float DX = Self.Position.X - static_cast<float>(Other.Position.X);
			const float DY = Self.Position.Y - static_cast<float>(Other.Position.Y);
			const float D2 = DX * DX + DY * DY;
			if (D2 <= 0.1f || (D2 >= Self.RadiusSquared && D2 >= Other.RadiusSquared)) continue;
			const float Distance = FMath::Sqrt(D2);
			if (D2 < Self.RadiusSquared)
			{
				const float Scale = (1.0f - Distance / Self.Diameter) / Distance;
				Force.X += DX * Scale;
				Force.Y += DY * Scale;
			}
			if (D2 < Other.RadiusSquared)
			{
				const float Scale = Weight * ((1.0f - Distance / Other.Diameter) / Distance);
				Result[Other.Store][Other.Row].X -= DX * Scale;
				Result[Other.Store][Other.Row].Y -= DY * Scale;
			}
		}
		Result[Self.Store][Self.Row] += Force * Weight;
	}
	return Result;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeBoidsParityTest, "FluxPrime.Critical.BoidsParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeBoidsParityTest::RunTest(const FString& Parameters)
{
	for (int32 Scenario = 0; Scenario < 6; ++Scenario)
	{
		FBoidsFixture Fixture;
		const float Weight = Scenario == 5 ? 0.0f : 2.0f;
		if (!TestTrue(TEXT("Fixture initializes"), Fixture.Initialize(64, Weight))) return false;
		for (int32 S = 0; S < 2; ++S)
			for (int32 I = 0; I < 64; ++I)
			{
				const int32 N = S * 64 + I;
				FVector P((N % 16) * 5.0 + 0.1234567, (N / 16) * 6.0 + 0.9876543, N * 10.0);
				if (Scenario == 1) P = FVector((N % 16) * 75.0 - 600.0, (N / 16) * 75.0 - 300.0, 0.0);
				if (Scenario == 2) P += FVector(N % 2 ? 10000.0 : -10050.0, N % 3 ? 10000.0 : -10050.0, 0.0);
				if (Scenario == 3) P = FVector(10000000.1234567 + N * 0.3, 10000000.9876543 + (N % 4) * 0.31, 0.0);
				if (N % 11 == 0 && Scenario != 3) P = FVector::ZeroVector;
				Fixture.Buffer<FVector>(S, TAG_BoidsLocation)[I] = P;
				Fixture.Buffer<FFluxPrimeWrapperInt16>(S, TAG_BoidsSize)[I].ValueInt16 = 8 + N % 75;
			}
		for (int32 Pass = 0; Pass < 3; ++Pass)
		{
			if (Pass == 1)
			{
				Fixture.Stores[0].SetActiveEntityCount(9);
				Fixture.Stores[1].SetActiveEntityCount(17);
				Fixture.Buffer<FVector>(0, TAG_BoidsLocation)[0] = FVector(-9999.9, 9999.8, 40.0);
			}
			if (Pass == 2)
			{
				Fixture.Stores[0].SetActiveEntityCount(0);
				Fixture.Stores[1].SetActiveEntityCount(0);
			}
			Fixture.ResetAcceleration();
			const auto Expected = Reference(Fixture, Weight);
			Fixture.System.ExecuteSystem(1.0f / 60.0f, Fixture.Stores);
			for (int32 S = 0; S < 2; ++S)
				for (int32 I = 0; I < 64; ++I)
				{
					const FVector Actual = Fixture.Buffer<FVector>(S, TAG_BoidsAcceleration)[I];
					if (!TestTrue(FString::Printf(TEXT("Scenario %d pass %d store %d row %d: %s vs %s"),
						Scenario, Pass, S, I, *Actual.ToString(), *Expected[S][I].ToString()), Actual.Equals(Expected[S][I], 1e-4))) return false;
					TestEqual(TEXT("Z is unchanged"), Actual.Z, 11.0);
				}
		}
	}

	FBoidsFixture Pair;
	if (!TestTrue(TEXT("Pair initializes"), Pair.Initialize(1))) return false;
	Pair.Buffer<FVector>(0, TAG_BoidsLocation)[0] = FVector(0, 0, 50);
	Pair.Buffer<FVector>(1, TAG_BoidsLocation)[0] = FVector(30, 0, -50);
	Pair.Buffer<FFluxPrimeWrapperInt16>(0, TAG_BoidsSize)[0].ValueInt16 = 50;
	Pair.Buffer<FFluxPrimeWrapperInt16>(1, TAG_BoidsSize)[0].ValueInt16 = 25;
	Pair.ResetAcceleration();
	Pair.System.ExecuteSystem(1.0f / 60.0f, Pair.Stores);
	TestTrue(TEXT("Radius 60 pushes by -1 at distance 30"), Pair.Buffer<FVector>(0, TAG_BoidsAcceleration)[0].Equals(FVector(2, -7, 11), 1e-6));
	TestTrue(TEXT("Radius 30 excludes the exact boundary"), Pair.Buffer<FVector>(1, TAG_BoidsAcceleration)[0].Equals(FVector(3, -7, 11), 1e-6));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeBoidsPerformanceTest, "FluxPrime.Performance.Boids",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeBoidsPerformanceTest::RunTest(const FString& Parameters)
{
	for (int32 Dense = 0; Dense < 2; ++Dense)
	{
		FBoidsFixture Fixture;
		if (!TestTrue(TEXT("Benchmark initializes"), Fixture.Initialize(500))) return false;
		for (int32 S = 0; S < 2; ++S)
			for (int32 I = 0; I < 500; ++I)
			{
				const int32 N = S * 500 + I;
				const double Spacing = Dense ? 2.5 : 25.0;
				Fixture.Buffer<FVector>(S, TAG_BoidsLocation)[I] = FVector((N % 32) * Spacing + 0.125, (N / 32) * Spacing + 0.375, 0);
				Fixture.Buffer<FFluxPrimeWrapperInt16>(S, TAG_BoidsSize)[I].ValueInt16 = 30 + N % 31;
			}
		TArray<double> Samples;
		for (int32 Iteration = -100; Iteration < 1000; ++Iteration)
		{
			Fixture.ResetAcceleration();
			const double Start = FPlatformTime::Seconds();
			Fixture.System.ExecuteSystem(1.0f / 60.0f, Fixture.Stores);
			const double Milliseconds = (FPlatformTime::Seconds() - Start) * 1000.0;
			if (Iteration >= 0) Samples.Add(Milliseconds);
		}
		double Sum = 0;
		for (double Sample : Samples) Sum += Sample;
		Samples.Sort();
		AddInfo(FString::Printf(TEXT("BoidsBenchmark %s N=%d mean=%.6f p95=%.6f max=%.6f ms"),
			Dense ? TEXT("dense") : TEXT("distributed"), Samples.Num(), Sum / Samples.Num(), Samples[949], Samples.Last()));
	}
	return true;
}
#endif
