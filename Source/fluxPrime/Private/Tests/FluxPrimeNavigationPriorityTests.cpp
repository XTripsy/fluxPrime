#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Algo/Sort.h"
#include "Generics/Systems/Navigation/FluxPrimeBaseNavigationSystem.h"

namespace
{
FFluxPrimeNavigationDataRequest MakeNavigationRequest(const float WaitSeconds, const int32 InternalIndex)
{
	FFluxPrimeNavigationDataRequest Request;
	Request.PriorityScore = ComputeNavigationPriority(WaitSeconds, 0.0f, 0.0f);
	Request.InternalIndex = InternalIndex;
	return Request;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeNavigationPriorityTest, "FluxPrime.High.NavigationPriority",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeNavigationPriorityTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Ten minutes of waiting stays representable"),
		ComputeNavigationPriority(FluxNavigationConfig::MaxWaitTime, 0.0f, 0.0f),
		FluxNavigationConfig::MaxWaitTime * FluxNavigationConfig::WaitTimeScoreScale);

	TArray<FFluxPrimeNavigationDataRequest> sample;
	sample.Add(MakeNavigationRequest(0.0f, 0));
	sample.Add(MakeNavigationRequest(5.0f, 1));
	sample.Add(MakeNavigationRequest(30.0f, 2));
	sample.Add(MakeNavigationRequest(600.0f, 3));

	Algo::Sort(sample);

	for (int32 index = 1; index < sample.Num(); ++index)
	{
		TestTrue(TEXT("Sort is monotonic in wait time"),
			sample[index - 1].PriorityScore >= sample[index].PriorityScore);
	}

	TestEqual(TEXT("Longest waiter is scheduled first"), sample[0].InternalIndex, 3);
	TestEqual(TEXT("Shortest waiter is scheduled last"), sample[3].InternalIndex, 0);

	TArray<FFluxPrimeNavigationDataRequest> thousand;

	for (int32 index = 0; index < 1000; ++index)
	{
		const float waitSeconds = static_cast<float>(index) * (FluxNavigationConfig::MaxWaitTime / 999.0f);
		thousand.Add(MakeNavigationRequest(waitSeconds, index));
	}

	Algo::Sort(thousand);

	bool monotonic = true;

	for (int32 index = 1; index < thousand.Num(); ++index)
	{
		if (thousand[index - 1].PriorityScore < thousand[index].PriorityScore)
		{
			monotonic = false;
			break;
		}
	}

	TestTrue(TEXT("A thousand requests stay monotonic"), monotonic);
	TestEqual(TEXT("The oldest request outranks every newer one"), thousand[0].InternalIndex, 999);

	TestEqual(TEXT("An enormous wait time clamps to the score ceiling"),
		ComputeNavigationPriority(1.0e9f, 0.0f, 0.0f), FluxNavigationConfig::MaxPriorityScore);
	TestEqual(TEXT("A negative score clamps to zero"),
		ComputeNavigationPriority(-1000.0f, 0.0f, 0.0f), 0.0f);

	TestEqual(TEXT("A single huge frame cannot exceed the wait bound"),
		AccumulateNavigationWaitTime(0.0f, 100000.0f), FluxNavigationConfig::MaxWaitTime);
	TestEqual(TEXT("Wait accumulation is bounded across many frames"),
		[&]()
		{
			float waitTime = 0.0f;

			for (int32 frame = 0; frame < 1000; ++frame)
			{
				waitTime = AccumulateNavigationWaitTime(waitTime, 1.0f);
			}

			return waitTime;
		}(),
		FluxNavigationConfig::MaxWaitTime);
	TestEqual(TEXT("Normal wait accumulation is unchanged"),
		AccumulateNavigationWaitTime(10.0f, 1.0f), 11.0f);

	return true;
}
#endif
