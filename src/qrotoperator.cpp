#include "qrotoperator.h"

#include <cmath>

//------------------------------------------------------------

namespace QGen {
//------------------------------------------------------------

QRotOperator::QRotOperator( BASETYPE angle/* = BASETYPE(0)*/ )
{
    compute( angle );
}

QRotOperator::QRotOperator( const QRotOperator& op )
{
    m_curAngle = op.m_curAngle;
    m_matrix[0][0] = op.m_matrix[0][0];
    m_matrix[0][1] = op.m_matrix[0][1];
    m_matrix[1][0] = op.m_matrix[1][0];
    m_matrix[1][1] = op.m_matrix[1][1];
}

void QRotOperator::compute( BASETYPE angle )
{
    m_curAngle = angle;
    m_matrix[0][0] = std::cos( angle );
    m_matrix[1][1] = m_matrix[0][0];
    m_matrix[1][0] = std::sin( angle );
    m_matrix[0][1] = -m_matrix[1][0];
}

//------------------------------------------------------------
}
