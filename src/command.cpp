#include "command.h"
#include "chat.h"

inline void sendMsg(Session *s, std::string str)
{
    fprintf(s->fp, "%s", str.c_str());
}

void Register(std::list<std::string> &arg_str, Session *s)
{
    if (arg_str.size() != 3)
    {
        sendMsg(s, ErrorMsg::kRegUsageError);
        return;
    }

    // if user exists
    auto arg = arg_str.begin();
    std::advance(arg, 1);
    if (User::users.count(*arg))
        sendMsg(s, ErrorMsg::kRegDupUserError);
    else
    {
        const std::string username = *arg;
        std::advance(arg, 1);
        const std::string passwd = *arg;

        User::users.emplace(username, User::UserInfo(username, passwd));

        sendMsg(s, PromptMsg::kRegSuccess);
    }
}

void Login(std::list<std::string> &arg_str, Session *s)
{
    if (arg_str.size() != 3)
    {
        sendMsg(s, ErrorMsg::kLoginUsageError);
        return;
    }

    if (s->isLogin)
    {
        sendMsg(s, ErrorMsg::kNoLogoutError);
        return;
    }

    auto arg = arg_str.begin();
    std::advance(arg, 1);
    const std::string name = *arg;
    std::advance(arg, 1);
    const std::string passwd = *arg;

    auto user = User::users.find(name);

    // if user not exist or password incorrect
    if (user == User::users.end()) {
        sendMsg(s, ErrorMsg::kLoginFailError);
        return;
    }
    else if (user->second.isLogin)
    {
        sendMsg(s, ErrorMsg::kNoLogoutError);
        return;
    }
    else if (user->second.passwd != passwd)
    {
        sendMsg(s, ErrorMsg::kLoginFailError);
        return;
    }
    else if (user->second.violate >= 3) {
        std::string msg = ErrorMsg::kUserViolateLogin + user->second.name + "!\n";
        sendMsg(s, msg);
        return;
    }
    else
    {
        s->isLogin = true;
        s->user = &(user->second);
        s->user->isLogin = true;

        sendMsg(s, PromptMsg::kLoginSuccess + name + ".\n");
    }
}

void Logout(std::list<std::string> &arg_str, Session *s)
{
    if (arg_str.size() != 1) {
        sendMsg(s, ErrorMsg::kLogoutUsageError);
        return;
    }
    
    if (!s->isLogin)
    {
        sendMsg(s, ErrorMsg::kNoLoginError);
        return;
    }
    else
    {
        sendMsg(s, PromptMsg::kLogoutSuccess + s->user->name + ".\n");

        s->isLogin = false;
        s->user->isLogin = false;
        s->user = nullptr;
    }
}

void Exit(std::list<std::string> &arg_str, Session *s)
{
    if (arg_str.size() != 1) {
        sendMsg(s, ErrorMsg::kExitUsageError);
        return;
    }
    
    if (s->isLogin == true)
    {
        sendMsg(s, "Bye, " + s->user->name + ".\n");
        s->user->isLogin = false;
        s->isLogin = false;
        s->user = nullptr;
    }
    return;
}

void CreateBoard(std::list<std::string> &arg_str, Session *s)
{
    // usage error
    if (arg_str.size() != 2)
    {
        sendMsg(s, ErrorMsg::kBoardUsageError);
        return;
    }

    // check login
    if (!(s->isLogin))
    {
        sendMsg(s, ErrorMsg::kNoLoginError);
        return;
    }

    auto arg = arg_str.begin();
    std::advance(arg, 1);

    auto board = BBS::boards.emplace(
        *arg, Board(*arg, s->user->name));

    // check if board exists already
    if (board.second == false)
    {
        sendMsg(s, ErrorMsg::kBoardDupError);
    }
    else
    {
        BBS::board_history.push_back(&(board.first->second));
        sendMsg(s, PromptMsg::kBoardCreateSuccess);
    }
}

