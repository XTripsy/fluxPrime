#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Domains/Crowds/Niagara/FluxPrimeNiagaraDataInterfaceProjectileExport.h"
#include "NiagaraTypeRegistry.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeProjectileExportBindingTest,
	"FluxPrime.Medium.ProjectileBatch.ExportBinding",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeProjectileExportBindingTest::RunTest(const FString& Parameters)
{
	UNiagaraDataInterfaceProjectileExport* Export = NewObject<UNiagaraDataInterfaceProjectileExport>();
	TestEqual(TEXT("Default callback binding matches the subsystem"),
		Export->CallbackHandlerParameter.Parameter.GetName(), FName(TEXT("User.ProjectileCallback")));
	TestTrue(TEXT("Export DI is available to Niagara graph parameters"),
		FNiagaraTypeRegistry::GetRegisteredTypes().Contains(FNiagaraTypeDefinition(Export->GetClass())));
	TestTrue(TEXT("CPU collision exports are supported"), Export->CanExecuteOnTarget(ENiagaraSimTarget::CPUSim));
	TestFalse(TEXT("GPU collision cannot silently replace CPU collision"), Export->CanExecuteOnTarget(ENiagaraSimTarget::GPUComputeSim));
	UNiagaraDataInterfaceProjectileExport* Copy = DuplicateObject<UNiagaraDataInterfaceProjectileExport>(Export, GetTransientPackage());
	TestTrue(TEXT("Duplicated DI retains callback configuration"), Export->Equals(Copy));
	Copy->CallbackHandlerParameter.Parameter.SetName(TEXT("User.OtherCallback"));
	TestFalse(TEXT("Different callback bindings are not equivalent"), Export->Equals(Copy));
	return true;
}
#endif
