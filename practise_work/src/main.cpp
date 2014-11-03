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

        float fit = -1 * (val * val) + 100.0f;

        return fit < 0.0f ? 0.0f : fit;
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
        settings.catastropheThreshold = 4;
        //settings.targetFitness = 65536;

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
