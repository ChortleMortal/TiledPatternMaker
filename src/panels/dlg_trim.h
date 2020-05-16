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

#ifndef DLG_TRIM_H
#define DLG_TRIM_H

#include <QtWidgets>
#include <QSignalMapper>
#include "tile/Feature.h"
#include "makers/style_maker/style_editors.h"

class DlgTrim : public QDialog
{
    Q_OBJECT

public:
    DlgTrim(QWidget * parent = nullptr);

protected:

signals:
    void  sig_apply(qreal valX, qreal valY);

private slots:
    void slot_ok();
    void slot_apply();

private:
    DoubleSpinSet * trimmerX;
    DoubleSpinSet * trimmerY;
};

#endif
