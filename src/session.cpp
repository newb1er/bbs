#include "stdlib.h"

namespace User
{
    class UserInfo
    {
    public:
        std::string name;
        std::string passwd;
        int violate = 0;
        bool isLogin = false;

        UserInfo(std::string name, std::string passwd) : name(name), passwd(passwd) {}
    };

    std::map<std::string, UserInfo> users;
};

class Session
{
public:
    int fd;
    FILE *fp;
    bool isLogin = false;
    bool isChat = false;
    short chatVersion = 0;
    struct sockaddr_in addr;
    socklen_t addrlen = 0;
    User::UserInfo *user = nullptr;

    Session(int fd, struct sockaddr_in addr, socklen_t len) : fd(fd)
    {
        if ((fp = fdopen(fd, "r+")) == NULL)
            std::cerr << "fdopen error:\nerrno: " << errno << '\n';

        setvbuf(fp, NULL, _IONBF, 0);

        this->addr = addr;
        this->addrlen = len;
    }

    void logout() {
        isLogin = false;
        isChat = false;
        chatVersion = 0;
        bzero(&addr, sizeof(addr));
        addrlen = 0;
        user->isLogin = false;
        user = nullptr;
    }
};
