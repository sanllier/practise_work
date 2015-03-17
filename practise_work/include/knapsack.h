//------------------------------------------------------------
//-------------------------WARNING---------------------------
// Use this class only with 1xN-topology
//------------------------------------------------------------
//------------------------------------------------------------

#ifndef KNAPSACK_H
#define KNAPSACK_H

#include "parparser.h"
#include "qgen.h"
#include "basic_screen.h"

#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>

//------------------------------------------------------------

class Knapsack
{
public:
    struct Item
    {
        BASETYPE profit;
        BASETYPE weight;
    };

public:
    Knapsack()
        : m_size( BASETYPE(0) ) 
    {}

    int itemsNum() const { return m_items.size(); }
    BASETYPE capacity() const { return m_size; }

    Item& operator[]( int idx ) { return m_items[ idx ]; } 

    std::ostream& operator<<( std::ostream& oStr )
    {
        for ( int i = 0; i < (int)m_items.size(); ++i )
            oStr << "(" << m_items[i].profit << ", " << m_items[i].weight << ") ";

        return oStr;
    }

    std::istream& operator>>( std::istream& iStr )
    {
        iStr >> m_size;

        int elementsNum = 0;
        iStr >> elementsNum;
        for ( int i = 0; i < elementsNum; ++i )
        {
            m_items.push_back( Item() );
            iStr >> m_items.back().profit >> m_items.back().weight;
        }

        return iStr;
    }

    void loadFromFiles( const char* folder )
    {
        if ( !folder )
            throw std::string( "Invalid data folder." );

        std::string capFile = std::string( folder ).append( "/cap.txt" );
        std::string weightsFile = std::string( folder ).append( "/weights.txt" );
        std::string profitsFile = std::string( folder ).append( "/profits.txt" );

        std::ifstream capF;
        capF.open( capFile, std::ifstream::in );
        if ( !capF.good() )
            throw std::string( "Invalid cap file." );

        std::ifstream profitsF;
        profitsF.open( profitsFile, std::ifstream::in );
        if ( !profitsF.good() )
            throw std::string( "Invalid profits file." );

        std::ifstream weightsF;
        weightsF.open( weightsFile, std::ifstream::in );
        if ( !weightsF.good() )
            throw std::string( "Invalid weights file." );

        int elementsNum = 0;
        capF >> m_size >> elementsNum;
        capF.close();

        for ( int i = 0; i < elementsNum; ++i )
        {
            m_items.push_back( Item() );
            profitsF >> m_items.back().profit;
            weightsF >> m_items.back().weight;            
        }
        profitsF.close();
        weightsF.close();        
    }

private:
    std::vector< Item > m_items;
    BASETYPE m_size;
};

//------------------------------------------------------------

class KnapsackProblem : public QGen::IFitness
                      , public QGen::IRepair
{
public:
    KnapsackProblem( Knapsack* knapsack )
        : m_knapsack( knapsack )
    {
        if ( !knapsack )
            throw std::string( "KnapsackProblem is trying to use NULL knapsack" ).append( __FUNCTION__ );  

        for ( int i = 0; i < m_knapsack->itemsNum(); ++i )
        {
            m_ratios.push_back( ItemRatio() );
            m_ratios.back().pos = i;
            m_ratios.back().ratio = (*m_knapsack)[i].profit / (*m_knapsack)[i].weight;
        }

        std::sort( m_ratios.begin(), m_ratios.end() );
    }

    ~KnapsackProblem() 
    {
        delete m_knapsack;
    }

    //--------------------------------------------------------

        // QGen::IFitness

    virtual BASETYPE operator()( MPI_Comm indComm, const QGen::QObserveState& observeState, long long startQBit, int idx )
    {
        BASETYPE totalProfit = BASETYPE(0);
        Knapsack& knapPt = *m_knapsack;

        for ( int i = 0; i < knapPt.itemsNum(); ++i )
            if ( observeState.at(i) )
                totalProfit += knapPt[i].profit;

        return totalProfit;
    }

        // QGen::IRepair

    virtual void operator()( MPI_Comm indComm, QGen::QObserveState& observeState, long long startQBit, int idx )
    {
        bool overfilled = computeWeight( observeState ) > m_knapsack->capacity();
        int pos = m_ratios.size() - 1;
        while ( overfilled && pos >= 0 )
        {
            while ( pos >= 0 )
            {
                if ( observeState.at( m_ratios[ pos ].pos ) )
                {
                    observeState.set( m_ratios[ pos ].pos, false );
                    break;
                }
                --pos;
            }          
            overfilled = computeWeight( observeState ) > m_knapsack->capacity();
        }

        pos = 0;
        while ( !overfilled && pos < (int)( observeState.size() ) )
        {
            while ( pos < (int)( observeState.size() ) )
            {
                if ( !observeState.at( m_ratios[ pos ].pos ) )
                {
                    observeState.set( m_ratios[ pos ].pos, true );
                    break;
                }
                ++pos;
            }
            overfilled = computeWeight( observeState ) > m_knapsack->capacity();
        }

        observeState.set( m_ratios[ pos ].pos, false );
    }

    //--------------------------------------------------------

private:
    BASETYPE computeWeight( const QGen::QObserveState& observState )
    {
        BASETYPE weight = BASETYPE(0);
        for ( int i = 0; i < m_knapsack->itemsNum(); ++i )
            if ( observState.at(i) )
                weight += (*m_knapsack)[i].weight;

        return weight;
    }

private:
    Knapsack* m_knapsack;

    struct ItemRatio
    {
        BASETYPE ratio;
        int pos;

        bool operator<( const ItemRatio& item ) const { return ratio > item.ratio; } 
    };
    std::vector< ItemRatio > m_ratios;
};

//------------------------------------------------------------

int knapsack_main( parparser& args )
{
    const char* xmlFile = args.get( "xml" ).asString(0);

    try
    { 
        QGen::SParams params( MPI_COMM_WORLD, xmlFile );
        params.indSize = params.problemSize;
        const char* knapsackDataFolder = params.getCustomParameter( "knapsack-folder" );
        if ( knapsackDataFolder == 0 )
            throw std::string( "Unspecified knapsack folder data. " ).append( __FUNCTION__ );  

        Knapsack knapsack;
        knapsack.loadFromFiles( knapsackDataFolder );

        QScreen screenClass;
        KnapsackProblem workClass( &knapsack );
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
