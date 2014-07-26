/*
 * ============================================================================
 *
 *       Filename:  kmutil.h
 *
 *    Description:  Wrappers around std library functions
 *
 *        Version:  1.0
 *        Created:  04/05/14 14:10:50
 *       Revision:  none
 *        License:  GPLv3+
 *       Compiler:  gcc, clang
 *
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#ifndef KMUTIL_H
#define KMUTIL_H

/* #####   HEADER FILE INCLUDES   ########################################## */
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>


/*
 * Cross-platform bollocks. Thanks windows.
 */

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#define KM_PATHSEP "\\"
#else
#define KM_PATHSEP "/"
#include <unistd.h>
#endif



/*
 * Misc constants
 */

extern const char *kmlib_version;

#define KM_MAX_FN_LEN (1<<16)
/* Size of buffers for file IO */
#define    KM_FILEBUFFER_LEN (1<<20)
/* Starting point for allocing a char pointer. Set to slightly larger than the
   standard size of whatever you're reading in. */
#define    __INIT_LINE_LEN (128)

/*
 * Macro helpers from tor
 */

/* Expands to a syntactically valid empty statement.  */
#define STMT_NIL (void)0

/* Expands to a syntactically valid empty statement, explicitly (void)ing its
 * argument. */
#define STMT_VOID(a) while (0) { (void)(a); }

/* STMT_BEGIN and STMT_END are used to wrap blocks inside macros so that
 * the macro can be used as if it were a single C statement. */
#ifdef __GNUC__
    #define STMT_BEGIN (void) ({
    #define STMT_END })
#elif defined(sun) || defined(__sun__)
    #define STMT_BEGIN if (1) {
    #define STMT_END } else STMT_NIL
#else
    #define STMT_BEGIN do {
    #define STMT_END } while (0)
#endif

/* This can be helpful in some macros, particularly with #pragma */
#ifndef STRINGIFY
    #define STRINGIFY(a) #a
#endif

/*
 * General helper macros
 */
#define km_likely(x)      __builtin_expect(!!(x), 1)
#define km_unlikely(x)    __builtin_expect(!!(x), 0)


/*
 * Error handling functions
 */

/* use the stdlib exit function by default, during testing we can #define this
   to some kind of error handler if we need to. */
#ifndef  KM_EXIT_FN
    #define  KM_EXIT_FN exit
#endif


/* By default, we use this error handler. At compile or include time, we can
   chose another more appropriate one if we need to. */
#ifndef  KM_DEFAULT_ERR_FN
    #define  KM_DEFAULT_ERR_FN errprintexit
#endif


#define ERRFN_ARGS const char *msg,  const char *file, int line, ...
void errnil(ERRFN_ARGS);
void errprint (ERRFN_ARGS);
void errprintexit (ERRFN_ARGS)  __attribute__ ((noreturn));
typedef void (*errhandler_t) (const char*, const char *, int, ...);

/*
 * Memory allocation/deallocation
 */

extern void *km_calloc_ (size_t n, size_t size, errhandler_t onerr,
        const char *file, int line);
#define km_calloc(n, sz) \
    km_calloc_(n, sz, KM_DEFAULT_ERR_FN, __FILE__, __LINE__)
#define km_calloc_errnil(n, sz) \
    km_calloc_(n, sz, errnil, __FILE__, __LINE__)
#define km_calloc_errprint(n, sz) \
    km_calloc_(n, sz, errprint, __FILE__, __LINE__)
#define km_calloc_errprintexit(n, sz) \
    km_calloc_(n, sz, errprintexit, __FILE__, __LINE__)

extern void *km_malloc_ (size_t size, errhandler_t onerr, const char *file,
        int line);
#define km_malloc(sz) \
    km_malloc_(sz, KM_DEFAULT_ERR_FN, __FILE__, __LINE__)
#define km_malloc_errnil(sz) \
    km_malloc_(sz, errnil, __FILE__, __LINE__)
#define km_malloc_errprint(sz) \
    km_malloc_(sz, errprint, __FILE__, __LINE__)
#define km_malloc_errprintexit(sz) \
    km_malloc_(sz, errprintexit, __FILE__, __LINE__)

extern void *km_realloc_ (void *data, size_t size, errhandler_t onerr,
        const char *file, int line);
#define km_realloc(ptr, sz) \
    km_realloc_(ptr, sz, KM_DEFAULT_ERR_FN, __FILE__, __LINE__)
