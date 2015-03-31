#include "qap.h"
#include "onemax.h"
#include "knapsack.h"
#include "mapping.h"

#include "mpi.h"

#include <iostream>
#include <string.h>

//------------------------------------------------------------

int main( int argc, char**argv )
{
    MPI_Init( &argc, &argv );

    parparser arguments( argc, argv ); 
    const char* problem = arguments.get( "problem" ).asString(0);

    int retCode = 1;
    if ( problem )
    {
        if ( 0 == strcmp( "onemax", problem ) )
            retCode = onemax_main( arguments );
        else if ( 0 == strcmp( "knapsack", problem ) )
            retCode = knapsack_main( arguments );
        else if ( 0 ==  strcmp( "qap", problem ) )
            retCode = qap_main( arguments );
        else if ( 0 == strcmp( "mapping", problem ) )
            retCode = mapping_main( arguments );
    }
    else
    {  
        std::cout << "Invalid problem. Specify it via -problem parameter.\r\n";
        std::cout << "    Available problems: onemax, knapsack, qap, mapping\r\n";
    }
    
    MPI_Finalize();
    return retCode;
}

//------------------------------------------------------------
