
#include <iostream>
#include <csignal>

#include "engine/RoomEngine.h"

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#else
    #include <signal.h>
#endif

// Must remain at global scope for ModuleService.cpp.
RoomEngine* g_engine = nullptr;

#ifndef _WIN32
namespace
{
    volatile std::sig_atomic_t g_shutdownRequested = 0;

    void SignalHandler(int signal)
    {
        if (signal == SIGINT || signal == SIGTERM)
        {
            g_shutdownRequested = 1;
        }
    }
}
#endif

#ifdef _WIN32
BOOL WINAPI ConsoleHandler(DWORD signal)
{
    switch (signal)
    {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            if (g_engine != nullptr)
            {
                g_engine->Stop();
            }
            return TRUE;

        default:
            return FALSE;
    }
}
#endif

int main()
{
    std::cout << "Starting RoomEngine...\n";

    RoomEngine engine(5001);
    g_engine = &engine;

#ifdef _WIN32
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE))
    {
        std::cerr << "Failed to install console handler\n";
        g_engine = nullptr;
        return 1;
    }
#else
    struct sigaction action{};
    action.sa_handler = SignalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGINT, &action, nullptr) == -1 ||
        sigaction(SIGTERM, &action, nullptr) == -1)
    {
        std::cerr << "Failed to install signal handlers\n";
        g_engine = nullptr;
        return 1;
    }
#endif

    if (!engine.Start())
    {
        std::cerr << "Failed to start RoomEngine\n";

#ifdef _WIN32
        SetConsoleCtrlHandler(ConsoleHandler, FALSE);
#endif

        g_engine = nullptr;
        return 1;
    }

#ifdef _WIN32
    // The console handler calls Stop() when Ctrl+C is pressed.
    engine.Run();
#else
    // Linux signal handlers only set a flag.
    while (!g_shutdownRequested)
    {
        engine.Run();
    }

    engine.Stop();
#endif

#ifdef _WIN32
    SetConsoleCtrlHandler(ConsoleHandler, FALSE);
#endif

    g_engine = nullptr;

    std::cout << "RoomEngine stopped cleanly.\n";
    return 0;
}
