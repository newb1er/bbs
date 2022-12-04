#include "BBS.h"

void Board::getInfo(std::string &name, std::string &moderator)
{
    name = this->name;
    moderator = this->moderator;
}

void Board::addPost(uint64_t SN)
{
    posts.insert(SN);
}

void Board::removePost(uint64_t SN)
{
    posts.erase(SN);
}

std::set<uint64_t> &Board::getPosts()
{
    return posts;
}

void Post::addComment(std::string user, std::string comment)
{
    struct Comment c(user, comment);
    comments.push_back(c);
}

void Post::read(FILE *fp)
{
    fprintf(fp, "Author: %s\n", author.c_str());
    fprintf(fp, "Title: %s\n", title.c_str());
    fprintf(fp, "Date: %s\n", date.c_str());
    fprintf(fp, "--\n%s\n--\n", content.c_str());

    for (auto comment : comments)
        fprintf(fp, "%s: %s\n",
                comment.user.c_str(), comment.content.c_str());

    fp = nullptr;
}

void Post::update(bool isTitle, std::string str)
{
    if (isTitle)
        title = str;
    else
        content = str;
}

bool Post::validateOwner(std::string name)
{
    return name == author;
}

uint64_t Post::getSN()
{
    return this->SN;
}

std::string Post::getTitle()
{
    return this->title;
}

std::string Post::getAuthor()
{
    return this->author;
}

std::string Post::getDate()
{
    return this->date;
}