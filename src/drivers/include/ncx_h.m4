dnl Process this m4 file to produce 'C' language file.
dnl
dnl If you see this line, you can ignore the next one.
/* Do not edit this file. It is produced from the corresponding .m4 source */
dnl
/*
 *  Copyright (C) 2003, Northwestern University and Argonne National Laboratory
 *  See COPYRIGHT notice in top-level directory.
 */
/* "$Id$" */

#ifndef _NCX_H
#define _NCX_H

#include <stddef.h>    /* size_t */
#include <sys/types.h> /* off_t */
#include <errno.h>

#include <mpi.h>

#include <common.h>

/*
 * An external data representation interface.
 *
 * This started out as a general replacement for ONC XDR,
 * specifically, the xdrmem family of functions.
 *
 * We eventually realized that we could write more portable
 * code if we decoupled any association between the 'C' types
 * and the external types. (XDR has this association between the 'C'
 * types and the external representations, like xdr_int() takes
 * an int argument and goes to an external int representation.)
 * So, now there is a matrix of functions.
 *
 */

#if defined(_CRAY) && !defined(_CRAYIEEE) && !defined(__crayx1)
#define CRAYFLOAT 1 /* CRAY Floating point */
#elif defined(_SX) && defined(_FLOAT2) /* NEC SUPER-UX in CRAY mode */
#define CRAYFLOAT 1 /* CRAY Floating point */
#endif

/*
 * External sizes of the primitive elements.
 */
#define X_SIZEOF_BYTE		1
#define X_SIZEOF_CHAR		1
#define X_SIZEOF_SHORT		2
#define X_SIZEOF_INT		4	/* xdr_int */
#if 0
#define X_SIZEOF_LONG		8 */	/* xdr_long_long */
#endif
#define X_SIZEOF_FLOAT		4
#define X_SIZEOF_DOUBLE		8

/* additional data types in CDF-5 */
#define X_SIZEOF_UBYTE		1
#define X_SIZEOF_USHORT		2
#define X_SIZEOF_UINT		4
#define X_SIZEOF_LONGLONG	8
#define X_SIZEOF_ULONGLONG	8
#define X_SIZEOF_INT64		8
#define X_SIZEOF_UINT64		8

/*
 * For now, netcdf is limited to 32 bit sizes,
 * If compiled with support for "large files", then
 * netcdf will use a 64 bit off_t and it can then write a file
 * using 64 bit offsets.
 *  see also X_SIZE_MAX, X_OFF_MAX below
 */
/* #define X_SIZEOF_OFF_T	(sizeof(off_t)) */
#define X_SIZEOF_OFF_T		SIZEOF_OFF_T
#define X_SIZEOF_SIZE_T		X_SIZEOF_INT

/*
 * limits of the external representation
 */
#define X_SCHAR_MIN	(-128)
#define X_SCHAR_MAX	127
#define X_UCHAR_MAX	255U
#define X_SHORT_MIN	(-32768)
#define X_SHRT_MIN	X_SHORT_MIN	/* alias compatible with limits.h */
#define X_SHORT_MAX	32767
#define X_SHRT_MAX	X_SHORT_MAX	/* alias compatible with limits.h */
#define X_USHORT_MAX	65535U
#define X_USHRT_MAX	X_USHORT_MAX	/* alias compatible with limits.h */
#define X_INT_MIN	(-2147483647-1)
#define X_INT_MAX	2147483647
#define X_UINT_MAX	4294967295U
#define X_INT64_MIN	(-9223372036854775807LL-1LL)
#define X_INT64_MAX	9223372036854775807LL
#define X_UINT64_MAX	18446744073709551615ULL
#define X_FLOAT_MAX	3.402823466e+38f
#define X_FLOAT_MIN	(-X_FLOAT_MAX)
#define X_FLT_MAX	X_FLOAT_MAX	/* alias compatible with limits.h */
#if defined(CRAYFLOAT) && CRAYFLOAT != 0
/* ldexp(1. - ldexp(.5 , -46), 1024) */
#define X_DOUBLE_MAX	1.79769313486230e+308
#else
/* scalb(1. - scalb(.5 , -52), 1024) */
#define X_DOUBLE_MAX	1.7976931348623157e+308
#endif
#define X_DOUBLE_MIN	(-X_DOUBLE_MAX)
#define X_DBL_MAX	X_DOUBLE_MAX	/* alias compatible with limits.h */

#define X_SIZE_MAX	X_UINT_MAX
#define X_OFF_MAX	X_INT_MAX


/* Begin ncmpix_len */

