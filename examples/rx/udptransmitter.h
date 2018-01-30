#pragma once

#include <string>
#include <cstdint>

#ifdef _WIN32
#include "sockets.h"
#endif

class UDPTransmitter
{
public:
    UDPTransmitter( uint16_t port, std::string host, std::string multicastInterface="" );
    virtual ~UDPTransmitter();

    virtual void send(uint8_t *data, uint32_t data_len);

private:
#ifdef _WIN32
    cat::UDPSocket _udpsock;
#else
    int _fd = -1;
#endif
    uint16_t _port;
    std::string _host;
    std::string _multicastInterface;

protected:

};
