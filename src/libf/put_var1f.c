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
#define nfmpi_put_var1_ NFMPI_PUT_VAR1
#elif defined(F77_NAME_LOWER_2USCORE)
#define nfmpi_put_var1_ nfmpi_put_var1__
#elif !defined(F77_NAME_LOWER_USCORE)
#define nfmpi_put_var1_ nfmpi_put_var1
/* Else leave name alone */
#endif


/* Prototypes for the Fortran interfaces */
#include "mpifnetcdf.h"
FORTRAN_API void FORT_CALL nfmpi_put_var1_ ( int *v1, int *v2, int v3[], void*v4, int *v5, MPI_Fint *v6, MPI_Fint *ierr ){
    size_t *l3 = 0;

    { int ln = ncmpixVardim(*v1,*v2);
    if (ln > 0) {
        int li;
        l3 = (size_t *)malloc( ln * sizeof(size_t) );
        for (li=0; li<ln; li++) 
            l3[li] = v3[ln-1-li] - 1;
    }}
    *ierr = ncmpi_put_var1( *v1, *v2, l3, v4, *v5, (MPI_Datatype)(*v6) );

    if (l3) { free(l3); }
}
