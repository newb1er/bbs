#include "src/server.cpp"

int main(int argc, char **argv)
{
    Server s(htonl(INADDR_ANY), htons(argc > 1 ? atoi(argv[1]) : 2233));

    s.setup();
    s.start();

    return 0;
}