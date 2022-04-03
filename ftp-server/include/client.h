#ifndef CLIENT
#define CLIENT

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <jsoncpp/json/json.h>
#include <dirent.h>
#include <fcntl.h>

#include <string>
#include <vector>

#include "utils.h"
#include "serverConfig.h"

#define MAX_COMMAND_SIZE 1024
#define LINE_SIZE 1024

class Client {
  private:
  int commandSocketFD;
  int dataSocketFD;
  Json::ArrayIndex userId;
  bool qualified;
  bool isadmin;
  Json::Value* root;
  std::vector<bool>* isUsersAvailable;
  std::vector<int>* remainingSize;
  std::vector<std::string>* adminFiles;
  char basePath[1024];

  public:
  Client(int _commandSocketFD, int _dataSocketFD, Json::Value* _root, std::vector<bool>* _isUsersAvailable, std::vector<int>* _remainingSize, std::vector<std::string>* _adminFiles) : commandSocketFD(_commandSocketFD), dataSocketFD(_dataSocketFD), userId(-1), qualified(false), isadmin(false), root(_root), isUsersAvailable(_isUsersAvailable), remainingSize(_remainingSize), adminFiles(_adminFiles) {
    getcwd(basePath, 1024);
  }
  void handle();

  int getSocket() {
    return this -> commandSocketFD;
  }

  void handleResponse(char*);
};

#endif