#ifndef BASIC_SREEN_H
#define BASIC_SREEN_H

#include "qgen.h"

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
#endif
