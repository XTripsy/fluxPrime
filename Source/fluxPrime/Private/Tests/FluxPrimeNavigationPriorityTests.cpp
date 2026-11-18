#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Algo/Sort.h"
#include "Generics/Systems/Navigation/FluxPrimeNavigationSystem.h"

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

	TestEqual(TEXT("An empty queue selects nothing"),
		SelectHighestPriorityEntries(nullptr, 0, 5), 0);

	const int32 queueCapacity = 1000;
	TArray<FFluxPrimeNavigationQueueEntry> queue;
	queue.SetNum(queueCapacity);

	// Ascending scores, shuffled by a fixed stride so the selection cannot win by index order.
	for (int32 index = 0; index < queueCapacity; ++index)
	{
		const int32 shuffled = (index * 37) % queueCapacity;
		queue[index].PriorityScore = static_cast<float>(shuffled) * 10.0f;
		queue[index].InternalIndex = shuffled;
	}

	TArray<FFluxPrimeNavigationQueueEntry> reference = queue;
	Algo::Sort(reference);

	const int32 selectedCount = SelectHighestPriorityEntries(queue.GetData(), queue.Num(), 5);
	TestEqual(TEXT("Selection returns the per-frame limit"), selectedCount, 5);
	TestEqual(TEXT("Selection keeps the highest score first"), queue[0].InternalIndex, 999);
	TestEqual(TEXT("Selection matches a full sort"), queue[4].InternalIndex, reference[4].InternalIndex);

	for (int32 index = 1; index < selectedCount; ++index)
	{
		TestTrue(TEXT("Selection is ordered by priority"), queue[index - 1].PriorityScore >= queue[index].PriorityScore);
	}

	const float weakestSelected = queue[selectedCount - 1].PriorityScore;
	bool everyLoserIsWeaker = true;

	for (int32 index = selectedCount; index < queueCapacity; ++index)
	{
		if (queue[index].PriorityScore > weakestSelected)
		{
			everyLoserIsWeaker = false;
			break;
		}
	}

	TestTrue(TEXT("Nothing outside the selection outranks it"), everyLoserIsWeaker);

	for (int32 index = 0; index < queueCapacity; ++index)
	{
		queue[index].PriorityScore = 1.0f;
	}

	const int32 tiedCount = SelectHighestPriorityEntries(queue.GetData(), queue.Num(), 20);
	TestEqual(TEXT("Ties still fill the limit"), tiedCount, 20);
	bool tiedOrderIsFlat = true;

	for (int32 index = 1; index < tiedCount; ++index)
	{
		tiedOrderIsFlat = tiedOrderIsFlat && queue[index - 1].PriorityScore == queue[index].PriorityScore;
	}

	TestTrue(TEXT("Ties keep a flat order"), tiedOrderIsFlat);

	const int32 underLimitCount = SelectHighestPriorityEntries(queue.GetData(), 3, 5);
	TestEqual(TEXT("A queue under the limit is fully processed"), underLimitCount, 3);
	TestEqual(TEXT("A zero limit processes nothing"), SelectHighestPriorityEntries(queue.GetData(), queue.Num(), 0), 0);
	TestEqual(TEXT("A negative limit processes nothing"), SelectHighestPriorityEntries(queue.GetData(), queue.Num(), -4), 0);

	return true;
}
#endif
