// ShieldGS.
// please fucking kill me...
#include <Windows.h>
#include "SDK.hpp"
#include "Globals.h"
#include "Memory.h"

using namespace SDK;

static void SetupOthers() {
    Globals::Engine = UObject::FindObject<UFortEngine>("FortEngine_");
    Globals::World = Globals::Engine->GameViewport->World;
}

static void* ProcessEventHook(UObject* Object, UFunction* Function, void* Params) {
    auto ObjName = Object->GetName();
    auto FunName = Function->GetName();

    if (FunName.find("BP_PlayButton") != std::string::npos) {
        Globals::Engine->GameInstance->LocalPlayers[0]->PlayerController->SwitchLevel(L"Athena_Terrain");
    }
}

DWORD WINAPI Main(LPVOID) {
    AllocConsole();
    FILE* Handle;
    freopen_s(&Handle, "CONOUT$", "w", stdout);

    auto BaseAddr = (uintptr_t)GetModuleHandle(0);
    auto GObjAddr = Memory::FindPattern("48 8B 05 ? ? ? ? 48 8D 1C C8 81 4B ? ? ? ? ? 49 63 76 30", true, 3);
    auto FNTSAddr = Memory::FindPattern("48 89 5C 24 ? 57 48 83 EC 40 83 79 04 00 48 8B DA 48 8B F9");
    auto FreeAddr = Memory::FindPattern("48 85 C9 74 1D 4C 8B 05 ? ? ? ? 4D 85 C0");

    UObject::GObjects = decltype(UObject::GObjects)(GObjAddr);
    FNameToString = decltype(FNameToString)(FNTSAddr);
    FMemory_Free = decltype(FMemory_Free)(FreeAddr);

    SetupOthers(); // Engine and World
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