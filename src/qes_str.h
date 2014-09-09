/*
 * ============================================================================
 *
 *       Filename:  qes_str.h
 *
 *    Description:  String handling functions
 *
 *        Version:  1.0
 *        Created:  22/03/14 13:00:00
 *       Revision:  none
 *        License:  GPLv3+
 *       Compiler:  gcc, clang
 *
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#ifndef QES_STR_H
#define QES_STR_H

#include <qes_util.h>

struct qes_str {
    char *s;
    size_t l;
    size_t m;
};


/*===  FUNCTION  ============================================================*
Name:           qes_str_ok
Parameters:     const struct qes_str *str: `struct qes_str` ref to check.
Description:    Checks if a string is OK to use, i.e. checks that `str->s` is
                a valid `char` array.
Returns:        An `int` that evaluates to true or false.
 *===========================================================================*/
static inline int
qes_str_ok (const struct qes_str *str)
{
    return str != NULL && str->s != NULL && str->m > 0;
}

/*===  FUNCTION  ============================================================*
Name:           qes_str_init
Parameters:     struct qes_str *str: String to initialise (normally on the stack).
                size_t len: Initial capacity of `struct qes_str`.
Description:    Initialise a `struct qes_str` (typically on the stack) with room for
                `len` characters.
Returns:        void
 *===========================================================================*/
static inline void
qes_str_init (struct qes_str *str, size_t capacity)
{
    if (str == NULL) return;
    str->l = 0;
    str->s = qes_calloc(capacity, sizeof(*str->s));
    str->m = capacity;
}

/*===  FUNCTION  ============================================================*
Name:           qes_str_create
Parameters:     size_t len: Initial capacity of `struct qes_str`.
Description:    Create a `struct qes_str` on the heap with room for `len` characters.
Returns:        `struct qes_str *` Pointer to a memory address.
 *===========================================================================*/
static inline struct qes_str *
qes_str_create (size_t capacity)
{
    struct qes_str *str = qes_calloc(1, sizeof(*str));

    /* We don't worry about NULL here. qes_str_init will return before
     * derefencing and we'll return NULL below. */
    qes_str_init(str, capacity);
    return str;
}

static inline int
qes_str_fill_charptr (struct qes_str *str, const char *cp, size_t len)
{
    if (str == NULL || cp == NULL) return 0;
    if (len == 0) {
        len = strlen(cp);
    }
    if (str->m < len + 1) {
        while (str->m < len + 1) {
            str->m = qes_roundupz(str->m);
        }
        str->s = qes_realloc(str->s, str->m * sizeof(*str->s));
    }
    /* FIXME: check for null after realloc */
    memcpy(str->s, cp, len);
    str->s[len] = '\0';
    str->l = len;
    return 1;
}

/*===  FUNCTION  ============================================================*
Name:           qes_str_nullify
Paramters:      struct qes_str *str: `struct qes_str` to nullify.
Description:    Invalidates a `struct qes_str` without freeing the `char []`.
Returns:        int: 0 on success, otherwise 1.
 *===========================================================================*/
static inline int
qes_str_nullify (struct qes_str *str)
{
    if (!qes_str_ok(str)) return 1;
    str->s[0] = '\0';
    str->l = 0;
    return 0;
}

static inline int
qes_str_copy (struct qes_str *dest, const struct qes_str *src)
{
    if (!qes_str_ok(src) || dest == NULL) return 1;
    if (!qes_str_ok(dest)) qes_str_init(dest, src->m);
    memcpy(dest->s, src->s, src->m);
    return 0;
}


extern void qes_str_print (const struct qes_str *str, FILE *stream);

/*===  FUNCTION  ============================================================*
Name:           qes_str_destroy
Paramters:      struct qes_str *: `struct qes_str` to destroy.
Description:    Frees `str->s` and the struct qes_str struct itself.
Returns:        void
 *===========================================================================*/
extern void qes_str_destroy (struct qes_str *str);

/*===  FUNCTION  ============================================================*
Name:           qes_str_destroy_cp
Paramters:      struct qes_str *: String to destrop
Description:    Frees `str->s` without freeing the struct qes_str struct itself. For use
                on `struct qes_str`s allocated on the stack.
Returns:        void
 *===========================================================================*/
extern void qes_str_destroy_cp (struct qes_str *str);


#endif /* QES_STR_H */