void CreatePost(std::list<std::string> &arg_str, Session *s)
{
    // usage error
    if (arg_str.size() != 6)
    {
    CreatePostUsageError:
        sendMsg(s, ErrorMsg::kCreatePostUsageError);
        return;
    }
    // check login
    if (!(s->isLogin))
    {
        sendMsg(s, ErrorMsg::kNoLoginError);
        return;
    }

    auto arg = arg_str.begin();
    std::advance(arg, 1);

    auto board = BBS::boards.find(*arg);

    // check board exists
    if (board == BBS::boards.end())
    {
        sendMsg(s, ErrorMsg::kBoardNullError);
        return;
    }

    // parse arguments
    std::advance(arg, 1); // now at index 2
    bool isTitle;
    std::string title = "", content = "";
    std::string *ptr;

CreatePostParseArg:
    // if reach end of arg_str
    if (arg == arg_str.end())
        goto CreatePostUsageError;

    if (*arg == "--title")
    {
        isTitle = true;
        ptr = &title;
    }
    else if (*arg == "--content")
    {
        isTitle = false;
        ptr = &content;
    }
    else
        goto CreatePostUsageError;

    std::advance(arg, 1);
    while (*arg != (isTitle ? "--content" : "--title") && arg != arg_str.end())
    {
        // parse <br>
        if (!isTitle)
        {
            auto found = (*arg).find("<br>");
            if (found != std::string::npos)
            {
                (*arg).replace(found, 4, "\n");
            }
        }

        *ptr += *arg + ' ';
        std::advance(arg, 1);
    }

    if (title == "" || content == "")
        goto CreatePostParseArg;

    // TODO: add date
    time_t now = std::time(nullptr);
    std::tm *tm = std::gmtime(&now);
    std::string date = std::to_string((tm->tm_mon) + 1) + '/' + std::to_string((tm->tm_mday));
    Post newPost(BBS::SN++, title, s->user->name, date, content);
    BBS::posts.emplace(newPost.getSN(), newPost);
    board->second.addPost(newPost.getSN());

    sendMsg(s, PromptMsg::kCreatePostSuccess);
}

void ListBoard(std::list<std::string> &arg_str, Session *s)
{
    int idx = 1;
    std::string name, moderator;

    fprintf(s->fp, "Index Name Moderator\n");
    for (auto board : BBS::board_history)
    {
        board->getInfo(name, moderator);
        fprintf(s->fp, "%d %s %s\n", idx++, name.c_str(), moderator.c_str());
    }
}

void ListPost(std::list<std::string> &arg_str, Session *s)
{
    // usage error
    if (arg_str.size() != 2)
    {
        sendMsg(s, ErrorMsg::kListPostUsageError);
        return;
    }

    auto arg = arg_str.begin();
    std::advance(arg, 1);

    // check existence of board
    auto board = BBS::boards.find(*arg);
    if (board == BBS::boards.end())
    {
        sendMsg(s, ErrorMsg::kBoardNullError);
        return;
    }

    fprintf(s->fp, "S/N Title Author Date\n");
    for (auto post : board->second.getPosts())
    {
        auto p = BBS::posts.find(post);
        if (p == BBS::posts.end())
        {
            board->second.removePost(p->first);
            continue;
        }
        fprintf(s->fp,
                "%ld %s %s %s\n",
                p->second.getSN(), p->second.getTitle().c_str(),
                p->second.getAuthor().c_str(), p->second.getDate().c_str());
    }
}

void Read(std::list<std::string> &arg_str, Session *s)
{
    // usage error
    if (arg_str.size() != 2)
    {
        sendMsg(s, ErrorMsg::kReadUsageError);
        return;
    }

    auto arg = arg_str.begin();
    std::advance(arg, 1);

    // check existence of post
    // * convert string to number
    auto post = BBS::posts.find(std::stoull(*arg));
    if (post == BBS::posts.end())
    {
        sendMsg(s, ErrorMsg::kPostNullError);
        return;
    }

    post->second.read(s->fp);
}

void DeletePost(std::list<std::string> &arg_str, Session *s)
{
    // usage error
    if (arg_str.size() != 2)
    {
        sendMsg(s, ErrorMsg::kDelPostUsageError);
        return;
    }

    // if user has not logged in
    if (!(s->isLogin))
    {
        sendMsg(s, ErrorMsg::kNoLoginError);
        return;
    }

    auto arg = arg_str.begin();
    advance(arg, 1);

    // if the post does not exist
    auto post = BBS::posts.find(std::stoull(*arg));
    if (post == BBS::posts.end())
    {
        sendMsg(s, ErrorMsg::kPostNullError);
        return;
    }

    // if user is not the owner of the post
    if (!(post->second.validateOwner(s->user->name)))
    {
        sendMsg(s, ErrorMsg::kPostAccessError);
        return;
    }

    // delete post
    BBS::posts.erase(post);
    sendMsg(s, PromptMsg::kDelPostSuccess);
}

