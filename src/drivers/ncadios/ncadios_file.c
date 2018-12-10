/*
 *  Copyright (C) 2017, Northwestern University and Argonne National Laboratory
 *  See COPYRIGHT notice in top-level directory.
 */
/* $Id$ */

/*
 * This file implements the following PnetCDF APIs
 *
 * ncmpi_create()           : dispatcher->create()
 * ncmpi_open()             : dispatcher->open()
 * ncmpi_close()            : dispatcher->close()
 * ncmpi_enddef()           : dispatcher->enddef()
 * ncmpi__enddef()          : dispatcher->_enddef()
 * ncmpi_redef()            : dispatcher->redef()
 * ncmpi_begin_indep_data() : dispatcher->begin_indep_data()
 * ncmpi_end_indep_data()   : dispatcher->end_indep_data()
 * ncmpi_abort()            : dispatcher->abort()
 * ncmpi_inq()              : dispatcher->inq()
 * ncmpi_inq_misc()         : dispatcher->inq_misc()
 * ncmpi_wait()             : dispatcher->wait()
 * ncmpi_wait_all()         : dispatcher->wait()
 * ncmpi_cancel()           : dispatcher->cancel()
 *
 * ncmpi_set_fill()         : dispatcher->set_fill()
 * ncmpi_fill_var_rec()     : dispatcher->fill_rec()
 * ncmpi_def_var_fill()     : dispatcher->def_var_fill()
 * ncmpi_inq_var_fill()     : dispatcher->inq()
 *
 * ncmpi_sync()             : dispatcher->sync()
 * ncmpi_flush()             : dispatcher->flush()
 * ncmpi_sync_numrecs()     : dispatcher->sync_numrecs()
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* strlen() */

#include <mpi.h>
#include <pnc_debug.h>
#include <common.h>
#include <ncadios_driver.h>

int
ncadios_create(MPI_Comm     comm,
             const char  *path,
             int          cmode,
             int          ncid,
             MPI_Info     info,
             void       **ncpp)  /* OUT */
{
    int err;
    void *ncp=NULL;
    NC_ad *ncadp;
    PNC_driver *driver=NULL;

    /* Read only driver */
    DEBUG_RETURN_ERROR(NC_ENOTSUPPORT);

    /* TODO: use comde to determine the true driver */

    /* Create a NC_ad object and save its driver pointer */
    ncadp = (NC_ad*) NCI_Malloc(sizeof(NC_ad));
    if (ncadp == NULL) DEBUG_RETURN_ERROR(NC_ENOMEM)

    ncadp->path = (char*) NCI_Malloc(strlen(path)+1);
    if (ncadp->path == NULL) {
        NCI_Free(ncadp);
        DEBUG_RETURN_ERROR(NC_ENOMEM)
    }
    strcpy(ncadp->path, path);
    ncadp->mode   = cmode;
    ncadp->flag   = 0;
    ncadp->comm   = comm;

    *ncpp = ncadp;

    return NC_NOERR;
}

int
ncadios_open(MPI_Comm     comm,
           const char  *path,
           int          omode,
           int          ncid,
           MPI_Info     info,
           void       **ncpp)
{
    int err, format;
    int i;
    void *ncp=NULL;
    NC_ad *ncadp;
    PNC_driver *driver=NULL;

    if (fIsSet(omode, NC_WRITE)){
        DEBUG_RETURN_ERROR(NC_ENOTSUPPORT);
    }

    /* Create a NC_ad object and save its driver pointer */
    ncadp = (NC_ad*) NCI_Malloc(sizeof(NC_ad));
    if (ncadp == NULL) DEBUG_RETURN_ERROR(NC_ENOMEM)
    
    ncadp->path = (char*) NCI_Malloc(strlen(path) + 1);
    if (ncadp->path == NULL) {
        NCI_Free(ncadp);
        DEBUG_RETURN_ERROR(NC_ENOMEM)
    }
    strcpy(ncadp->path, path);
    ncadp->mode   = omode;
    ncadp->flag   = 0;
    ncadp->comm   = comm;
    MPI_Comm_rank(ncadp->comm, &(ncadp->rank));

    *ncpp = ncadp;

    /*
     * Use a modified bp2ncd utility to parse header related information
     * to guarantee the driver conforms to the converted nc file
     * Attributes parsing is currently broken, we rely on ADIOS read 
     * API for attributes
     * Rank 0 parse the header and boardcast to other ranks
     */

    ncadiosi_var_list_init(&(ncadp->vars));
    //ncadiosi_att_list_init(&(ncadp->atts));
    ncadiosi_dim_list_init(&(ncadp->dims));

    if (ncadp->rank == 0) {
        ncadiosi_parse_header(ncadp);
    }

    /* Open with ADIOS read API */
    ncadp->fp = adios_read_open_file (path, ADIOS_READ_METHOD_BP, comm);
    if (ncadp->fp == NULL) {
        err = ncmpii_error_adios2nc(adios_errno, "Open");
        DEBUG_RETURN_ERROR(err);
    }

    if (ncadp->rank == 0) {
        // This require fp be opened
        ncadiosi_parse_attrs(ncadp);
    }
    ncadios_sync_header(ncadp);

    // Parse information regarding record dim
    ncadiosi_parse_rec_dim(ncadp);

    /* 
     * Build dimensionality list 
     * Another way to provide dimension information is to create our 
     * own dimension for each variable
     * It is currently not used
     */
    /*
    ncadp->ndims = (int*)NCI_Malloc(SIZEOF_INT * ncadp->fp->nvars);
    for (i = 0; i < ncadp->fp->nvars; i++) {
        ADIOS_VARINFO *v = adios_inq_var_byid (ncadp->fp, i);
        if (v == NULL){
            err = ncmpii_error_adios2nc(adios_errno, "inq_var");
            DEBUG_RETURN_ERROR(err);
        }
        err = adios_inq_var_stat (ncadp->fp, v, 0, 0);
        if (err != 0){
            err = ncmpii_error_adios2nc(adios_errno, "inq_var_stat");
            DEBUG_RETURN_ERROR(err);
        }
        ncadp->ndims[i] = v->ndim;

        adios_free_varinfo(v);
    }
    */

    return NC_NOERR;
}

