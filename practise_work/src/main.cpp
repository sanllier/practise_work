#include <iostream>
#include <cstdlib>
#include <ctime>

#include "qgen.h"

//------------------------------------------------------------

class Fitness : public QGen::QFitnessClass
{
public:
    virtual BASETYPE operator()( const QGen::QObservState& observState )
    {
        unsigned int val = 0;
        for ( int i = 0; i < (int)observState.size(); ++i )
        {
            val |= observState.at(i) ? 0x1 : 0x0;
            val <<= 1;
        }
        val >>= 1;

        const float x = 1.0f / float( val );
        if ( x > 1.0f || x < 0.01f )
            return -1000.0f;

        return 10.0f + sinf( 1.0f / x ) / ( ( x - 0.16f ) * ( x - 0.16f ) + 0.1f );
    }
};

//------------------------------------------------------------

int main( int argc, char**argv )
{
    srand( time(0) );

    try
    {
        Fitness fClass;
        QGen::QGenProcessSettings settings;
        settings.cycThreshold = 1000;
        settings.fClass = &fClass;
        settings.individsNum = 10;
        settings.indSize = 8;
        settings.timeThreshold = 1000;
        settings.catastropheThreshold = 5;
        settings.targetFitness = 19.8949;
        settings.accuracy = 0.5f;

        QGen::QGenProcess process( settings );
        process.process();

        const QGen::QIndivid bestInd = process.getBestIndivid();
        if ( process.isMaster() )
            for ( int i = 0; i < (int)bestInd.qsize(); ++i )
                std::cout << "(" << bestInd.at(i).a << ", " << bestInd.at(i).b << ")\r\n"; 
    }
    catch( std::string err )
    {
        std::cout << "ERROR OCCURED:\r\n    " << err << "\r\n";
        std::cout.flush();
    }

    return 0;
}
