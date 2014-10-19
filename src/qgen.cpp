#include "qgen.h"
#include "qrotoperator.h"

#include <string>

#define CHECK( __ERR_CODE__ ) checkMPIRes( __ERR_CODE__, __FUNCTION__ )

//------------------------------------------------------------

namespace QGen {
//------------------------------------------------------------

int QGenProcess::m_processesCount = 0;

enum
{
    ROOT_ID = 0
};

void checkMPIRes( int errCode, const char* location )
{
    if ( errCode != MPI_SUCCESS )
    {
        char errText[ MPI_MAX_ERROR_STRING ];
        int len;
        MPI_Error_string( errCode, errText, &len );
        throw std::string( errText ).append( location );
    }
}

//-----------------------------------------------------------

QGenProcess::QGenProcess( const QGenProcessSettings& settings, MPI_Comm comm/* = MPI_COMM_WORLD*/ )
{
    int isMPIInitialized = 0;
    CHECK( MPI_Initialized( &isMPIInitialized ) );
    if ( !isMPIInitialized )
        CHECK( MPI_Init( 0, 0 ) );

    CHECK( MPI_Comm_rank( comm, &m_myID ) );
    CHECK( MPI_Comm_size( comm, &m_commSize ) );

    m_comm = comm;
    if ( m_myID == ROOT_ID )
    {
        ++m_processesCount;
        CHECK( MPI_Bcast( &m_processesCount, 1, MPI_INT, ROOT_ID, comm ) );
    }
    else
    {
        CHECK( MPI_Bcast( &m_processesCount, 1, MPI_INT, ROOT_ID, comm ) );
    }

    m_indsNumPerProc = settings.individsNum / m_commSize;
    if ( m_indsNumPerProc == 0 )
    {
        if ( m_myID < settings.individsNum )
        {
            m_indsNumPerProc = 1;
            m_commSize = settings.individsNum;
            m_active = true;
        }
        else
        {
            CHECK( MPI_Finalize() );
            m_active = false;
        }
    }

    m_Individs.resize( m_indsNumPerProc );
    for ( auto& ind : m_Individs )
    {
        ind.resize( settings.indSize );
        ind.setInitial();
    }

    m_settings = settings;
    if ( !m_settings.fClass )
        throw std::string( "Invalid fitness class( NULL pointer )." ).append( __FUNCTION__ );
}

BASETYPE func( BASETYPE a, BASETYPE b )
{
    return 0;
}

void QGenProcess::process()
{
    if ( !m_active )
        return;

    double elapsedTime = 0;
    long long catastropheGen = 1;
    double factor = 0.0;
    QRotOperator op;

    for ( long long cycle = 0; cycle < m_settings.cycThreshold; ++cycle )
    {
        factor = 0.5 * ( ( m_settings.cycThreshold - cycle ) / catastropheGen ) / m_settings.cycThreshold;
        for ( auto& ind : m_Individs )
        {
            op.compute( factor * func( 0, 0 ) );
            ind.tick( op );
        }        
    }
}

//------------------------------------------------------------
}
