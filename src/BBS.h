#include "stdlib.h"

struct Comment
{
    std::string user;
    std::string content;
    Comment(std::string user, std::string content)
        : user(user), content(content) {}
};

class Board
{
public:
    Board(std::string name, std::string moderator)
        : name(name), moderator(moderator) {}

    void getInfo(std::string &name, std::string &moderator);
    std::set<uint64_t> &getPosts();
    void addPost(uint64_t SN);
    void removePost(uint64_t SN);

private:
    std::string name;
    std::string moderator;
    std::set<uint64_t> posts;
};

class Post
{
public:
    Post(uint64_t SN, std::string title, std::string author, std::string date, std::string content)
        : SN(SN), title(title), author(author), date(date), content(content) {}

    void addComment(std::string user, std::string comment);
    void read(FILE *fp);
    void update(bool isTitle, std::string str);
    bool validateOwner(std::string name);
    uint64_t getSN();
    std::string getTitle();
    std::string getAuthor();
    std::string getDate();

private:
    uint64_t SN;
    std::string title;
    std::string author;
    std::string date;
    std::string content;
    std::list<struct Comment> comments;
};

namespace BBS
{
    std::map<std::string, Board> boards;
    std::list<Board *> board_history;
    std::map<uint64_t, Post> posts;
    uint64_t SN = 1;
}