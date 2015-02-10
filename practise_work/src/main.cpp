#include <iostream>

#include "parparser.h"
#include "qgen.h"
#include "cgen.h"

#include "onemax.h"

//------------------------------------------------------------

class QScreen: public QGen::QProcessScreen
{
public:
    void operator()( long long cycle, 
                     const int coords[2], 
         const QGen::QBaseIndivid& totalBest, 
         const QGen::QBaseIndivid& iterBest )
    {
        if ( coords[0] == 0 && coords[1] == 0 )
        {
            BASETYPE test = totalBest.getFitness();
            std::cout << test << "\r\n";
            std::cout.flush();
        }
    }
};

//------------------------------------------------------------

class CScreen: public CGen::CProcessScreen
{
public:
    void operator()( long long cycle, const CGen::CIndivid& individ )
    {
        std::cout << individ.getFitness() << "\r\n";
        std::cout.flush();
    }
};

//------------------------------------------------------------
int main( int argc, char**argv )
{
	parparser arguments( argc, argv ); 
    const char* xmlFile = arguments.get( "xml" ).asString(0);
    bool useCGen        = arguments.get( "cgen" ).asBool( false );

    try
    {     
        if ( useCGen )
        {
            CScreen screenClass;
            COneMaxFitness fClass;
            CGen::SCGenParams settings( xmlFile, &fClass, 0, &screenClass );
                        
            double processTime = 0.0;
            CGen::CGenProcess process( settings );
            processTime = process.process();
            std::cout << "TOTAL TIME (CGEN): " << processTime << "\r\n";
            std::cout.flush();
        }
        else
        {
            QScreen screenClass;
            QOneMaxFitness fClass;
            QGen::SQGenParams settings( xmlFile, &fClass, 0, &screenClass );

            double processTime = 0.0;
            QGen::QGenProcess process( settings );
            processTime = process.process();
            if ( process.isMaster() )
            {
            #ifdef GPU
                std::cout << "TOTAL TIME (QGEN-" << (settings.gpu ? "GPU" : "CPU") << "): " << processTime << "\r\n";
            #else
                std::cout << "TOTAL TIME (QGEN-CPU): " << processTime << "\r\n";
                std::cout.flush();
            #endif
            }
        }     
    }
    catch( std::string err )
    {
        std::cout << "ERROR OCCURED:\r\n    " << err << "\r\n";
        std::cout.flush();
    }

    return 0;
}
