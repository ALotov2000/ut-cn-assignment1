#include "../include/client.h"

void Client::handle() {
  char ccommand[MAX_COMMAND_SIZE], cresponse[MAX_COMMAND_SIZE];
  std::string command;
  std::string data;
  std::string alias;
  int fd = this->commandSocketFD;

  while(true) {
    //read command
    if(recv(fd, &ccommand, MAX_COMMAND_SIZE, 0) < 0) {
      printf("FD: %d :Error: read failed\n", fd);
      return;
    }
    command = std::string(ccommand);
    printf("FD: %d :got command: \"%s\"\n", fd, command.c_str());

    //break command into pieces
    std::vector<std::string> splitted = split(command, ' ');
    if(splitted.size() < 1) {
      sprintf(cresponse, "501: Error: command not determined\n");
      this->handleResponse(cresponse);
      continue;
    }

    //quit
    if(splitted[0] == "quit") {
      if((int)this->userId > -1) {
        (*(this->isUsersAvailable))[(int)userId] = true;
      }
      sprintf(cresponse, "221: successfully quitted\n");
      this->handleResponse(cresponse);
      return;
    }
    
    //user [username]
    if(splitted[0] == "user") {
      if(this->qualified) {
        sprintf(cresponse, "304: Error: already logged in\n");
        this->handleResponse(cresponse);
        continue;
      }
      if(splitted.size() < 2) {
        sprintf(cresponse, "430: Error: invalid username or password\n");
        this->handleResponse(cresponse);
        continue;
      }
      bool isFound = false;
      for(Json::ArrayIndex i = 0; i < (*(this->root))["users"].size(); ++i) {
        if(!(*(this->isUsersAvailable))[i])
          continue;
        if((*(this->root))["users"][i]["user"] == splitted[1]) {
          isFound = true;
          this->userId = i;
          break;
        }
      }
      if(isFound) {
        sprintf(cresponse, "331: username is okay, need password\n");
        this->handleResponse(cresponse);
        continue;
      }
      else {
        sprintf(cresponse, "430: Error: invalid username or password\n");
        this->handleResponse(cresponse);
        continue;
      }
    }
    
    //pass [password]
    if(splitted[0] == "pass") {
      if(this->qualified) {
        sprintf(cresponse, "304: Error: already logged in\n");
        this->handleResponse(cresponse);
        continue;
      }
      if(this->userId == (Json::ArrayIndex)-1) {
        sprintf(cresponse, "503 Error: bad sequence of commands\n");
        this->handleResponse(cresponse);
        continue;
      }
      if(splitted.size() < 2) {
        sprintf(cresponse, "430: Error: invalid username or password\n");
        this->handleResponse(cresponse);
        continue;
      }
      
      std::string givenPassword = splitted[1];
      if(givenPassword == (*(this->root))["users"][this->userId]["password"].asString()) {
        sprintf(cresponse, "230: user logged in, proceed\n");

        this->qualified = true;
        if((*(this->root))["users"][this->userId]["admin"].asString() == "true")
          this->isadmin = true;
        else
          this->isadmin = false;
        

        (*(this->isUsersAvailable))[(int)this->userId] = false;
        this->handleResponse(cresponse);
        continue;
      }
      else {
        sprintf(cresponse, "430: Error: invalid username or password\n");
        this->handleResponse(cresponse);
        continue;
      }
    }
    
    //pwd
    if(splitted[0] == "pwd") {
      if(!this->qualified) {
        sprintf(cresponse, "332: Error: need account\n");
        this->handleResponse(cresponse);
        continue;
      }
      char cwd[512];
      getcwd(cwd, 512);
      sprintf(cresponse, "257: current directory is \"%s\"\n", cwd);
      this->handleResponse(cresponse);
      continue;
    }

    //mkd [directoryName]
    if(splitted[0] == "mkd") {
      if(!this->qualified) {
        sprintf(cresponse, "332: Error: need account\n");
        this->handleResponse(cresponse);
        continue;
      }
      if(mkdir(splitted[1].c_str(), 0777) < 0) {
        sprintf(cresponse, "500: Error: can not create directory\n");
        this->handleResponse(cresponse);
        continue;
      }

      sprintf(cresponse, "257: directory \"%s\" has been created\n", splitted[1].c_str());
      this->handleResponse(cresponse);
      continue;
    }

    //dele [(-f/-d)] [(file/directory) name]
    if(splitted[0] == "dele") {
      if(!this->qualified) {
        sprintf(cresponse, "332: Error: need account\n");
        this->handleResponse(cresponse);
        continue;
      }
      //delete files
      if(splitted[1] == "-f") {
        if(unlink(splitted[2].c_str()) < 0) {
          sprintf(cresponse, "500: Error: can not delete file\n");
          this->handleResponse(cresponse);
          continue;
        }
        sprintf(cresponse, "250: file \"%s\" has been deleted\n", splitted[2].c_str());
        this->handleResponse(cresponse);
        continue;
      }
      //delete directories
      else if(splitted[1] == "-d") {
        if(rmdir(splitted[2].c_str()) < 0) {
          sprintf(cresponse, "500: Error: can not delete directory\n");
          this->handleResponse(cresponse);
          continue;
        }
        sprintf(cresponse, "250: directory \"%s\" has been deleted\n", splitted[2].c_str());
        this->handleResponse(cresponse);
        continue;
      }
      else {
        sprintf(cresponse, "501: Error: wrong flags\n");
        this->handleResponse(cresponse);
        continue;
      }
    }

    //ls
    if(splitted[0] == "ls") {
      if(!this->qualified) {
        sprintf(cresponse, "332: Error: need account\n");
        this->handleResponse(cresponse);
        continue;
      }

      DIR* d;
      struct dirent* dir;
      char line[LINE_SIZE];
      int size;

      d = opendir(".");
      data = "";      
      if(d) {
        while ((dir = readdir(d)) != NULL) {
          sprintf(line, "%s\n", dir->d_name);
          data += std::string(line);
        }
        closedir(d);
      }
      printf("\"\"\"\n%s\n\"\"\"\n", data.c_str());
      size = data.size() + 1;
      if((*(this->remainingSize))[this->userId] < size + LINE_SIZE) {
        sprintf(cresponse, "425: can not open data connection (not enough remaining size)\n");
        this->handleResponse(cresponse);
      }

      (*(this->remainingSize))[this->userId] -= size + LINE_SIZE;

      char csize[LINE_SIZE]; sprintf(csize, "%d", size);

      send(this->dataSocketFD, csize, LINE_SIZE, 0);
      send(this->dataSocketFD, data.c_str(), size, 0);

      sprintf(cresponse, "226: list transfer has been done\n");
      this->handleResponse(cresponse);
      continue;
    }

    //cwd [path name]
    if(splitted[0] == "cwd") {
      if(!this->qualified) {
        sprintf(cresponse, "332: Error: need account\n");
        this->handleResponse(cresponse);
        continue;
      }
      if(splitted.size() < 2) {
        if(chdir(this->basePath) < 0) {
          sprintf(cresponse, "500: Error: can not change working directory\n");
          this->handleResponse(cresponse);
          continue;
        }

        sprintf(cresponse, "250: successful change to the default directory\n");
        this->handleResponse(cresponse);
        continue;
      }

      if(chdir(splitted[1].c_str()) < 0) {
        sprintf(cresponse, "500: Error: can not change working directory\n");
        this->handleResponse(cresponse);
        continue;
      }

      sprintf(cresponse, "250: successfully changed\n");
      this->handleResponse(cresponse);
      continue;
    }
    
    //rename [old name] [new name]
    if(splitted[0] == "rename") {
      if(!this->qualified) {
        sprintf(cresponse, "332: Error: need account\n");
        this->handleResponse(cresponse);
        continue;
      }
      if(splitted.size() < 3) {
        sprintf(cresponse, "501: Error: not enough arguments\n");
        this->handleResponse(cresponse);
        continue;
      }

      if(rename(splitted[1].c_str(), splitted[2].c_str()) < 0) {
        sprintf(cresponse, "500: Error: can not rename the file\n");
        this->handleResponse(cresponse);
        continue;
      }

      sprintf(cresponse, "250: successfully renamed\n");
      this->handleResponse(cresponse);
      continue;
    }

    //retr [file name]
    if(splitted[0] == "retr") {
      if(!this->qualified) {
        sprintf(cresponse, "332: Error: need account\n");
        this->handleResponse(cresponse);
        continue;
      }

      if(splitted.size() < 2) {
        sprintf(cresponse, "501: Error: not enough arguments\n");
        this->handleResponse(cresponse);
        continue;
      }

      bool doesRequireAdmin = false;

      struct stat stat0;
      stat(splitted[1].c_str(), &stat0);

      for(std::size_t i = 0; i < (*(this->adminFiles)).size(); ++i) {
        struct stat stat1;
        stat((std::string(this->basePath) + '/' + (*(this->adminFiles))[i]).c_str(), &stat1);
        if(stat0.st_ino == stat1.st_ino) {
          doesRequireAdmin = true;
          break;
        }
      }

      if(!(this->isadmin) && doesRequireAdmin) {
        sprintf(cresponse, "500: Error: need adminastrative access\n");
        this->handleResponse(cresponse);
        continue;
      }

      char ch[LINE_SIZE];
      std::string data = "";
      int size;
      FILE* fp = fopen(splitted[1].c_str(), "r");

      if(fp == NULL) {
        sprintf(cresponse, "500: Error: can not open the file\n");
        this->handleResponse(cresponse);
        continue;
      }

      while(!feof(fp)) {
        fread(ch, LINE_SIZE, 1, fp);
        data += ch;
      }
      fclose(fp);
      size = data.size() + 1;

      if((*(this->remainingSize))[this->userId] < size + LINE_SIZE) {
        sprintf(cresponse, "425: can not open data connection (not enough remaining size)\n");
        this->handleResponse(cresponse);
      }

      (*(this->remainingSize))[this->userId] -= size + LINE_SIZE;

      char csize[LINE_SIZE]; sprintf(csize, "%d", size);

      send(this->dataSocketFD, csize, LINE_SIZE, 0);
      send(this->dataSocketFD, data.c_str(), size, 0);

      sprintf(cresponse, "226: successfully retrieved\n");
      this->handleResponse(cresponse);
      
      continue;
    }
    
    if(splitted[0] == "help") {

      char ch[LINE_SIZE];
      std::string data = "";
      int size;
      FILE* fp = fopen((std::string(this->basePath) + '/' + "scripts/help.txt").c_str(), "r");

      if(fp == NULL) {
        sprintf(cresponse, "500: Error: can not open the help file\n");
        this->handleResponse(cresponse);
        continue;
      }

      while(!feof(fp)) {
        fread(ch, LINE_SIZE, 1, fp);
        data += ch;
      }
      fclose(fp);
      size = data.size() + 1;

      if((*(this->remainingSize))[this->userId] < size + LINE_SIZE) {
        sprintf(cresponse, "425: can not open data connection (not enough remaining size)\n");
        this->handleResponse(cresponse);
      }

      (*(this->remainingSize))[this->userId] -= size + LINE_SIZE;

      char csize[LINE_SIZE]; sprintf(csize, "%d", size);

      send(this->dataSocketFD, csize, LINE_SIZE, 0);
      send(this->dataSocketFD, data.c_str(), size, 0);

      sprintf(cresponse, "226: successfully retrieved\n");
      this->handleResponse(cresponse);
      
      continue;
    }
    //default response
    sprintf(cresponse, "501: Error: invalid command\n");
    this->handleResponse(cresponse);
    continue;
  }
}

void Client::handleResponse(char* text) {
  int fd = this->commandSocketFD;
  int size = (this->userId == (Json::ArrayIndex)-1) ? 0: (*(this->remainingSize))[(int)this->userId];
  send(fd, text, MAX_COMMAND_SIZE, 0);
  printf("%s:\nFD: %d (userId = %d, qualified: %s, isadmin: %s, size: %d) : %s", getTime().c_str(), fd, this->userId, this->qualified? "true": "false", this->isadmin? "true": "false", size, text);
}