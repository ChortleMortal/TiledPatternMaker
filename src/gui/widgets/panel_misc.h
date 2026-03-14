#pragma once
#ifndef PANEL_MISC_H
#define PANEL_MISC_H

#include <QVBoxLayout>
#include <QStackedWidget>
#include <QColorDialog>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QEvent>
#include <QMouseEvent>

namespace panel_misc
{
    void eraseLayout(QLayout * layout);
}

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
    AQColorDialog(QWidget * parent);
    AQColorDialog(const QColor & initial, QWidget *parentB);

protected:
    void set_CustomColors();
};

class AQTableWidget : public QTableWidget
{
public:
    AQTableWidget(QWidget *parent = nullptr);

    void    adjustTableSize(int maxWidth = 0, int maxHeight = 0);
    void    selectRow(int row);

private:
    void    adjustTableWidth(int maxWidth = 0);
    void    adjustTableHeight(int maxHeight = 0);

    int     getTableWidth(int maxWidth);
    int     getTableHeight(int maxHeight);
};

class AQLineEdit : public QLineEdit
{
public:
    AQLineEdit(QWidget * parent=nullptr);

    void         leaveEvent(QEvent *event) override;
    virtual void enterEvent(QEnterEvent *event) override;

    void    setText(QString txt);

    bool blocked;

private:
    QString backgroundColorName;
};

class AQComboBox : public QComboBox
{
public:
    AQComboBox(QWidget * parent = nullptr);

    void select(int id);
};

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


class AQCheckBox : public QCheckBox
{
public:
    AQCheckBox();
    AQCheckBox(QString string);
};

class Separator : public AQHBoxLayout
{
public:
    Separator();

    QFrame * frame;
};

class TableRowSelector : public QObject
{
public:
    TableRowSelector(QTableWidget *table) : QObject(table), table(table)
    {
        table->viewport()->installEventFilter(this);
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (obj == table->viewport() && event->type() == QEvent::MouseButtonPress)
        {
            auto *me = static_cast<QMouseEvent*>(event);
            int row = table->indexAt(me->pos()).row();
            if (row >= 0)
            {
                table->setCurrentCell(row, 0);
                table->selectRow(row);
            }
        }
        return QObject::eventFilter(obj, event);
    }

private:
    QTableWidget *table;
};
#endif // MISC_H
