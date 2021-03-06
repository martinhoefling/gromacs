/* -*- mode: c; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; c-file-style: "stroustrup"; -*-
 *
 * This file is part of GROMACS.
 * Copyright (c) 2012-
 *
 * Written by the Gromacs development team under coordination of
 * David van der Spoel, Berk Hess, and Erik Lindahl.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * To help us fund GROMACS development, we humbly ask that you cite
 * the research papers on the package. Check out http://www.gromacs.org
 *
 * And Hey:
 * Gnomes, ROck Monsters And Chili Sauce
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <stdio.h>
#include <string.h>

#include "nb_kernel.h"
#include "smalloc.h"
#include "string2.h"
#include "gmx_fatal.h"


/* Static data structures to find kernels */
static nb_kernel_info_t *   kernel_list           = NULL;
static unsigned int         kernel_list_size      = 0;
static int *                kernel_list_hash      = NULL;
static unsigned int         kernel_list_hash_size = 0;

static unsigned int
nb_kernel_hash_func(const char *   arch,
                    const char *   elec,
                    const char *   elec_mod,
                    const char *   vdw,
                    const char *   vdw_mod,
                    const char *   geom,
                    const char *   other,
                    const char *   vf)
{
    unsigned int hash;

    hash = gmx_string_hash_func(arch,gmx_string_hash_init);
    hash = gmx_string_hash_func(elec,hash);
    hash = gmx_string_hash_func(elec_mod,hash);
    hash = gmx_string_hash_func(vdw,hash);
    hash = gmx_string_hash_func(vdw_mod,hash);
    hash = gmx_string_hash_func(geom,hash);
    hash = gmx_string_hash_func(other,hash);
    hash = gmx_string_hash_func(vf,hash);

    return hash;
}

void
nb_kernel_list_add_kernels(nb_kernel_info_t *   new_kernel_list,
                           int                  new_kernel_list_size)
{
    srenew(kernel_list,kernel_list_size+new_kernel_list_size);
    memcpy(kernel_list+kernel_list_size,new_kernel_list,new_kernel_list_size*sizeof(nb_kernel_info_t));
    kernel_list_size += new_kernel_list_size;
}


int
nb_kernel_list_hash_init(void)
{
    unsigned int            i;
    unsigned int            index;

    kernel_list_hash_size   = kernel_list_size*5;
    snew(kernel_list_hash,kernel_list_hash_size);

    for(i=0;i<kernel_list_hash_size;i++)
    {
        kernel_list_hash[i] = -1;
    }
    for(i=0;i<kernel_list_size;i++)
    {
        index=nb_kernel_hash_func(kernel_list[i].architecture,
                                  kernel_list[i].electrostatics,
                                  kernel_list[i].electrostatics_modifier,
                                  kernel_list[i].vdw,
                                  kernel_list[i].vdw_modifier,
                                  kernel_list[i].geometry,
                                  kernel_list[i].other,
                                  kernel_list[i].vf) % kernel_list_hash_size;

        /* Check for collisions and advance if necessary */
        while( kernel_list_hash[index] != -1 )
        {
            index = (index+1) % kernel_list_hash_size;
        }

        kernel_list_hash[index] = i;
    }
    return 0;
}

void
nb_kernel_list_hash_destroy()
{
    sfree(kernel_list_hash);
    kernel_list_hash = NULL;
    kernel_list_hash_size = 0;
}


nb_kernel_t *
nb_kernel_list_findkernel(FILE *              log,
                          const char *        arch,
                          const char *        electrostatics,
                          const char *        electrostatics_modifier,
                          const char *        vdw,
                          const char *        vdw_modifier,
                          const char *        geometry,
                          const char *        other,
                          const char *        vf)
{
    int                 i;
    unsigned int        index;
    nb_kernel_info_t *  kernelinfo_ptr;

    if(kernel_list_hash_size==0)
    {
        return NULL;
    }

    index=nb_kernel_hash_func(arch,
                              electrostatics,
                              electrostatics_modifier,
                              vdw,
                              vdw_modifier,
                              geometry,
                              other,
                              vf) % kernel_list_hash_size;

    kernelinfo_ptr = NULL;
    while( (i=kernel_list_hash[index]) != -1)
    {
        if(!gmx_strcasecmp_min(kernel_list[i].architecture,arch) &&
           !gmx_strcasecmp_min(kernel_list[i].electrostatics,electrostatics) &&
           !gmx_strcasecmp_min(kernel_list[i].electrostatics_modifier,electrostatics_modifier) &&
           !gmx_strcasecmp_min(kernel_list[i].vdw,vdw) &&
           !gmx_strcasecmp_min(kernel_list[i].vdw_modifier,vdw_modifier) &&
           !gmx_strcasecmp_min(kernel_list[i].geometry,geometry) &&
           !gmx_strcasecmp_min(kernel_list[i].other,other) &&
           !gmx_strcasecmp_min(kernel_list[i].vf,vf))
        {
            kernelinfo_ptr = kernel_list+i;
            break;
        }
        index = (index+1) % kernel_list_hash_size;
    }

    if(debug && kernelinfo_ptr!=NULL)
    {
        fprintf(debug,
                "NB kernel %s() with architecture '%s' used for neighborlist with\n"
                "    Elec: '%s', Modifier: '%s'\n"
                "    Vdw:  '%s', Modifier: '%s'\n"
                "    Geom: '%s', Other: '%s', Calc: '%s'\n\n",
                kernelinfo_ptr->kernelname,arch,electrostatics,electrostatics_modifier,
                vdw,vdw_modifier,geometry,other,vf);
    }

    /* If we did not find any kernel the pointer will still be NULL */
    return (kernelinfo_ptr !=NULL) ? kernelinfo_ptr->kernelptr : NULL;
}
