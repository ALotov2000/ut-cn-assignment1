#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <cstring>
#include <stdio.h>

std::vector<std::string> split(std::string, char);

int initiateConnection(int);
