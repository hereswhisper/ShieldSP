#pragma once
#include "SDK.hpp"

using namespace SDK;
using namespace SDK;

namespace Globals {
	UFortEngine* Engine;
	UWorld* World;
	APlayerPawn_Athena_C* Character;
	APlayerController* Controller;
	AFortPlayerControllerAthena* GameController;
	UGameplayStatics* GameplayStatics;

	static auto BaseTransform = FTransform{};
}