/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*  
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 * This file is automatically generated by buildiface -infile=../lib/pnetcdf.h -deffile=defs
 * DO NOT EDIT
 */
#include "mpinetcdf_impl.h"


#ifdef F77_NAME_UPPER
#define nfmpi_get_vara_int1_ NFMPI_GET_VARA_INT1
#elif defined(F77_NAME_LOWER_2USCORE)
#define nfmpi_get_vara_int1_ nfmpi_get_vara_int1__
#elif !defined(F77_NAME_LOWER_USCORE)
#define nfmpi_get_vara_int1_ nfmpi_get_vara_int1
/* Else leave name alone */
#endif


/* Prototypes for the Fortran interfaces */
#include "mpifnetcdf.h"
FORTRAN_API int FORT_CALL nfmpi_get_vara_int1_ ( int *v1, int *v2, int v3[], int v4[], char *v5 FORT_MIXED_LEN(d5) FORT_END_LEN(d5) ){
    int ierr;
    ierr = ncmpi_get_vara_schar( *v1, *v2, (const MPI_Offset *)(v3), (const MPI_Offset *)(v4), v5 );
    return ierr;
}
