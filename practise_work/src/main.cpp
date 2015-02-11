#include <iostream>
#include <fstream>
#include <string>

#include "parparser.h"
#include "qgen.h"
#ifdef CGEN
    #include "cgen.h"
#endif

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
            BASETYPE fitness = totalBest.getFitness();

            if ( !m_outFile.empty() )
            {
                std::ofstream oStr;              
                oStr.open( m_outFile.c_str(), std::ofstream::out | std::ofstream::app );
                oStr << fitness << "\n";
                oStr.close();
            }
            else
            {            
                std::cout << fitness << "\n";
                std::cout.flush();
            }
        }
    }

    void setOutFile( const std::string& outFile )
    {
        m_outFile = outFile;
    }

private:
    std::string m_outFile;
};

//------------------------------------------------------------

#ifdef CGEN

class CScreen: public CGen::CProcessScreen
{
public:
    void operator()( long long cycle, const CGen::CIndivid& individ )
    {
        std::cout << individ.getFitness() << "\r\n";
        std::cout.flush();
    }
};

#endif

//------------------------------------------------------------
int main( int argc, char**argv )
{
	parparser arguments( argc, argv ); 
    const char* xmlFile = arguments.get( "xml" ).asString(0);
#ifdef CGEN 
    bool useCGen        = arguments.get( "cgen" ).asBool( false );
#endif

    try
    {    
    #ifdef CGEN 
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
    #endif
            QScreen screenClass;
            QOneMaxFitness fClass;
            QGen::SQGenParams settings( xmlFile, &fClass, 0, &screenClass );

            screenClass.setOutFile( settings.outFile );

            double processTime = 0.0;
            QGen::QGenProcess process( settings );
            processTime = process.process();
            if ( process.isMaster() )
            {
                if ( !settings.outFile.empty() )
                {
                    std::ofstream oStr;
                    oStr.open( settings.outFile.c_str(), std::ofstream::out | std::ofstream::app );
                #ifdef GPU
                    oStr << "TOTAL TIME (QGEN-" << (settings.gpu ? "GPU" : "CPU") << "): " << processTime << "\r\n";
                #else
                    oStr << "TOTAL TIME (QGEN-CPU): " << processTime << "\r\n";
                    oStr.flush();
                #endif
                    oStr.close();
                }
                else
                {
                #ifdef GPU
                    std::cout << "TOTAL TIME (QGEN-" << (settings.gpu ? "GPU" : "CPU") << "): " << processTime << "\r\n";
                #else
                    std::cout << "TOTAL TIME (QGEN-CPU): " << processTime << "\r\n";
                    std::cout.flush();
                #endif
                }
            }
    #ifdef CGEN  
        }  
    #endif   
    }
    catch( std::string err )
    {
        std::cout << "ERROR OCCURED:\r\n    " << err << "\r\n";
        std::cout.flush();
    }

    return 0;
}
