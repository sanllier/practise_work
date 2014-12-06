#ifndef KMULTINAPSACK_H
#define KMULTINAPSACK_H

#include <vector>
#include <iostream>
#include <string>

#include "qgen.h"
#include "knapsack.h"

//------------------------------------------------------------

struct Item
{
    float profit;
    float weight;
};

//------------------------------------------------------------

class MultiKnapsackFitness : public QGen::QFitnessClass
{
public:
    MultiKnapsackFitness( std::vector< Knapsack >* knaps )
        : m_knaps( knaps )
    {
        if ( !knaps )
            throw std::string( "KnapsackFitness is trying to use NULL knapsacks" ).append( __FUNCTION__ );  
    }
    ~MultiKnapsackFitness() {}

    virtual BASETYPE operator()( const QGen::QObservState& observState )
    {
        float totalProfit = 0.0f;
        std::vector< Knapsack >& knapsPt = *m_knaps;

        int obsStatePos = 0;
        for ( int i = 0; i < (int)knapsPt.size(); ++i )
            for ( int q = 0; q < knapsPt[i].size(); ++q )
                if ( observState.at( obsStatePos++ ) )
                    totalProfit += knapsPt[i][q].profit;

        return totalProfit;
    }

private:
    std::vector< Knapsack >* m_knaps;
};

//------------------------------------------------------------

class MultiKnapsackRepair : public QGen::QRepairClass
{
public:
    MultiKnapsackRepair( std::vector< Knapsack >* knaps )
        : m_knaps( knaps )
    {
        if ( !knaps )
            throw std::string( "KnapsackFitness is trying to use NULL knapsacks" ).append( __FUNCTION__ );  

        //for ( int i = 0; i < m_knap->size(); ++i )
        //{
        //    m_ratios.push_back( ItemRatio() );
        //    m_ratios.back().pos = i;
        //    m_ratios.back().ratio = (*m_knap)[i].profit / (*m_knap)[i].weight;
        //}
        //
        //std::sort( m_ratios.begin(), m_ratios.end() );
    }
    ~MultiKnapsackRepair() {}

    //virtual void operator()( QGen::QObservState& observState )
    //{
    //    bool overfilled = computeWeight( observState ) > m_knap->knapSize();
    //    int pos = m_ratios.size() - 1;
    //    while ( overfilled && pos >= 0 )
    //    {
    //        while ( pos >= 0 )
    //        {
    //            if ( observState.at( m_ratios[ pos ].pos ) )
    //            {
    //                observState.setBit( m_ratios[ pos ].pos, false );
    //                break;
    //            }
    //            --pos;
    //        }          
    //        overfilled = computeWeight( observState ) > m_knap->knapSize();
    //    }
    //
    //    pos = 0;
    //    while ( !overfilled && pos < (int)( observState.size() ) )
    //    {
    //        while ( pos < (int)( observState.size() ) )
    //        {
    //            if ( !observState.at( m_ratios[ pos ].pos ) )
    //            {
    //                observState.setBit( m_ratios[ pos ].pos, true );
    //                break;
    //            }
    //            ++pos;
    //        }
    //        overfilled = computeWeight( observState ) > m_knap->knapSize();
    //    }
    //
    //    observState.setBit( m_ratios[ pos ].pos, false );
    //}

private:
    float computeWeight( const QGen::QObservState& observState )
    {
        float weight = 0.0f;
        int obsStatePos = 0;
        for ( int i = 0; i < m_knaps->size(); ++i )
            for ( int q = 0; q < (*m_knaps)[i].size(); ++q )
                if ( observState.at( obsStatePos++ ) )
                    weight += (*m_knaps)[i][q].weight;

        return weight;
    }

private:
    struct ItemRatio
    {
        float ratio;
        int pos;

        bool operator<( const ItemRatio& item ) { return ratio > item.ratio; } 
    };
    std::vector< ItemRatio > m_ratios;

    std::vector< Knapsack >* m_knaps;
};

//------------------------------------------------------------
#endif
