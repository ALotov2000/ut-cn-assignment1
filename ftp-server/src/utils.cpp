#include "../include/utils.h"


std::vector<std::string> split(std::string command, char sep) {
  std::vector<std::string> res = std::vector<std::string>();

  while(true) {
    int i = (command).find(sep);
    if(i < 0) {
      if(command.size() > 0)
        res.push_back(command);
      command = "";
      break;
    }
    else {
      if(i > 0)
        res.push_back((command).substr(0, i));
      command = (command).substr(i + 1, (command).size());
    }
  }

  return res;
}

int createMasterSocket(int port, struct sockaddr_in* socketAddress) {
  char clog[1024];

  int socketFD;
  //create master socket started
  if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    sprintf(clog, "%s:\nError: socket failed\n", getTime().c_str()); ServerConfig::log(std::string(clog));
    exit(EXIT_FAILURE);
  }
  sprintf(clog, "%s:\nsocket with fd = %d has been created\n", getTime().c_str(), socketFD); ServerConfig::log(std::string(clog));
    //set socket options started
  int opt = 1;
  if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    sprintf(clog, "%s:\nError: setsockopt failed", getTime().c_str()); ServerConfig::log(std::string(clog));
    exit(EXIT_FAILURE);
  }
    //set socket options finished
  //create master socket finished
  //bind started
    //set address started
  struct sockaddr_in sockAddr;
  socketAddress = &sockAddr;
  sockAddr.sin_family = AF_INET;
  sockAddr.sin_port = htons(port);
  if(inet_pton(AF_INET, "127.0.0.1", &sockAddr.sin_addr) <= 0) {
    sprintf(clog, "%s:\nError: invalid address: address not supported\n", getTime().c_str()); ServerConfig::log(std::string(clog));
    exit(EXIT_FAILURE);
  }
    //set address finished  
  if (bind(socketFD, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) < 0) {
    sprintf(clog, "%s:\nError: bind failed\n", getTime().c_str()); ServerConfig::log(std::string(clog));
    exit(EXIT_FAILURE);
  }

  sprintf(clog, "%s:\nsocket has binded with address 127.0.0.1:%u\n", getTime().c_str(), port); ServerConfig::log(std::string(clog));
  //bind finished
  return socketFD;
}

std::string getTime() {

  time_t rawtime;
  struct tm * timeinfo;
  char buffer [80];

  time (&rawtime);
  timeinfo = localtime (&rawtime);

  strftime (buffer,80,"%Y, %h %e %H:%M:%S",timeinfo);
  
  return std::string(buffer);
}