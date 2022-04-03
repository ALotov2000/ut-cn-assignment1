#ifndef SERVER_CONFIG
#define SERVER_CONFIG

#include <jsoncpp/json/json.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "client.h"
#include "utils.h"

class ServerConfig {
    private:
    static Json::Value root;
    static int commandMasterSocketFD;
    static int dataMasterSocketFD;
    static struct sockaddr_in commandMasterSocketAddress;
    static struct sockaddr_in dataMasterSocketAddress;
    static std::vector<bool> isUsersAvailable;
    static std::vector<int> remainingSize;
    static std::vector<std::string> adminFiles;

    public:
    static void resetLogFile();
    static void log(std::string);
    static void setConfig(char*);
    static void establishServer();
    static void startService();
    static void* handleClient(void*);
    
    static struct sockaddr_in getAddress() {
        return ServerConfig::commandMasterSocketAddress;
    }
};
#endif