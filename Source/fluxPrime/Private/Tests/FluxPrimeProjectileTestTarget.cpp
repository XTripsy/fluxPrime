#include "Tests/FluxPrimeProjectileTestTarget.h"

#include "Components/SphereComponent.h"
#include "Engine/DamageEvents.h"

AFluxPrimeProjectileTestTarget::AFluxPrimeProjectileTestTarget()
{
	PrimaryActorTick.bCanEverTick = false;
	SetCanBeDamaged(true);

	USphereComponent* Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(40.0f);
	Collision->SetCanEverAffectNavigation(false);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Collision->SetGenerateOverlapEvents(false);
	RootComponent = Collision;
}

float AFluxPrimeProjectileTestTarget::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	TotalDamage += DamageAmount;
	++DamageEventCount;
	return DamageAmount;
}
