#ifndef QINDIVID_H
#define QINDIVID_H

#include <complex>

#include "defs.h"
#include "qrotoperator.h"

//------------------------------------------------------------

namespace QGen {
//------------------------------------------------------------

struct QBit
{
    QComplex a;
    QComplex b;
};

class QIndivid
{
public:
    QIndivid( size_t size = 0 );
    QIndivid( const QIndivid& ind );
    virtual ~QIndivid();

    void resize( size_t size );
    void setInitial();
    
    inline const QBit& at( size_t pos ) const { const static QBit dummy = { 0, 0 }; return ( pos < 0 || pos >= m_dataLogicSize ) ? dummy : m_data[ pos ]; }
    inline QBit& at( size_t pos ) { static QBit dummy = { 0, 0 }; return ( pos < 0 || pos >= m_dataLogicSize ) ? dummy : m_data[ pos ]; }

    void tick( const QRotOperator& op );

    inline size_t qsize() const { return m_dataLogicSize; }

private:
    QBit* m_data;
    size_t m_dataLogicSize;
};

//------------------------------------------------------------
}

//------------------------------------------------------------
#endif