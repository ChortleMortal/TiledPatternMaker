#include <QHBoxLayout>

#include "gui/model_editors/motif_edit/motif_type_selector.h"
#include "gui/widgets/panel_misc.h"
#include "model/motifs/motif.h"

/////////////////////////////////////////////////////////
///
/// MotifTypeSelector
///
////////////////////////////////////////////////////////

MotifTypeSelector:: MotifTypeSelector()
{
    AQHBoxLayout * hbox = new AQHBoxLayout;
    hbox->setContentsMargins(0,9,0,9);
    hbox->addWidget(&types);
    hbox->addWidget(&versions);
    hbox->addStretch();

    setLayout(hbox);

    connect(&types,    &QComboBox::currentIndexChanged, this, &MotifTypeSelector::slot_motifTypeChanged);
    connect(&versions, &QComboBox::currentIndexChanged, this, &MotifTypeSelector::slot_motifVersionChanged);
}

void MotifTypeSelector::updateTypeChoices(MotifPtr motif)
{
    // types
    types.blockSignals(true);

    types.clear();

    if (motif->isRadial())
    {
        add(Motif::Type::Rosette,             "Rosette");
        add(Motif::Type::Rosette2,            "Rosette2 (New)");
        add(Motif::Type::Star,                "Star");
        add(Motif::Type::Star2,               "Star2 (New)");
        add(Motif::Type::ExplicitTile,        "Explicit Tile");
    }
    else
    {
        add(Motif::Type::MapMotif,            "Explicit Map");
        add(Motif::Type::Inferred,            "Inferred");
        add(Motif::Type::AdaptiveRosette,     "Adaptive Rosette");
        add(Motif::Type::AdaptiveStar,        "Adaptive Star");
        add(Motif::Type::Hourglass,           "Hourglass");
        add(Motif::Type::Girih,               "Girih");
        add(Motif::Type::Intersect,           "Intersect");
        add(Motif::Type::ExplicitTile,        "Explicit Tile");
        add(Motif::Type::NoMapMotif,          "No Motif (Empty)");
    }

    int index = getIndex(motif->getMotifType());
    if (index >= 0)
    {
        types.setCurrentIndex(index);
        qDebug() << "MotifTypeCombo - set index=" << index;
    }
    else
    {
        types.setCurrentIndex(0);
        qDebug() << "MotifTypeCombo - forcing index=0";
    }

    adjustComboWidth(&types);


    setVersion(motif);
}

void MotifTypeSelector::adjustComboWidth(QComboBox * box)
{
    QFontMetrics fm(font());
    int maxWidth = 0;
    for (int i = 0; i < box->count(); ++i)
        maxWidth = qMax(maxWidth, fm.horizontalAdvance(box->itemText(i)));

    int frame   = box->style()->pixelMetric(QStyle::PM_ComboBoxFrameWidth, nullptr, this) * 2;
    int arrow   = box->style()->pixelMetric(QStyle::PM_MenuButtonIndicator, nullptr, this);
    int padding = 16;
    int total   = maxWidth + frame + arrow + padding;

    box->setMinimumWidth(total);
    box->setMaximumWidth(total);

    box->blockSignals(false);
}

void MotifTypeSelector::setVersion(MotifPtr motif)
{
    // versions
    int maxv = motif->maxVersions();
    if (maxv > 1)
    {
        versions.blockSignals(true);
        versions.clear();
        for (int i = 0; i < maxv;  i++)
        {
            versions.addItem(QString("Version %1").arg(i+1));
        }

        versions.blockSignals(false);
        adjustComboWidth(&versions);
        versions.show();
    }
    else
    {
        versions.hide();
    }
}

void MotifTypeSelector::add(Motif::Type type, QString name)
{
    types.addItem(name,QVariant((int)type));
}

int MotifTypeSelector::getIndex(Motif::Type type)
{
    //qDebug().noquote()  << "type is" << Motif::getTypeString(type);

    return types.findData(QVariant((int)type));
}

void MotifTypeSelector::slot_motifTypeChanged(int index)
{
    Q_UNUSED(index);

    QVariant qv = types.currentData();
    Motif::Type type = static_cast<Motif::Type>(qv.toInt());
    qDebug() << "MotifTypeCombo type="  << sMotifType[(int)type];

    emit sig_motifTypeChanged(type);
}

void MotifTypeSelector::slot_motifVersionChanged(int index)
{
    int version = index+1;

    emit sig_motifVersionChanged(version);
}
