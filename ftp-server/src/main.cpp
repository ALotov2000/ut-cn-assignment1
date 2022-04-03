#include "../include/main.h"

#include <string.h>

using namespace std;

int main(int argc, char** args) {
  if(argc < 2) {
    perror("Error: no config settings given\n");
    exit(EXIT_FAILURE);
  }

  ServerConfig::setConfig(args[1]);
  
  ServerConfig::establishServer();

  ServerConfig::startService();

  exit(EXIT_SUCCESS);
}