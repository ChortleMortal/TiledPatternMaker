#include "panels/panel_status.h"

PanelStatus::PanelStatus()
{
    QFont f = font();
    f.setPointSize(13);
    setFont(f);
    setAlignment(Qt::AlignCenter);
    setFixedHeight(25);

    color = QLabel::palette().color(QLabel::backgroundRole());
}

void PanelStatus::display(QString txt)
{
    msgStack.push(txt);
    setText(txt);
    repaint();
}

void PanelStatus::hide()
{
    if (msgStack.count())
    {
        msgStack.pop();
    }
    if (msgStack.count())
    {
        QString txt = msgStack.top();
        setText(txt);
    }
    else
    {
        setStyleSheet("QLabel { background-color : +color+; }");
        setText("");
    }
}
