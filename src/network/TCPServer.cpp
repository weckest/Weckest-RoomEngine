//
// Created by Weckest on 2026-09-23.
//

#include "TcpServer.h"

#include <iostream>
#include <vector>
#include <chrono>

#include "hardware.pb.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "services/ModuleService.h"

#ifdef _WIN32

    #include <winsock2.h>
    #include <ws2tcpip.h>

#else

    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>

#endif


TcpServer::TcpServer(int port)
    : port(port), serverSocket(-1), running(false), moduleService(*this)
{}

TcpServer::~TcpServer()
{
    Stop();

    WSACleanup();
}

bool TcpServer::Start()
{
    if (running)
    {
        return true;
    }

    /*
     * Initialize Winsock.
     */
    WSADATA wsaData{};

    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0)
    {
        std::cerr << "WSAStartup failed: " << result << '\n';

        return false;
    }

    if (!SetupServerSocket())
    {
        WSACleanup();
        return false;
    }

    running = true;

    std::cout << "RoomEngine listening on port " << port << '\n';

    return true;
}

void TcpServer::Update()
{
    if (!running) return;

    AcceptNewConnections();

    ProcessConnections();

    RemoveDisconnectedConnections();
}

void TcpServer::Stop()
{
    if (!running) return;

    std::cout << "\nStopping RoomEngine TCP server...\n";

    running = false;

    std::cout << "Closing " << connections.size() << " client connection(s)...\n";

    for (auto& connection : connections)
    {
        if (connection->IsConnected())
        {
            std::cout << "Closing client socket: " << connection->GetSocket() << '\n';

            connection->Close();
        }
    }

    connections.clear();

    if (serverSocket != -1)
    {
        std::cout << "Closing listening socket...\n";

        shutdown(static_cast<SOCKET>(serverSocket), SD_BOTH);

        closesocket(static_cast<SOCKET>(serverSocket));

        serverSocket = -1;
    }

    std::cout << "All TCP connections closed.\n";
}

bool TcpServer::IsRunning() const {
    return running;
}

bool TcpServer::Send(int clientSocket, const ModuleMessage &message)
{
    // Encode the protobuf message
    uint8_t buffer[128];

    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    if (!pb_encode(&stream, ModuleMessage_fields, &message))
    {
        std::cerr << "Failed to encode message: " << PB_GET_ERROR(&stream) << '\n';
        return false;
    }

    uint32_t messageSize = static_cast<uint32_t>(stream.bytes_written);

    // Create 4-byte big-endian size header
    uint8_t header[4]
    {
        static_cast<uint8_t>((messageSize >> 24) & 0xFF),
        static_cast<uint8_t>((messageSize >> 16) & 0xFF),
        static_cast<uint8_t>((messageSize >> 8) & 0xFF),
        static_cast<uint8_t>(messageSize & 0xFF)
    };

    // Send the header
    int headerSent = send(clientSocket, reinterpret_cast<const char*>(header), 4, 0);

    if (headerSent != 4)
    {
        std::cerr << "Failed to send message header\n";
        return false;
    }

    // Send the protobuf message
    int messageSent = send(clientSocket, reinterpret_cast<const char*>(buffer), static_cast<int>(messageSize), 0);

    if (messageSent != static_cast<int>(messageSize))
    {
        std::cerr << "Failed to send protobuf message\n";
        return false;
    }

    return true;
}

void TcpServer::RemoveDisconnectedConnections()
{
    connections.erase(
        std::remove_if(
            connections.begin(),
            connections.end(),

            [](const std::unique_ptr<TcpConnection>& connection)
            {
                return !connection->IsConnected();
            }
        ),

        connections.end()
    );
}

void TcpServer::ProcessConnections()
{
    for (auto& connection : connections)
    {
        if (!connection->IsConnected()) continue;

        std::vector<ModuleMessage> messages;

        if (!connection->Update(messages)) continue;

        /*
         * A single TCP update may contain multiple
         * complete protobuf messages.
         */
        for (const ModuleMessage& message : messages)
        {
            moduleService.HandleMessage(message, connection->GetSocket(), *connection);
        }
    }
}

void TcpServer::AcceptNewConnections()
{
    while (running)
    {
        sockaddr_in clientAddress{};
        int clientAddressSize = sizeof(clientAddress);

        SOCKET clientSocket = accept(
            static_cast<SOCKET>(serverSocket),
            reinterpret_cast<sockaddr*>(&clientAddress),
            &clientAddressSize
        );

        if (clientSocket == INVALID_SOCKET)
        {
            int error = WSAGetLastError();

            /*
             * No more clients waiting.
             *
             * This is normal because the listening socket
             * is non-blocking.
             */
            if (error == WSAEWOULDBLOCK) return;

            std::cerr << "accept() failed with error " << error << '\n';

            return;
        }

        char clientIp[INET_ADDRSTRLEN]{};

        inet_ntop(AF_INET, &clientAddress.sin_addr, clientIp, sizeof(clientIp));

        std::cout << "Module connected from " << clientIp << '\n';

        auto connection = std::make_unique<TcpConnection>(static_cast<int>(clientSocket));

        if (!connection->IsConnected())
        {
            std::cerr << "Failed to initialize TCP connection.\n";

            connection->Close();

            continue;
        }

        std::cout << "Client socket: " << connection->GetSocket() << '\n';

        connections.push_back(std::move(connection));
    }
}

bool TcpServer::SetupServerSocket()
{
    serverSocket = static_cast<int>(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));

    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Failed to create server socket.\n";

        serverSocket = -1;

        return false;
    }

    /*
     * Allow the server to reuse the port after shutdown.
     */
    BOOL reuseAddress = TRUE;

    if (setsockopt(
        static_cast<SOCKET>(serverSocket),
        SOL_SOCKET,
        SO_REUSEADDR,
        reinterpret_cast<const char*>(&reuseAddress),
        sizeof(reuseAddress)
    ) == SOCKET_ERROR)
    {
        std::cerr << "Failed to configure SO_REUSEADDR.\n";

        closesocket(static_cast<SOCKET>(serverSocket));

        serverSocket = -1;

        return false;
    }

    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(static_cast<u_short>(port));

    if (bind(
        static_cast<SOCKET>(serverSocket),
        reinterpret_cast<sockaddr*>(&serverAddress),
        sizeof(serverAddress)
    ) == SOCKET_ERROR)
    {
        std::cerr << "Failed to bind server socket to port " << port << ".\n";

        closesocket(static_cast<SOCKET>(serverSocket));

        serverSocket = -1;

        return false;
    }

    if (listen(static_cast<SOCKET>(serverSocket), SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Failed to listen on server socket.\n";

        closesocket(static_cast<SOCKET>(serverSocket));

        serverSocket = -1;

        return false;
    }

    /*
     * Make the listening socket non-blocking.
     *
     * This is important because RoomEngine::Update()
     * must never get stuck waiting for a new client.
     */
    u_long mode = 1;

    if (ioctlsocket(static_cast<SOCKET>(serverSocket), FIONBIO, &mode) != 0)
    {
        std::cerr << "Failed to make server socket non-blocking.\n";

        closesocket(static_cast<SOCKET>(serverSocket));

        serverSocket = -1;

        return false;
    }

    return true;
}



