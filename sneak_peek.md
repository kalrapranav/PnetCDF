------------------------------------------------------------------------------
This is essentially a placeholder for the next release note ...
------------------------------------------------------------------------------

* New features
  + ADIOS driver -- Read ADIOS 1.x BP formated file. 
    ADIOS_READ_METHOD_BP must be set when open BP file.
    Does not support low-level and non blocking API.

* New optimization
  + When inserting nonblocking requests into pending queues, keep the queues
    sorted (insert sort) in the increasing order of variable starting file
    offsets. This can avoid a quick sort when flushing the pending requests.
    See [pull request #37](https://github.com/Parallel-NetCDF/PnetCDF/pull/37).
    To avoid internal sorting completely, users are recommended to post
    nonblocking requests in the increasing order of variable IDs and
    fixed-size variables followed by record variables.

* New Limitations
  + ADIOS driver are ready only. No vard, varn, low-level, and nonblocking support.

* Update configure options
  + Enable ADIOS support.
    - `--enable-adios`: enable NetCDF4 format classic mode support
    - `--with-adios=/path/to/netcdf-4`: path to NetCDF-4 library installation
  + When building with NetCDF-4 feature, using NetCDF-4 library built with
    PnetCDF enabled, i.e. --enable-pnetcdf, is not supported. See
    [Issue #33](https://github.com/Parallel-NetCDF/PnetCDF/issues/33).

* Update configure options
  + none

* New constants
  + none

* New APIs
  + none

* API syntax changes
  + none

* API semantics updates
  + none

* New error code precedence
  + none

* Updated error strings
  + none

* New error code
  + none

* New PnetCDF hint
  + none

* New run-time environment variables
  + none

* Build recipes
  + none

* New/updated utility program
  + none

* Other updates:
  + Add a check whether the MPI library is built with shared-library support.
    If not and `--enable-shared` is used, configure process of PnetCDF will
    fail.
  + In NetCDF-4 driver, `nc4io_inq_var()` adds a check of arguments `no_fill`
    and `fill_value` for NULL. If both are NULL, it skips the call to
    `nc_inq_var_fill`.
  + File header extent area between end of header and first variable is padded
    with null bytes if PnetCDF is configured with option
    `--enable-null-byte-header-padding`.

* Bug fixes
  + Fix error checking for programs in examples/C to ignore NC_ENOTENABLED
    if PnetCDF was not built with --enable-profiling. Thanks to Bruno Pagani
    and see [Issue #34](https://github.com/Parallel-NetCDF/PnetCDF/issues/34).

* New example programs
  + examples/adios/read_all.c - Dump all metadata in a ADIOS BP file.

* New programs for I/O benchmarks
  + none

* New test program
  + test/adios/open.c - tests if PnetCDF recognize ADIOS file.
  + test/adios/header.c - tests if PnetCDF can parse ADIOS header.
  + test/adios/var.c - tests if PnetCDF can access ADIOS variables.
  + test/adios/varm.c - tests if PnetCDF can access ADIOS variables with discontiguous memory.
  + test/adios/vars.c - tests if PnetCDF access ADIOS variables with stride.
  + test/adios/atts.c - tests if PnetCDF access ADIOS attributes.
  + test/burst_buffer/varn.c -- to test varn API when burst buffer driver is
    used. The test includes cases when argument counts is NULL or some of the
    elements in counts are NULL.

* Conformity with NetCDF library
  + none

* Discrepancy from NetCDF library
  + none

* Issues related to MPI library vendors:
  + none

* Issues related to Darshan library:
  + none

* Clarifications
  + Padding -- NetCDF classic file format specification states "Header padding
    uses null (\x00) bytes. In data, padding uses variable's fill value."
    PnetCDF implements the header padding specification but only enforces it
    when the configure option `--enable-null-byte-header-padding` is set. Note
    PnetCDF has not yet implemented the padding for data section.


