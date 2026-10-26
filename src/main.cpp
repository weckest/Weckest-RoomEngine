#include <iostream>

#include <iostream>
#include <windows.h>

#include "engine/RoomEngine.h"

RoomEngine* g_engine = nullptr;

BOOL WINAPI ConsoleHandler(DWORD signal)
{
    switch (signal)
    {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
        {
            std::cout << "\nShutdown signal received...\n";

            if (g_engine != nullptr)
            {
                g_engine->Stop();
            }

            return TRUE;
        }

        default:
            return FALSE;
    }
}

int main()
{
    std::cout << "Starting RoomEngine...\n";

    RoomEngine engine(5001);
    g_engine = &engine;

    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE))
    {
        std::cerr << "Failed to install console handler\n";

        return 1;
    }

    if (!engine.Start())
    {
        std::cerr << "Failed to start RoomEngine\n";

        SetConsoleCtrlHandler(ConsoleHandler, FALSE);
        return 1;
    }

    engine.Run();

    SetConsoleCtrlHandler(ConsoleHandler, FALSE);

    std::cout << "RoomEngine stopped cleanly.\n";

    return 0;
}
