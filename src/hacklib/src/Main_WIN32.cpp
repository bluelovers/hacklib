#include "hacklib/Main.h"
#include "hacklib/MessageBox.h"
#include <Windows.h>


hl::ModuleHandle hl::GetCurrentModule()
{
    static hl::ModuleHandle hModule = NULL;

    if (!hModule)
    {
        if (GetModuleHandleEx(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCTSTR)GetCurrentModule,
            &hModule) == 0)
        {
            throw std::runtime_error("GetModuleHandleEx failed");
        }
    }

    return hModule;
}

std::string hl::GetModulePath()
{
    static std::string modulePath;

    if (modulePath == "")
    {
        char path[MAX_PATH];
        if (GetModuleFileNameA(hl::GetCurrentModule(), path, MAX_PATH) == 0)
        {
            throw std::runtime_error("GetModuleFileName failed");
        }
        modulePath = path;
    }

    return modulePath;
}


static DWORD WINAPI ThreadFunc(LPVOID param)
{
    auto self = (hl::StaticInitImpl*)param;
    self->mainThread();
    return 0;
}

void hl::StaticInitImpl::runMainThread()
{
    // Must use WinAPI threads instead of std threads, because current std
    // implementation blocks within the loader lock.

    HANDLE hThread = CreateThread(NULL, 0, ThreadFunc, (LPVOID)this, 0, NULL);
    if (hThread == NULL)
    {
        throw std::runtime_error(std::string("CreateThread failed with code ") + std::to_string(GetLastError()));
    }
    else
    {
        // Thread will be exited by suiciding with FreeLibraryAndExitThread.
        CloseHandle(hThread);
    }
}

void hl::StaticInitImpl::unloadSelf()
{
    FreeLibraryAndExitThread(hl::GetCurrentModule(), 0);
}
