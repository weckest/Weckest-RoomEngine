//
// Created by Weckest on 2026-09-23.
//


#include "TCPServer.h"

#include <iostream>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <cerrno>
    #include <fcntl.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

TcpServer::TcpServer(int port)
    : port(port),
      serverSocket(INVALID_TCP_SOCKET),
      running(false),
      moduleService(*this)
{
}

TcpServer::~TcpServer()
{
    Stop();
}

bool TcpServer::Start()
{
    if (running)
    {
        return true;
    }

#ifdef _WIN32
    WSADATA wsaData{};

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed." << std::endl;
        return false;
    }

    winsockInitialized = true;
#endif

    if (!SetupServerSocket())
    {
        std::cerr << "Failed to set up TCP server socket." << std::endl;

#ifdef _WIN32
        if (winsockInitialized)
        {
            WSACleanup();
            winsockInitialized = false;
        }
#endif

        return false;
    }

    running = true;

    std::cout << "TCP server listening on port "
              << port << std::endl;

    return true;
}

void TcpServer::Stop()
{
    running = false;

    for (auto& connection : connections)
    {
        if (connection)
        {
            connection->Close();
        }
    }

    connections.clear();

    if (serverSocket != INVALID_TCP_SOCKET)
    {
#ifdef _WIN32
        shutdown(serverSocket, SD_BOTH);
        closesocket(serverSocket);
#else
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
#endif

        serverSocket = INVALID_TCP_SOCKET;
    }

#ifdef _WIN32
    if (winsockInitialized)
    {
        WSACleanup();
        winsockInitialized = false;
    }
#endif
}

bool TcpServer::IsRunning() const
{
    return running;
}

void TcpServer::Update()
{
    if (!running)
    {
        return;
    }

    AcceptNewConnections();
    ProcessConnections();
    RemoveDisconnectedConnections();
}

bool TcpServer::Send(
    TcpSocket clientSocket,
    const ModuleMessage& message)
{
    const auto it = std::find_if(
        connections.begin(),
        connections.end(),
        [clientSocket](const std::unique_ptr<TcpConnection>& connection)
        {
            return connection &&
                   connection->GetSocket() == clientSocket;
        });

    if (it == connections.end())
    {
        std::cerr << "Cannot send message: client connection not found."
                  << std::endl;
        return false;
    }

    return (*it)->Send(message);
}

bool TcpServer::SetupServerSocket()
{
    serverSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_TCP_SOCKET)
    {
#ifdef _WIN32
        std::cerr << "socket() failed with error "
                  << WSAGetLastError() << std::endl;
#else
        std::cerr << "socket() failed: "
                  << std::strerror(errno) << std::endl;
#endif
        return false;
    }

    int reuseAddress = 1;

#ifdef _WIN32
    if (setsockopt(
            serverSocket,
            SOL_SOCKET,
            SO_REUSEADDR,
            reinterpret_cast<const char*>(&reuseAddress),
            sizeof(reuseAddress)) == SOCKET_ERROR)
    {
        std::cerr << "setsockopt() failed with error "
                  << WSAGetLastError() << std::endl;

        closesocket(serverSocket);
        serverSocket = INVALID_TCP_SOCKET;
        return false;
    }
#else
    if (setsockopt(
            serverSocket,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuseAddress,
            sizeof(reuseAddress)) == -1)
    {
        std::cerr << "setsockopt() failed: "
                  << std::strerror(errno) << std::endl;

        close(serverSocket);
        serverSocket = INVALID_TCP_SOCKET;
        return false;
    }
#endif

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(
        static_cast<uint16_t>(port));

    if (::bind(
            serverSocket,
            reinterpret_cast<sockaddr*>(&serverAddress),
            sizeof(serverAddress)) != 0)
    {
#ifdef _WIN32
        std::cerr << "bind() failed with error "
                  << WSAGetLastError() << std::endl;

        closesocket(serverSocket);
#else
        std::cerr << "bind() failed: "
                  << std::strerror(errno) << std::endl;

        close(serverSocket);
#endif

        serverSocket = INVALID_TCP_SOCKET;
        return false;
    }

    if (::listen(serverSocket, SOMAXCONN) != 0)
    {
#ifdef _WIN32
        std::cerr << "listen() failed with error "
                  << WSAGetLastError() << std::endl;

        closesocket(serverSocket);
#else
        std::cerr << "listen() failed: "
                  << std::strerror(errno) << std::endl;

        close(serverSocket);
#endif

        serverSocket = INVALID_TCP_SOCKET;
        return false;
    }

    // Make the listening socket non-blocking.
#ifdef _WIN32
    u_long nonBlocking = 1;

    if (ioctlsocket(
            serverSocket,
            FIONBIO,
            &nonBlocking) != 0)
    {
        std::cerr << "ioctlsocket() failed with error "
                  << WSAGetLastError() << std::endl;

        closesocket(serverSocket);
        serverSocket = INVALID_TCP_SOCKET;
        return false;
    }
#else
    const int flags = fcntl(serverSocket, F_GETFL, 0);

    if (flags == -1 ||
        fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        std::cerr << "Failed to make listening socket non-blocking: "
                  << std::strerror(errno) << std::endl;

        close(serverSocket);
        serverSocket = INVALID_TCP_SOCKET;
        return false;
    }
#endif

    return true;
}

void TcpServer::AcceptNewConnections()
{
    if (serverSocket == INVALID_TCP_SOCKET)
    {
        return;
    }

    while (true)
    {
        sockaddr_in clientAddress{};

#ifdef _WIN32
        int clientAddressSize = sizeof(clientAddress);
#else
        socklen_t clientAddressSize = sizeof(clientAddress);
#endif

        const TcpSocket clientSocket = ::accept(
            serverSocket,
            reinterpret_cast<sockaddr*>(&clientAddress),
            &clientAddressSize);

        if (clientSocket == INVALID_TCP_SOCKET)
        {
#ifdef _WIN32
            const int error = WSAGetLastError();

            if (error == WSAEWOULDBLOCK)
            {
                break;
            }

            std::cerr << "accept() failed with error "
                      << error << std::endl;
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }

            if (errno == EINTR)
            {
                continue;
            }

            std::cerr << "accept() failed: "
                      << std::strerror(errno) << std::endl;
#endif
            break;
        }

        char clientIp[INET_ADDRSTRLEN]{};

        if (inet_ntop(
                AF_INET,
                &clientAddress.sin_addr,
                clientIp,
                sizeof(clientIp)) != nullptr)
        {
            std::cout << "Module connected from "
                      << clientIp << ":"
                      << ntohs(clientAddress.sin_port)
                      << std::endl;
        }

        auto connection =
            std::make_unique<TcpConnection>(clientSocket);

        if (!connection->IsConnected())
        {
            connection->Close();
            continue;
        }

        connections.push_back(std::move(connection));
    }
}

void TcpServer::ProcessConnections()
{
    for (auto& connection : connections)
    {
        if (!connection || !connection->IsConnected())
        {
            continue;
        }

        std::vector<ModuleMessage> messages;

        if (!connection->Update(messages))
        {
            continue;
        }

        for (const auto& message : messages)
        {
            moduleService.HandleMessage(
                message,
                connection->GetSocket(),
                *connection);
        }
    }
}

void TcpServer::RemoveDisconnectedConnections()
{
    connections.erase(
        std::remove_if(
            connections.begin(),
            connections.end(),
            [](const std::unique_ptr<TcpConnection>& connection)
            {
                return !connection || !connection->IsConnected();
            }),
        connections.end());
}




