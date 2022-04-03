#include "../include/serverConfig.h"

Json::Value ServerConfig::root = Json::nullValue;
int ServerConfig::commandMasterSocketFD = 0;
int ServerConfig::dataMasterSocketFD = 0;

struct sockaddr_in ServerConfig::commandMasterSocketAddress, ServerConfig::dataMasterSocketAddress;
std::vector<bool> ServerConfig::isUsersAvailable = std::vector<bool>(0);
std::vector<int> ServerConfig::remainingSize = std::vector<int>(0);
std::vector<std::string> ServerConfig::adminFiles = std::vector<std::string>(0);

void ServerConfig::resetLogFile() {
  unlink("log.txt");
}

void ServerConfig::log(std::string response) {
  printf("%s", response.c_str());
  FILE* fp = fopen("log.txt", "a");
  fwrite(response.c_str(), response.size(), 1, fp);
  fclose(fp);
}

void ServerConfig::setConfig(char* configFile) {
  char clog[1024];
  //read config.json
  std::ifstream config(configFile, std::ifstream::binary);
  config >> ServerConfig::root;

  ServerConfig::isUsersAvailable = std::vector<bool>(ServerConfig::root["users"].size(), true);
  for(Json::ArrayIndex i; i < ServerConfig::root["users"].size(); ++i) {
    ServerConfig::remainingSize.push_back(atoi(ServerConfig::root["users"][i]["size"].asString().c_str()));
  }

  for(Json::ArrayIndex i; i < ServerConfig::root["files"].size(); ++i) {
    ServerConfig::adminFiles.push_back(ServerConfig::root["files"][i].asString());
  }
  sprintf(clog, "%s:\n%d users can use this server\n", getTime().c_str(), ServerConfig::root["users"].size()); ServerConfig::log(std::string(clog));

}

void ServerConfig::establishServer() {
  ServerConfig::commandMasterSocketFD = createMasterSocket(ServerConfig::root["commandChannelPort"].asInt(), &ServerConfig::commandMasterSocketAddress);

  ServerConfig::dataMasterSocketFD = createMasterSocket(ServerConfig::root["dataChannelPort"].asInt(), &ServerConfig::dataMasterSocketAddress);
}

void ServerConfig::startService() {
  char clog[1024];
  //listen started
  if (listen(ServerConfig::commandMasterSocketFD, 3) < 0) {  
    sprintf(clog, "%s:\nError: listen failed\n", getTime().c_str()); ServerConfig::log(std::string(clog));  
    exit(EXIT_FAILURE);  
  }
  if (listen(ServerConfig::dataMasterSocketFD, 3) < 0) {  
    sprintf(clog, "%s:\nError: listen failed\n", getTime().c_str()); ServerConfig::log(std::string(clog));
    exit(EXIT_FAILURE);  
  }
  
  sprintf(clog, "%s:\nsockets started listening\n", getTime().c_str()); ServerConfig::log(std::string(clog));
  //listen finished

  //loop to accept multiple clients
  while(true) {
    //accept started
    int commandAddrlen = sizeof(ServerConfig::commandMasterSocketAddress);
    int dataAddrlen = sizeof(ServerConfig::dataMasterSocketAddress);
    int newCommandClientSocketFD, newDataClientSocketFD;
    if((newCommandClientSocketFD = accept(ServerConfig::commandMasterSocketFD, (struct sockaddr*) &ServerConfig::commandMasterSocketAddress, (socklen_t*) &commandAddrlen)) < 0) {
      sprintf(clog, "%s:\nError: command socket accept failed", getTime().c_str()); ServerConfig::log(std::string(clog));
      continue;
    }
    if((newDataClientSocketFD = accept(ServerConfig::dataMasterSocketFD, (struct sockaddr*) &ServerConfig::dataMasterSocketAddress, (socklen_t*) &dataAddrlen)) < 0) {
      sprintf(clog, "%s:\nError: data socket accept failed", getTime().c_str()); ServerConfig::log(std::string(clog));
      continue;
    }

    sprintf(clog, "%s:\ncommand socket with fd = %d and data socket with fd = %d have been accepted\n", getTime().c_str(), newCommandClientSocketFD, newDataClientSocketFD); ServerConfig::log(std::string(clog));
    //accept finished

    //create thread started
    pthread_t tid;
    Client client = Client(newCommandClientSocketFD, newDataClientSocketFD, &ServerConfig::root, &ServerConfig::isUsersAvailable, &ServerConfig::remainingSize, &ServerConfig::adminFiles);
    pthread_create(&tid, NULL, ServerConfig::handleClient, (void*) &client);

    sprintf(clog, "%s:\nthread with tid = %lu has been created to handle command socket with fd = %d and data socket with fd = %d\n", getTime().c_str(), tid, newCommandClientSocketFD, newDataClientSocketFD); ServerConfig::log(std::string(clog));
    //create thread finished
  }

}


void* ServerConfig::handleClient(void* _args) {
  char clog[1024];

  Client* client = (Client*) _args;
  client -> handle();
  close(client -> getSocket());
  sprintf(clog, "%s:\nFD: %d: successfully finished and thread has exitted\n", getTime().c_str(), client -> getSocket()); ServerConfig::log(std::string(clog));
  pthread_exit(NULL);
}