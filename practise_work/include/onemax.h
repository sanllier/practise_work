#ifndef ONEMAX_H
#define ONEMAX_H

#include "qgen.h"

//------------------------------------------------------------

class OneMaxFitness : public QGen::QFitnessClass
{
public:
    OneMaxFitness() {}
    ~OneMaxFitness() {}

    virtual BASETYPE operator()( const QGen::QObservState& observState )
    {
        float sum = 0.0f;
        for ( int i = 0; i < (int)( observState.size() ); ++i )
            if ( observState.at(i) )
                ++sum;

        return sum;
    }
};

//------------------------------------------------------------
#endif
