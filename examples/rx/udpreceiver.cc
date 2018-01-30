#include "udpreceiver.h"
#ifndef _WIN32
#include "udputils.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <poll.h>

#include <unistd.h>
#endif
#include <cstring>

#include <iostream>

using namespace std;

#undef USE_LOGGING

UDPReceiver::UDPReceiver( uint16_t port, const std::string& networkInterface, const std::string& multicastGroup, int timeout )
    :_address(multicastGroup), _port(port), _networkInterface(networkInterface), _timeout(timeout)
{
#ifdef _WIN32
    if (_udpsock.Create())
#else
    if ((_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
#endif
    {
#ifdef USE_LOGGING
        LOG_S(ERROR) << "Error opening datagram socket.";
#endif
    }

    {
#ifndef _WIN32
        int reuse = 1;
        if(setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
        {
#ifdef USE_LOGGING
            LOG_S(INFO) << "Setting SO_REUSEADDR...ERROR.";
#endif
            close(_fd);
        }
        else
        {
#ifdef USE_LOGGING
            LOG_S(INFO) << "Setting SO_REUSEADDR...OK.";
#endif
        }
#endif

#ifndef _WIN32
        /* Bind to the proper port number with the IP address */
        /* specified as INADDR_ANY. */
        struct sockaddr_in localSock;
        memset((char *) &localSock, 0, sizeof(localSock));
        localSock.sin_family = AF_INET;
        localSock.sin_port = htons(_port);
        localSock.sin_addr.s_addr = INADDR_ANY;
        if(bind(_fd, (struct sockaddr*)&localSock, sizeof(localSock)))
#else
        if (_udpsock.Bind(_port))
#endif
        {
#ifdef USE_LOGGING
            LOG_S(ERROR) << "Binding datagram socket ERROR";
#endif
#ifndef _WIN32
            close(_fd);
#endif
        }
        else
        {
#ifdef USE_LOGGING
            LOG_S(INFO) << "Binding datagram socket...OK.";
#endif
        }

#ifndef _WIN32
        // Joining multicast group if needed...
        if( isMulticastAddress( _address ) && _address.size() )
        {
            struct ip_mreq group;
            group.imr_multiaddr.s_addr = inet_addr( _address.c_str() );
            if( _networkInterface.size() > 0 )
            {
                group.imr_interface.s_addr = inet_addr( getNetworkInterfaceIP(_networkInterface).c_str() );
#ifdef USE_LOGGING
                LOG_S(INFO) << "Using interface " << _networkInterface;
#endif
            }
            else
            {
                group.imr_interface.s_addr = inet_addr( "0.0.0.0" );
#ifdef USE_LOGGING
                LOG_S(INFO) << "Using interface " << "ANY";
#endif
            }

            if(setsockopt( _fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
            {
#ifdef USE_LOGGING
                LOG_S(ERROR) << "ERROR adding multicast group";
#endif
                close(_fd);
            }
            else
            {
#ifdef USE_LOGGING
                LOG_S(INFO) << "Adding multicast group " << _address << " ...OK.";
#endif
            }
        }
        else
        {

        }
#endif
    }

}

UDPReceiver::~UDPReceiver()
{
    cerr << "UDPReceiver::~UDPReceiver()" << endl;
#ifndef _WIN32
    if( _fd >= 0 )
        close(_fd);
#endif
}

int UDPReceiver::receive(uint8_t *buffer, uint32_t bufferSize)
{
#ifndef _WIN32
    char buf[ 2048 ];
    struct sockaddr_in si_other;
    int slen=sizeof(si_other);

    int rlen = recv( _fd, buf, 1400, 0 );
    if( rlen > 0 )
    {
        memcpy( buffer, buf, rlen );
        return rlen;
    }
    return -1;
#else
    return recv(_udpsock.GetSocket(), (char*)buffer, bufferSize, 0);
#endif
}
