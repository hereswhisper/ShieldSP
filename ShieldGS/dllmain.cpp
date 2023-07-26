// ShieldGS.
// please fucking kill me...
#include <Windows.h>
#include "SDK.hpp"
#include "Globals.h"
#include "Memory.h"
#include "Helpers.h"
#include "MinHook.h"

using namespace SDK;

static void SetupOthers() {
    Globals::World = Globals::Engine->GameViewport->World;
    Globals::Controller = Globals::Engine->GameInstance->LocalPlayers[0]->PlayerController;
}

static SDK::AActor* SpawnActor(SDK::UClass* ActorClass, SDK::FVector Location, SDK::FRotator Rotation)
{
    SDK::FQuat Quat;
    SDK::FTransform Transform;
    Quat.W = 0;
    Quat.X = Rotation.Pitch;
    Quat.Y = Rotation.Roll;
    Quat.Z = Rotation.Yaw;

    Transform.Rotation = Quat;
    Transform.Scale3D = SDK::FVector{ 1,1,1 };
    Transform.Translation = Location;

    auto Actor = ((UGameplayStatics*)UGameplayStatics::StaticClass())->STATIC_BeginSpawningActorFromClass(Globals::Engine->GameViewport->World, ActorClass, Transform, false, nullptr);
    ((UGameplayStatics*)UGameplayStatics::StaticClass())->STATIC_FinishSpawningActor(Actor, Transform);
    return Actor;
}

static bool bHasStarted = false;

static bool bHasDroppedLS = false;

UGameplayStatics* GetGameplayStatics()
{
    return reinterpret_cast<UGameplayStatics*>(UGameplayStatics::StaticClass());
}

static void* ProcessEventHook(UObject* Object, UFunction* Function, void* Params) {
    if (!Object || !Function) {
        return ProcessEvent(Object, Function, Params);
    }

    //printf("Lolll");

    auto ObjName = Object->GetName();
    auto FunName = Function->GetName();

    if (FunName.find("BP_PlayButton") != std::string::npos) {
        Globals::Controller->SwitchLevel(L"Athena_Terrain");
    }

    if (FunName.find("ReadyToStartMatch") != std::string::npos) {
        if (bHasStarted == false) {
            bHasStarted = true;
            Globals::World = Globals::Engine->GameViewport->World;
            auto GameMode = reinterpret_cast<AFortGameModeAthena*>(Object);
            auto GameState = (AAthena_GameState_C*)GameMode->GameState;
            auto Controller = reinterpret_cast<AFortPlayerControllerAthena*>(Globals::Engine->GameInstance->LocalPlayers[0]->PlayerController);
            auto PlayerState = Controller->PlayerState;

            Globals::GameController = Controller;

            Globals::GameController->CheatManager = reinterpret_cast<UCheatManager*>(GetGameplayStatics()->STATIC_SpawnObject(UCheatManager::StaticClass(), Globals::GameController));

            FTransform SpawnPos = Globals::BaseTransform;

            SpawnPos.Translation = { 0,0,10000 };

            auto Pawn = GetGameplayStatics()->STATIC_BeginDeferredActorSpawnFromClass(Globals::World, APlayerPawn_Athena_C::StaticClass(), SpawnPos, ESpawnActorCollisionHandlingMethod::AlwaysSpawn, nullptr);
            Pawn = GetGameplayStatics()->STATIC_FinishSpawningActor(Pawn, SpawnPos);

            Globals::Character = (APlayerPawn_Athena_C*)Pawn;

            Globals::GameController->Possess(Globals::Character);

            Globals::GameController->CheatManager->God();
            Globals::GameController->ServerSetClientHasFinishedLoading(true);
            Globals::GameController->bHasServerFinishedLoading = true;
            Globals::GameController->OnRep_bHasServerFinishedLoading();

            UFortHero* Hero = Globals::GameController->StrongMyHero;

            for (int i = 0; i < Hero->CharacterParts.Num(); i++) {
                UCustomCharacterPart* Part = Hero->CharacterParts[i];
                ((APlayerPawn_Athena_C*)Pawn)->ServerChoosePart(Part->CharacterPartType, Part);
            }
            ((AFortPlayerStateAthena*)PlayerState)->OnRep_CharacterParts();

            UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_Playground.Playlist_Playground");

            GameState->CurrentPlaylistData = Playlist;
            GameState->CurrentPlaylistId = Playlist->PlaylistId;

            GameState->OnRep_CurrentPlaylistData();
            GameState->OnRep_CurrentPlaylistId();

            GameState->GamePhase = EAthenaGamePhase::Warmup;
            GameState->OnRep_GamePhase(EAthenaGamePhase::None);

            Globals::GameController->ServerReadyToStartMatch();

            Globals::GameController->CheatManager->DestroyAll(AFortHLODSMActor::StaticClass());
            
        }
    }

    if (FunName.find("ServerLoadingScreenDropped") != std::string::npos) {
        if (bHasDroppedLS == false) {
            bHasDroppedLS = true;

        }
    }

    return ProcessEvent(Object, Function, Params);
}

DWORD WINAPI Main(LPVOID) {
    AllocConsole();
    FILE* Handle;
    freopen_s(&Handle, "CONOUT$", "w", stdout);

    MH_Initialize();

    auto BaseAddr = (uintptr_t)GetModuleHandle(0);
    auto GObjAddr = Memory::FindPattern("48 8B 05 ? ? ? ? 48 8D 1C C8 81 4B ? ? ? ? ? 49 63 76 30", true, 3);
    auto FNTSAddr = Memory::FindPattern("48 89 5C 24 ? 57 48 83 EC 40 83 79 04 00 48 8B DA 48 8B F9");
    auto FreeAddr = Memory::FindPattern("48 85 C9 74 1D 4C 8B 05 ? ? ? ? 4D 85 C0 0F 84 ? ? ? ? 49 8B 00 48 8B D1 49 8B C8 48 FF 60 20 C3");

    UObject::GObjects = decltype(UObject::GObjects)(GObjAddr);
    FNameToString = decltype(FNameToString)(FNTSAddr);
    FMemory_Free = decltype(FMemory_Free)(FreeAddr);

    Globals::Engine = UObject::FindObject<UFortEngine>("FortEngine_");

    Globals::GameplayStatics = UObject::FindObject<UGameplayStatics>("Default__GameplayStatics");

    Globals::BaseTransform.Scale3D = { 1,1,1 };

    ProcessEvent = decltype(ProcessEvent)(Globals::Engine->Vtable[0x40]);

    SetupOthers(); // Engine and World

    printf("[Shield Singleplayer] Successfully setup!");
    MH_CreateHook((void*)Globals::Engine->Vtable[0x40], ProcessEventHook, (void**)&ProcessEvent);
    MH_EnableHook(Globals::Engine->Vtable[0x40]);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, Main, 0, 0, 0);
        break;
    }
    return TRUE;
}