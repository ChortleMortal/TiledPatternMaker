#include "gui/panels/panel_status.h"

PanelStatus::PanelStatus()
{
    QFont f = font();
    f.setPointSize(13);
    setFont(f);
    setAlignment(Qt::AlignCenter);
    setFixedHeight(25);

    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    preamble  = "<span style=\"color:rgb(0,240,0)\">";
    postamble = "</span>";

    page = -1;
}

void PanelStatus::setBacgroundColor(QColor color)
{
    QString str = QString("QLabel { background-color : %1; }").arg(color.name());
    setStyleSheet(str);
}

void PanelStatus::overridePanelStatus(const QString & txt)
{
    QString msg = preamble + txt + postamble;
    setText(msg);
    repaint();
}

void  PanelStatus::restorePanelStatus()
{
    if (page != -1)
    {
        QString msg = preamble + pageStatus + postamble;
        setText(msg);
    }
    else
    {
        clear();
    }
    repaint();
}

void PanelStatus::setPageStatus(ePanelPage pp,const QString & str)
{
    pageStatus = str;
    page       = pp;
    QString msg = preamble + pageStatus + postamble;
    setText(msg);
    repaint();
}

void PanelStatus::clearPageStatus(ePanelPage pp)
{
    if (page == pp)
    {
        page = -1;
        clear();
        repaint();
    }
}

