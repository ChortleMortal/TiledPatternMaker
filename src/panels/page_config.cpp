#include <QPushButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QFileDialog>

#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
#include <QApplication>
#include <QStyleHints>
#endif

#include "panels/page_config.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"
#include "tiledpatternmaker.h"
#include "widgets/dlg_textedit.h"
#include "misc/qtapplog.h"
#include "misc/sys.h"
#include "viewers/view.h"

page_config:: page_config(ControlPanel * cpanel)  : panel_page(cpanel,PAGE_CONFIG,"Configuration")
{
    log = qtAppLog::getInstance();

    auto appGroup  = createAppConfig();
    auto pathGroup = createMediaPaths();

    QPushButton * restartBtn = new QPushButton("Restart Tiled Pattern Maker");
    QPushButton * pbAbout    = new QPushButton("About Tiled Pattern Maker");
    QPushButton * btnPrimary = new QPushButton("Move to Primary Screen");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(restartBtn);
    hbox->addStretch();
    hbox->addWidget(btnPrimary);
    hbox->addStretch();
    hbox->addWidget(pbAbout);

    // put it all together
    vbox->addWidget(appGroup);
    vbox->addSpacing(7);
    vbox->addWidget(pathGroup);
    vbox->addSpacing(7);
    vbox->addLayout(hbox);
    vbox->addStretch();

    connect(btnPrimary,     &QPushButton::clicked,   theApp, &TiledPatternMaker::slot_bringToPrimaryScreen);
    connect(pbAbout,        &QPushButton::clicked,   this,   &page_config::slot_about);
    connect(restartBtn,     &QPushButton::clicked,   this,   &page_config::restartApp);
}

QGroupBox * page_config::createAppConfig()
{
    QRadioButton * designerMode = new QRadioButton("Designer Mode (recommended)");
    QRadioButton * insightMode  = new QRadioButton("Insight Mode");
    QButtonGroup * btnGroup     = new QButtonGroup;
    btnGroup->addButton(designerMode,0);
    btnGroup->addButton(insightMode,1);
    int button  = (config->insightMode) ? 1 : 0;
    btnGroup->button(button)->setChecked(true);

    QRadioButton * autoTheme = new QRadioButton("Auto Theme");
    QRadioButton * darkTheme = new QRadioButton("Dark Theme");
    QRadioButton * liteTheme = new QRadioButton("Light Theme");
    btnGroup2 = new QButtonGroup;
    btnGroup2->addButton(autoTheme,0);
    btnGroup2->addButton(liteTheme,1);
    btnGroup2->addButton(darkTheme,2);
    btnGroup2->button(int(config->colorTheme))->setChecked(true);

    QCheckBox   * chkSplit      = new QCheckBox("Split Screen");
    QCheckBox   * chkBigScreen  = new QCheckBox("Large Screen Mode (e.g. UHD-3840x2160)");
    chkSplit->setChecked(config->splitScreen);
    chkBigScreen->setChecked(config->bigScreen);
    QCheckBox   * chkLimitView  = new QCheckBox("Limit View Size to available Screen Size");
    chkLimitView->setChecked(config->limitViewSize);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(designerMode);
    hbox->addWidget(insightMode);
    hbox->addStretch();

    QHBoxLayout * hbox3 = new QHBoxLayout;
    hbox3->addWidget(autoTheme);
    hbox3->addWidget(liteTheme);
    hbox3->addWidget(darkTheme);
    hbox3->addStretch();

    QHBoxLayout * hbox4 = new QHBoxLayout();
    hbox4->addWidget(chkSplit);
    hbox4->addWidget(chkBigScreen);
    hbox4->addStretch();

    QHBoxLayout * hbox5 = new QHBoxLayout();
    hbox5->addWidget(chkLimitView);
    hbox5->addStretch();

    QVBoxLayout * vbox2 = new QVBoxLayout;
    vbox2->addLayout(hbox);
    vbox2->addLayout(hbox3);
    vbox2->addLayout(hbox4);
    vbox2->addLayout(hbox5);

    QGroupBox * appGroup = new QGroupBox("Application");
    appGroup->setLayout(vbox2);

    connect(chkSplit,       &QCheckBox::clicked, this, [this](bool checked) { config->splitScreen = checked; restartApp(); });
    connect(chkLimitView,   &QCheckBox::clicked, this, [this](bool checked) { config->limitViewSize = checked; emit sig_refreshView(); });
    connect(chkBigScreen,   &QCheckBox::clicked, this,    &page_config::slot_bigScreen);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(btnGroup,      SIGNAL(buttonClicked(int)), this, SLOT(slot_mode(int)));
    connect(btnGroup2,      SIGNAL(buttonClicked(int)), this, SLOT(slot_darkThemeChanged(int)));
#else
    connect(btnGroup,      &QButtonGroup::idClicked,  this,  &page_config::slot_mode);
    connect(btnGroup2,     &QButtonGroup::idClicked,  this,  &page_config::slot_darkThemeChanged);
#endif

    return appGroup;
}

