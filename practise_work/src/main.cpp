#include <iostream>
#include <fstream>
#include <string>

#include "qgen.h"
#include "parparser.h"
#include "qap.h"

//------------------------------------------------------------

class QScreen: public QGen::IScreen
{
public:
    void operator()( long long cycle, 
                     const int coords[2], 
         const QGen::QBaseIndivid& totalBest, 
         const QGen::QBaseIndivid& iterBest )
    {
        if ( coords[0] == 0 && coords[1] == 0 )
        {
            BASETYPE fitness = totalBest.getFitness();
          
            std::cout << fitness << "\n";
            std::cout.flush();
        }
    }
};

//------------------------------------------------------------
int main( int argc, char**argv )
{
    MPI_Init( &argc, &argv );

	parparser arguments( argc, argv ); 
    const char* xmlFile = arguments.get( "xml" ).asString(0);

    try
    {    
        QScreen screenClass;
        QAPFitness fClass;
        QGen::SParams params( MPI_COMM_WORLD, xmlFile, &fClass, 0, &screenClass );

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

    MPI_Finalize();

    return 0;
}

//------------------------------------------------------------