/*
 * ncmpix_len_xxx() interfaces are defined as macros below,
 * These give the length of an array of nelems of the type.
 * N.B. The 'char' and 'short' interfaces give the X_ALIGNED length.
 */
#define X_ALIGN		4	/* a.k.a. BYTES_PER_XDR_UNIT */

#define ncmpix_len_char(nelems) \
	_RNDUP((nelems), X_ALIGN)

#define ncmpix_len_short(nelems) \
	(((nelems) + (nelems)%2)  * X_SIZEOF_SHORT)

#define ncmpix_len_int(nelems) \
	((nelems) * X_SIZEOF_INT)

#define ncmpix_len_long(nelems) \
	((nelems) * X_SIZEOF_LONG)

#define ncmpix_len_float(nelems) \
	((nelems) * X_SIZEOF_FLOAT)

#define ncmpix_len_double(nelems) \
	((nelems) * X_SIZEOF_DOUBLE)

#define ncmpix_len_ubyte(nelems) \
	_RNDUP((nelems), X_ALIGN)

#define ncmpix_len_ushort(nelems) \
	(((nelems) + (nelems)%2)  * X_SIZEOF_USHORT)

#define ncmpix_len_uint(nelems) \
	((nelems) * X_SIZEOF_UINT)

#define ncmpix_len_int64(nelems) \
	((nelems) * X_SIZEOF_INT64)

#define ncmpix_len_uint64(nelems) \
	((nelems) * X_SIZEOF_UINT64)

/* End ncmpix_len */

/*
 * Primitive numeric conversion functions.
 * The ``put'' functions convert from native internal
 * type to the external type, while the ``get'' functions
 * convert from the external to the internal.
 *
 * These take the form
 *	int ncmpix_get_{external_type}_{internal_type}(
 *		const void *xp,
 *		internal_type *ip
 *	);
 *	int ncmpix_put_{external_type}_{internal_type}(
 *		void *xp,
 *		const internal_type *ip
 *	);
 * where
 *	``external_type'' chosen from
 *      	NC_CHAR
 *      	NC_BYTE
 *      	NC_SHORT
 *      	NC_INT
 *      	NC_FLOAT
 *      	NC_DOUBLE
 *      	NC_UBYTE
 *      	NC_USHORT
 *      	NC_UINT
 *      	NC_INT64
 *      	NC_UINT64
 *
 *	``internal_type'' chosen from
 *      	schar
 *      	uchar
 *      	short
 *      	int
 *      	long
 *      	float
 *      	double
 *      	ushort
 *      	uint
 *      	longlong
 *      	ulonglong
 *
 * Not all combinations make sense, i.e. those causing NC_ECHAR.
 * We may not implement all combinations that make sense.
 * The netcdf functions that use this ncmpix interface do not
 * use these primitive conversion functions. They use the
 * aggregate conversion functions declared below.
 *
 * Storage for a single element of external type is at the ``void * xp''
 * argument.
 *
 * Storage for a single element of internal type is at ``ip'' argument.
 *
 * These functions return NC_NOERR when no error occurred,
 * or NC_ERANGE when the value being converted is too large.
 * When NC_ERANGE occurs, an undefined (implementation dependent)
 * conversion may have occurred.
 *
 * Note that loss of precision may occur silently.
 *
 */

/*
 * Other primitive conversion functions
 * N.B. slightly different interface
 * Used by netcdf.
 */

/* ncmpix_get_int_size_t */
extern int
ncmpix_get_size_t(const void **xpp, size_t *ulp);
/* ncmpix_get_int_off_t */
extern int
ncmpix_get_off_t(const void **xpp, off_t *lp, size_t sizeof_off_t);

/* ncmpix_put_int_size_t */
extern int
ncmpix_put_size_t(void **xpp, const size_t *ulp);
/* ncmpix_put_int_off_t */
extern int
ncmpix_put_off_t(void **xpp, const off_t *lp, size_t sizeof_off_t);

extern int
ncmpix_get_uint32(const void **xpp, unsigned int *ip);
extern int
ncmpix_get_uint64(const void **xpp, unsigned long long *ip);
extern int
ncmpix_put_uint32(void **xpp, const unsigned int ip);
extern int
ncmpix_put_uint64(void **xpp, const unsigned long long ip);