int
ncadios_close(void *ncdp)
{
    int err;
    NC_ad *ncadp = (NC_ad*)ncdp;

    ncadiosi_var_list_free(&(ncadp->vars));
    //ncadiosi_att_list_free(&(ncadp->atts));
    ncadiosi_dim_list_free(&(ncadp->dims));

    if (ncadp == NULL) DEBUG_RETURN_ERROR(NC_EBADID)

    err = adios_read_close(ncadp->fp);
    if (err != 0){
        err = ncmpii_error_adios2nc(adios_errno, "open");
        DEBUG_RETURN_ERROR(err);
    }

    //NCI_Free(ncadp->ndims);
    NCI_Free(ncadp->path);
    NCI_Free(ncadp);

    return err;
}

int
ncadios_enddef(void *ncdp)
{
    int err;
    NC_ad *ncadp = (NC_ad*)ncdp;

    /* Read only driver */
    DEBUG_RETURN_ERROR(NC_ENOTSUPPORT);

    return NC_NOERR;
}

int
ncadios__enddef(void       *ncdp,
              MPI_Offset  h_minfree,
              MPI_Offset  v_align,
              MPI_Offset  v_minfree,
              MPI_Offset  r_align)
{
    int err;
    NC_ad *ncadp = (NC_ad*)ncdp;

    /* Read only driver */
    DEBUG_RETURN_ERROR(NC_ENOTSUPPORT);

    return NC_NOERR;
}

int
ncadios_redef(void *ncdp)
{
    int err;
    NC_ad *ncadp = (NC_ad*)ncdp;

    /* Read only driver */
    DEBUG_RETURN_ERROR(NC_ENOTSUPPORT);

    return NC_NOERR;
}

int
ncadios_begin_indep_data(void *ncdp)
{
    int err;
    NC_ad *ncadp = (NC_ad*)ncdp;

    /* Make sure we are in data mode */
    if (fIsSet(ncadp->flag, NC_MODE_DEF)){
        DEBUG_RETURN_ERROR(NC_EINDEFINE);
    }

    /* Set indep flag */
    fSet(ncadp->flag, NC_MODE_INDEP);

    return NC_NOERR;
}

int
ncadios_end_indep_data(void *ncdp)
{
    int err;
    NC_ad *ncadp = (NC_ad*)ncdp;

    /* Make sure we are in data mode */
    if (fIsSet(ncadp->flag, NC_MODE_DEF)){
        DEBUG_RETURN_ERROR(NC_EINDEFINE);
    }

    /* Clear indep flag */
    fClr(ncadp->flag, NC_MODE_INDEP);

    return NC_NOERR;
}

int
ncadios_abort(void *ncdp)
{
    int err;
    NC_ad *ncadp = (NC_ad*)ncdp;

    /* Read only driver */
    DEBUG_RETURN_ERROR(NC_ENOTSUPPORT);

    if (ncadp == NULL) DEBUG_RETURN_ERROR(NC_EBADID)

    NCI_Free(ncadp->path);
    NCI_Free(ncadp);

    return err;
}

int
ncadios_inq(void *ncdp,
          int  *ndimsp,
          int  *nvarsp,
          int  *nattsp,
          int  *xtendimp)
{
    int err;
    int i;
    NC_ad *ncadp = (NC_ad*)ncdp;

    if (ndimsp != NULL){
        *ndimsp = ncadp->dims.cnt;
    }

    if (nvarsp != NULL){
        *nvarsp = ncadp->vars.cnt;
    }

    if (nattsp != NULL){
        *nattsp = ncadp->fp->nattrs;
    }

    if (xtendimp != NULL){
        *xtendimp = ncadp->recdim;
    }

    return NC_NOERR;
}

