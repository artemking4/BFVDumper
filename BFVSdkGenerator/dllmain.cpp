#include <Windows.h>
#include <thread>
#include "SdkGenerator.h"
#include "json.hpp"
#include <fstream>

using namespace BackendBf4;
void Thread(HMODULE mod) {
    AllocConsole();
    AttachConsole(GetCurrentProcessId());
    freopen("CON", "w", stdout);

    printf("Oh, hello there\n");

    PSDK_GENERATOR sdkGenerator = new SDK_GENERATOR();
    //fb::TypeInfo** g_firstTypeInfoPtr = (fb::TypeInfo**)0x144F7E670;
    fb::TypeInfo* g_firstTypeInfo = (fb::TypeInfo*)0x144F7E670;

    fb::TypeInfo* currentTypeInfo = g_firstTypeInfo;
    do
    {
        sdkGenerator->Register(currentTypeInfo);
    } while ((currentTypeInfo = currentTypeInfo->m_Next) != NULL);

    sdkGenerator->Analyze();
    nlohmann::json results = sdkGenerator->Generate();

    auto str = results.dump(4);
    std::ofstream f("dump.json");
    f << str;
    f.close();
    printf("Done\n");

    delete sdkGenerator;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        std::thread(Thread, hModule).detach();

    return TRUE;
}

