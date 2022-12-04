#include "stdlib.h"
#include "command.cpp"
#include "arg_parser.cpp"

const int kBufSize = 1024;
const std::string kShellTitle = "% ";

inline void WelcomeMsg(int connfd)
{
    const std::string msg[3] = {
        "********************************\n",
        "** Welcome to the BBS server. **\n",
        "********************************\n"};

    for (int i = 0; i < 3; ++i)
        send(connfd, msg[i].c_str(), msg[i].size(), 0);
}

class Server
{
public:
    Server(in_addr_t addr, in_port_t port);
    void setup();
    void start();

private:
    struct sockaddr_in servaddr;
    int tcpfd;
    int udpfd;
    int maxfd;
    Session *client[FD_SETSIZE];
    int maxconn;
    fd_set rset, allset;

    void Select(int &);
    void IncommingReq(int &);
    void ConnectedReq(int &);
    void CloseConnection(int &, int &);
    void ChatRoom(unsigned char[], int &);
};