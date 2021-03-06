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
 * Implements classes in plot.h.
 *
 * \ingroup module_analysisdata
 * \author Teemu Murtola <teemu.murtola@cbr.su.se>
 */
#include "gromacs/analysisdata/modules/plot.h"

#include <string>
#include <vector>

#include <cstdio>
#include <cstring>

#include <boost/shared_ptr.hpp>

#include "gromacs/legacyheaders/gmxfio.h"
#include "gromacs/legacyheaders/oenv.h"
#include "gromacs/legacyheaders/vec.h"
#include "gromacs/legacyheaders/xvgr.h"

#include "gromacs/options/basicoptions.h"
#include "gromacs/analysisdata/dataframe.h"
#include "gromacs/options/options.h"
#include "gromacs/options/timeunitmanager.h"
#include "gromacs/selection/selectioncollection.h"
#include "gromacs/utility/exceptions.h"
#include "gromacs/utility/gmxassert.h"
#include "gromacs/utility/stringutil.h"

namespace
{

//! Enum values for plot formats.
const char *const g_plotFormats[] = {
    "none", "xmgrace", "xmgr", NULL
};

} // namespace

namespace gmx
{

/********************************************************************
 * AnalysisDataPlotSettings
 */

AnalysisDataPlotSettings::AnalysisDataPlotSettings()
    : selections_(NULL), timeUnit_(eTimeUnit_ps), plotFormat_(1)
{
}

void
AnalysisDataPlotSettings::setSelectionCollection(const SelectionCollection *selections)
{
    selections_ = selections;
}


void
AnalysisDataPlotSettings::addOptions(Options *options)
{
    options->addOption(StringOption("xvg").enumValue(g_plotFormats)
                           .defaultValue("xmgrace")
                           .storeEnumIndex(&plotFormat_)
                           .description("Plot formatting"));
}


/********************************************************************
 * AbstractPlotModule::Impl
 */

class AbstractPlotModule::Impl
{
    public:
        explicit Impl(const AnalysisDataPlotSettings &settings);
        ~Impl();

        void closeFile();

        AnalysisDataPlotSettings settings_;
        std::string             filename_;
        FILE                   *fp_;

