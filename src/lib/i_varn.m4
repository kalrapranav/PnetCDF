dnl Process this m4 file to produce 'C' language file.
dnl
dnl If you see this line, you can ignore the next one.
/* Do not edit this file. It is produced from the corresponding .m4 source */
dnl
/*
 *  Copyright (C) 2014, Northwestern University and Argonne National Laboratory
 *  See COPYRIGHT notice in top-level directory.
 */
/* $Id$ */

#if HAVE_CONFIG_H
# include <ncconfig.h>
#endif

#include <stdio.h>
#include <unistd.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <assert.h>

#include <mpi.h>

#include "nc.h"
#include "ncx.h"
#include "ncmpidtype.h"
#include "macro.h"

/* ncmpi_iget/iput_varn_<type>_<mode> API:
 *    type:   data type of I/O buffer, buf
 *    mode:   indpendent (<nond>) or collective (_all)
 *
 * arguments:
 *    num:    number of start and count pairs
 *    starts: an 2D array of size [num][ndims]. Each starts[i][*] indicates
 *            the starting array indices for a subarray request. ndims is
 *            the number of dimensions of the defined netCDF variable.
 *    counts: an 2D array of size [num][ndims]. Each counts[i][*] indicates
 *            the number of array elements to be accessed. This argument
 *            can be NULL, equivalent to counts with all 1s.
 *    bufcount and buftype: these 2 arguments are only available for flexible
 *            APIs, indicating the I/O buffer memory layout. When buftype is
 *            MPI_DATATYPE_NULL, bufcount is ignored and the data type of buf
 *            is considered matched the variable data type defined in the file.
 *    reqid:  request ID returned to user
 */

static int
ncmpii_igetput_varn(int               ncid,
                    int               varid,
                    int               num,
                    MPI_Offset* const starts[],  /* [num][varp->ndims] */
                    MPI_Offset* const counts[],  /* [num][varp->ndims] */
                    void             *buf,
                    MPI_Offset        bufcount,
                    MPI_Datatype      buftype,   /* data type of the bufer */
                    int              *reqid,
                    int               rw_flag,
                    int               use_abuf);

