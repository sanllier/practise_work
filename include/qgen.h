#ifndef QGEN_H
#define QGEN_H

#include <complex>
#include <vector>

#include "mpi.h"

#include "defs.h"
#include "qobservstate.h"
#include "qindivid.h"

//------------------------------------------------------------

namespace QGen {
//------------------------------------------------------------

class QFitnessClass
{
public:
    virtual ~QFitnessClass() {}
    virtual BASETYPE operator()( const QObservState& observState ) = 0;
};

//------------------------------------------------------------

struct QGenProcessSettings
{
    long long cycThreshold;
    double timeThreshold;
    int individsNum;
    int indSize;
    QFitnessClass* fClass;

    QGenProcessSettings()
        : cycThreshold(0)
        , timeThreshold( 0.0 )
        , individsNum(0)
        , indSize(0)
        , fClass(0) {}
};

class QGenProcess
{
public:
    QGenProcess( const QGenProcessSettings& settings, MPI_Comm comm = MPI_COMM_WORLD );
    ~QGenProcess() {}

    inline bool active() const { return m_active; }
    void process();

private:
    void findBest();

private:
    bool m_active;

    static int m_instancesCount;
    MPI_Comm m_comm;
    int m_myID;
    int m_commSize;

    std::vector< QIndivid > m_Individs;
    int m_indsNumPerProc;

    QGenProcessSettings m_settings;

    struct Solution
    {
        int procRank;
        int localNum;
    } m_bestSolution;
};

//-----------------------------------------------------------
}

//------------------------------------------------------------
#endif
