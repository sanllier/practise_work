#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>

#include "parparser.h"
#include "qgen.h"

#include "knapsack.h"
#include "multiknapsack.h"
#include "onemax.h"

//------------------------------------------------------------

const static int RANDOM_TEST_SIZE = 500;

//------------------------------------------------------------

int main( int argc, char**argv )
{
	parparser arguments( argc, argv ); 
	const int cycThreshold = arguments.get( "ct" ).asInt( 1000 );
	const int individsNum = arguments.get( "inds" ).asInt( 1 );
	const int indSize = arguments.get( "indsize" ).asInt( RANDOM_TEST_SIZE );
	const int catastropheThreshold = arguments.get( "catt" ).asInt( 100 );
    const int immigrationThreshold = arguments.get( "immt" ).asInt( 20 );
    const int immigrationSize = arguments.get( "imms" ).asInt( 10 );
    const float timeThreshold = arguments.get( "tt" ).asFloat( 0.0f );
    const float targetFitness = arguments.get( "fit" ).asFloat( 0.0f );
    const float accuracy = arguments.get( "acc" ).asFloat( 0.0f );
    
    srand( (unsigned)( time(0) ) );

    try
    {
        OneMaxFitness fClass;
        //KnapsackFitness fClass;
        //Repair repClass;

        QGen::QGenProcessSettings settings;
        settings.cycThreshold = cycThreshold;
        settings.fClass   = &fClass;
        //settings.repClass = &repClass;
        settings.individsNum   = individsNum;
        settings.indSize       = indSize;
        settings.timeThreshold = timeThreshold;
        settings.catastropheThreshold = catastropheThreshold;
        settings.immigrationThreshold = immigrationThreshold;
        settings.immigrationSize = immigrationSize;
        settings.targetFitness   = targetFitness;
        settings.accuracy = accuracy;

        QGen::QGenProcess process( settings );
        process.process();

        const QGen::QIndivid bestInd = process.getBestIndivid();
        //if ( process.isMaster() )
        //    for ( int i = 0; i < (int)bestInd.qsize(); ++i )
        //        std::cout << "(" << bestInd.at(i).a << ", " << bestInd.at(i).b << ")\r\n"; 
    }
    catch( std::string err )
    {
        std::cout << "ERROR OCCURED:\r\n    " << err << "\r\n";
        std::cout.flush();
    }

    return 0;
}