QGroupBox * page_config::createMediaPaths()
{
    le_rootMedia   = new QLineEdit();
    le_rootImages  = new QLineEdit();
    le_xmlTool     = new QLineEdit();
    le_diffTool    = new QLineEdit();
    le_logName     = new QLineEdit();
    le_logPath     = new QLineEdit();

    le_rootMedia->setMinimumWidth(501);

    rootMediaBtn   = new QPushButton("Root Media Directory");
    rootImagesBtn  = new QPushButton("Root Image Directory");
    QPushButton * xmlToolBtn  = new QPushButton("XML Viewer/Editor");
    QPushButton * diffToolBtn = new QPushButton("Difference Tool");

    defaultDesigns = new QCheckBox("Default Media Directory");
    defaultImages  = new QCheckBox("Default Images Directory");

    QCheckBox * chklogApp = new QCheckBox("Log To App Directory");
    chklogApp->setChecked(config->logToAppDir);

    QLabel * logLabel = new QLabel("Log name");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(defaultDesigns);
    hbox->addWidget(defaultImages);
    hbox->addStretch();

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(logLabel);
    hbox2->addSpacing(3);
    hbox2->addWidget(le_logName);
    hbox2->addStretch();

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

    row++;
    configGrid->addWidget(diffToolBtn,row,0);
    configGrid->addWidget(le_diffTool,row,1);

    row++;
    configGrid->addWidget(chklogApp,row,0);
    configGrid->addWidget(le_logPath,row,1);

    row++;
    configGrid->addWidget(logLabel,row,0);
    configGrid->addWidget(le_logName,row,1);

    QGroupBox * pathGroup = new QGroupBox("File Paths");
    pathGroup->setLayout(configGrid);

    updatePaths();

    connect(rootImagesBtn,  &QPushButton::clicked,   this,  &page_config::slot_selectRootImageDir);
    connect(rootMediaBtn,   &QPushButton::clicked,   this,  &page_config::slot_selectRootMediaDir);
    connect(xmlToolBtn,     &QPushButton::clicked,   this,  &page_config::slot_selectXMLTool);
    connect(diffToolBtn,    &QPushButton::clicked,   this,  &page_config::slot_selectDiffTool);

    connect(le_rootMedia,   &QLineEdit::textChanged,     this, &page_config::slot_rootDesignChanged);
    connect(le_rootImages,  &QLineEdit::textChanged,     this, &page_config::slot_rootImageChanged);
    connect(le_logName,     &QLineEdit::textChanged,     this, [this](QString txt) {config->baseLogName = txt;});
    connect(le_logName,     &QLineEdit::editingFinished, this, [this]() {restartApp();});

    connect(defaultDesigns, &QCheckBox::clicked, this, &page_config::slot_designDefaultChanged);
    connect(defaultImages,  &QCheckBox::clicked, this, &page_config::slot_imageDefaultChanged);
    connect(chklogApp,      &QCheckBox::clicked, this, &page_config::slot_logToAppDir);

    return pathGroup;
}

void  page_config::onEnter()
{}

void  page_config::onRefresh()
{}

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
    QString fileName = QFileDialog::getOpenFileName(this, "Select XML Editor/Viewer", old);

    le_xmlTool->setText(fileName);
    config->xmlTool = fileName;
}

void page_config::slot_selectDiffTool()
{
    QString old = config->diffTool;
    QString fileName = QFileDialog::getOpenFileName(this, "Select Difference Tool", old);

    le_diffTool->setText(fileName);
    config->diffTool = fileName;
}

void page_config::slot_rootDesignChanged(QString txt)
{
    Sys::rootMosaicDir= txt;
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

void page_config::slot_darkThemeChanged(int id)
{
    config->colorTheme = eColorTheme(id);
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
    le_diffTool->setText(config->diffTool);
    le_logName->setText(config->baseLogName);
    le_logPath->setText(log->logDir());

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
    view->unloadView();

#if 0
    QString cmd = qApp->arguments()[0];

    guard->release();
    qApp->quit();

    QStringList empty;
    QProcess::startDetached(cmd,empty);
#else
    qDebug() << "Performing application reboot...";
    qApp->exit(TiledPatternMaker::EXIT_CODE_REBOOT);
#endif
}

void page_config::slot_bigScreen(bool enb)
{
    config->bigScreen = enb;
    if (config->splitScreen)
    {
        restartApp();
    }
}

void page_config::slot_about()
{
    DlgAbout dlg;
    dlg.exec();
}

void page_config::slot_logToAppDir(bool checked)
{
    config->logToAppDir = checked;
    log->logToAppDir(checked);
    log->init();
    updatePaths();
}
