#include <QPushButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QFileDialog>

#include "panels/page_config.h"
#include "panels/panel.h"
#include "settings/configuration.h"
#include "tiledpatternmaker.h"
#include "viewers/viewcontrol.h"
#include "widgets/dlg_textedit.h"

extern void closeHandle();

page_config:: page_config(ControlPanel * cpanel)  : panel_page(cpanel,"Configuration")
{
    le_rootMedia   = new QLineEdit();
    le_rootImages  = new QLineEdit();
    le_xmlTool     = new QLineEdit();

    le_rootMedia->setMinimumWidth(501);

    rootMediaBtn = new QPushButton("Root Media Directory");
    rootImagesBtn  = new QPushButton("Root Image Directory");
    xmlToolBtn     = new QPushButton("XML Viewer/Editor App");

    defaultDesigns = new QCheckBox("Default Media Directory");
    defaultImages  = new QCheckBox("Default Images Directory");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(defaultDesigns);
    hbox->addWidget(defaultImages);
    hbox->addStretch();

    QGridLayout *configGrid = new QGridLayout();

    int row = 0;
    configGrid->addLayout(hbox,row,1);

    row++;
    configGrid->addWidget(rootMediaBtn,row,0);
    configGrid->addWidget(le_rootMedia,row,1);

    row++;
    configGrid->addWidget(rootImagesBtn,row,0);
    configGrid->addWidget(le_rootImages,row,1);

    row++;
    configGrid->addWidget(xmlToolBtn,row,0);
    configGrid->addWidget(le_xmlTool,row,1);

    QGroupBox * pathGroup = new QGroupBox("Media File Paths");
    pathGroup->setLayout(configGrid);

    QRadioButton * designerMode = new QRadioButton("Designer Mode (reccomended)");
    QRadioButton * insightMode  = new QRadioButton("Insight Mode");
    QButtonGroup * btnGroup     = new QButtonGroup;
    btnGroup->addButton(designerMode,0);
    btnGroup->addButton(insightMode,1);

    QCheckBox * chkDarkTheme = new QCheckBox("Dark Theme");
    QPushButton * pbAbout    = new QPushButton("About TiledPatternMaker");

    int button  = (config->insightMode) ? 1 : 0;
    btnGroup->button(button)->setChecked(true);
    chkDarkTheme->setChecked(config->darkTheme);

    hbox = new QHBoxLayout;
    hbox->addWidget(designerMode);
    hbox->addWidget(insightMode);
    hbox->addStretch();
    hbox->addWidget(chkDarkTheme);
    hbox->addStretch();
    hbox->addWidget(pbAbout);

    QGroupBox * appGroup = new QGroupBox("Application");
    appGroup->setLayout(hbox);

    QGroupBox    * vctrl  = createViewControl();

    // put it all together
    vbox->addWidget(appGroup);
    vbox->addWidget(pathGroup);
    vbox->addWidget(vctrl);

    updatePaths();

    connect(rootImagesBtn,  &QPushButton::clicked,   this,  &page_config::slot_selectRootImageDir);
    connect(rootMediaBtn,   &QPushButton::clicked,   this,  &page_config::slot_selectRootMediaDir);
    connect(xmlToolBtn,     &QPushButton::clicked,   this,  &page_config::slot_selectXMLTool);
    connect(pbAbout,        &QPushButton::clicked,   this,  &page_config::slot_about);

    connect(le_rootMedia,   &QLineEdit::textChanged,   this, &page_config::slot_rootDesignChanged);
    connect(le_rootImages,  &QLineEdit::textChanged,   this, &page_config::slot_rootImageChanged);

    connect(defaultDesigns, &QCheckBox::clicked, this, &page_config::slot_designDefaultChanged);
    connect(defaultImages,  &QCheckBox::clicked, this, &page_config::slot_imageDefaultChanged);
    connect(chkDarkTheme,   &QCheckBox::clicked, this, &page_config::slot_darkThemeChanged);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(btnGroup,      SIGNAL(buttonClicked(int)), this, SLOT(slot_mode(int)));
#else
    connect(btnGroup,      &QButtonGroup::idClicked,  this,  &page_config::slot_mode);
#endif
}

