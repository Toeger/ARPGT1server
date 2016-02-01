#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <array>
#include <cassert>
#include "Utility/asserts.h"

template <class Tout, class Tin>
Tout *any_cast(Tin *p){
	void *vp = p;
	return static_cast<Tout *>(vp);
}

namespace Config{
	const std::size_t MAX_UDP_PAYLOAD = 512;
	const short int port = 12345;
}

void handle(std::array<unsigned char, Config::MAX_UDP_PAYLOAD> &buffer, int size, const char *address){
	(void)buffer;
	(void)size;
	std::cout << size << " bytes from " << address << " with content: " << std::string(buffer.data(), buffer.data() + size) << '\n' << std::flush;
}

void reader(int fd){
	std::array<unsigned char, Config::MAX_UDP_PAYLOAD> buffer;
	for(;;){
		assert_fast(buffer.size() == Config::MAX_UDP_PAYLOAD);
		sockaddr_in sender = {};
		socklen_t length = sizeof sender;
		auto size = recvfrom(fd, buffer.data(), buffer.size(), 0, any_cast<sockaddr>(&sender), &length);
		if (size == -1){ //error
			perror("recvfrom error");
			continue;
		}
		//data recieved
		handle(buffer, size, inet_ntoa(sender.sin_addr));
	}
}

int main()
{
	int fd;
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		perror("cannot create socket");
		return 0;
	}
	sockaddr_in in = {};
	in.sin_addr.s_addr = htonl(INADDR_ANY);
	in.sin_family = AF_INET;
	in.sin_port = htons(Config::port);

	if (bind(fd, any_cast<sockaddr>(&in), sizeof in) < 0){
		perror("bind failed");
		return 0;
	}

	reader(fd);
}
