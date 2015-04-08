#ifndef ONEMAX_H
#define ONEMAX_H

#include "parparser.h"
#include "qgen.h"
#include "basic_screen.h"
#include "mpicheck.h"

#include <string>
#include <sstream>

//------------------------------------------------------------

class OneMaxProblem : public QGen::IFitness
{
public:
        // QGen::IFitness

    virtual BASETYPE operator()( MPI_Comm indComm, const QGen::QObserveState& observeState, long long startQBit, int idx )
    {
        BASETYPE sum = BASETYPE(0);
        for ( int i = 0; i < (int)( observeState.size() ); ++i )
            if ( observeState.at(i) )
                ++sum;

        BASETYPE total = BASETYPE(0); 
        CHECK( MPI_Allreduce( &sum, &total, 1, MPI_BASETYPE, MPI_SUM, indComm ) );

        return total;
    }
};

//------------------------------------------------------------

int onemax_main( parparser& args )
{
    const char* xmlFile = args.get( "xml" ).asString(0);

    try
    {
        OneMaxProblem workClass;
        QGen::SParams params( MPI_COMM_WORLD, xmlFile, &workClass, 0, 0 );

        QFileScreen screenClass( params.outFile.c_str() );
        params.indSize = params.problemSize;
        params.screenClass = &screenClass;
  
        QGen::QGenProcess process( params );
        double processTime = process.process();
        if ( process.isMaster() )
        {
            std::stringstream sStr;
            sStr << "TOTAL TIME (QGEN-CPU): " << processTime;
            screenClass.printSStream( sStr );
        }
    }
    catch( std::string err )
    {
        std::cerr << "ERROR OCCURED:\n    " << err << "\n";
        std::cerr.flush();
    }

    return 0;
}

//------------------------------------------------------------
#endif
