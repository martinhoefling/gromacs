/*
 *
 *                This source code is part of
 *
 *                 G   R   O   M   A   C   S
 *
 *          GROningen MAchine for Chemical Simulations
 *
 * Written by David van der Spoel, Erik Lindahl, Berk Hess, and others.
 * Copyright (c) 1991-2000, University of Groningen, The Netherlands.
 * Copyright (c) 2001-2009, The GROMACS development team,
 * check out http://www.gromacs.org for more information.

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * If you want to redistribute modifications, please consider that
 * scientific software is very special. Version control is crucial -
 * bugs must be traceable. We will be happy to consider code for
 * inclusion in the official distribution, but derived work must not
 * be called official GROMACS. Details are found in the README & COPYING
 * files - if they are missing, get the official version at www.gromacs.org.
 *
 * To help us fund GROMACS development, we humbly ask that you cite
 * the papers on the package - you can find them in the top README file.
 *
 * For more info, check our website at http://www.gromacs.org
 */
/*! \internal \file
 * \brief Internal header file used by the selection tokenizer.
 *
 * \author Teemu Murtola <teemu.murtola@cbr.su.se>
 * \ingroup module_selection
 */
#ifndef SELECTION_SCANNER_INTERNAL_H
#define SELECTION_SCANNER_INTERNAL_H

#include <boost/exception_ptr.hpp>

#include "parser.h"

namespace gmx
{
class MessageStringCollector;
}

/* These need to be defined before including scanner_flex.h, because it
 * uses YY_EXTRA_TYPE. But we also need to include it before defining
 * gmx_sel_lexer_t; hence the forward declaration. */
struct gmx_sel_lexer_t;
#define YY_EXTRA_TYPE struct gmx_sel_lexer_t *

/* We cannot include scanner_flex.h from the scanner itself, because it
 * seems to break everything. */
/* And we need to define YY_NO_UNISTD_H here as well, otherwise unistd.h
 * gets included in other files than scanner.cpp... */
#ifndef FLEX_SCANNER
#define YY_NO_UNISTD_H
#include "scanner_flex.h"
#endif

/*! \internal \brief
 * Internal data structure for the selection tokenizer state.
 */
typedef struct gmx_sel_lexer_t
{
    //! Selection collection to put parsed selections in.
    struct gmx_ana_selcollection_t  *sc;
    //! Error reporter object.
    gmx::MessageStringCollector     *errors;
    //! Stores an exception that occurred during parsing.
    boost::exception_ptr             exception;
    //! Whether external index groups have been set.
    bool                             bGroups;
    //! External index groups for resolving \c group keywords.
    struct gmx_ana_indexgrps_t      *grps;
    //! Number of selections at which the parser should stop.
    int                              nexpsel;

    //! Whether the parser is interactive.
    bool                             bInteractive;

    //! Pretty-printed version of the string parsed since last clear.
    char                            *pselstr;
    //! Length of the string in \a pselstr.
    int                              pslen;
    //! Number of bytes allocated for \a pselstr.
    int                              nalloc_psel;

    //! Stack of methods in which parameters should be looked up.
    struct gmx_ana_selmethod_t     **mstack;
    //! Index of the top of the stack in \a mstack.
    int                              msp;
    //! Number of elements allocated for \a mstack.
    int                              mstack_alloc;

    //! Number of END_OF_METHOD tokens to return before \a nextparam.
    int                              neom;
    //! Parameter symbol to return before resuming scanning.
    struct gmx_ana_selparam_t       *nextparam;
    //! Whether \a nextparam was a boolean parameter with a 'no' prefix.
    bool                             bBoolNo;
    /*! \brief
     * Method symbol to return before resuming scanning
     *
     * Only used when \p nextparam is NULL.
     */
    struct gmx_ana_selmethod_t      *nextmethod;
    //! Used to track whether the previous token was a position modifier.
    int                              prev_pos_kw;

    //! Whether the 'of' keyword is acceptable as the next token.
    bool                             bMatchOf;
    //! Whether boolean values (yes/no/on/off) are acceptable as the next token.
    bool                             bMatchBool;
    //! Whether the next token starts a new selection.
    bool                             bCmdStart;

    //! Whether an external buffer is set for the scanner.
    bool                             bBuffer;
    //! The current buffer for the scanner.
    YY_BUFFER_STATE                  buffer;
} gmx_sel_lexer_t;

/* Because Flex defines yylval, yytext, and yyleng as macros,
 * and this file is included from scanner.l,
 * we cannot have them here as parameter names... */
/** Internal function for cases where several tokens need to be returned. */
int
_gmx_sel_lexer_process_pending(YYSTYPE *, gmx_sel_lexer_t *state);
/** Internal function that processes identifier tokens. */
int
_gmx_sel_lexer_process_identifier(YYSTYPE *, char *, size_t,
                                  gmx_sel_lexer_t *state);
/** Internal function to add a token to the pretty-printed selection text. */
void
_gmx_sel_lexer_add_token(const char *str, int len, gmx_sel_lexer_t *state);

#endif