/*
 * Aggregate numeric conversion functions.
 * Convert an array.  Replaces xdr_array(...).
 * These functions are used by netcdf. Unlike the xdr
 * interface, we optimize for aggregate conversions.
 * This functions should be implemented to take advantage
 * of multiple processor / parallel hardware where available.
 *
 * These take the form
 *	int ncmpix_getn_{external_type}_{internal_type}(
 *		const void *xpp,
 *		size_t nelems,
 *		internal_type *ip
 *	);
 *	int ncmpix_putn_{external_type}_{internal_type}(
 *		void **xpp,
 *		size_t nelems,
 *		const internal_type *ip
 *	);
 * Where the types are as in the primitive numeric conversion functions.
 *
 * The value of the pointer to pointer argument, *xpp, is
 * expected to reference storage for ``nelems'' of the external
 * type.  On return, it modified to reference just past the last
 * converted external element.
 *
 * The types whose external size is less than X_ALIGN also have ``pad''
 * interfaces. These round (and zero fill on put) *xpp up to X_ALIGN
 * boundaries. (This is the usual xdr behavior.)
 *
 * The ``ip'' argument should point to an array of ``nelems'' of
 * internal_type.
 *
 * Range errors (NC_ERANGE) for a individual values in the array
 * DO NOT terminate the array conversion. All elements are converted,
 * with some having undefined values.
 * If any range error occurs, the function returns NC_ERANGE.
 *
 */

include(`foreach.m4')

dnl
dnl CONV_GET_XTYPE_ITYPE(op, xtype, itype)
dnl
define(`CONV_GET_XTYPE_ITYPE',dnl
`dnl
extern int
ncmpix_$1_$2_$3(const void **xpp, MPI_Offset nelems, $3 *ip);
')dnl
dnl
dnl CONV_PUT_XTYPE_ITYPE(op, xtype, itype)
dnl
define(`CONV_PUT_XTYPE_ITYPE',dnl
`dnl
extern int
ncmpix_$1_$2_$3(void **xpp, MPI_Offset nelems, const $3 *ip, void *fillp);
')dnl
dnl
define(`_concat3', $1$2$3)dnl
dnl
dnl for external types that need padding
foreach(`xtype', (NC_BYTE, NC_UBYTE, NC_SHORT, NC_USHORT),
`/*---- xtype ----------------------------------------------------------------*/
foreach(`op', (getn, pad_getn),
`foreach(`itype', (schar, uchar, short, ushort, int, uint, long, float, double, longlong, ulonglong),
                  `_concat3(CONV_GET_XTYPE_ITYPE(op,xtype,itype))')
')dnl
foreach(`op', (putn, pad_putn),
`foreach(`itype', (schar, uchar, short, ushort, int, uint, long, float, double, longlong, ulonglong),
                  `_concat3(CONV_PUT_XTYPE_ITYPE(op,xtype,itype))')
')'dnl
)
dnl
dnl for external types that need no padding
foreach(`xtype', (NC_INT, NC_UINT, NC_FLOAT, NC_DOUBLE, NC_INT64, NC_UINT64),
`/*---- xtype ----------------------------------------------------------------*/
foreach(`op', (getn),
`foreach(`itype', (schar, uchar, short, ushort, int, uint, long, float, double, longlong, ulonglong),
                  `_concat3(CONV_GET_XTYPE_ITYPE(op,xtype,itype))')
')dnl
foreach(`op', (putn),
`foreach(`itype', (schar, uchar, short, ushort, int, uint, long, float, double, longlong, ulonglong),
                  `_concat3(CONV_PUT_XTYPE_ITYPE(op,xtype,itype))')
')'dnl
)
dnl
/*
 * Other aggregate conversion functions.
 */

/* read ASCII characters */
extern int
ncmpix_getn_text(const void **xpp, MPI_Offset nchars, char *cp);
extern int
ncmpix_pad_getn_text(const void **xpp, MPI_Offset nchars, char *cp);

/* write ASCII characters */
extern int
ncmpix_putn_text(void **xpp, MPI_Offset nchars, const char *cp);
extern int
ncmpix_pad_putn_text(void **xpp, MPI_Offset nchars, const char *cp);

/* for symmetry */
#define ncmpix_getn_char_char(xpp, nelems, fillp) ncmpix_getn_text(xpp, nelems, fillp)
#define ncmpix_putn_char_char(xpp, nelems, fillp) ncmpix_putn_text(xpp, nelems, fillp)

/* read opaque data */
extern int
ncmpix_getn_void(const void **xpp, MPI_Offset nchars, void *vp);
extern int
ncmpix_pad_getn_void(const void **xpp, MPI_Offset nchars, void *vp);

/* write opaque data */
extern int
ncmpix_putn_void(void **xpp, MPI_Offset nchars, const void *vp);
extern int
ncmpix_pad_putn_void(void **xpp, MPI_Offset nchars, const void *vp);

#endif /* _NCX_H_ */