        bool                    bPlain_;
        bool                    gOmitX_;
        std::string             title_;
        std::string             subtitle_;
        std::string             xlabel_;
        std::string             ylabel_;
        std::vector<std::string>  legend_;
        char                    xformat_[15];
        char                    yformat_[15];
        real                    xscale_;
};

AbstractPlotModule::Impl::Impl(const AnalysisDataPlotSettings &settings)
    : settings_(settings), fp_(NULL), bPlain_(false), gOmitX_(false),
      xscale_(1.0)
{
    strcpy(xformat_, "%11.3f");
    strcpy(yformat_, " %8.3f");
}

AbstractPlotModule::Impl::~Impl()
{
    closeFile();
}


void
AbstractPlotModule::Impl::closeFile()
{
    if (fp_ != NULL)
    {
        if (bPlain_)
        {
            gmx_fio_fclose(fp_);
        }
        else
        {
            xvgrclose(fp_);
        }
        fp_ = NULL;
    }
}


/********************************************************************
 * AbstractPlotModule
 */
/*! \cond libapi */
AbstractPlotModule::AbstractPlotModule()
    : impl_(new Impl(AnalysisDataPlotSettings()))
{
}

AbstractPlotModule::AbstractPlotModule(const AnalysisDataPlotSettings &settings)
    : impl_(new Impl(settings))
{
}
//! \endcond

AbstractPlotModule::~AbstractPlotModule()
{
}


void
AbstractPlotModule::setSettings(const AnalysisDataPlotSettings &settings)
{
    impl_->settings_ = settings;
}


void
AbstractPlotModule::setFileName(const std::string &filename)
{
    impl_->filename_ = filename;
}


void
AbstractPlotModule::setPlainOutput(bool bPlain)
{
    impl_->bPlain_ = bPlain;
}


void
AbstractPlotModule::setOmitX(bool bOmitX)
{
    impl_->gOmitX_ = bOmitX;
}


void
AbstractPlotModule::setTitle(const char *title)
{
    impl_->title_ = title;
}


void
AbstractPlotModule::setSubtitle(const char *subtitle)
{
    impl_->subtitle_ = subtitle;
}


void
AbstractPlotModule::setXLabel(const char *label)
{
    impl_->xlabel_ = label;
}


void
AbstractPlotModule::setXAxisIsTime()
{
    TimeUnitManager manager(impl_->settings_.timeUnit());
    impl_->xlabel_ = formatString("Time (%s)", manager.timeUnitAsString());
    impl_->xscale_ = manager.inverseTimeScaleFactor();
}


void
AbstractPlotModule::setYLabel(const char *label)
{
    impl_->ylabel_ = label;
}


void
AbstractPlotModule::setLegend(int nsets, const char * const *setname)
{
    impl_->legend_.reserve(impl_->legend_.size() + nsets);
    for (int i = 0; i < nsets; ++i)
    {
        appendLegend(setname[i]);
    }
}


void
AbstractPlotModule::appendLegend(const char *setname)
{
    impl_->legend_.push_back(setname);
}


void
AbstractPlotModule::setXFormat(int width, int precision, char format)
{
    GMX_RELEASE_ASSERT(width >= 0 && precision >= 0
                       && width <= 99 && precision <= 99,
                       "Invalid width or precision");
    GMX_RELEASE_ASSERT(strchr("eEfFgG", format) != NULL,
                       "Invalid format specifier");
    sprintf(impl_->xformat_, "%%%d.%d%c", width, precision, format);
}


void
AbstractPlotModule::setYFormat(int width, int precision, char format)
{
    GMX_RELEASE_ASSERT(width >= 0 && precision >= 0
                       && width <= 99 && precision <= 99,
                       "Invalid width or precision");
    GMX_RELEASE_ASSERT(strchr("eEfFgG", format) != NULL,
                       "Invalid format specifier");
    sprintf(impl_->yformat_, " %%%d.%d%c", width, precision, format);
}


int
AbstractPlotModule::flags() const
{
    return efAllowMulticolumn | efAllowMultipoint;
}


void
AbstractPlotModule::dataStarted(AbstractAnalysisData *data)
{
    if (!impl_->filename_.empty())
    {
        if (impl_->bPlain_)
        {
            impl_->fp_ = gmx_fio_fopen(impl_->filename_.c_str(), "w");
        }
        else
        {
            time_unit_t time_unit
                = static_cast<time_unit_t>(impl_->settings_.timeUnit() + 1);
            xvg_format_t xvg_format
                = (impl_->settings_.plotFormat() > 0
                    ? static_cast<xvg_format_t>(impl_->settings_.plotFormat())
                    : exvgNONE);
            output_env_t oenv;
            output_env_init(&oenv, 0, NULL, time_unit, FALSE, xvg_format, 0, 0);
            boost::shared_ptr<output_env> oenvGuard(oenv, &output_env_done);
            impl_->fp_ = xvgropen(impl_->filename_.c_str(), impl_->title_.c_str(),
                                  impl_->xlabel_.c_str(), impl_->ylabel_.c_str(),
                                  oenv);
            const SelectionCollection *selections
                = impl_->settings_.selectionCollection();
            if (selections != NULL)
            {
                selections->printXvgrInfo(impl_->fp_, oenv);
            }
            if (!impl_->subtitle_.empty())
            {
                xvgr_subtitle(impl_->fp_, impl_->subtitle_.c_str(), oenv);
            }
            if (output_env_get_print_xvgr_codes(oenv)
                && !impl_->legend_.empty())
            {
                std::vector<const char *> legend;
                legend.reserve(impl_->legend_.size());
                for (size_t i = 0; i < impl_->legend_.size(); ++i)
                {
                    legend.push_back(impl_->legend_[i].c_str());
                }
                xvgr_legend(impl_->fp_, legend.size(), &legend[0], oenv);
            }
        }
    }
}


void
AbstractPlotModule::frameStarted(const AnalysisDataFrameHeader &frame)
{
    if (!isFileOpen())
    {
        return;
    }
    if (!impl_->gOmitX_)
    {
        std::fprintf(impl_->fp_, impl_->xformat_, frame.x() * impl_->xscale_);
    }
}


void
AbstractPlotModule::frameFinished(const AnalysisDataFrameHeader & /*header*/)
{
    if (!isFileOpen())
    {
        return;
    }
    std::fprintf(impl_->fp_, "\n");
}


void
AbstractPlotModule::dataFinished()
{
    impl_->closeFile();
}

/*! \cond libapi */
bool
AbstractPlotModule::isFileOpen() const
{
    return impl_->fp_ != NULL;
}


void
AbstractPlotModule::writeValue(real value) const
{
    GMX_ASSERT(isFileOpen(), "File not opened, but write attempted");
    std::fprintf(impl_->fp_, impl_->yformat_, value);
}
//! \endcond

/********************************************************************
 * DataPlotModule
 */

AnalysisDataPlotModule::AnalysisDataPlotModule()
{
}

AnalysisDataPlotModule::AnalysisDataPlotModule(
        const AnalysisDataPlotSettings &settings)
    : AbstractPlotModule(settings)
{
}


void
AnalysisDataPlotModule::pointsAdded(const AnalysisDataPointSetRef &points)
{
    if (!isFileOpen())
    {
        return;
    }
    for (int i = 0; i < points.columnCount(); ++i)
    {
        writeValue(points.y(i));
    }
}


/********************************************************************
 * DataVectorPlotModule
 */

AnalysisDataVectorPlotModule::AnalysisDataVectorPlotModule()
{
    for (int i = 0; i < DIM; ++i)
    {
        bWrite_[i] = true;
    }
    bWrite_[DIM] = false;
}


AnalysisDataVectorPlotModule::AnalysisDataVectorPlotModule(
        const AnalysisDataPlotSettings &settings)
    : AbstractPlotModule(settings)
{
    for (int i = 0; i < DIM; ++i)
    {
        bWrite_[i] = true;
    }
    bWrite_[DIM] = false;
}


void
AnalysisDataVectorPlotModule::setWriteX(bool bWrite)
{
    bWrite_[XX] = bWrite;
}


void
AnalysisDataVectorPlotModule::setWriteY(bool bWrite)
{
    bWrite_[YY] = bWrite;
}


void
AnalysisDataVectorPlotModule::setWriteZ(bool bWrite)
{
    bWrite_[ZZ] = bWrite;
}


void
AnalysisDataVectorPlotModule::setWriteNorm(bool bWrite)
{
    bWrite_[DIM] = bWrite;
}


void
AnalysisDataVectorPlotModule::setWriteMask(bool bWrite[DIM + 1])
{
    for (int i = 0; i < DIM + 1; ++i)
    {
        bWrite_[i] = bWrite[i];
    }
}


void
AnalysisDataVectorPlotModule::pointsAdded(const AnalysisDataPointSetRef &points)
{
    if (points.firstColumn() % DIM != 0)
    {
        GMX_THROW(APIError("Partial data points"));
    }
    if (!isFileOpen())
    {
        return;
    }
    for (int i = 0; i < points.columnCount(); i += 3)
    {
        for (int d = 0; d < DIM; ++d)
        {
            if (bWrite_[i])
            {
                writeValue(points.y(i + d));
            }
        }
        if (bWrite_[DIM])
        {
            rvec y = { points.y(i), points.y(i + 1), points.y(i + 2) };
            writeValue(norm(y));
        }
    }
}

} // namespace gmx
