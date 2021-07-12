/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SHARED_H
#define SHARED_H

#include <QtGlobal>

#undef  EXPLICIT_DESTRUCTOR

#define  WARN_BAD
//#define  CRITICAL_BAD

#ifdef WARN_BAD
    #define badness  qWarning
#else
    #define badness  qCritical
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5,15,0))
#include <QTextStream>
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
using Qt::endl;
#else
#define endl Qt::endl
#endif
#endif

#define GOLDEN_RATIO  1.61803398874989484820
#define DEFAULT_WIDTH  1500
#define DEFAULT_HEIGHT 1100

#define DAC_DEPRECATED QT_DEPRECATED

extern class TiledPatternMaker * theApp;

#endif // SHARED_H
