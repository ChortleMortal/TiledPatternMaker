#include <QPushButton>
#include <QBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QDateTime>

#include "gui/widgets/dlg_savename.h"
#include "model/settings/configuration.h"
#include "sys/sys.h"
#include "sys/version.h"

DlgSaveName::DlgSaveName(QWidget *parent) : QDialog(parent)
{
    QSettings s;
    selections = s.value("logSaveOptions", 0xff).toUInt();

    setWindowTitle("Select log filename");

    QPushButton * pbNew     = new QPushButton("new");
    QPushButton * pbOld     = new QPushButton("old");
    QPushButton * pbGood    = new QPushButton("good");
    QPushButton * pbBad     = new QPushButton("bad");
    QPushButton * pbBefore  = new QPushButton("before");
    QPushButton * pbAfter   = new QPushButton("after");

    pbNew->setCheckable(false);
    pbOld->setCheckable(false);
    pbGood->setCheckable(false);
    pbBad->setCheckable(false);
    pbBefore->setCheckable(false);
    pbAfter->setCheckable(false);

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addStretch();
    hbox2->addWidget(pbNew);
    hbox2->addWidget(pbOld);
    hbox2->addWidget(pbGood);
    hbox2->addWidget(pbBad);
    hbox2->addWidget(pbBefore);
    hbox2->addWidget(pbAfter);
    hbox2->addStretch();

    QCheckBox * cbTimestamp = new QCheckBox("Timestamp");
    QCheckBox * cbHost      = new QCheckBox("Host");
    QCheckBox * cbSwVer     = new QCheckBox("SW Version");
    QCheckBox * cbGitRoot   = new QCheckBox("Git dir");
    QCheckBox * cbGitBr     = new QCheckBox("Git branch");
    QCheckBox * cbGitSha    = new QCheckBox("Git sha");
    QCheckBox * cbMosaic    = new QCheckBox("Loaded Mosaic");

    QHBoxLayout * hbox3 = new QHBoxLayout;
    //hbox3->addStretch();
    hbox3->addWidget(cbTimestamp);
    hbox3->addWidget(cbHost);
    hbox3->addWidget(cbSwVer);
    hbox3->addWidget(cbGitRoot);
    hbox3->addWidget(cbGitBr);
    hbox3->addWidget(cbGitSha);
    hbox3->addWidget(cbMosaic);
    hbox3->addStretch();

    QPushButton * okBtn  = new QPushButton("OK");
    okBtn->setDefault(true);

    leName = new QLineEdit();
    leName->setFixedWidth(600);

    QHBoxLayout * hbox4 = new QHBoxLayout;
    hbox4->addWidget(leName);
    hbox4->addWidget(okBtn);

    QPushButton * canBtn = new QPushButton("Cancel");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(canBtn);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox2);
    vbox->addSpacing(13);
    vbox->addLayout(hbox3);
    //vbox->addSpacing(13);
    vbox->addLayout(hbox4);
    vbox->addSpacing(13);
    vbox->addLayout(hbox);

    setLayout(vbox);

    if (selections & SN_TIMESTAMP)
        cbTimestamp->setChecked(true);
    if (selections & SN_HOST)
        cbHost->setChecked(true);
    if (selections & SN_SWVER)
        cbSwVer->setChecked(true);
    if (selections & SN_GITROOT)
        cbGitRoot->setChecked(true);
    if (selections & SN_GITBRANCH)
        cbGitBr->setChecked(true);
    if (selections & SN_GITSHA)
        cbGitSha->setChecked(true);
    if (selections & SN_MOSAIC)
        cbMosaic->setChecked(true);


    connect(cbTimestamp,&QCheckBox::clicked, this, [this] (bool checked) { select(SN_TIMESTAMP,checked); });
    connect(cbHost,     &QCheckBox::clicked, this, [this] (bool checked) { select(SN_HOST,checked); });
    connect(cbSwVer,    &QCheckBox::clicked, this, [this] (bool checked) { select(SN_SWVER,checked); });
    connect(cbGitBr,    &QCheckBox::clicked, this, [this] (bool checked) { select(SN_GITBRANCH,checked); });
    connect(cbGitRoot,  &QCheckBox::clicked, this, [this] (bool checked) { select(SN_GITROOT,checked); });
    connect(cbGitSha,   &QCheckBox::clicked, this, [this] (bool checked) { select(SN_GITSHA,checked); });
    connect(cbMosaic,   &QCheckBox::clicked, this, [this] (bool checked) { select(SN_MOSAIC,checked); });

    connect(leName,     &QLineEdit::textChanged, this,  [this] (const QString & txt) { _name = txt; } );

    connect(pbNew,      &QPushButton::pressed, this,    [this] { leName->setText("new"); accept();} );
    connect(pbOld,      &QPushButton::pressed, this,    [this] { leName->setText("old"); accept();} );
    connect(pbGood,     &QPushButton::pressed, this,    [this] { leName->setText("good"); accept();} );
    connect(pbBad,      &QPushButton::pressed, this,    [this] { leName->setText("bad"); accept();} );
    connect(pbBefore,   &QPushButton::pressed, this,    [this] { leName->setText("before"); accept();} );
    connect(pbAfter,    &QPushButton::pressed, this,    [this] { leName->setText("after"); accept(); });

    connect(canBtn,     &QPushButton::clicked, this,    &QDialog::reject);
    connect(okBtn,      &QPushButton::clicked, this,    &QDialog::accept);

    genName();
}

void DlgSaveName::select(uint option, bool sel)
{
    if (sel)
        selections |= option;
    else
        selections &= ~option;

    genName();

    QSettings s;
    s.setValue("logSaveOptions", selections);
}

void DlgSaveName::genName()
{
    QString cdt    = QDateTime::currentDateTime().toString("yy.MM.dd-hh.mm.ss");
    QString host   = QSysInfo::machineHostName();
    QString branch = Sys::gitBranch;
    QString sha    = Sys::gitSha;
    QString root   = Sys::gitRoot;
    QString mosaic = Sys::config->lastLoadedMosaic.get();

    QString name;
    if (selections & SN_TIMESTAMP)
        name += cdt + "-";
    if (selections & SN_HOST)
        name += host + + "-";
    if (selections & SN_SWVER)
        name  += tpmVersion.trimmed();
    if (selections & SN_GITROOT && !root.isEmpty())
        name += "-" + root;
    if (selections & SN_GITBRANCH && !branch.isEmpty())
        name += "-" + branch;
    if (selections & SN_GITSHA && !sha.isEmpty())
        name += "-" + sha;
    if (selections & SN_MOSAIC  && !mosaic.isEmpty())
        name += "-" + mosaic;

#ifdef QT_DEBUG
    name += "-DEB";
#else
    name += "-REL";
#endif

    leName->setText(name);
}
