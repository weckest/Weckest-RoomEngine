#include <iostream>
#include <csignal>

#include "engine/RoomEngine.h"

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#else
    #include <pthread.h>
    #include <signal.h>
    #include <thread>
#endif

// Must remain at global scope because ModuleService.cpp uses extern.
RoomEngine* g_engine = nullptr;

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

#ifndef _WIN32
    // Block shutdown signals so the dedicated thread can receive them
    // synchronously using sigwait().
    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);
    sigaddset(&signals, SIGTERM);

    if (pthread_sigmask(SIG_BLOCK, &signals, nullptr) != 0)
    {
        std::cerr << "Failed to block shutdown signals\n";
        return 1;
    }
#endif

    RoomEngine engine(5001);
    g_engine = &engine;

#ifdef _WIN32
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE))
    {
        std::cerr << "Failed to install console handler\n";
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

#ifndef _WIN32
    std::thread shutdownThread([&engine, &signals]()
    {
        int signal = 0;

        if (sigwait(&signals, &signal) == 0)
        {
            std::cout << "\nShutdown signal received.\n";
            engine.Stop();
        }
    });

    // Run owns the engine's update loop and returns when Stop() ends it.
    engine.Run();

    // Run has returned, so ensure the shutdown thread has finished.
    shutdownThread.join();

#else
    // The Windows console handler calls Stop() when Ctrl+C is pressed.
    engine.Run();
#endif

#ifdef _WIN32
    SetConsoleCtrlHandler(ConsoleHandler, FALSE);
#endif

    g_engine = nullptr;

    std::cout << "RoomEngine stopped cleanly.\n";
    return 0;
}