#define km_realloc_errnil(ptr, sz) \
    km_realloc_(ptr, sz, errnil, __FILE__, __LINE__)
#define km_realloc_errprint(ptr, sz) \
    km_realloc_(ptr, sz, errprint, __FILE__, __LINE__)
#define km_realloc_errprintexit(ptr, sz) \
    km_realloc_(ptr, sz, errprintexit, __FILE__, __LINE__)
#define km_free(data)               \
    STMT_BEGIN                      \
    if (data != NULL) {             \
        free(data);                 \
        data = NULL;                \
    }                               \
    STMT_END

/* kmroundupz:
 *   Round up a `size_t` to the next highest power of two.
 */
/* Flogged from http://stackoverflow.com/a/1322548 and
   http://graphics.stanford.edu/~seander/bithacks.html, and kseq.h */
/* Round a 32-bit int up to nearest base-2 number */
static inline size_t
kmroundupz (size_t sz)
{
    /* Decrement v only if v is not already a power of 2 */
    /* I.e, roundup things already a power of 2 */
    if (km_unlikely((sz & (sz - 1)) != 0)) sz--;
    /* mask all bits below MSB to 1 */
    sz |= sz>>1;
    sz |= sz>>2;
    sz |= sz>>4;
    sz |= sz>>8;
    sz |= sz>>16;
#if UINTPTR_MAX == 0xffffffffffffffff /* 64-bit system */
    sz |= sz>>32;
#endif
    return sz + 1;
}

static inline uint32_t
kmroundup32 (uint32_t u32)
{
    /* Roundup things already a power of 2 */
    if ((u32 & (u32 - 1)) != 0) u32--;
    /* mask all bits below MSB to 1 */
    u32 |= u32>>1;
    u32 |= u32>>2;
    u32 |= u32>>4;
    u32 |= u32>>8;
    u32 |= u32>>16;
    return u32 + 1;
}

static inline uint64_t
kmroundup64 (uint64_t u64)
{
    /* Roundup things already a power of 2 */
    if ((u64 & (u64 - 1)) != 0) u64--;
    /* mask all bits below MSB to 1 */
    u64 |= u64>>1;
    u64 |= u64>>2;
    u64 |= u64>>4;
    u64 |= u64>>8;
    u64 |= u64>>16;
    u64 |= u64>>32;
    return u64 + 1;
}




/* IO helpers */

/* Definitions to make changing fp type easy */
#ifdef HAVE_ZLIB
#   include <zlib.h>
#   define    KM_ZTYPE gzFile
#   define    KM_ZOPEN gzopen
#   define    KM_ZDOPEN gzdopen
#   define    KM_ZCLOSE gzclose
#   define    KM_ZREAD gzread
#   define    KM_ZWRITE gzwrite
#   define    KM_ZFLUSH gzflush
#   define    KM_ZFPRINTF gzprintf
#   define    KM_ZFPUTS gzputs
#   define    KM_ZFPUTC gzputc
#   define    KM_ZFGETS gzgets
#   define    KM_ZFGETC gzgetc
#   define    KM_ZFUNGETC gzungetc
#   define    KM_ZERR gzerror
#   define    KM_ZEOF gzeof
#   define    KM_ZBUFFER gzbuffer
#   define    KM_ZSEEK gzseek
#   define    KM_ZTELL gztell
#   define    KM_ZREWIND gzrewind
#else
#   define    KM_ZTYPE FILE*
#   define    KM_ZOPEN fopen
#   define    KM_ZCLOSE fclose
#   define    KM_ZDOPEN fdopen
#   define    KM_ZCLOSE fclose
#   define    KM_ZREAD fread
#   define    KM_ZWRITE fwrite
#   define    KM_ZFLUSH fflush
#   define    KM_ZFPRINTF fprintf
#   define    KM_ZFPUTS fputs
#   define    KM_ZFPUTC fputc
#   define    KM_ZFGETS fgets
#   define    KM_ZFGETC fgetc
#   define    KM_ZFUNGETC fungetc
#   define    KM_ZERR ferror
#   define    KM_ZEOF feof
#   define    KM_ZBUFFER fbuffer
#   define    KM_ZSEEK fseek
#   define    KM_ZTELL ftell
#   define    KM_ZREWIND frewind
#endif

typedef enum __rwmode {
    RW_UNKNOWN,
    RW_READ,
    RW_WRITE,
    RW_READWRITE,
} rwmode_t;

#endif /* KMUTIL_H */
