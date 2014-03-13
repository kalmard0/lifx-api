#include "stdafx.h"
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <sstream>
#include "Socket.h"
#include "Packet.h"

using namespace std;

#if 0
uint8_t ntoh(const uint8_t from) { return from; }
uint8_t hton(const uint8_t from) { return from; }
uint16_t ntoh(const uint16_t from) { return ntohs(from); }
uint16_t hton(const uint16_t from) { return htons(from); }
uint32_t ntoh(const uint32_t from) { return ntohl (from); }
uint32_t hton(const uint32_t from) { return htonl(from); }
uint64_t ntoh(const uint64_t from) { return ntohll (from); }
uint64_t hton(const uint64_t from) { return htonll(from); }
#endif


#if 0
std::string LastErrorStr(int error) {
	char* errorText = NULL;

	FormatMessageA(
		// use system message tables to retrieve error text
		FORMAT_MESSAGE_FROM_SYSTEM
		// allocate buffer on local heap for error text
		|FORMAT_MESSAGE_ALLOCATE_BUFFER
		// Important! will fail otherwise, since we're not 
		// (and CANNOT) pass insertion parameters
		|FORMAT_MESSAGE_IGNORE_INSERTS,  
		NULL,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
		error,
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
		(char*)&errorText,  // output 
		0, // minimum size for output buffer
		NULL);   // arguments - see note 

	std::string ret(errorText);

	if ( NULL != errorText )
	{
		// ... do something with the string - log it, display it to the user, etc.

		// release memory allocated by FormatMessage()
		LocalFree(errorText);
		errorText = NULL;
	}

	return ret;
}
#endif

using namespace lifx;

int _tmain(int argc, _TCHAR* argv[])
{
	Socket* socket = Socket::Create("192.168.66.255");
	Packet packet;
	packet.SetType(PacketType::GetPanGateway);
	socket->Send(packet);

	while (true)
	{
		if (socket->Receive(packet)) {
			cout << packet.ToString() << "\n";
		}
	}

	delete socket;

	return 0;
}

