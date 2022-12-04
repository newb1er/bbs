#include "stdlib.h"
#include "BBS.cpp"
#include "session.cpp"

namespace PromptMsg
{
    const std::string kRegSuccess = "Register successfully.\n";
    const std::string kLoginSuccess = "Welcome, ";
    const std::string kLogoutSuccess = "Bye, ";
    const std::string kEmptyMailBox = "Your message box is empty.\n";
    const std::string kBoardCreateSuccess = "Create board successfully.\n";
    const std::string kCreatePostSuccess = "Create post successfully.\n";
    const std::string kDelPostSuccess = "Delete successfully.\n";
    const std::string kUpdatePostSuccess = "Update successfully.\n";
    const std::string kCommentSuccess = "Comment successfully.\n";
    const std::string kEnterChatSuccess = "Welcome to public chat room.\n";
};

namespace ErrorMsg
{
    const std::string kRegUsageError = "Usage: register <username> <password>\n";
    const std::string kRegDupUserError = "Username is already used.\n";
    const std::string kLoginUsageError = "Usage: login <username> <password>\n";
    const std::string kLoginDupError = "User already login.\n";
    const std::string kNoLogoutError = "Please logout first.\n";
    const std::string kLoginFailError = "Login failed.\n";
    const std::string kNoLoginError = "Please login first.\n";
    const std::string kLogoutUsageError = "Usage: logout\n";
    const std::string kExitUsageError = "Usage: exit\n";
    const std::string kSendUsageError = "Usage: send <username> <message>\n";
    const std::string kNoUserError = "User not existed.\n";
    const std::string kUserViolateLogin = "We don't welcome ";
    const std::string kReceiveUsageError = "Usage: receive <username>\n";
    const std::string kBoardUsageError = "Usage: create-board <name>\n";
    const std::string kBoardDupError = "Board already exists.\n";
    const std::string kBoardNullError = "Board does not exist.\n";
    const std::string kPostNullError = "Post does not exist.\n";
    const std::string kCreatePostUsageError = "Usage: create-post <board-name> --title <title> --content <content>\n";
    const std::string kListPostUsageError = "Usage: list-post <board-name>\n";
    const std::string kReadUsageError = "Usage: read <post-S/N>\n";
    const std::string kDelPostUsageError = "Usage: delete-post <post-S/N>\n";
    const std::string kPostAccessError = "Not the post owner.\n";
    const std::string kUpdatePostUsageError = "Usage: update-post <post-S/N> --title/content <new>\n";
    const std::string kCommentUsageError = "Usage: comment <post-S/N> <comment>\n";
    const std::string kEnterChatUsageError = "Usage: enter-chat-room <port> <version>\n";
};

inline void sendMsg(Session *s, std::string str);

typedef void (*FuncPtr)(std::list<std::string> &arg_str, Session *fd);

void Register(std::list<std::string> &arg_str, Session *s);
void Login(std::list<std::string> &arg_str, Session *s);
void Logout(std::list<std::string> &arg_str, Session *s);
void Exit(std::list<std::string> &arg_str, Session *s);
void CreateBoard(std::list<std::string> &arg_str, Session *s);
void CreatePost(std::list<std::string> &arg_str, Session *s);
void ListBoard(std::list<std::string> &arg_str, Session *s);
void ListPost(std::list<std::string> &arg_str, Session *s);
void Read(std::list<std::string> &arg_str, Session *s);
void DeletePost(std::list<std::string> &arg_str, Session *s);
void UpdatePost(std::list<std::string> &arg_str, Session *s);
void Comment(std::list<std::string> &arg_str, Session *s);
void EnterChat(std::list<std::string> &arg_str, Session *s);

std::unordered_map<std::string, FuncPtr> kCommandFuntions = {
    {"register", Register},
    {"login", Login},
    {"logout", Logout},
    {"exit", Exit},
    {"create-board", CreateBoard},
    {"create-post", CreatePost},
    {"list-board", ListBoard},
    {"list-post", ListPost},
    {"read", Read},
    {"delete-post", DeletePost},
    {"update-post", UpdatePost},
    {"comment", Comment},
    {"enter-chat-room", EnterChat}};
