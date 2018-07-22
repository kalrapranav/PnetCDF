/*
 *  Copyright (C) 2013, Northwestern University and Argonne National Laboratory
 *  See COPYRIGHT notice in top-level directory.
 */

/*
 * This program tests thread-safe capability. Each MPI process creates 6
 * threads and each thread does the followings (one unique file per thread):
 * 1. creates a unique new file,
 * 2. writes 2 records to a record variable
 * 3. writes a fix-sized variable,
 * 4. closes the file,
 * 5. re-open the file,
 * 6. reads the record variable and check contents,
 * 7. reads the fix-sized variable and check contents,
 * 8. closes file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h> /* basename() */
#include <unistd.h> /* unlink() */

#include <mpi.h>
#include <pnetcdf.h>

#include <testutils.h>

#ifdef ENABLE_THREAD_SAFE
#include <pthread.h>

#define NTHREADS 6
#define NY 5
#define NX 4

static pthread_barrier_t barr;

typedef struct {
    int  id;
    char fname[256];
} thread_arg;

void* thread_func(void *arg)
{
    char filename[256];
    int i, id, nprocs, cmode, err, nerrs=0, ncid, *ret, dimid[2], varid[2];
    int *ibuf;
    double *dbuf;
    MPI_Offset start[2], count[2];

    id = ((thread_arg*)arg)->id;
    sprintf(filename, "%s.%d", ((thread_arg*)arg)->fname, id);

    /* allocate I/O buffers */
    ibuf = (int*)    malloc(NY * NX * sizeof(int));
    dbuf = (double*) malloc(NY * NX * sizeof(double));
    for (i=0; i<NY*NX; i++) {
        ibuf[i] = id;
        dbuf[i] = 1.0 * id;
    }

    /* create a file */
    cmode = NC_CLOBBER;
    err = ncmpi_create(MPI_COMM_SELF, filename, cmode, MPI_INFO_NULL, &ncid);
    CHECK_ERR
    /* define dimensions */
    err = ncmpi_def_dim(ncid, "time", NC_UNLIMITED, &dimid[0]); CHECK_ERR
    err = ncmpi_def_dim(ncid, "X", NX, &dimid[1]); CHECK_ERR
    /* define a record variable ivar */
    err = ncmpi_def_var(ncid, "ivar", NC_INT, 2, dimid, &varid[0]); CHECK_ERR
    /* define a fix-sized variable dvar */
    err = ncmpi_def_dim(ncid, "Y", NY, &dimid[0]); CHECK_ERR
    err = ncmpi_def_var(ncid, "dvar", NC_DOUBLE, 2, dimid, &varid[1]); CHECK_ERR
    err = ncmpi_enddef(ncid); CHECK_ERR
    /* now we are in data mode */

    /* write a record to the record variable */
    start[0] = 0;
    start[1] = 0;
    count[0] = 1;
    count[1] = NX;
    err = ncmpi_put_vara_int_all(ncid, varid[0], start, count, ibuf); CHECK_ERR

    /* write another record to the record variable */
    start[0] = 2;
    start[1] = 0;
    count[0] = 1;
    count[1] = NX;
    err = ncmpi_put_vara_int_all(ncid, varid[0], start, count, ibuf); CHECK_ERR

    /* write to the fix-sized variable */
    err = ncmpi_put_var_double_all(ncid, varid[1], dbuf); CHECK_ERR

    err = ncmpi_sync(ncid); CHECK_ERR
    err = ncmpi_close(ncid); CHECK_ERR

    /* synchronize all processes (only one thread per process participates) */
    if (id % NTHREADS == 0) MPI_Barrier(MPI_COMM_WORLD);

    /* synchronize all threads in each process to ensure all threads to finish
     * their file writes */
    pthread_barrier_wait(&barr);

    /* open a different file (round-robin shift), read variables back, and
     * check contents */
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    id = (id + 1) % (nprocs * NTHREADS);
    sprintf(filename, "%s.%d", ((thread_arg*)arg)->fname, id);

    err = ncmpi_open(MPI_COMM_SELF, filename, NC_NOWRITE, MPI_INFO_NULL, &ncid);
    CHECK_ERR
    err = ncmpi_inq_varid(ncid, "ivar", &varid[0]); CHECK_ERR
    err = ncmpi_inq_varid(ncid, "dvar", &varid[1]); CHECK_ERR

    /* read first record of the record variable */
    for (i=0; i<NX; i++) ibuf[i] = -1;
    start[0] = 0;
    start[1] = 0;
    count[0] = 1;
    count[1] = NX;
    err = ncmpi_get_vara_int_all(ncid, varid[0], start, count, ibuf); CHECK_ERR
    for (i=0; i<NX; i++) {
        if (ibuf[i] != id) {
            printf("Error at %s line %d: expect ibuf[%d]=%d but got %d\n",
            __FILE__, __LINE__, i, id, ibuf[i]);
            nerrs++;
            break;
        }
    }

    /* read the 3rd record of the record variable */
    for (i=0; i<NX; i++) ibuf[i] = -1;
    start[0] = 2;
    start[1] = 0;
    count[0] = 1;
    count[1] = NX;
    err = ncmpi_get_vara_int_all(ncid, varid[0], start, count, ibuf); CHECK_ERR
    for (i=0; i<NX; i++) {
        if (ibuf[i] != id) {
            printf("Error at %s line %d: expect ibuf[%d]=%d but got %d\n",
            __FILE__, __LINE__, i, id, ibuf[i]);
            nerrs++;
            break;
        }
    }

    /* read the fix-sized variable */
    err = ncmpi_get_var_double_all(ncid, varid[1], dbuf); CHECK_ERR
    for (i=0; i<NY*NX; i++) {
        if (dbuf[i] != (double)id) {
            printf("Error at %s line %d: expect ibuf[%d]=%d but got %f\n",
            __FILE__, __LINE__, i, id, dbuf[i]);
            nerrs++;
            break;
        }
    }
    err = ncmpi_close(ncid); CHECK_ERR

    free(ibuf);
    free(dbuf);

    // unlink(filename);

    /* return number of errors encountered */
    ret = (int*)malloc(sizeof(int));
    *ret = nerrs;
    pthread_exit(ret);
}
#endif