int
ncadios_inq_misc(void       *ncdp,
               int        *pathlen,
               char       *path,
               int        *num_fix_varsp,
               int        *num_rec_varsp,
               int        *striping_size,
               int        *striping_count,
               MPI_Offset *header_size,
               MPI_Offset *header_extent,
               MPI_Offset *recsize,
               MPI_Offset *put_size,
               MPI_Offset *get_size,
               MPI_Info   *info_used,
               int        *nreqs,
               MPI_Offset *usage,
               MPI_Offset *buf_size)
{
    int err;
    NC_ad *ncadp = (NC_ad*)ncdp;

    if (pathlen != NULL){
        *pathlen = strlen(ncadp->path);
    }

    if (path != NULL){
        strcpy(path, ncadp->path);
    }

    if (num_fix_varsp != NULL){
        // All variables - number of record variables
        int i, j;
        *num_fix_varsp = ncadp->vars.cnt;
        for(i = 0; i < ncadp->vars.cnt; i++){
            for(j = 0; j < ncadp->vars.data[i].ndim; j++){
                if (ncadp->dims.data[ncadp->vars.data[i].dimids[j]].len == NC_UNLIMITED){
                    *num_rec_varsp -= 1;
                    break;
                }
            }
        }
    }

    if (num_rec_varsp != NULL){
        // We count those variable with unlimited dim as rec variable
        int i, j;
        *num_rec_varsp = 0;
        for(i = 0; i < ncadp->vars.cnt; i++){
            for(j = 0; j < ncadp->vars.data[i].ndim; j++){
                if (ncadp->dims.data[ncadp->vars.data[i].dimids[j]].len == NC_UNLIMITED){
                    *num_rec_varsp += 1;
                    break;
                }
            }
        }
    }

    if (striping_size != NULL){
        *striping_size = 0;
    }

    if (striping_count != NULL){
        *striping_count = 0;
    }

    if (header_size != NULL){
        *header_size = 0;
    }

    if (header_extent != NULL){
        *header_extent = 0;
    }

    if (recsize != NULL){
        *recsize = ncadp->nrec;
    }

    if (put_size != NULL){
        *put_size = 0;
    }

    //TODO: Count get size
    if (get_size != NULL){
        *get_size = 0;
    }

    if (info_used != NULL){
        *info_used = MPI_INFO_NULL;
    }

    //TODO: Wire up nonblocking req
    if (nreqs != NULL){
        *nreqs = 0;
    }

    //TODO: Wire up nonblocking req
    if (usage != NULL){
        *usage = 0;
    }

    //TODO: Wire up nonblocking req
    if (buf_size != NULL){
        *buf_size = MPI_INFO_NULL;
    }
    
    return NC_NOERR;
}

int
ncadios_cancel(void *ncdp,
             int   num_req,
             int  *req_ids,
             int  *statuses)
{
    int err;
    NC_ad *ncadp = (NC_ad*)ncdp;

    /* TODO: Nonblocking IO support */
    DEBUG_RETURN_ERROR(NC_ENOTSUPPORT);

    return NC_NOERR;
}

int
ncadios_wait(void *ncdp,
           int   num_reqs,
           int  *req_ids,
           int  *statuses,
           int   reqMode)
{
    int err;
    NC_ad *ncadp = (NC_ad*)ncdp;

    /* TODO: Nonblocking IO support */
    DEBUG_RETURN_ERROR(NC_ENOTSUPPORT);

    return NC_NOERR;
}

int
ncadios_set_fill(void *ncdp,
               int   fill_mode,
               int  *old_fill_mode)
{
    int err;
    NC_ad *ncadp = (NC_ad*)ncdp;

    /* Read only driver */
    DEBUG_RETURN_ERROR(NC_ENOTSUPPORT);

    return NC_NOERR;
}

int
ncadios_fill_var_rec(void      *ncdp,
                   int        varid,
                   MPI_Offset recno)
{
    int err;
    NC_ad *ncadp = (NC_ad*)ncdp;

    /* Read only driver */
    DEBUG_RETURN_ERROR(NC_ENOTSUPPORT);

    return NC_NOERR;
}

int
ncadios_def_var_fill(void       *ncdp,
                   int         varid,
                   int         no_fill,
                   const void *fill_value)
{
    int err;
    NC_ad *ncadp = (NC_ad*)ncdp;

    /* Read only driver */
    DEBUG_RETURN_ERROR(NC_ENOTSUPPORT);

    return NC_NOERR;
}

int
ncadios_sync_numrecs(void *ncdp)
{
    int err;
    NC_ad *ncadp = (NC_ad*)ncdp;

    /* Read only driver */
    DEBUG_RETURN_ERROR(NC_ENOTSUPPORT);

    return NC_NOERR;
}

int
ncadios_sync(void *ncdp)
{
    int err;
    NC_ad *ncadp = (NC_ad*)ncdp;

    /* Read only driver */
    DEBUG_RETURN_ERROR(NC_ENOTSUPPORT);

    return NC_NOERR;
}

int
ncadios_flush(void *ncdp)
{
    int err;
    NC_ad *ncadp = (NC_ad*)ncdp;

    /* Read only driver */
    DEBUG_RETURN_ERROR(NC_ENOTSUPPORT);

    return NC_NOERR;
}

