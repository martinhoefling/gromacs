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
 * \brief
 * Tests creation of basic option types.
 *
 * Most of the tests for the basic options are in optionsassigner.cpp.
 * This file only tests behavior that should fail in initialization.
 *
 * \author Teemu Murtola <teemu.murtola@cbr.su.se>
 * \ingroup module_options
 */
#include <vector>
#include <string>

#include <gtest/gtest.h>

#include "gromacs/options/basicoptions.h"
#include "gromacs/options/options.h"
#include "gromacs/utility/exceptions.h"

namespace
{

TEST(OptionsTest, FailsOnNonsafeStorage)
{
    gmx::Options options(NULL, NULL);
    int value = -1;
    using gmx::IntegerOption;
    ASSERT_THROW(options.addOption(IntegerOption("name").store(&value)
                                       .multiValue()),
                 gmx::APIError);
}

TEST(OptionsTest, FailsOnIncorrectEnumDefaultValue)
{
    gmx::Options options(NULL, NULL);
    std::string                 value;
    const char * const          allowed[] = { "none", "test", "value", NULL };
    using gmx::StringOption;
    ASSERT_THROW(options.addOption(StringOption("name").store(&value)
                                       .enumValue(allowed)
                                       .defaultValue("unknown")),
                 gmx::APIError);
}

} // namespace
