#include "panels/panel_status.h"

const viewMsg PanelStatus::viewMsgs[] = {
    {VIEW_DESIGN,         ""},
    {VIEW_MOSAIC,         ""},
    {VIEW_DESIGN_ELEMENT, ""},
    {VIEW_MOTIF_MAKER,    "<body style=\"background-color=#000000\"><font color=green>figure</font>  |  <font color=magenta>feature boundary</font>  |  <font color=red>radial figure boundary</font>  |  <font color=yellow>extended boundary</font></body>"},
    {VIEW_TILING,         ""},
    {VIEW_TILING_MAKER,   "<body style=\"background-color=ffffffff; font=bold;\"><span style=\"color:rgb(217,217,255)\">excluded</span>  |  <span style=\"color:rgb(255,217,217)\">included</span>  |  <span style=\"color:rgb(205,102, 25)\">overlapping</span>  |  <span style=\"color:rgb( 25,102,205)\">touching</span> |  <span style=\"color:rgb(127,255,127)\">under-mouse</span>  |  <span style=\"color:rgb(206,179,102)\">dragging</span></body>"},
    {VIEW_MAP_EDITOR,     ""},
    {VIEW_FACE_SET,       ""},
    {VIEW_UNDEFINED,      ""}
};

const QString preamble("<span style=\"color:rgb(0,240,0)\">");
const QString postamble("</span>");

PanelStatus::PanelStatus()
{
    viewType = VIEW_UNDEFINED;
    QFont f = font();
    f.setPointSize(13);
    setFont(f);
    setAlignment(Qt::AlignCenter);
    setFixedHeight(25);

    setStyleSheet("QLabel { background-color : black; }");
}

void PanelStatus::display(QString txt)
{
    msgStack.push(txt);
    QString msg = preamble + txt + postamble;
    setText(msg);
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
        QString msg = preamble + txt + postamble;
        setText(msg);
    }
    else
    {
        setText(getMsg());
    }
    repaint();
}

void PanelStatus::setCurrentView(eViewType vtype)
{
    viewType = vtype;
    if (msgStack.empty())
    {
        setText(getMsg());
        repaint();
    }
}

QString PanelStatus::getMsg()
{
    for (int i=0; i <= LAST_VIEW_TYPE; i++)
    {
        const viewMsg & vm = viewMsgs[i];
        if (vm.view == viewType)
        {
            return vm.msg;
        }
    }
    return QString();
}