QGroupBox * page_config::createViewControl()
{
    QGroupBox * ViewControlBox = new QGroupBox("View Control");

    QCheckBox   * chkSplit      = new QCheckBox("Split Screen");

    QCheckBox   * showCenterChk = new QCheckBox("Show Center");
    QPushButton * btnPrimary    = new QPushButton("Move to Primary Screen");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(showCenterChk);
    hbox->addWidget(chkSplit);
    hbox->addSpacing(5);
    hbox->addWidget(btnPrimary);
    hbox->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox);

    ViewControlBox->setLayout(vbox);

    chkSplit->setChecked(config->splitScreen);
    showCenterChk->setChecked(config->showCenterDebug);

    connect(btnPrimary,     &QPushButton::clicked,    theApp,   &TiledPatternMaker::slot_bringToPrimaryScreen);
    connect(chkSplit,       &QPushButton::clicked,    theApp,   &TiledPatternMaker::slot_splitScreen);
    connect(showCenterChk,  &QCheckBox::stateChanged, this,     &page_config::slot_showCenterChanged);

    return ViewControlBox;
}

void  page_config::onEnter()
{
}

void  page_config::onRefresh()
{
}

void page_config::slot_selectRootMediaDir()
{
    QString old = config->rootMediaDir;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Media Directory"),
                   old, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
    {
        qDebug() << "Cancel Pressed";
        config->defaultMediaRoot = true;
        return;
    }

    le_rootMedia->setText(dir);
    config->rootMediaDir = dir;
    config->defaultMediaRoot = false;
    slot_reconfigurePaths();
}

void page_config::slot_selectRootImageDir()
{
    QString old = config->rootImageDir;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Image Directory"),
                   old, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);


    if (dir.isEmpty())
    {
        qDebug() << "Cancel Pressed";
        config->defaultImageRoot = true;
        return;
    }
    if (!dir.endsWith("/"))
    {
        dir += "/";
    }

    le_rootImages->setText(dir);
    config->rootImageDir = dir;
    config->defaultImageRoot = false;
    updatePaths();
}

void page_config::slot_selectXMLTool()
{
    QString old = config->xmlTool;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select XML Editor/Viewer"), old);

    le_xmlTool->setText(fileName);
    config->xmlTool = fileName;
}

void page_config::slot_rootDesignChanged(QString txt)
{
    config->rootDesignDir= txt;
    if (!config->defaultMediaRoot)
    {
        slot_reconfigurePaths();    // reboot
    }
}

void page_config::slot_rootImageChanged(QString txt)
{
    config->rootImageDir = txt;
    if (!config->defaultImageRoot)
    {
        config->configurePaths();   // does not reboot
    }
    updatePaths();
}

void page_config::slot_designDefaultChanged(bool checked)
{
    config->defaultMediaRoot = checked;
    slot_reconfigurePaths();        // reboots
}

void page_config::slot_imageDefaultChanged(bool checked)
{
    config->defaultImageRoot = checked;
    config->configurePaths();       // does not reboot
    updatePaths();
}

void page_config::slot_darkThemeChanged(bool checked)
{
    config->darkTheme = checked;
    restartApp();
}

void page_config::slot_reconfigurePaths()
{
    config->configurePaths();
    restartApp();
}

void page_config::updatePaths()
{
    // basic config
    defaultDesigns->setChecked(config->defaultMediaRoot);
    defaultImages->setChecked(config->defaultImageRoot);

    le_rootMedia->setText(config->rootMediaDir);
    le_rootImages->setText(config->rootImageDir);
    le_xmlTool->setText(config->xmlTool);

    update();
}

void page_config::slot_mode(int id)
{
    config->insightMode = (id == 1);
    restartApp();
}

void page_config::restartApp()
{
    // restart the application
    config->save();

    QString cmd = qApp->arguments()[0];

    closeHandle();
    qApp->quit();

    QStringList empty;
    QProcess::startDetached(cmd,empty);
}

void page_config::slot_showCenterChanged(int id)
{
    config->showCenterDebug = (id == Qt::Checked);
    view->update();
}

void page_config::slot_about()
{
    DlgAbout dlg;
    dlg.exec();
}
