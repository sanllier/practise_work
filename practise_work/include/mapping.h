#ifndef MAPPING_H
#define MAPPING_H

#include "parparser.h"
#include "qgen.h"
#include "mpi.h"
#include "mpicheck.h"
#include "basic_screen.h"
#include "helpers.h"

#include <math.h>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <fstream>

//------------------------------------------------------------

static const int SMPCoresPerNode = 4;
static const int partitionTypesNum = 5;
static const int partitionSizes[ partitionTypesNum ] = {128, 256, 512, 1024, 2048};
static const int gridDims[ partitionTypesNum ][4]    = { {4, 4, 8,   16}, 
                                                         {8, 4, 8,   32}, 
                                                         {8, 8, 8,   64}, 
                                                         {8, 8, 16,  64}, 
                                                         {8, 16, 16, 128} };

//------------------------------------------------------------

class MappingProblem : public QGen::IFitness
                     , public QGen::IRepair
{
public:
    MappingProblem()  {}
    ~MappingProblem() {}

    //--------------------------------------------------------

    void ReadDataFile( const char* filename, MPI_Comm comm = MPI_COMM_WORLD )
    {
        int isMPIInitialized = 0;
        CHECK( MPI_Initialized( &isMPIInitialized ) );
        if ( !isMPIInitialized )
            throw std::string( "MPI was not initialized." ).append( __FUNCTION__ );

        if ( !filename || !filename[0] )
            throw std::string( "Invalid data file path. " ).append( __FUNCTION__ );

        MPI_File fp = MPI_FILE_NULL;
        CHECK( MPI_File_open( comm, const_cast<char*>(filename), MPI_MODE_RDONLY, MPI_INFO_NULL, &fp ) );
    
        MPI_Offset fileSize = 0;
        CHECK( MPI_File_get_size( fp, &fileSize ) );
        if ( fileSize <= 0 )
            throw std::string( "Invalid data file. " ).append( __FUNCTION__ );

        std::string buf;
        buf.resize( (unsigned)fileSize );
        MPI_Status status;    
        CHECK( MPI_File_read_at_all( fp, 0, &buf[0], int( fileSize ), MPI_CHAR, &status ) );
        CHECK( MPI_File_close( &fp ) );

        std::stringstream sStream(buf);

        std::string mappingType;
        sStream >> mappingType;
        m_isSMP = 0 == mappingType.compare( "smp" );
        if ( !m_isSMP )
            sStream.seekg(0);

        int lines = 0;
        sStream >> m_facilitiesNum >> m_facilitiesNum >> lines;
        if ( lines <= 0 )
            throw std::string( "Invalid data file format. " ).append( __FUNCTION__ );

        m_data.resize( m_facilitiesNum );

        int from     = 0;
        int to       = 0;
        float weight = 0.0f;
        for ( int i = 0; i < lines; ++i )
        {
            sStream >> from >> to >> weight;

            if ( from >= m_facilitiesNum || to >= m_facilitiesNum || from < 0 || to < 0 )
                throw std::string( "Invalid data file format. " ).append( __FUNCTION__ );

            if ( to == from )
                continue;

            CommData::iterator q = m_data[ from ].begin();
            for ( ; q != m_data[ from ].end(); ++q )
            {
                if ( q->first == to )
                {
                    q->second += fabsf( weight );
                    break;
                }
            }

            if ( q == m_data[ from ].end() )
                m_data[ from ].push_back( std::make_pair( to, fabsf( weight ) ) );
        }

        m_locationsNum = partitionSizes[0];
        m_gridType = 0;

        for ( int i = partitionTypesNum - 1; i >= 0; --i )
        {
            if ( m_facilitiesNum > partitionSizes[i] )
            {
                m_locationsNum = partitionSizes[i];
                m_gridType = i;
            }
        }

        m_bitsPerPlace = int( ceilf( log2( float( m_locationsNum ) ) ) );
        m_problemSize = m_facilitiesNum * m_bitsPerPlace;
        m_occupiedPlaces.resize( m_locationsNum );
        m_reinterpretedObsState.resize( m_facilitiesNum );
    }

    //--------------------------------------------------------

    void SaveMappingFile( const char* filename, QGen::QBaseIndivid& bestInd, const int coords[2] )
    {
        if ( !filename || !filename[0] )
            throw std::string( "Invalid mapping file name. " ).append( __FUNCTION__ );

        if ( coords[0] == 0 )
        {
            if ( !m_isSMP )
                throw std::string( "Non-SMP configuration is not supported. " ).append( __FUNCTION__ );

            (*this)( MPI_COMM_NULL, bestInd.getObservState(), 0, 0 );

            std::ofstream oStr;
            oStr.open( filename, std::ofstream::out );
            if ( !oStr.good() )
                throw std::string( "Invalid mapping file. " ).append( __FUNCTION__ );

            for ( int i = 0; i < m_reinterpretedObsState.size(); ++i )
            {
                GridPos pos = GetGridPos( m_reinterpretedObsState[i] );
                oStr << pos.x << " " << pos.y << " " << pos.z << " 0\n";
            }

            oStr.close();
        }
    }

    //--------------------------------------------------------

        // QGen::IFitness

    virtual float operator()( MPI_Comm indComm, const QGen::QObserveState& observeState, long long startQBit, int idx )
    {       
        double sum = 0.0f;
        for ( size_t i = 0; i < m_data.size(); ++i )
        {                        
            for ( CommData::iterator q = m_data[i].begin(); q != m_data[i].end(); ++q )
            {
                GridPos fromPos = GetGridPos( m_reinterpretedObsState[i] );
                GridPos toPos   = GetGridPos( m_reinterpretedObsState[ q->first ] );

                sum += fromPos.GetShortestWay( toPos, gridDims[ m_gridType ] ) * q->second;
            }
        }

        return sum == 0.0f ? 1000000.0f : float( 1000000.0 / sum );    // it is not a misprint
    }

        // QGen::IRepair

    virtual void operator()( MPI_Comm indComm, QGen::QObserveState& observeState, long long startQBit, int idx )
    {
        std::fill( m_reinterpretedObsState.begin(), m_reinterpretedObsState.end(), 0 );
        std::fill( m_occupiedPlaces.begin(), m_occupiedPlaces.end(), 0 );

        int observeStatePos = 0;
        for ( int i = 0; i < m_facilitiesNum; ++i )
        {
            int& current = m_reinterpretedObsState[i];

            for ( int q = 0; q < m_bitsPerPlace; ++q )
            {
                current <<= 1;
                current |=  observeState.at( observeStatePos++ ) ? 1 : 0;
            }

            if ( current < m_locationsNum )
                ++m_occupiedPlaces[ current ];
        }

        const int processesPerNode = m_isSMP ? 1 : 4;

        unsigned emptyPlacePos = 0;
        for ( int i = 0; i < m_facilitiesNum; ++i )
        {
            int& current = m_reinterpretedObsState[i];
            if ( current >= m_locationsNum )
            {
                while ( m_occupiedPlaces[ emptyPlacePos ] >= processesPerNode )
                    ++emptyPlacePos;
             
                ++m_occupiedPlaces[ emptyPlacePos ];                
                current = emptyPlacePos; 
            } 
            else if ( m_occupiedPlaces[ current ] > processesPerNode )
            {
                --m_occupiedPlaces[ current ];
                while ( (int)emptyPlacePos < m_locationsNum && m_occupiedPlaces[ emptyPlacePos ] >= processesPerNode )
                    ++emptyPlacePos;

                if ( (int)emptyPlacePos < m_locationsNum )
                {
                    ++m_occupiedPlaces[ emptyPlacePos ];                
                    current = emptyPlacePos;
                }
                else
                {
                    throw std::string( "Unexpected error. " ).append( __FUNCTION__ );
                }
            }
        }
    }

    //--------------------------------------------------------

    int GetProblemSize() const { return m_problemSize;  }

private:
    struct GridPos
    {
        int x;
        int y;
        int z;

        inline int GetShortestWay( const GridPos& other, const int dims[4] ) const
        {
            const int dXa = int( labs( x - other.x ) );
            const int dYa = int( labs( y - other.y ) );
            const int dZa = int( labs( z - other.z ) );

            //const int dXb = x + dims[0] - other.x;
            //const int dZb = y + dims[1] - other.y;
            //const int dYb = z + dims[2] - other.z;

            //return std::min( dXa, dXb ) + std::min( dYa, dYb ) + std::min( dZa, dZb );
            return dXa + dYa + dZa;
        }
    };

    inline GridPos GetGridPos( int absolutePos ) const
    {       
        GridPos res;

        res.z = absolutePos / gridDims[ m_gridType ][3];
        const int sliceSize = absolutePos % gridDims[ m_gridType ][3];
        res.y = sliceSize / gridDims[ m_gridType ][0];
        res.x = sliceSize % gridDims[ m_gridType ][0];        

        return res;
    }

private:
    int m_problemSize;

    int m_locationsNum;
    int m_facilitiesNum;
    bool m_isSMP;
    int m_gridType;

    typedef std::list< std::pair< int, float > > CommData; 
    std::vector<CommData> m_data;

    std::vector<int> m_occupiedPlaces;
    std::vector<int> m_reinterpretedObsState;
    int m_bitsPerPlace;
};

//------------------------------------------------------------

int mapping_main( parparser& args )
{
    const char* xmlFile = args.get( "xml" ).asString(0);

    try
    {
        QGen::SParams params( MPI_COMM_WORLD, xmlFile );

        MappingProblem mapping;
        mapping.ReadDataFile( params.getCustomParameter( "datafile" ) );

        params.indSize     = mapping.GetProblemSize();

        QFileScreen screenClass( params.outFile.c_str() );

        params.fClass      = &mapping;
        params.repClass    = &mapping;
        params.screenClass = &screenClass;

        double processTime = 0.0;
        QGen::QGenProcess process( params );
        processTime = process.process();

        if ( process.isMaster() )
        {
            std::stringstream sStr;
            sStr << "TOTAL TIME (QGEN-CPU): " << processTime;
            screenClass.printSStream( sStr );
        }

        int coords[2];
        QGen::QBaseIndivid& bestIndivid = *process.getBestIndivid( coords );
        mapping.SaveMappingFile( params.getCustomParameter( "mapfile" ), bestIndivid, coords );
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
