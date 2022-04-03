#ifndef UTILS
#define UTILS

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>
#include <vector>

std::vector<std::string> split(std::string, char);

int createMasterSocket(int, struct sockaddr_in*);

#endif