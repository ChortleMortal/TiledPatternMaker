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

#ifndef PANEL_MISC_H
#define PANEL_MISC_H

#include <QtWidgets>

static const QChar MathSymbolSquareRoot(0x221A);
static const QChar MathSymbolPi(0x03A0);
static const QChar MathSymbolDelta(0x0394);
static const QChar MathSymbolSigma(0x03A3);

class AQWidget : public QWidget
{
public:
    AQWidget(QWidget * parent = nullptr);

};

class AQVBoxLayout : public QVBoxLayout
{
public:
    AQVBoxLayout();
};

class AQHBoxLayout : public QHBoxLayout
{
public:
    AQHBoxLayout();
};

class AQStackedWidget : public QStackedWidget
{
public:
    AQStackedWidget();
};

class AQColorDialog : public QColorDialog
{
public:
    AQColorDialog(QWidget * parent = nullptr);
    AQColorDialog(const QColor & initial, QWidget *parent = nullptr);

protected:
    void set_CustomColors();
};

class AQTableWidget : public QTableWidget
{
public:
    AQTableWidget(QWidget *parent = nullptr);

    void    adjustTableSize(int maxWidth = 0, int maxHeight = 0);
    void    selectRow(int row);

protected:
    void    adjustTableWidth(int maxWidth = 0);
    void    adjustTableHeight(int maxHeight = 0);

    int     getTableWidth(int maxWidth);
    int     getTableHeight(int maxHeight);
};

class AQLineEdit : public QLineEdit
{
public:
    AQLineEdit(QWidget * parent=nullptr);

    void mousePressEvent(QMouseEvent * event);
};

extern void eraseLayout(QLayout *layout);

class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ClickableLabel(QWidget* parent = Q_NULLPTR);
    ~ClickableLabel();

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event);
};

class AQPushButton : public QPushButton
{
public:
    AQPushButton(const QString &text, QWidget * parent = nullptr);
};


class BQPushButton : public QPushButton
{
public:
    BQPushButton(const QString &text, QWidget * parent = nullptr);
};

#endif // MISC_H
