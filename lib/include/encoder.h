#pragma once

#include <map>

#include <cstdint>

#include "streamencoder.h"

class Encoder
{
public:
    Encoder();
    virtual ~Encoder();

    void addStream( int stremaId, StreamEncoder* encoder );
    void removeStream( int streamId );

    void addData( int streamId, std::shared_ptr<SourceBlock> sb );
    void addData( int streamId, const void* buffer, int bufferSize );

    bool hasEncodedBlocks();
    std::shared_ptr< CodeBlock > getEncodedBlock();

private:
    std::map< uint8_t, StreamEncoder* > _encoders;
    uint32_t _curGlobalIdx;

protected:

};
