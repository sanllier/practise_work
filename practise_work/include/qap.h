#ifndef QAP_H
#define QAP_H

#include "qgen.h"
#include "mpicheck.h"
#include <math.h>

//------------------------------------------------------------

class QAPFitness : public QGen::IFitness
{
public:
    void init( int size )
    {
        m_size = int( log2( size ) );
        m_bits = 
        m_buf = new char[ int( log2( size ) ) ];
    }

    ~QAPFitness()
    {
        delete[] m_buf;
    }

    virtual BASETYPE operator()( MPI_Comm indComm, const QGen::QObserveState& observeState, long long startQBit, int idx )
    {
        for (  )

        int sum = 0;
        int factor = startQBit / sizeof(int) + 1;
        for ( int i = 0; i < (int)observeState.size() / (sizeof(int) * 8); ++i )
            sum += (factor++) * data[i];

        BASETYPE total = BASETYPE(0); 
        CHECK( MPI_Allreduce( &sum, &total, 1, MPI_BASETYPE, MPI_SUM, indComm ) );

        return total;
    }

private:
    BASETYPE log2( BASETYPE arg ) const
    {
        return logf( arg ) / logf(2);
    }

private:
    char* m_buf;
    int m_size;
};

//------------------------------------------------------------

class QAPRepair : public QGen::IRepair
{
public:
    virtual void operator()( MPI_Comm indComm, QGen::QObserveState& observeState, long long startQBit, int idx )
    {

    }
};


//------------------------------------------------------------
#endif
