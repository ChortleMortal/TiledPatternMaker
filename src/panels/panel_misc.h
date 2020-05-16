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

class PanelListWidget : public QListWidget
{
    Q_OBJECT

public:
    PanelListWidget(QWidget *parent = nullptr);

    void setCurrentRow(QString name);
    void removeItem(QString name);

    void addSeparator();

    void mousePressEvent(QMouseEvent * event);

signals:
    void detachWidget(QString name);

protected slots:
    void slot_floatAction();

protected:
    QAction * floatAction;

private:
    volatile bool localDrop;
    int  floatIndex;

};

class AQColorDialog : public QColorDialog
{
public:
    AQColorDialog(QWidget * parent = nullptr);
    AQColorDialog(const QColor & initial, QWidget *parent = nullptr);

protected:
    void set_CustomColors();
};


class LoaderListWidget : public QListWidget
{
    Q_OBJECT

public:
    LoaderListWidget(QWidget *parent = nullptr);

    void addItemList(QStringList list);
    bool selectItemByName(QString name);
    bool selectItemByValue(QVariant val);


private:
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;

signals:
    void rightClick(QPoint pos);
    void leftDoubleClick(QPoint pos);
    void listEnter();
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

class AQLabel : public QLabel
{
    Q_OBJECT

public:
    AQLabel();

    void keyPressEvent( QKeyEvent *k ) Q_DECL_OVERRIDE;

signals:
    void sig_takeNext();
    void sig_cyclerQuit();
    void sig_view_images();
    void sig_close();
};

#endif // MISC_H