dnl
define(`IsBput',    `ifelse(`$1',`bput', `1', `0')')dnl
define(`ReadWrite', `ifelse(`$1',`iget', `READ_REQ', `WRITE_REQ')')dnl
define(`BufConst',  `ifelse(`$1',`iget', , `const')')dnl
dnl
dnl VARN_FLEXIBLE()
dnl
define(`VARN_FLEXIBLE',dnl
`dnl
/*----< ncmpi_$1_varn() >-----------------------------------------------------*/
int
ncmpi_$1_varn(int                 ncid,
              int                 varid,
              int                 num,
              MPI_Offset* const   starts[],
              MPI_Offset* const   counts[],
              BufConst($1) void  *buf,
              MPI_Offset          bufcount,
              MPI_Datatype        buftype,
              int                *reqid)
{
    return ncmpii_igetput_varn(ncid, varid, num, starts, counts, (void*)buf,
                               bufcount, buftype, reqid, ReadWrite($1), IsBput($1));
}
')dnl

dnl PnetCDF flexible APIs
VARN_FLEXIBLE(iput)
VARN_FLEXIBLE(iget)
VARN_FLEXIBLE(bput)

dnl
dnl VARN()
dnl
define(`VARN',dnl
`dnl
/*----< ncmpi_$1_varn_$2() >--------------------------------------------------*/
int
ncmpi_$1_varn_$2(int                ncid,
                 int                varid,
                 int                num,
                 MPI_Offset* const  starts[],
                 MPI_Offset* const  counts[],
                 BufConst($1) $3   *buf,
                 int               *reqid)
{
    /* set bufcount to -1 indicating non-flexible API */
    return ncmpii_igetput_varn(ncid, varid, num, starts, counts, (void*)buf,
                               -1, $4, reqid, ReadWrite($1), IsBput($1));
}
')dnl

VARN(iput, text,      char,               MPI_CHAR)
VARN(iput, schar,     schar,              MPI_BYTE)
VARN(iput, uchar,     uchar,              MPI_UNSIGNED_CHAR)
VARN(iput, short,     short,              MPI_SHORT)
VARN(iput, ushort,    ushort,             MPI_UNSIGNED_SHORT)
VARN(iput, int,       int,                MPI_INT)
VARN(iput, uint,      uint,               MPI_UNSIGNED)
VARN(iput, long,      long,               MPI_LONG)
VARN(iput, float,     float,              MPI_FLOAT)
VARN(iput, double,    double,             MPI_DOUBLE)
VARN(iput, longlong,  long long,          MPI_LONG_LONG_INT)
VARN(iput, ulonglong, unsigned long long, MPI_UNSIGNED_LONG_LONG)

VARN(iget, text,      char,               MPI_CHAR)
VARN(iget, schar,     schar,              MPI_BYTE)
VARN(iget, uchar,     uchar,              MPI_UNSIGNED_CHAR)
VARN(iget, short,     short,              MPI_SHORT)
VARN(iget, ushort,    ushort,             MPI_UNSIGNED_SHORT)
VARN(iget, int,       int,                MPI_INT)
VARN(iget, uint,      uint,               MPI_UNSIGNED)
VARN(iget, long,      long,               MPI_LONG)
VARN(iget, float,     float,              MPI_FLOAT)
VARN(iget, double,    double,             MPI_DOUBLE)
VARN(iget, longlong,  long long,          MPI_LONG_LONG_INT)
VARN(iget, ulonglong, unsigned long long, MPI_UNSIGNED_LONG_LONG)

VARN(bput, text,      char,               MPI_CHAR)
VARN(bput, schar,     schar,              MPI_BYTE)
VARN(bput, uchar,     uchar,              MPI_UNSIGNED_CHAR)
VARN(bput, short,     short,              MPI_SHORT)
VARN(bput, ushort,    ushort,             MPI_UNSIGNED_SHORT)
VARN(bput, int,       int,                MPI_INT)
VARN(bput, uint,      uint,               MPI_UNSIGNED)
VARN(bput, long,      long,               MPI_LONG)
VARN(bput, float,     float,              MPI_FLOAT)
VARN(bput, double,    double,             MPI_DOUBLE)
VARN(bput, longlong,  long long,          MPI_LONG_LONG_INT)
VARN(bput, ulonglong, unsigned long long, MPI_UNSIGNED_LONG_LONG)


/*----< ncmpii_igetput_varn() >-----------------------------------------------*/
static int
ncmpii_igetput_varn(int               ncid,
                    int               varid,
                    int               num,
                    MPI_Offset* const starts[],  /* [num][varp->ndims] */
                    MPI_Offset* const counts[],  /* [num][varp->ndims] */
                    void             *buf,
                    MPI_Offset        bufcount,
                    MPI_Datatype      buftype,   /* data type of the bufer */
                    int              *reqid,     /* OUT: request ID */
                    int               rw_flag,   /* WRITE_REQ or READ_REQ */
                    int               use_abuf)  /* if use attached buffer */
{
    int i, j, el_size, status=NC_NOERR, free_cbuf=0, isSameGroup;
    void *cbuf=NULL;
    char *bufp;
    MPI_Offset **_counts=NULL;
    MPI_Datatype ptype;
    NC     *ncp;
    NC_var *varp=NULL;

    *reqid = NC_REQ_NULL;

    /* check for zero-size request */
    if (num == 0 || bufcount == 0) return NC_NOERR;

    /* check if ncid is valid, if yes, get varp from varid */
    SANITY_CHECK(ncid, ncp, varp, rw_flag, NONBLOCKING_IO, status)
    if (status != NC_NOERR) return status;

    if (use_abuf && ncp->abuf == NULL) return NC_ENULLABUF;

    /* it is illegal for starts to be NULL */
    if (starts == NULL) return NC_ENULLSTART;

    cbuf = buf;
    if (buftype == MPI_DATATYPE_NULL) {
        /* In this case, bufcount is ignored and will be recalculated to match
         * counts[]. Note buf's data type must match the data type of
         * variable defined in the file - no data conversion will be done.
         */
        bufcount = 0;
        for (j=0; j<num; j++) {
            MPI_Offset bufcount_j = 1;
            for (i=0; i<varp->ndims; i++) {
                if (counts[j][i] < 0) /* no negative counts[][] */
                    return NC_ENEGATIVECNT;
                bufcount_j *= counts[j][i];
            }
            bufcount += bufcount_j;
        }
        /* assign buftype match with the variable's data type */
        ptype = buftype = ncmpii_nc2mpitype(varp->type);
        MPI_Type_size(ptype, &el_size); /* buffer element size */
    }
    else if (bufcount == -1) { /* if (IsPrimityMPIType(buftype)) */
        /* this subroutine is called from a high-level API */
        ptype = buftype;
        MPI_Type_size(ptype, &el_size); /* buffer element size */
    }
    else { /* (bufcount > 0) flexible API is used */
        /* pack buf into cbuf, a contiguous buffer */
        int isderived, iscontig_of_ptypes;
        MPI_Offset bnelems;

        /* ptype (primitive MPI data type) from buftype
         * el_size is the element size of ptype
         * bnelems is the total number of ptype elements in buftype
         */
        status = ncmpii_dtype_decode(buftype, &ptype, &el_size, &bnelems,
                                     &isderived, &iscontig_of_ptypes);

        if (status != NC_NOERR) return status;

        /* check if buftype is contiguous, if not, pack to one, cbuf */
        if (! iscontig_of_ptypes && bnelems > 0) {
            int position = 0;
            int packsize = bnelems*el_size;

            cbuf = NCI_Malloc(packsize);
            free_cbuf = 1;
            /* if not use_abuf, need a callback to free cbuf */

            if (rw_flag == WRITE_REQ)
                MPI_Pack(buf, bufcount, buftype, cbuf, packsize, &position,
                         MPI_COMM_SELF);
        }
    }

    /* We allow counts == NULL and treat this the same as all 1s */
    if (counts == NULL) {
        _counts    = (MPI_Offset**) NCI_Malloc(num * sizeof(MPI_Offset*));
        _counts[0] = (MPI_Offset*)  NCI_Malloc(num * varp->ndims *
                                               sizeof(MPI_Offset));
        for (i=1; i<num; i++)
            _counts[i] = _counts[i-1] + varp->ndims;
        for (i=0; i<num; i++)
            for (j=0; j<varp->ndims; j++)
                _counts[i][j] = 1;
    }
    else
        _counts = (MPI_Offset**) counts;

    /* break buf into num pieces */
    isSameGroup=0;
    bufp = (char*)cbuf;
    for (i=0; i<num; i++) {
        MPI_Offset buflen;

        /* access boundaries check */
        status = NCcoordck(ncp, varp, starts[i], rw_flag);
        if (status != NC_NOERR) goto err_check;
        status = NCedgeck(ncp, varp, starts[i], _counts[i]);
        if (status != NC_NOERR) goto err_check;
        if (rw_flag == READ_REQ && IS_RECVAR(varp) &&
            starts[i][0] + _counts[i][0] > NC_get_numrecs(ncp)) {
            status = NC_EEDGE;
            goto err_check;
        }

        for (buflen=1, j=0; j<varp->ndims; j++) {
            if (_counts[i][j] < 0) { /* any negative counts[][] is illegal */
                status = NC_ENEGATIVECNT;
                goto err_check;
            }
            buflen *= _counts[i][j];
        }
        if (buflen == 0) continue;
        status = ncmpii_igetput_varm(ncp, varp, starts[i], _counts[i], NULL,
                                     NULL, bufp, buflen, ptype, reqid,
                                     rw_flag, use_abuf, isSameGroup);
        if (status != NC_NOERR) goto err_check;

        /* use isSamegroup so we end up with one nonblocking request (only the
         * first request gets a request ID back, the rest reuse the same ID.
         * This single ID represents num nonblocking requests */
        isSameGroup=1;
        bufp += buflen * el_size;
    }

    /* add callback if buftype is noncontiguous */
    if (free_cbuf) { /* cbuf != buf, cbuf is temp allocated */
        if (rw_flag == READ_REQ) {
            /* tell wait() to unpack cbuf to buf and free cbuf */
            status = ncmpii_set_iget_callback(ncp, *reqid, cbuf, buf,
                                              bufcount, buftype);
        }
        else { /* WRITE_REQ */
            if (use_abuf)
                /* cbuf has been copied to the attached buffer, so it is safe
                 * to free cbuf now */
                NCI_Free(cbuf);
            else
                /* tell wait() to free cbuf once done */
                status = ncmpii_set_iput_callback(ncp, *reqid, cbuf);
        }
    }

err_check:
    if (_counts != NULL && _counts != counts) {
        NCI_Free(_counts[0]);
        NCI_Free(_counts);
    }

    if (status != NC_NOERR) {
        if (*reqid != NC_REQ_NULL) /* cancel pending nonblocking request */
            ncmpii_cancel(ncp, 1, reqid, NULL);
        if (free_cbuf) NCI_Free(cbuf);
    }

    return status;
}
