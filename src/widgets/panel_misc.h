#pragma once
#ifndef PANEL_MISC_H
#define PANEL_MISC_H

class QWidget;

#include <QVBoxLayout>
#include <QStackedWidget>
#include <QColorDialog>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>

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

    void mousePressEvent(QMouseEvent * event);
};

class AQComboBox : public QComboBox
{
public:
    AQComboBox(QWidget * parent = nullptr);

    void select(int id);
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


class AQCheckBox : public QCheckBox
{
public:
    AQCheckBox();
    AQCheckBox(QString string);
};

#endif // MISC_H