/*----< main() >-------------------------------------------------------------*/
int main(int argc, char **argv) {
    char filename[256];
    int  i, err, nerrs=0, rank, nprocs, providedT, verbose=0;
#ifdef ENABLE_THREAD_SAFE
    pthread_t threads[NTHREADS];

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &providedT);
#else
    MPI_Init(&argc, &argv);
#endif

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0 && verbose) {
        switch (providedT) {
            case MPI_THREAD_SINGLE:      printf("Support MPI_THREAD_SINGLE\n");
                                         break;
            case MPI_THREAD_FUNNELED:    printf("Support MPI_THREAD_FUNNELED\n");
                                         break;
            case MPI_THREAD_SERIALIZED:  printf("Support MPI_THREAD_SERIALIZED\n");
                                         break;
            case MPI_THREAD_MULTIPLE:    printf("Support MPI_THREAD_MULTIPLE\n");
                                         break;
            default: printf("Error MPI_Init_thread()\n"); break;
        }
    }

    if (argc > 2) {
        if (!rank) printf("Usage: %s [filename]\n",argv[0]);
        MPI_Finalize();
        return 1;
    }
    if (argc == 2) snprintf(filename, 256, "%s", argv[1]);
    else           strcpy(filename, "testfile.nc");
    MPI_Bcast(filename, 256, MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        char *cmd_str = (char*)malloc(strlen(argv[0]) + 256);
        sprintf(cmd_str, "*** TESTING C   %s for thread safety ", basename(argv[0]));
        printf("%-66s ------ ", cmd_str); fflush(stdout);
        free(cmd_str);
    }

#ifdef ENABLE_THREAD_SAFE
    /* initialize thread barrier */
    pthread_barrier_init(&barr, NULL, NTHREADS);

    /* create threads, each calls thread_func() */
    for (i=0; i<NTHREADS; i++) {
        thread_arg t_arg[NTHREADS]; /* must be unique to each thread */
        t_arg[i].id = i + rank * NTHREADS;
        sprintf(t_arg[i].fname, "%s",filename);
        if (pthread_create(&threads[i], NULL, thread_func, &t_arg[i])) {
            fprintf(stderr, "Error creating thread %d\n", i);
            nerrs++;
        }
    }

    /* wait for all threads to finish */
    for (i=0; i<NTHREADS; i++) {
        void *ret;
        if (pthread_join(threads[i], (void**)&ret)) {
            fprintf(stderr, "Error joining thread %d\n", i);
        }
        nerrs += *(int*)ret;
        free(ret);
    }

    /* check if PnetCDF freed all internal malloc */
    MPI_Offset malloc_size, sum_size;
    err = ncmpi_inq_malloc_size(&malloc_size);
    if (err == NC_NOERR) {
        MPI_Reduce(&malloc_size, &sum_size, 1, MPI_OFFSET, MPI_SUM, 0, MPI_COMM_WORLD);
        if (rank == 0 && sum_size > 0)
            printf("heap memory allocated by PnetCDF internally has %lld bytes yet to be freed\n",
                   sum_size);
        if (malloc_size > 0) ncmpi_inq_malloc_list();
    }

    MPI_Allreduce(MPI_IN_PLACE, &nerrs, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    if (rank == 0) {
        if (nerrs) printf(FAIL_STR,nerrs);
        else       printf(PASS_STR);
    }
#else
    if (rank == 0) printf(SKIP_STR);
#endif

    MPI_Finalize();
    return (nerrs > 0);
}