#ifndef _NB_VERLET_
#define _NB_VERLET_

#include "cutypedefs_ext.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Atom locality indicator: local, non-local, all (used for f, x). */
enum { eatLocal = 0, eatNonlocal = 1, eatAll  };

#define LOCAL_A(x)               ((x) == eatLocal)
#define NONLOCAL_A(x)            ((x) == eatNonlocal)
#define LOCAL_OR_NONLOCAL_A(x)   (LOCAL_A(x) || NONLOCAL_A(x))

/* Interaction locality indicator (used in pair-list search/calculations): 
    - local interactions require local atom data and affect local output only;
    - non-local interactions require both local and non-local atom data and 
      affect both local- and non-local output. */
enum { eintLocal = 0, eintNonlocal = 1 };

#define LOCAL_I(x)               ((x) == eintLocal)
#define NONLOCAL_I(x)            ((x) == eintNonlocal)

typedef struct {
    nbnxn_pairlist_set_t nbl_lists; /* pair list(s) */
    nbnxn_atomdata_t *nbat;         /* atom data */
    int         kernel_type;        /* non-bonded kernel - see enum above */
} nonbonded_verlet_group_t;

/* non-bonded data structure with Verlet-type cut-off */
typedef struct {
    gmx_nbsearch_t    nbs;          /* bounding box type neighbor searching data */
    int nloc;                       /* number of interaction groups */
    nonbonded_verlet_group_t grp[2]; /* local and non-local interaction group */

    gmx_bool    useGPU;             /* TRUE when GPU acceleration is used */

    /* GPU/CUDA nonbonded data structure */
    cu_nonbonded_t  gpu_nb;          /* CUDA non-bonded data  */
    int             min_ci_balanced; /* neighbor-lists are balanced to this size 
                                       - only used for the 8x8x8 CUDA kernels */
} nonbonded_verlet_t;

#ifdef __cplusplus
}
#endif

#endif /* _NB_VERLET_ */