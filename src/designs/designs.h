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
#ifndef TPM_DESIGN_H
#define TPM_DESIGN_H

#include <QtCore>
#include "designs/design.h"

class Design5 : public Design
{
public:
    Design5(eDesign design, QString title);
    bool build();
};

class Design6 : public Design
{
public:
    Design6(eDesign design, QString title);
    bool build();
};

class Design7 : public Design
{
public:
    Design7(eDesign design, QString title);
    bool build();
};

class Design8 : public Design
{
public:
    Design8(eDesign design, QString title);
    bool build();
};

class Design9 : public Design
{
public:
    Design9(eDesign design, QString title);
    bool build();
};

class DesignHuPacked : public Design
{
public:
    DesignHuPacked(eDesign design, QString title);
    bool build();
};

class DesignHuInsert : public Design
{
public:
    DesignHuInsert(eDesign design, QString title);
    bool build();
};

class Design11 : public Design
{
public:
    Design11(eDesign design, QString title);
    bool build();
};

class Design12 : public Design
{
public:
    Design12(eDesign design, QString title);
    bool build();
};

class Design13 : public Design
{
public:
    Design13(eDesign design, QString title);
    bool build();
    //void repeat();
};

class Design14 : public Design
{
public:
    Design14(eDesign design, QString title);
    bool build();
    //void repeat();
};

class Design16 : public Design
{
public:
    Design16(eDesign design, QString title);
    bool build();
};

class Design17 : public Design
{
public:
    Design17(eDesign design, QString title);
    bool build();
};

class Design18 : public Design
{
public:
    Design18(eDesign design, QString title);
    bool build();
    //void repeat();
};

class Design19 : public Design
{
public:
    Design19(eDesign design, QString title);
    bool build();
};

class DesignKumiko1 : public Design
{
public:
    DesignKumiko1(eDesign design, QString title);
    bool build();
};

class DesignKumiko2 : public Design
{
public:
    DesignKumiko2(eDesign design, QString title);
    void init() override;
    bool build() override;
};

#endif //TPM_DESIGN_H
