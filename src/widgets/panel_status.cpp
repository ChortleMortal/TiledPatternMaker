#include "widgets/panel_status.h"

static const QString preamble("<span style=\"color:rgb(0,240,0)\">");
static const QString postamble("</span>");

PanelStatus::PanelStatus()
{
    QFont f = font();
    f.setPointSize(13);
    //f.setBold(true);
    setFont(f);
    setAlignment(Qt::AlignCenter);
    setFixedHeight(25);

    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    setStyleSheet("QLabel { background-color : black; }");
}

void PanelStatus::pushStack(QString & txt)
{
    if (msgStack.count() && msgStack.top() == txt)
        return;

    msgStack.push(txt);
    QString msg = preamble + txt + postamble;
    setText(msg);
    repaint();
}

void PanelStatus::popStack()
{
    if (msgStack.count())
    {
        msgStack.pop();
    }
    if (msgStack.count())
    {
        QString txt = msgStack.top();
        QString msg = preamble + txt + postamble;
        setText(msg);
    }
    else
    {
        setText("");
    }
    repaint();
}


