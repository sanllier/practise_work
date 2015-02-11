#ifndef ONEMAX_H
#define ONEMAX_H

#include "qgen.h"
#ifdef CGEN
    #include "cgen.h"
#endif
#include "mpicheck.h"

//------------------------------------------------------------

class QOneMaxFitness : public QGen::QFitnessClass
{
public:
    QOneMaxFitness() {}
    ~QOneMaxFitness() {}

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

#ifdef CGEN

class COneMaxFitness : public CGen::CFitnessClass
{
public:
    COneMaxFitness() {}
    ~COneMaxFitness() {}

    virtual BASETYPE operator()( const CGen::CIndivid& individ )
    {
        BASETYPE sum = BASETYPE(0);
        for ( int i = 0; i < (int)( individ.size() ); ++i )
            if ( individ.data()[i] )
                ++sum;

        return sum;
    }
};

#endif

//------------------------------------------------------------
#endif
