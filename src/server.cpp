#include "server.h"

#include <iostream>

inline void PrintShellTitle(int fd)
{
    write(fd, kShellTitle.c_str(), kShellTitle.size());
}

inline void ReadCommand(int fd, void *buf)
{
    // ! must clear buffer before read
    bzero(buf, kBufSize);
    read(fd, buf, kBufSize);
}

// parse command and do action
// return :
//      true: close connection
//      false: keep reading
bool ParseCommand(std::stringstream &ss, Session *s, std::list<std::string> &cmd)
{
    Parser(ss, cmd);

    if (cmd.empty())
        return false;

    auto func = kCommandFuntions.find(cmd.front());
    if (func != kCommandFuntions.end())
    {
        (*func->second)(cmd, s);

        if (cmd.size() == 1 && func->first == "exit")
            return true;
    }

    return false;
}

// start a server on given address and port
//
// Example :
//      Server s(htonl(INADDR_ANY), htons(2233);
//      s.setup();
//      s.start();

/**
 * @brief Construct a new Server:: Server object
 * 
 * @param addr server listening address
 * @param port server listening port
 */
Server::Server(in_addr_t addr, in_port_t port)
{
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = addr;
    servaddr.sin_port = port;

    maxconn = -1;
}

/**
 * @brief setup descriptors, socket options, etc.
 */
void Server::setup()
{
    tcpfd = socket(AF_INET, SOCK_STREAM, 0);
    udpfd = socket(AF_INET, SOCK_DGRAM, 0);

    int flag = 1;
    setsockopt(tcpfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
    setsockopt(udpfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));

    maxfd = udpfd;
    FD_ZERO(&allset);
    FD_SET(tcpfd, &allset);
    FD_SET(udpfd, &allset);

    for (int i = 0; i < FD_SETSIZE; ++i)
        client[i] = nullptr;
}

/**
 * @brief start the server
 */
void Server::start()
{
    bind(tcpfd, (SA *)&servaddr, sizeof(servaddr));
    listen(tcpfd, SOMAXCONN);

    bind(udpfd, (SA *)&servaddr, sizeof(servaddr));

    for (;;)
    {
        int nready;
        Select(nready);

        if (nready == -1 && errno == EINTR)
            continue;

        IncommingReq(nready);
        ConnectedReq(nready);
    }
}

/**
 * @brief setup select bits and select
 * 
 * @param nready number of descriptors that are ready to I/O
 */
void Server::Select(int &nready)
{
    rset = allset;
    nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
}

/**
 * @brief handle incomming requests, and accept connections
 * 
 * @param nready number of descriptors that are ready to I/O
 */
void Server::IncommingReq(int &nready)
{
    if (nready == 0)
        return;
    if (FD_ISSET(tcpfd, &rset))
    {
        int i;
        int connfd;
        struct sockaddr_in cliaddr;
        socklen_t len = sizeof(cliaddr);

        connfd = accept(tcpfd, (SA *)&cliaddr, &len);

        for (i = 0; i < FD_SETSIZE; ++i)
        {
            if (client[i] == nullptr)
            {
                client[i] = new Session(connfd, (struct sockaddr_in) cliaddr, len);
                break;
            }
        }
        if (i == FD_SETSIZE)
            std::cerr << "too many clients\n";

        FD_SET(connfd, &allset);

        WelcomeMsg(connfd);
        PrintShellTitle(connfd);

        maxfd = std::max(maxfd, connfd);
        maxconn = std::max(maxconn, i);
        --nready;
    }
}

/**
 * @brief handle requests from connected socket
 * 
 * @param nready number of descriptors that are ready to I/O
 */
void Server::ConnectedReq(int &nready)
{
    if (nready <= 0)
        return;

    Session *s;
    char buf[kBufSize];
    std::stringstream ss;
    std::list<std::string> cmd;
    std::string name, msg;

    if (FD_ISSET(udpfd, &rset)) {
        unsigned char ubuf[4096];
        if (recvfrom(udpfd, ubuf, 4096, 0, NULL, NULL) >= 0) {
            ChatRoom(ubuf, udpfd);
        }
    }

    for (int i = 0; i <= maxconn; ++i)
    {
        if ((s = client[i]) == nullptr)
            continue;

        if (FD_ISSET(s->fd, &rset))
        {
            bzero(buf, kBufSize);

            if ((read(s->fd, buf, kBufSize)) == 0)
            {
                CloseConnection(s->fd, i);
            }
            else
            {
                // ! clear ss before read
                ss.str("");
                ss.clear();

                ss << buf;

                if (ParseCommand(ss, s, cmd))
                    CloseConnection(s->fd, i);

                PrintShellTitle(s->fd);
            }

            if (--nready <= 0)
                break;
        }
    }
}

/**
 * @brief close existed socket connection
 * 
 * @param sockfd the socket descriptor to close
 * @param connidx index in "client" array
 */
void Server::CloseConnection(int &sockfd, int &connidx)
{
    FD_CLR(sockfd, &allset);
    client[connidx] = nullptr;
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
}

void Server::ChatRoom(unsigned char buf[], int &fd) {
    std::array<std::pair<unsigned char*, size_t>, 2> outbuf;
    std::string username;
    bool violate;

    if (!Chat::ParseMessage(buf, username, violate, outbuf)) return;
    // send to every client in chatroom
    for (int i = 0; i <= maxconn; ++i) {
        Session *s = client[i];
        if (s == nullptr || s->isChat == false) continue;

        if (s->user->name == username && violate) {
            if (++(s->user->violate) >= 3) {
                sendMsg(s, "Bye, " + s->user->name + ".\n");
                s->logout();
                PrintShellTitle(s->fd);
                continue;
            }
        }

        if (s->chatVersion == 1) {
            if (sendto(fd, outbuf[0].first, outbuf[0].second, 0, (struct sockaddr *) &s->addr, s->addrlen) == -1) 
                std::cout << errno << '\n';
        }
        else if (s->chatVersion == 2) {
            if (sendto(fd, outbuf[1].first, outbuf[1].second, 0, (struct sockaddr *) &s->addr, s->addrlen) == -1)
                std::cout << errno << '\n';
        }
    }
}