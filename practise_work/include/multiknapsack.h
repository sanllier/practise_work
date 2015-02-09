#ifndef KMULTIKNAPSACK_H
#define KMULTIKNAPSACK_H

#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <cmath>

#include "knapsack.h"
#include "qgen.h"

//------------------------------------------------------------

inline double log2( double n ) 
{ 
    return log(n) / log(2); 
}

//------------------------------------------------------------

class Knapsacks
{
public:
    Knapsacks()  {}
    ~Knapsacks() {}

    inline int size() const { return m_items.size(); }
    inline BASETYPE knapSize( int kNum ) const { return m_knapsSizes[ kNum ]; }
    inline int knapsNum() const { return m_knapsSizes.size(); }

    Item& operator[]( int idx ) { return m_items[ idx ]; }
    std::ostream& operator<<( std::ostream& oStr )
    {
        for ( int i = 0; i < (int)m_items.size(); ++i )
            oStr << "(" << m_items[i].profit << ", " << m_items[i].weight << ") ";

        return oStr;
    }
    std::istream& operator>>( std::istream& iStr )
    {
        int knapsNum = 0;
        iStr >> knapsNum;
        for ( int i = 0; i < knapsNum; ++i )
        {
            m_knapsSizes.push_back(0);
            iStr >> m_knapsSizes.back();
        }

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
        int knapsNum = 0;
        capF >> knapsNum;
        for ( int i = 0; i < knapsNum; ++i )
        {
            m_knapsSizes.push_back(0);
            capF >> m_knapsSizes.back();
        }

        capF >> elementsNum;
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

    int getIndSize() const
    {
        return m_items.size() + int( ceil( log2( knapsNum() ) ) * knapsNum() );
    }

private:
    std::vector< BASETYPE > m_knapsSizes;
    std::vector< Item > m_items;
};

//------------------------------------------------------------

class MultiKnapsackFitness : public QGen::QFitnessClass
{
public:
    MultiKnapsackFitness( Knapsacks* knaps )
        : m_knaps( knaps )
    {
        if ( !knaps )
            throw std::string( "MultiKnapsackFitness is trying to use NULL knapsacks" ).append( __FUNCTION__ );  
    }
    ~MultiKnapsackFitness() {}

    virtual BASETYPE operator()( MPI_Comm indComm, const QGen::QObservState& observState, long long startQBit, int idx )
    {
        static const int skipBits = int( ceil( log2( m_knaps->knapsNum() ) ) * m_knaps->knapsNum() );
        BASETYPE totalProfit = 0.0f;       

        std::vector< int > seq;
        std::vector< bool > filled;
        seq.resize( m_knaps->knapsNum(), 0 );
        filled.resize( m_knaps->knapsNum(), false );
        compKnapsSeq( observState, seq );

        int pos = skipBits;
        for ( int i = 0; i < m_knaps->knapsNum(); ++i )
        {
            if ( filled[i] )
                continue;

            if ( seq[i] < m_knaps->knapsNum() )
            {
                totalProfit += fillKnap( observState, pos, seq[i] );
                filled[ seq[i] ] = true;
            }
        }

        for ( int i = 0; i < m_knaps->knapsNum(); ++i )
        {
            if ( filled[i] )
                continue;

            totalProfit += fillKnap( observState, pos, i );
        }

        return totalProfit;
    }

private:
    void compKnapsSeq( const QGen::QObservState& observState, std::vector< int >& seq )
    {
        static const int bitsPerKnap = int( ceil( log2( m_knaps->knapsNum() ) ) );
        
        for ( int i = 0; i < m_knaps->knapsNum(); ++i )
        {            
           unsigned temp = 0;
           for ( int q = 0; q < bitsPerKnap; ++q )
           {
               temp <<= 1;
               if ( observState.at( i * bitsPerKnap + q ) )
                   temp |= 0x1;
           }
           seq[i] = temp;
        }
    }

