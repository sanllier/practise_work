#ifndef KNAPSACK_H
#define KNAPSACK_H

#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>

#include "qgen.h"

//-----------------------------------------------------------
//-------------------------WARNING--------------------------
// Use this class only with 1xN-topology
//------------------------------------------------------------

struct Item
{
    BASETYPE profit;
    BASETYPE weight;
};

//------------------------------------------------------------

class Knapsack
{
public:
    Knapsack()
        : m_size(0.0f) 
    {}
    ~Knapsack() {}

    inline int size() const { return m_items.size(); }
    inline BASETYPE knapSize() const { return m_size; }

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

        std::ifstream weightsF;
        weightsF.open( weightsFile, std::ifstream::in );
        if ( !weightsF.good() )
            throw std::string( "Invalid weights file." );

        std::ifstream profitsF;
        profitsF.open( profitsFile, std::ifstream::in );
        if ( !profitsF.good() )
            throw std::string( "Invalid profits file." );

        int elementsNum = 0;
        capF >> m_size >> elementsNum;
        capF.close();

        for ( int i = 0; i < elementsNum; ++i )
        {
            m_items.push_back( Item() );
            weightsF >> m_items.back().weight;
            profitsF >> m_items.back().profit;
        }
        weightsF.close();
        profitsF.close();
    }

private:
    std::vector< Item > m_items;
    BASETYPE m_size;
};

//------------------------------------------------------------

class KnapsackFitness : public QGen::QFitnessClass
{
public:
    KnapsackFitness( Knapsack* knap )
        : m_knap( knap )
    {
        if ( !knap )
            throw std::string( "KnapsackFitness is trying to use NULL knapsack" ).append( __FUNCTION__ );  
    }
    ~KnapsackFitness() {}

    virtual BASETYPE operator()( MPI_Comm indComm, const QGen::QObservState& observState, long long startQBit, int idx )
    {
        BASETYPE totalProfit = 0.0f;
        Knapsack& knapPt = *m_knap;

        for ( int i = 0; i < knapPt.size(); ++i )
            if ( observState.at(i) )
                totalProfit += knapPt[i].profit;

        return totalProfit;
    }

private:
    Knapsack* m_knap;
};

//------------------------------------------------------------

class KnapsackRepair : public QGen::QRepairClass
{
public:
    KnapsackRepair( Knapsack* knap )
        : m_knap( knap )
    {
        if ( !knap )
            throw std::string( "KnapsackRepair is trying to use NULL knapsack" ).append( __FUNCTION__ );  

        for ( int i = 0; i < m_knap->size(); ++i )
        {
            m_ratios.push_back( ItemRatio() );
            m_ratios.back().pos = i;
            m_ratios.back().ratio = (*m_knap)[i].profit / (*m_knap)[i].weight;
        }

        std::sort( m_ratios.begin(), m_ratios.end() );
    }
    ~KnapsackRepair() {}

    virtual void operator()( MPI_Comm indComm, QGen::QObservState& observState, long long startQBit, int idx )
    {
        bool overfilled = computeWeight( observState ) > m_knap->knapSize();
        int pos = m_ratios.size() - 1;
        while ( overfilled && pos >= 0 )
        {
            while ( pos >= 0 )
            {
                if ( observState.at( m_ratios[ pos ].pos ) )
                {
                    observState.setBit( m_ratios[ pos ].pos, false );
                    break;
                }
                --pos;
            }          
            overfilled = computeWeight( observState ) > m_knap->knapSize();
        }

        pos = 0;
        while ( !overfilled && pos < (int)( observState.size() ) )
        {
            while ( pos < (int)( observState.size() ) )
            {
                if ( !observState.at( m_ratios[ pos ].pos ) )
                {
                    observState.setBit( m_ratios[ pos ].pos, true );
                    break;
                }
                ++pos;
            }
            overfilled = computeWeight( observState ) > m_knap->knapSize();
        }

        observState.setBit( m_ratios[ pos ].pos, false );
    }

private:
    BASETYPE computeWeight( const QGen::QObservState& observState )
    {
        BASETYPE weight = 0.0f;
        for ( int i = 0; i < m_knap->size(); ++i )
            if ( observState.at(i) )
                weight += (*m_knap)[i].weight;

        return weight;
    }

private:
    struct ItemRatio
    {
        BASETYPE ratio;
        int pos;

        bool operator<( const ItemRatio& item ) { return ratio > item.ratio; } 
    };
    std::vector< ItemRatio > m_ratios;

    Knapsack* m_knap;
};

//------------------------------------------------------------
#endif
