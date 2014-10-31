#include "qindivid.h"

#include <string>
#include <cstring>
#include <cmath>

//------------------------------------------------------------

namespace QGen {
//------------------------------------------------------------

QIndivid::QIndivid( size_t size/* = 0*/ )
{
    if ( size < 0 )
        throw std::string( "Invalid individ size. " ).append( __FUNCTION__ );

    resize( size );
    setInitial();
}

QIndivid::QIndivid( const QIndivid& ind )
{
    m_data = new QBit[ ind.m_dataLogicSize ];
    m_dataLogicSize = ind.m_dataLogicSize;

    std::memcpy( m_data, ind.m_data, m_dataLogicSize * sizeof( QBit ) );
}

QIndivid::~QIndivid()
{
    delete[] m_data;
}

void QIndivid::resize( size_t size )
{
    m_data = new QBit[ size ];
    m_dataLogicSize = size;
}

void QIndivid::setInitial()
{
    const BASETYPE defVal = 1.0 / std::sqrt( 2 );
    for ( size_t i = 0; i < m_dataLogicSize; ++i )
    {
        m_data[i].a = defVal;
        m_data[i].b = defVal;
    }
}

void QIndivid::tick( const QRotOperator& op )
{
    QBit temp;
    for ( size_t i = 0; i < m_dataLogicSize; ++i )
    {
        temp.a = m_data[i].a * op[0][0] + m_data[i].b * op[0][1]; 
        temp.b = m_data[i].a * op[1][0] + m_data[i].b * op[1][1]; 
        m_data[i] = temp;
    }
}

//------------------------------------------------------------
}
