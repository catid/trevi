#ifdef _WIN32
#define NOMINMAX /* Do not let windows.h stomp on std::max */
#include "sockets.h"
#endif

#include <iostream>
#include <trevi.h>

#include "udpreceiver.h"
#include "udptransmitter.h"
#include "Timer.h"
#include "cmdline.h"

using namespace std;

int main( int argc, char** argv )
{
    cmdline::parser a;

    a.add<int>("input_port", 'i', "UDP port for encoded input data", false, 5001 );
    a.add<int>("output_port", 'o', "UDP port for decoded output data", false, 5000 );
    a.add<string>("output_host", 'h', "address of destination host for decoded data", false, "127.0.0.1" );
    a.add<int>("window_size", 'd', "Decoding window size (must be strictly superior to encoding window size)", false, 64 );

    a.parse_check(argc, argv);

    int udpInputPort = a.get<int>("input_port");
    int udpOutputPort = a.get<int>("output_port");
    string udpOutputHost = a.get<string>("output_host");

    cerr << "Starting Trevi UDP decoder: " << endl;
    cerr << "UDP input port for encoded data: \t\t\t" << udpInputPort << endl;
    cerr << "UDP output for decoded data: \t\t\t" << udpOutputHost << ":" << udpOutputPort << endl;
#ifdef _WIN32
    ::Sleep(2000);
    cat::Sockets::OnInitialize();
#else
    sleep(2);
#endif

    trevi_init();

    UDPReceiver udpr( udpInputPort );
    UDPTransmitter udpt( udpOutputPort, udpOutputHost );

    int dsize = 0;
    unsigned int pktIdx;

    uint8_t buffer[ 2048 ];

    trevi_decoder * decoder = trevi_create_decoder();
    trevi_decoder_add_stream( decoder, 0, a.get<int>("window_size") );

    double t_sum = 0.0;
    int iterCpt = 0;

    // Unblock the transmitter - will get ignored because stream id is non-zero
    for (unsigned i = 0; i < 2048; ++i)
        buffer[i] = 0xff;
    udpt.send(buffer, 1400);

    for(;;)
    {
        int rsize = udpr.receive( buffer, 2048 );
        if( rsize > 0)
        {
            // cerr << "rx size=" << rsize << endl;
            Timer t;
            t.start();
            trevi_decode( decoder, buffer, rsize );
            t.stop();
            t_sum += t.getElapsedTimeInMicroSec();

            while( true )
            {
                dsize = trevi_decoder_get_decoded_data( decoder, buffer, &pktIdx );
                if( dsize < 0 )
                {
                    break;
                }
                else
                {
                    // cerr << "dsize=" << dsize << " pktIdx=" << pktIdx << endl;
                    udpt.send( buffer, dsize );
                }
            }
        }

        iterCpt++;

        if( iterCpt % 1000 == 0 )
        {
            double t_decode = t_sum / (double)iterCpt;
            cerr << "Average decode processing time = " << t_decode << " microsec." << endl;
        }

        dump_profiling_info();

    }

    return 0;
}
