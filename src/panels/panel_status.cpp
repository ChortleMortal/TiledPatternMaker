#include "panels/panel_status.h"

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

    preamble  = "<span style=\"color:rgb(0,240,0)\">";
    postamble = "</span>";
}

void PanelStatus::setStatusText(QString & txt)
{
    if (txt.isEmpty())
    {
        clear();
    }
    else
    {
        QString msg = preamble + txt + postamble;
        setText(msg);
    }
    update();
}

void PanelStatus::clearStatusText()
{
    clear();
    update();
}
