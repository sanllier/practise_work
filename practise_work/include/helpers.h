#ifndef HELPERS_H
#define HELPERS_H

#include <math.h>

//------------------------------------------------------------

float log2( float arg ) { return logf( arg ) / logf(2); }
float roundUpToPowOfTwo( float arg ) { return powf( 2, ceil( log2( arg ) ) ); }

//------------------------------------------------------------

#endif
