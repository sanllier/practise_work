#ifndef BASIC_SREEN_H
#define BASIC_SREEN_H

#include "qgen.h"

#include <fstream>

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

    void printString( const char* str )
    {
        std::cout << str << "\n";
        std::cout.flush();
    }
};

//------------------------------------------------------------

class QFileScreen: public QGen::IScreen
{
public:
    QFileScreen( const char* fileName )
    {
        if ( !fileName || !fileName[0] )
            throw std::string( "Invalid out file name. " ).append( __FUNCTION__ ); 

        oFstr.open( fileName, std::fstream::out );
        if ( !oFstr.good() )
            throw std::string( "Invalid out file. " ).append( __FUNCTION__ );
    }
    ~QFileScreen() 
    {
        oFstr.close();
    }

    void operator()( long long cycle, 
                     const int coords[2], 
         const QGen::QBaseIndivid& totalBest, 
         const QGen::QBaseIndivid& iterBest )
    {
        if ( coords[0] == 0 && coords[1] == 0 )
        {
            BASETYPE fitness = totalBest.getFitness();
          
            oFstr << fitness << "\n";
            oFstr.flush();
        }
    }

    void printString( const char* str )
    {
        oFstr << str << "\n";
        oFstr.flush();
    }

private:
    std::ofstream oFstr;
};

//------------------------------------------------------------
#endif