void UpdatePost(std::list<std::string> &arg_str, Session *s)
{
    // usage error
    if (arg_str.size() != 4)
    {
        sendMsg(s, ErrorMsg::kUpdatePostUsageError);
        return;
    }

    // if user has not logged in
    if (!(s->isLogin))
    {
        sendMsg(s, ErrorMsg::kNoLoginError);
        return;
    }

    auto arg = arg_str.begin();
    advance(arg, 1);

    // if the post does not exist
    auto post = BBS::posts.find(std::stoull(*arg));
    if (post == BBS::posts.end())
    {
        sendMsg(s, ErrorMsg::kPostNullError);
        return;
    }

    // if user is not the owner of the post
    if (!(post->second.validateOwner(s->user->name)))
    {
        sendMsg(s, ErrorMsg::kPostAccessError);
        return;
    }

    // update post
    advance(arg, 1); // now idx = 2
    std::string str = "";
    bool isTitle = *arg == "--title" ? true : false;

    advance(arg, 1);
    while (arg != arg_str.end())
    {
        if (!isTitle)
        {
            auto found = arg->find("<br>");
            if (found != std::string::npos)
            {
                arg->replace(found, 4, "\n");
            }
        }

        str += *arg + ' ';

        advance(arg, 1);
    }

    post->second.update(isTitle, str);
    sendMsg(s, PromptMsg::kUpdatePostSuccess);
}

void Comment(std::list<std::string> &arg_str, Session *s)
{
    // usage error
    if (arg_str.size() != 3)
    {
        sendMsg(s, ErrorMsg::kCommentUsageError);
        return;
    }

    // if user has not logged in
    if (!(s->isLogin))
    {
        sendMsg(s, ErrorMsg::kNoLoginError);
        return;
    }

    auto arg = arg_str.begin();
    advance(arg, 1);

    // post does not exist
    auto post = BBS::posts.find(std::stoull(*arg));
    if (post == BBS::posts.end())
    {
        sendMsg(s, ErrorMsg::kPostNullError);
        return;
    }

    // add comment
    // read comment
    advance(arg, 1);
    std::string str = "";
    while (arg != arg_str.end())
    {
        str += *arg + ' ';
        advance(arg, 1);
    }

    post->second.addComment(s->user->name, str);
    sendMsg(s, PromptMsg::kCommentSuccess);
}

void EnterChat(std::list<std::string> &arg_str, Session *s)
{
    if (arg_str.size() != 3)
    {
        sendMsg(s, ErrorMsg::kEnterChatUsageError);
        return;
    }

    // if user has not logged in
    if (!(s->isLogin))
    {
        sendMsg(s, ErrorMsg::kNoLoginError);
        return;
    }

    auto arg = arg_str.begin();
    advance(arg, 1);

    int port;
    try {
        port = stoi(*arg);
    }
    catch (const std::invalid_argument& ia) {
        port = 0;
    }
    if (port < 1 || port > 65535)
    {
        std::string msg = "Port " + *arg + " is not valid.\n";
        sendMsg(s, msg);
        return;
    }
    advance(arg, 1);

    int version;
    try {
        version = stoi(*arg);
    }
    catch (const std::invalid_argument& ia) {
        version = 0;
    }
    if (version < 1 || version > 2)
    {
        std::string msg = "Version " + *arg + " is not supported.\n";
        sendMsg(s, msg);
        return;
    }

    s->isChat = true;
    s->addr.sin_port = htons(port);
    s->chatVersion = version;

    std::stringstream ss;
    ss << PromptMsg::kEnterChatSuccess;
    ss << "Port:" << port << "\nVersion:" << version << '\n';

    for (auto s : Chat::history)
        ss << s.first << ':' << s.second << '\n';

    sendMsg(s, ss.str());
}