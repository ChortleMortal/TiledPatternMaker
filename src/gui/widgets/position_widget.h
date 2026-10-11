#pragma once

#include <QDialog>

enum class Pos
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    PLUS,
    MINUS
};

class PositionWidget : public QDialog
{
    Q_OBJECT

public:
    PositionWidget(QString name, QWidget * parent);

signals:
    void sig_pos(Pos);
};

