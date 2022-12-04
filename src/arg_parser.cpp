#include "stdlib.h"

inline std::string &rtrim(std::string &s)
{
    if (s.empty())
        return s;

    s.erase(s.find_last_not_of(" \n\r") + 1);
    return s;
}

void Parser(std::stringstream &ss, std::list<std::string> &args)
{
    std::string str;

    while (std::getline(ss, str, ' '))
    {
        if (str[0] == '"')
        {
            std::string temp = str.substr(1);
            std::getline(ss, str, '"');
            str = temp + ' ' + str;
        }
        args.push_back(rtrim(str));
    }
}