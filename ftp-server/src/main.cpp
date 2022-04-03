#include "../include/main.h"

#include <string.h>

using namespace std;

int main(int argc, char** args) {
  if(argc < 2) {
    printf("%s:\nError: no config settings given\n", getTime().c_str());
    exit(EXIT_FAILURE);
  }

  ServerConfig::resetLogFile();

  ServerConfig::setConfig(args[1]);
  
  ServerConfig::establishServer();

  ServerConfig::startService();

  exit(EXIT_SUCCESS);
}