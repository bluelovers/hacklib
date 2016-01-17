#include "hacklib/Main.h"
#include "hacklib/PageAllocator.h"
#include <dlfcn.h>
#include <pthread.h>
#include <cstring>
#include <thread>


hl::ModuleHandle hl::GetCurrentModule()
{
    static hl::ModuleHandle hModule = nullptr;

    if (!hModule)
    {
        Dl_info info = { 0 };
        if (dladdr((void*)GetCurrentModule, &info) == 0)
        {
            throw std::runtime_error("dladdr failed");
        }
        hModule = info.dli_fbase;
    }

    return hModule;
}

std::string hl::GetModulePath()
{
    static std::string modulePath;

    if (modulePath == "")
    {
        Dl_info info = { 0 };
        if (dladdr((void*)GetModulePath, &info) == 0)
        {
            throw std::runtime_error("dladdr failed");
        }
        modulePath = info.dli_fname;
    }

    return modulePath;
}


void FreeLibAndExitThread(void *hModule, int(*adr_dlclose)(void*), void(*adr_pthread_exit)(void*))
{
    // This can not be executed from inside the module.
    // Don't generate any code that uses relative addressing to the IP.
    adr_dlclose(hModule);
    adr_pthread_exit((void*)0);
}
void FreeLibAndExitThread_after()
{
}

void hl::StaticInitBase::runMainThread()
{
    // TODO: sigsegv handler

    std::thread th(&StaticInitImpl::mainThread, this);
    th.detach();
}

bool hl::StaticInitBase::protectedInit()
{
    // TODO: sigsegv handler
    return init();
}

void hl::StaticInitImpl::unloadSelf()
{
    // Get own module handle by path name. The dlclose just restores the refcount.
    auto modName = hl::GetModulePath();
    auto hModule = dlopen(modName.c_str(), RTLD_NOW | RTLD_LOCAL);
    dlclose(hModule);

    /*
    Linux does not have an equivalent to FreeLibraryAndExitThread, so a race between
    dlclose and pthread_exit would exist and lead to crashes.

    The following method of detaching leaks a page = 4KiB each time, but is 100% reliable.
    Alternative: Start a thread on dlclose. => Too unreliable, even with priority adjustments.
    Alternative: Let the injector do dlclose. => Signaling mechanism needed; injector might die or be killed.
    */

    size_t codeSize = (size_t)((uintptr_t)&FreeLibAndExitThread_after - (uintptr_t)&FreeLibAndExitThread);
    hl::code_page_vector code(codeSize);
    memcpy(code.data(), (void*)&FreeLibAndExitThread, codeSize);
    decltype(&FreeLibAndExitThread)(code.data())(hModule, &dlclose, &pthread_exit);
}