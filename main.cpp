#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <array>
#include <cassert>
#include <vector>
#include <string>

#include "Utility/asserts.h"
#include "ECS/entity.h"
#include "ECS/entity_handle.h"

namespace Config{
	const std::size_t MAX_UDP_PAYLOAD = 512;
	const short int port = 12345;
}

template <class Tout, class Tin>
Tout *any_cast(Tin *p){
	void *vp = p;
	return static_cast<Tout *>(vp);
}

bool operator ==(in_addr lhs, in_addr rhs){
	return lhs.s_addr == rhs.s_addr;
}

bool operator !=(in_addr lhs, in_addr rhs){
	return lhs.s_addr != rhs.s_addr;
}

bool operator <(in_addr lhs, in_addr rhs){
	return lhs.s_addr < rhs.s_addr;
}

struct Connections{
	using Address = std::pair<in_addr, in_port_t>;
	static ECS::Entity_handle get(Address address){
		auto pos = std::lower_bound(begin(addresses), end(addresses), address);
		if (*pos != address)
			return {};
		return handles[pos - begin(addresses)];
	}
	static void add(Address address, ECS::Entity_handle entity){
		auto pos = std::lower_bound(begin(addresses), end(addresses), address);
		assert_fast(*pos != address);
		handles.insert(pos - begin(addresses) + begin(handles), entity);
		addresses.insert(pos, address);
	}

private:
	static std::vector<Address> addresses;
	static std::vector<ECS::Entity_handle> handles;
};

std::vector<Connections::Address> Connections::addresses;
std::vector<ECS::Entity_handle> Connections::handles;

struct Player : ECS::Entity{
	in_addr address;

};

void handle(std::array<unsigned char, Config::MAX_UDP_PAYLOAD> &buffer, int size, sockaddr_in &sender, int fd){
	std::cout << size << " bytes from " << inet_ntoa(sender.sin_addr)
			  << " with content: " << std::string(buffer.data(), buffer.data() + size) << '\n' << std::flush;
	std::string reply = "Echo: " + std::string(begin(buffer), begin(buffer) + size);
	sendto(fd, reply.data(), reply.size(), 0, any_cast<sockaddr>(&sender), sizeof sender);
	auto entity = Connections::get({sender.sin_addr, sender.sin_port});
	if (!entity){
		//someone who is not logged in
		return;
	}
	//someone who is logged in
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
		handle(buffer, size, sender, fd);
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