    BASETYPE fillKnap( const QGen::QObservState& observState, int& pos, int kNum )
    {
        static const int skipBits = int( ceil( log2( m_knaps->knapsNum() ) ) * m_knaps->knapsNum() );

        BASETYPE profit = BASETYPE(0);
        BASETYPE weight = BASETYPE(0);

        while ( weight <= m_knaps->knapSize( kNum ) && pos < observState.size() )
        {
            if ( observState.at( pos ) )
            {
                profit += (*m_knaps)[ pos - skipBits ].profit;
                weight += (*m_knaps)[ pos - skipBits ].weight;
            }
            ++pos;
        }

        if ( weight > m_knaps->knapSize( kNum ) )
        {
            --pos;
            profit -= (*m_knaps)[ pos - skipBits ].profit;
        }

        return profit;
    }

private:
    Knapsacks* m_knaps;
};

//------------------------------------------------------------

//class MultiKnapsackRepair : public QGen::QRepairClass
//{
//public:
//    MultiKnapsackRepair( Knapsacks* knaps )
//        : m_knaps( knaps )
//    {
//        if ( !knaps )
//            throw std::string( "MultiKnapsackRepair is trying to use NULL knapsacks" ).append( __FUNCTION__ );  
//
//        for ( int i = 0; i < m_knaps->size(); ++i )
//        {
//            m_ratios.push_back( ItemRatio() );
//            m_ratios.back().pos = i;
//            m_ratios.back().ratio = (*m_knaps)[i].profit / (*m_knaps)[i].weight;
//        }
//        
//        std::sort( m_ratios.begin(), m_ratios.end() );
//
//        m_places.resize( m_knaps->size() );
//    }
//    ~MultiKnapsackRepair() {}
//
//    virtual void operator()( MPI_Comm indComm, QGen::QObservState& observState, long long startQBit, int idx )
//    {
//        static const int skipBits = ceil( log2( m_knaps->knapsNum() ) ) * m_knaps->knapsNum();
//
//        for ( int i = 1; i <= m_knaps->knapsNum(); ++i )
//        {
//            bool overfilled = computeWeight( observState ) > m_knap->knapSize();
//            int pos = m_ratios.size() - 1;
//            while ( overfilled && pos >= 0 )
//            {
//                while ( pos >= 0 )
//                {
//                    if ( observState.at( m_ratios[ pos ].pos ) )
//                    {
//                        observState.setBit( m_ratios[ pos ].pos, false );
//                        break;
//                    }
//                    --pos;
//                }          
//                overfilled = computeWeight( observState ) > m_knap->knapSize();
//            }
//    
//            pos = 0;
//            while ( !overfilled && pos < (int)( observState.size() ) )
//            {
//                while ( pos < (int)( observState.size() ) )
//                {
//                    if ( !observState.at( m_ratios[ pos ].pos ) )
//                    {
//                        observState.setBit( m_ratios[ pos ].pos, true );
//                        break;
//                    }
//                    ++pos;
//                }
//                overfilled = computeWeight( observState ) > m_knap->knapSize();
//            }
//    
//            observState.setBit( m_ratios[ pos ].pos, false );
//        }
//    }
//
//private:
//    BASETYPE computeWeight( const QGen::QObservState& observState, int kNum )
//    {
//        static const int skipBits = ceil( log2( m_knaps->knapsNum() ) ) * m_knaps->knapsNum();
//
//        BASETYPE weight = BASETYPE(0);
//        int obsStatePos = 0;
//        for ( int i = skipBits; i < m_knaps->size(); ++i )
//            if ( observState.at( obsStatePos++ ) && m_places[i] == kNum )
//                 weight += (*m_knaps)[i].weight;
//
//        return weight;
//    }
//
//    void compKnapsSeq( const QGen::QObservState& observState, std::vector< int >& seq )
//    {
//        static const int bitsPerKnap = ceil( log2( m_knaps->knapsNum() ) );
//        
//        for ( int i = 0; i < m_knaps->knapsNum(); ++i )
//        {            
//           unsigned temp = 0;
//           for ( int q = 0; q < bitsPerKnap; ++q )
//           {
//               temp <<= 1;
//               if ( observState.at( i * bitsPerKnap + q ) )
//                   temp |= 0x1;
//           }
//           seq[i] = temp;
//        }
//    }
//
//    void fillKnap( QGen::QObservState& observState, int kNum )
//    {
//        static const int skipBits = ceil( log2( m_knaps->knapsNum() ) ) * m_knaps->knapsNum();
//
//        bool overfilled = false;
//        int pos = 0;
//        while ( !overfilled && pos < (int)( observState.size() ) )
//        {
//            while ( pos < (int)( observState.size() ) )
//            {
//                if ( !observState.at( m_ratios[ pos ].pos ) )
//                {
//                    observState.setBit( m_ratios[ pos ].pos, true );
//                    break;
//                }
//                ++pos;
//            }
//            overfilled = computeWeight( observState ) > m_knap->knapSize();
//        }
//
//        observState.setBit( m_ratios[ pos ].pos, false );
//    }
//
//private:
//    struct ItemRatio
//    {
//        BASETYPE ratio;
//        int pos;
//
//        bool operator<( const ItemRatio& item ) { return ratio > item.ratio; } 
//    };
//    std::vector< ItemRatio > m_ratios;
//    std::vector< int > m_places;
//
//    Knapsacks* m_knaps;
//};

//------------------------------------------------------------
#endif
