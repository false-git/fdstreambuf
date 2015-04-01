#include "fdstreambuf.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <list>
#include <thread>

static std::list<std::iostream *> streamList;

static void wall(const std::string& line) {
    std::cout << line << std::endl;
    for (auto s : streamList) {
	(*s) << line << std::endl;
    }
}

static void handleClient(int fd, std::string remote) {
    wl::fdstreambuf sbuf(fd);
    std::iostream io(&sbuf);
    streamList.push_back(&io);
    while (true) {
	std::string line;
	std::getline(io, line);
	if (io.fail()) {
	    break;
	}
	wall(line);
    }
    streamList.remove(&io);
    close(fd);
    std::cout << "Disconnect from " << remote << std::endl;
}

int main(int argc, char *argv[]) {
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in me;
    std::memset(&me, 0, sizeof(me));
    me.sin_family = PF_INET;
    me.sin_port = htons(9876);
    me.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, reinterpret_cast<struct sockaddr *>(&me), sizeof(me))) {
	perror("bind");
	exit(1);
    }
    if (listen(sockfd, 1)) {
	perror("listen");
	exit(1);
    }
    while (true) {
	struct sockaddr_in dst;
	socklen_t size = sizeof(dst);
	int cfd = accept(sockfd, reinterpret_cast<struct sockaddr *>(&dst), &size);
	std::string remote(inet_ntoa(dst.sin_addr));
	std::cout << "Connect from " << remote << std::endl;
	auto th = std::thread(handleClient, cfd, remote);
	th.detach();
    }
    return 0;
}
