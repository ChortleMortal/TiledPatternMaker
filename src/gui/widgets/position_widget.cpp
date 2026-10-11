#include "gui/widgets/position_widget.h"

#include <QStyle>
#include <QPushButton>
#include <QGridLayout>

PositionWidget::PositionWidget(QString name, QWidget * parent) : QDialog(parent)
{
    setWindowTitle(name);

    QIcon up = style()->standardIcon(QStyle::SP_ArrowUp);
    QIcon down = style()->standardIcon(QStyle::SP_ArrowDown);
    QIcon left = style()->standardIcon(QStyle::SP_ArrowLeft);
    QIcon right = style()->standardIcon(QStyle::SP_ArrowRight);

    QPushButton * up_btn =  new QPushButton;
    QPushButton * down_btn  = new QPushButton;
    QPushButton * left_btn  = new QPushButton;
    QPushButton * right_btn = new QPushButton;
    QPushButton * plus_btn  = new QPushButton;
    QPushButton * minus_btn = new QPushButton;

    plus_btn->setMaximumWidth(25);
    minus_btn->setMaximumWidth(25);

    up_btn->setIcon(up);
    down_btn->setIcon(down);
    left_btn->setIcon(left);
    right_btn->setIcon(right);
    plus_btn->setText("+");
    minus_btn->setText("-");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(minus_btn);
    hbox->addWidget(plus_btn);

    QGridLayout * grid = new QGridLayout;
    grid->addWidget(up_btn,0,1);
    grid->addWidget(left_btn,1,0);
    grid->addWidget(right_btn,1,2);
    grid->addWidget(down_btn,2,1);
    grid->addLayout(hbox,1,1);

    setLayout(grid);

    connect(up_btn,    &QPushButton::clicked, this, [this] { emit sig_pos(Pos::UP);    } );
    connect(down_btn,  &QPushButton::clicked, this, [this] { emit sig_pos(Pos::DOWN);  } );
    connect(left_btn,  &QPushButton::clicked, this, [this] { emit sig_pos(Pos::LEFT);  } );
    connect(right_btn, &QPushButton::clicked, this, [this] { emit sig_pos(Pos::RIGHT); } );
    connect(plus_btn,  &QPushButton::clicked, this, [this] { emit sig_pos(Pos::PLUS);  } );
    connect(minus_btn, &QPushButton::clicked, this, [this] { emit sig_pos(Pos::MINUS); } );
}
