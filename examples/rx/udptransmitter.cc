#include "udptransmitter.h"

#ifndef _WIN32
#include "udputils.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <cstring>

#include <iostream>
using namespace std;

UDPTransmitter::UDPTransmitter(uint16_t port, std::string host, std::string multicastInterface )
    :_host(host), _port(port), _multicastInterface( multicastInterface )
{
#ifdef _WIN32
    if (!_udpsock.Create() || !_udpsock.Bind(0))
#else
    if (( _fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
#endif
    {
        cerr << "Failed to open UDP socket." << endl;
    }

#ifndef _WIN32
    struct sockaddr_in groupSock;
    memset((char *) &groupSock, 0, sizeof(groupSock));
    groupSock.sin_family = AF_INET;
    groupSock.sin_addr.s_addr = inet_addr(_host.c_str());
    groupSock.sin_port = htons(_port);

    if( isMulticastAddress( _host ) )
    {
        struct in_addr localInterface;
        std::string intIp = getNetworkInterfaceIP( _multicastInterface );
        localInterface.s_addr = inet_addr(intIp.c_str());
        if(setsockopt(_fd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
        {
            cerr << "setsockopt Multicast failed on interface " << _multicastInterface << " IP_ADDR=" << intIp << endl;
        }
        else
        {
            cerr << "Setting multicast to " << _host << " on interface " << _multicastInterface << " : OK - IP_ADDR = " << intIp << endl;
        }
    }
    else
    {

    }
#endif
}

UDPTransmitter::~UDPTransmitter()
{
#ifndef _WIN32
    if( _fd >= 0 ){
        close(_fd);
    }
#endif
}

void UDPTransmitter::send(uint8_t *data, uint32_t data_len)
{
#ifndef _WIN32
    struct sockaddr_in groupSock;

    int s, i, slen=sizeof(groupSock);
    char buf[8192];

    memset((char *) &groupSock, 0, sizeof(groupSock));
    groupSock.sin_family = AF_INET;

    // if( isMulticastAddress( _host ) )
    groupSock.sin_addr.s_addr = inet_addr( _host.c_str() );
    groupSock.sin_port = htons(_port);
    if (inet_aton(_host.c_str(), &groupSock.sin_addr)==0) {
        fprintf(stderr, "inet_aton() failed\n");
    }

    if (sendto(_fd, data, data_len, 0, (const struct sockaddr*)(&groupSock), slen)==-1)
    {

    }
#else
    cat::NetAddr addr;
    if (addr.SetFromString(_host.c_str(), _port))
    {
        sendto(_udpsock.GetSocket(), (const char*)data, data_len, 0, (const sockaddr*)&addr._ip, sizeof(sockaddr));
    }
#endif
}
