#ifndef QAP_H
#define QAP_H

#include "parparser.h"
#include "qgen.h"
#include "mpicheck.h"
#include "basic_screen.h"
#include <math.h>

//------------------------------------------------------------

BASETYPE log2( BASETYPE arg ) { return logf( arg ) / logf(2); }
BASETYPE roundUpToPowOfTwo( BASETYPE arg ) { return powf( 2, ceil( log2( arg ) ) ); }

//------------------------------------------------------------

class QuadraticAssignmentProblem : public QGen::IFitness
                                 , public QGen::IRepair
{
public:
    QuadraticAssignmentProblem( int problemSize )
        : m_convertedObservState(0)
        , m_occupiedPlaces(0)
    {
        if ( problemSize <= 0 )
            throw std::string( "Invalid problem size. " ).append( __FUNCTION__ );

        m_convertedObservState = new unsigned char[ problemSize ];
        m_occupiedPlaces = new int[ problemSize ];
        m_placesNum = problemSize;
        m_bytesPerPlace = int( ceil( log2( float( problemSize ) ) ) );
    }

    ~QuadraticAssignmentProblem()
    {
        delete[] m_convertedObservState;
        delete[] m_occupiedPlaces;
    }

    //--------------------------------------------------------

        // QGen::IFitness

    virtual BASETYPE operator()( MPI_Comm indComm, const QGen::QObserveState& observeState, long long startQBit, int idx )
    {       
        long long sum = 0;
        for ( int i = 0; i < m_placesNum; ++i )
            sum += (m_convertedObservState[i] + 1) * (i + 1);

        return BASETYPE( sum );
    }

        // QGen::IRepair

    virtual void operator()( MPI_Comm indComm, QGen::QObserveState& observeState, long long startQBit, int idx )
    {
        memset( m_convertedObservState, 0, m_placesNum * sizeof( *m_convertedObservState ) );
        memset( m_occupiedPlaces, 0, m_placesNum * sizeof( *m_occupiedPlaces ) );

        int observeStatePos = 0;
        for ( int i = 0; i < m_placesNum; ++i )
        {
            unsigned char& current = m_convertedObservState[i];

            for ( int q = 0; q < m_bytesPerPlace; ++q )
            {
                current <<= 1;
                current |=  observeState.at( observeStatePos++ ) ? 1 : 0;
            }

            if ( current < m_placesNum )
                ++m_occupiedPlaces[ current ];
        }

        unsigned emptyPlacePos = 0;
        for ( int i = 0; i < m_placesNum; ++i )
        {
            unsigned char& current = m_convertedObservState[i];
            if ( current >= m_placesNum )
            {
                while ( m_occupiedPlaces[ emptyPlacePos ] != 0 )
                    ++emptyPlacePos;
                                
                ++m_occupiedPlaces[ emptyPlacePos ];                
                current = emptyPlacePos; 
            } else if ( m_occupiedPlaces[ current ] > 1 )
            {
                --m_occupiedPlaces[ current ];
                while ( m_occupiedPlaces[ emptyPlacePos ] != 0 )
                    ++emptyPlacePos;

                ++m_occupiedPlaces[ emptyPlacePos ];                
                current = emptyPlacePos; 
            }
        }
    }

    //--------------------------------------------------------

private:
    int m_placesNum;
    int* m_occupiedPlaces;
    unsigned char* m_convertedObservState;
    int m_bytesPerPlace;
};

//------------------------------------------------------------

int qap_main( parparser& args )
{
    const char* xmlFile = args.get( "xml" ).asString(0);

    try
    {            
        QGen::SParams params( MPI_COMM_WORLD, xmlFile );
        params.indSize = int( ceilf( log2( float( params.problemSize ) ) ) ) * params.problemSize;

        QScreen screenClass;
        QuadraticAssignmentProblem workClass( params.problemSize );
        params.fClass = &workClass;
        params.repClass = &workClass;
        params.screenClass = &screenClass;

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

    return 0;
}

//------------------------------------------------------------
#endif
