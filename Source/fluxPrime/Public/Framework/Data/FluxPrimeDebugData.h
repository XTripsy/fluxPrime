#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeDebugData.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeDebugLimits
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget", meta = (ClampMin = "0"))
	float MaxCameraDistance = 5000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget", meta = (ClampMin = "0"))
	int32 MaxScannedRows = 512;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget", meta = (ClampMin = "0"))
	int32 MaxLabels = 256;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget", meta = (ClampMin = "0"))
	int32 MaxPrimitives = 2048;
};

enum class EFluxPrimeDebugShapeKind : uint8 { Line, Arrow, Circle, Box, Point };

struct FFluxPrimeDebugShape
{
	EFluxPrimeDebugShapeKind Kind = EFluxPrimeDebugShapeKind::Line;
	FVector Start = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;
	FVector Center = FVector::ZeroVector;
	FVector Extent = FVector::ZeroVector;
	float Radius = 0.0f;
	FColor Color = FColor::White;
	float Thickness = 1.0f;
};

struct FFluxPrimeDebugLabel
{
	FVector Location = FVector::ZeroVector;
	FString Text;
	FColor Color = FColor::White;
	float FontScale = 0.8f;
};

struct FFluxPrimeDebugArchetypeMeta
{
	FName Name;
	int32 MaxHealth = 0;
};

struct FFluxPrimeDebugRowRef
{
	int32 Archetype = INDEX_NONE;
	int32 Row = INDEX_NONE;
	double DistanceSq = 0.0;
	FVector Location = FVector::ZeroVector;
};

struct FFluxPrimeDebugFrame
{
	TArray<FFluxPrimeDebugLabel> Labels;
	TArray<FFluxPrimeDebugShape> Shapes;
	int32 ScannedRows = 0;
	bool bScanTruncated = false;
	bool bLabelsTruncated = false;
	bool bPrimitivesTruncated = false;
	FFluxPrimeDebugLimits Limits;
	float LabelBaseZ = 180.0f;
	float LabelStepZ = 24.0f;
	TMap<uint64, int32> LabelStacks;
};

inline bool FluxPrimeAddDebugShape(FFluxPrimeDebugFrame& Frame, FFluxPrimeDebugShape Shape)
{
#if !UE_BUILD_SHIPPING
	if (Frame.Shapes.Num() >= FMath::Max(Frame.Limits.MaxPrimitives, 0))
	{
		Frame.bPrimitivesTruncated = true;
		return false;
	}
	Frame.Shapes.Add(MoveTemp(Shape));
	return true;
#else
	return false;
#endif
}

inline void FluxPrimeAddDebugLabel(FFluxPrimeDebugFrame& Frame, int32 Archetype, int32 Row,
	const FVector& EntityLocation, const FString& Text, const FColor& Color, float FontScale)
{
#if !UE_BUILD_SHIPPING
	if (Frame.Labels.Num() >= FMath::Max(Frame.Limits.MaxLabels, 0))
	{
		Frame.bLabelsTruncated = true;
		return;
	}
	const uint64 Key = (static_cast<uint64>(static_cast<uint32>(Archetype)) << 32) | static_cast<uint32>(Row);
	int32& Stack = Frame.LabelStacks.FindOrAdd(Key);
	FFluxPrimeDebugLabel& Label = Frame.Labels.AddDefaulted_GetRef();
	Label.Location = EntityLocation + FVector(0, 0, Frame.LabelBaseZ + Stack * Frame.LabelStepZ);
	Label.Text = Text;
	Label.Color = Color;
	Label.FontScale = FontScale;
	++Stack;
#endif
}


