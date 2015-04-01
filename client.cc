#include "fdstreambuf.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <thread>

static void handleServer(std::iostream *io) {
    std::string data;
    while (true) {
	std::getline(*io, data);
	if (io->fail()) {
	    break;
	}
	std::cout << data << std::endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
	std::cout << "Usage: client server_host your_name" << std::endl;
	exit(1);
    }
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in dst;
    std::memset(&dst, 0, sizeof(dst));
    dst.sin_family = PF_INET;
    dst.sin_port = htons(9876);
    struct hostent *hp;
    hp = gethostbyname(argv[1]);
    std::memcpy(&dst.sin_addr, hp->h_addr, hp->h_length);
    connect(sockfd, (struct sockaddr *)&dst, sizeof(dst));
    wl::fdstreambuf sbuf(sockfd);
    std::iostream io(&sbuf);
    auto th1 = std::thread(handleServer, &io);
    while (!io.fail()) {
	std::string data;
	std::getline(std::cin, data);
	if (std::cin.fail()) {
	    std::cout << "Finish" << std::endl;
	    close(sockfd);
	    break;
	}
	io << argv[2] << ": " << data << std::endl;
    }
    th1.join();
    return 0;
}
