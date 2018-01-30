#pragma once

#include <string>

#ifdef _WIN32
#include "sockets.h"
#endif

class UDPReceiver
{
public:
    UDPReceiver(uint16_t port, std::string networkInterface="", std::string multicastGroup="" , int timeout = 1000 );
    virtual ~UDPReceiver();

    virtual int receive(uint8_t *buffer, uint32_t bufferSize);

private:
    int _timeout;
#ifdef _WIN32
    cat::UDPSocket _udpsock;
#else
    int _fd = -1;
#endif
    uint16_t _port;
    std::string _address;
    std::string _networkInterface;

protected:

};
