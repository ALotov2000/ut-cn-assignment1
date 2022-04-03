#include "../include/main.h"

int main(int argc, char** argv) {
  if(argc < 3) {
    printf("Error: no port given\n");
    exit(EXIT_FAILURE);
  }
  int commandPort = atoi(argv[1]), dataPort = atoi(argv[2]);

  int commandSocketFD = initiateConnection(commandPort);
  int dataSocketFD = initiateConnection(dataPort);

  printf("ready to communicate on command socket (fd = %d) and data socket (fd = %d)\n", commandSocketFD, dataSocketFD);

  //start communication with server
  char ccommand[1024], cresponse[1024], csize[1024], cdata[102400];

  while(true) {
    printf(">"); scanf("%[^\n]%*c", ccommand);
    std::string command = std::string(ccommand);
    std::vector<std::string> splitted = split(command, ' ');

    send(commandSocketFD, ccommand, 1024, 0);
    recv(commandSocketFD, &cresponse, 1024, 0);
    printf("%s", cresponse);

    //empty command
    if(splitted.size() < 1)
      continue;

    //quit
    if(splitted[0] == "quit")
      break;

    //ls
    if(splitted[0] == "ls" && std::string(cresponse).substr(0, 3) == "226") {
      int size;
      recv(dataSocketFD, &csize, 1024, 0);
      size = atoi(csize);
      recv(dataSocketFD, &cdata, size, 0);
      printf("%s\n", cdata);
    }

    //retr
    if(splitted[0] == "retr" && std::string(cresponse).substr(0, 3) == "226") {
      FILE* fp = fopen(splitted[1].c_str(), "w");

      int size;
      recv(dataSocketFD, &csize, 1024, 0);
      size = atoi(csize);
      recv(dataSocketFD, &cdata, size, 0);
      fwrite(cdata, size, 1, fp);
      fclose(fp);
    }

    //help
    if(splitted[0] == "help" && std::string(cresponse).substr(0, 3) == "226") {
      int size;
      recv(dataSocketFD, &csize, 1024, 0);
      size = atoi(csize);
      recv(dataSocketFD, &cdata, size, 0);
      printf("%s\n", cdata);
    }
  }
  close(commandSocketFD);
  close(dataSocketFD);
  return 0;
}

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

int initiateConnection(int port) {
  int socketFD;
  struct sockaddr_in socketAddress;

  printf("creating socket on localhost::%d\n", port);

  //create socket
  if((socketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Error: command socket creation failed\n");
    exit(EXIT_FAILURE);
  }

  //configure addresses
  socketAddress.sin_family = AF_INET;
  socketAddress.sin_port = htons(port);

  if(inet_pton(AF_INET, "127.0.0.1", &socketAddress.sin_addr) <= 0) {
    printf("Error: invalid address: command socket address not supported\n");
    exit(EXIT_FAILURE);
  }

  //connect to the server
  if (connect(socketFD, (struct sockaddr *)&socketAddress, sizeof(socketAddress)) < 0) {
    printf("Error: socket connection failed\n");
    exit(EXIT_FAILURE);
  }

  printf("creating socket done\n");
  return socketFD;
}