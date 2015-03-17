#ifndef ONEMAX_H
#define ONEMAX_H

#include "parparser.h"
#include "qgen.h"
#include "basic_screen.h"
#include "mpicheck.h"

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
        QScreen screenClass;
        OneMaxProblem workClass;

        QGen::SParams params( MPI_COMM_WORLD, xmlFile, &workClass, 0, &screenClass );
        params.indSize = params.problemSize;

        double processTime = 0.0;
        QGen::QGenProcess process( params );
        processTime = process.process();
        if ( process.isMaster() )
        {
            std::cout << "TOTAL TIME (QGEN-CPU): " << processTime << "\r\n";
            std::cout.flush();
        }
    }
    catch( std::string err )
    {
        std::cout << "ERROR OCCURED:\r\n    " << err << "\r\n";
        std::cout.flush();
    }

    return 0;
}

//------------------------------------------------------------
#endif
