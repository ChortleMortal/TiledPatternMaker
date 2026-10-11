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

#include "gui/panels/page_settings.h"
#include "gui/top/controlpanel.h"
#include "gui/top/view_controller.h"
#include "gui/widgets/dlg_textedit.h"
#include "gui/widgets/memory_combo.h"
#include "model/settings/configuration.h"
#include "sys/qt/qtapplog.h"
#include "sys/sys.h"
#include "sys/sys/debugflags.h"
#include "sys/tiledpatternmaker.h"

page_settings:: page_settings(ControlPanel * cpanel)  : panel_page(cpanel,PAGE_CONFIG,"App Settings")
{
    log = qtAppLog::getInstance();

    auto appGroup     = createAppSetup();
    auto pathGroup    = createMediaPaths();
    auto controlGroup = createAppControl();
    auto infoGroup    = createAppInfo();

    auto hbox = new QHBoxLayout();
    hbox->addWidget(appGroup);
    hbox->addSpacing(7);
    hbox->addWidget(controlGroup);
    hbox->addStretch();

    vbox->addLayout(hbox);
    vbox->addWidget(pathGroup);
    vbox->addWidget(infoGroup);
    vbox->addStretch();
}

QGroupBox * page_settings::createAppControl()
{
    QPushButton * pbFirst        = new QPushButton("First Birthday");
    QPushButton * pbEraseHistory = new QPushButton("Eraase Debug History");
    QPushButton * restartBtn     = new QPushButton("Restart Tiled Pattern Maker");
    QPushButton * btnPrimary     = new QPushButton("Move to Primary Screen");


    auto vbox = new QVBoxLayout();
    vbox->addWidget(restartBtn);
    vbox->addWidget(btnPrimary);
    vbox->addWidget(pbEraseHistory);
    vbox->addWidget(pbFirst);

    QGroupBox * actions = new QGroupBox("App Control");
    actions->setLayout(vbox);

    connect(pbFirst,        &QPushButton::clicked,   this,  &page_settings::slot_firstBirthday);
    connect(pbEraseHistory, &QPushButton::clicked,   this,  &page_settings::slot_eraseDebugFlagHistory);
    connect(btnPrimary,     &QPushButton::clicked,   theApp,&TiledPatternMaker::slot_bringToPrimaryScreen);
    connect(restartBtn,     &QPushButton::clicked,   this,  &page_settings::restartApp);

    return  actions;
}

QGroupBox * page_settings::createAppInfo()
{
    QPushButton * pbAbout    = new QPushButton("About Tiled Pattern Maker");
    QPushButton * pbStartup  = new QPushButton("Startup options");
    QPushButton * btnSupport = new QPushButton("Support");
    QPushButton * btnKbdd    = new QPushButton("Keyboard and Shortcuts");

    QGridLayout * grid = new QGridLayout();
    grid->addWidget(btnKbdd,0,0);
    grid->addWidget(pbStartup,0,1);
    grid->addWidget(pbAbout,0,2);
    grid->addWidget(btnSupport,0,3);

    QGroupBox * info = new QGroupBox("App Information");
    info->setLayout(grid);

    connect(pbAbout,        &QPushButton::clicked,   this,   &page_settings::slot_about);
    connect(pbStartup,      &QPushButton::clicked,   this,   &page_settings::slot_start_options);
    connect(btnSupport,     &QPushButton::clicked,   this,   &page_settings::slot_showSupport);
    connect(btnKbdd,        &QPushButton::clicked,   this,   &page_settings::slot_showShortcuts);

    return info;
}

QGroupBox * page_settings::createAppSetup()
{
    QRadioButton * designerMode = new QRadioButton("Designer Mode (basic)");
    QRadioButton * insightMode  = new QRadioButton("Insight Mode (advanced)");
    QButtonGroup * btnGroup     = new QButtonGroup;
    btnGroup->addButton(designerMode,0);
    btnGroup->addButton(insightMode,1);
    int button  = (config->insightMode) ? 1 : 0;
    btnGroup->button(button)->setChecked(true);

    QRadioButton * autoTheme = new QRadioButton("Auto Theme");
    QRadioButton * darkTheme = new QRadioButton("Dark Theme");
    QRadioButton * liteTheme = new QRadioButton("Light Theme");
    btnGroup2 = new QButtonGroup;
    btnGroup2->addButton(liteTheme,1);
    btnGroup2->addButton(darkTheme,2);
    btnGroup2->addButton(autoTheme,0);
    btnGroup2->button(int(config->colorTheme))->setChecked(true);

    QRadioButton * dualScreen   = new QRadioButton("Dual Windows (recommended)");
    QRadioButton * monoScreen   = new QRadioButton("Integrated Window");
    QCheckBox   * chkBigScreen  = new QCheckBox("Large Integrated Window (e.g. UHD-3840x2160)");
    dualScreen->setChecked(!config->integratedScreen);
    monoScreen->setChecked(config->integratedScreen);
    chkBigScreen->setChecked(config->bigScreen);

    QCheckBox   * chkLimitView  = new QCheckBox("Limit View Size to available Screen Size");
    chkLimitView->setChecked(config->limitViewSize);

    QCheckBox   * chkSplash = new QCheckBox("Disable splash screens");
    chkSplash->setChecked(config->disableSplash);

    QCheckBox   * chkResize = new QCheckBox("Disable resize notifications");
    chkResize->setChecked(config->disableResizeNotify);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(designerMode);
    hbox->addWidget(insightMode);
    hbox->addStretch();

    QHBoxLayout * hbox3 = new QHBoxLayout;
    hbox3->addWidget(autoTheme);
    hbox3->addWidget(liteTheme);
    hbox3->addWidget(darkTheme);
    hbox3->addStretch();

    QHBoxLayout * hbox4a = new QHBoxLayout();
    hbox4a->addWidget(dualScreen);
    hbox4a->addWidget(monoScreen);
    hbox4a->addStretch();

    QHBoxLayout * hbox4b = new QHBoxLayout();
    hbox4b->addWidget(chkBigScreen);
    hbox4b->addStretch();

    QHBoxLayout * hbox5 = new QHBoxLayout();
    hbox5->addWidget(chkLimitView);
    hbox5->addStretch();

    QHBoxLayout * hbox6 = new QHBoxLayout();
    hbox6->addWidget(chkSplash);
    hbox6->addStretch();

    QHBoxLayout * hbox7 = new QHBoxLayout();
    hbox7->addWidget(chkResize);
    hbox7->addStretch();

    QVBoxLayout * vbox2 = new QVBoxLayout;
    vbox2->addLayout(hbox);
    vbox2->addLayout(hbox3);
    vbox2->addLayout(hbox4a);
    vbox2->addLayout(hbox4b);
    vbox2->addLayout(hbox5);
    vbox2->addLayout(hbox6);
    vbox2->addLayout(hbox7);

    QGroupBox * appGroup = new QGroupBox("App Settings");
    appGroup->setLayout(vbox2);

    connect(dualScreen,     &QRadioButton::clicked,   this, [this](bool checked) { config->integratedScreen = !checked; restartApp(); });
    connect(monoScreen,     &QRadioButton::clicked,   this, [this](bool checked) { config->integratedScreen =  checked; restartApp(); });
    connect(chkLimitView,   &QCheckBox::clicked,      this, [this](bool checked) { config->limitViewSize = checked; emit sig_reconstructView(); });
    connect(chkSplash,      &QCheckBox::clicked,      this, [this](bool checked) { config->disableSplash = checked; Sys::systemViewController->splash_disable(checked); });
    connect(chkResize,      &QCheckBox::clicked,      this, [this](bool checked) { config->disableResizeNotify = checked; });
    connect(chkBigScreen,   &QCheckBox::clicked,      this, &page_settings::slot_bigScreen);
    connect(btnGroup,       &QButtonGroup::idClicked, this, &page_settings::slot_mode);
    connect(btnGroup2,      &QButtonGroup::idClicked, this, &page_settings::slot_darkThemeChanged);

    return appGroup;
}

QGroupBox * page_settings::createMediaPaths()
{
    rootMediaCombo = new MemoryCombo("rootMediaCombo");
    rootImagesCombo= new MemoryCombo("rootImagesCombo");
    xmlToolCombo   = new MemoryCombo("xmlTool2");
    diffToolCombo  = new MemoryCombo("diffTool2");
    logPathCombo   = new MemoryCombo("logPathCombo");
    le_logName     = new QLineEdit();

    rootMediaCombo->setMinimumWidth(501);
    le_logName->setReadOnly(true);

    rootMediaBtn   = new QPushButton("Root Media Directory");
    rootImagesBtn  = new QPushButton("Root Image Directory");
    rootLogBtn     = new QPushButton("Log Directory");

    QPushButton * xmlToolBtn  = new QPushButton("XML Viewer/Editor");
    QPushButton * diffToolBtn =  new QPushButton("Difference Tool");

    defaultMedia   = new QCheckBox("Default Media Directory");
    defaultImages  = new QCheckBox("Default Images Directory");
    defaultLogs    = new QCheckBox("Default Log Directory");

    QLabel * logLabel = new QLabel("Log name");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(defaultMedia);
    hbox->addWidget(defaultImages);
    hbox->addWidget(defaultLogs);

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
    configGrid->addWidget(rootMediaCombo,row,1);

    row++;
    configGrid->addWidget(rootImagesBtn,row,0);
    configGrid->addWidget(rootImagesCombo,row,1);

    row++;
    configGrid->addWidget(xmlToolBtn,row,0);
    configGrid->addWidget(xmlToolCombo,row,1);

    row++;
    configGrid->addWidget(diffToolBtn,row,0);
    configGrid->addWidget(diffToolCombo,row,1);

    row++;
    configGrid->addWidget(rootLogBtn,row,0);
    configGrid->addWidget(logPathCombo,row,1);

    row++;
    configGrid->addWidget(logLabel,row,0);
    configGrid->addWidget(le_logName,row,1);

    QGroupBox * pathGroup = new QGroupBox("App Path Settings");
    pathGroup->setLayout(configGrid);

    updatePaths();

    connect(rootImagesBtn,  &QPushButton::clicked,   this,  &page_settings::slot_selectRootImageDir);
    connect(rootMediaBtn,   &QPushButton::clicked,   this,  &page_settings::slot_selectRootMediaDir);
    connect(rootLogBtn,     &QPushButton::clicked,   this,  &page_settings::slot_selectRootLogDir);

    connect(xmlToolBtn,     &QPushButton::clicked,   this,  &page_settings::slot_selectXMLTool);
    connect(diffToolBtn,    &QPushButton::clicked,   this,  &page_settings::slot_selectDiffTool);

    connect(rootMediaCombo, &QComboBox::currentIndexChanged, this, &page_settings::slot_rootMediaIndexChanged);
    connect(rootImagesCombo,&QComboBox::currentIndexChanged, this, &page_settings::slot_rootImagesIndexChanged);
    connect(logPathCombo,   &QComboBox::currentIndexChanged, this, &page_settings::slot_rootLogIndexChanged);
    connect(xmlToolCombo,   &QComboBox::currentIndexChanged, this, &page_settings::slot_XMLToolChanged1);
    connect(diffToolCombo,  &QComboBox::currentIndexChanged, this, &page_settings::slot_DiffToolChanged2);

    connect(rootMediaCombo->lineEdit(), &QLineEdit::editingFinished, this, &page_settings::slot_rootMediaEditChanged);
    connect(logPathCombo->lineEdit(),   &QLineEdit::editingFinished, this, &page_settings::slot_rootLogEditChanged);
    connect(rootImagesCombo->lineEdit(),&QLineEdit::editingFinished, this, &page_settings::slot_rootImagesEditChanged);
    connect(xmlToolCombo->lineEdit(),   &QLineEdit::editingFinished, this, &page_settings::slot_XMLToolChanged1);
    connect(diffToolCombo->lineEdit(),  &QLineEdit::editingFinished, this, &page_settings::slot_DiffToolChanged2);

    connect(defaultMedia,   &QCheckBox::clicked, this, &page_settings::slot_mediaDefaultChanged);
    connect(defaultImages,  &QCheckBox::clicked, this, &page_settings::slot_imageDefaultChanged);
    connect(defaultLogs,    &QCheckBox::clicked, this, &page_settings::slot_logDefaultChanged);

    return pathGroup;
}

void  page_settings::onEnter()
{}

void  page_settings::onRefresh()
{}

void page_settings::updatePaths()
{
    defaultMedia->setChecked(config->defaultMediaRoot);
    defaultImages->setChecked(config->defaultImageRoot);
    defaultLogs->setChecked(config->defaultLogRoot);

    rootMediaCombo->setCurrentText(Sys::getRootMediaDir());
    rootImagesCombo->setCurrentText(Sys::getRootImageDir());
    logPathCombo->setCurrentText(Sys::getLogDir());

    le_logName->setText(Sys::logName);

    Sys::xmlTool  = xmlToolCombo->getCurrentText();
    Sys::diffTool = diffToolCombo->getCurrentText();

    rootMediaCombo->setEnabled(!config->defaultMediaRoot);
    rootImagesCombo->setEnabled(!config->defaultImageRoot);
    logPathCombo->setEnabled(!config->defaultLogRoot);
}

//////////////////////////////////////////////////
///
///  Media
///
//////////////////////////////////////////////////

void page_settings::slot_mediaDefaultChanged(bool checked)
{
    config->defaultMediaRoot = checked;
    config->configureMediaPaths(true);
    updatePaths();
}

void page_settings::slot_rootMediaIndexChanged(int index)
{
    Q_UNUSED(index);
    QString str = rootMediaCombo->currentText();

    rootMediaChanged(str);
}

void page_settings::slot_rootMediaEditChanged()
{
    QString str = rootMediaCombo->currentText();

    rootMediaChanged(str);
}

void page_settings::slot_selectRootMediaDir()
{
    QString old = Sys::getRootMediaDir();
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Media Directory"),
                                                    old, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
    {
        qDebug() << "Cancel Pressed";
        return;
    }

    if (!dir.endsWith("/"))
        dir += "/";

    rootMediaCombo->setCurrentText(dir);

    config->defaultMediaRoot = false;

    rootMediaChanged(dir);
}

void page_settings::rootMediaChanged(QString dir)
{
    Q_ASSERT(config->defaultMediaRoot == false);

    if (dir == Sys::getRootMediaDir())
    {
        return;     // not a real change
    }

    config->setAlternateRootMediaDir(dir);
    config->configureMediaPaths(true);
    updatePaths();
}

//////////////////////////////////////////////////
///
///  Log
///
//////////////////////////////////////////////////

void page_settings::slot_logDefaultChanged(bool checked)
{
    config->defaultLogRoot = checked;
    config->configureLogPaths(true);
    updatePaths();
}

void page_settings::slot_selectRootLogDir()
{
    QString old = Sys::getLogDir();
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Log Directory"),
                                                    old, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
    {
        qDebug() << "Cancel Pressed";
        return;
    }

    if (!dir.endsWith("/"))
        dir += "/";

    logPathCombo->setCurrentText(dir);

    config->defaultLogRoot = false;

    logPathChanged(dir);
}

void page_settings::slot_rootLogIndexChanged(int index)
{
    Q_UNUSED(index);
    QString str = logPathCombo->currentText();

    logPathChanged(str);
}

void page_settings::slot_rootLogEditChanged()
{
    QString str = logPathCombo->currentText();

    logPathChanged(str);
}

void page_settings::logPathChanged(QString dir)
{
    Q_ASSERT(config->defaultLogRoot == false);

    if (dir == Sys::getLogDir())
    {
        return;     // not a real change
    }

    config->setAlternateRootLogDir(dir);
    config->configureLogPaths(true);
    updatePaths();
}

//////////////////////////////////////////////////
///
///  Images
///
//////////////////////////////////////////////////

void page_settings::slot_imageDefaultChanged(bool checked)
{
    config->defaultImageRoot = checked;
    config->configureImagePaths(true);
    updatePaths();
}

void page_settings::slot_rootImagesIndexChanged(int index)
{
    Q_UNUSED(index);
    QString str = rootImagesCombo->currentText();

    rootImagesChanged(str);
}

void page_settings::slot_rootImagesEditChanged()
{
    QString str = rootImagesCombo->currentText();

    rootImagesChanged(str);
}

void page_settings::slot_selectRootImageDir()
{
    QString old = Sys::getRootImageDir();
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Image Directory"),
                                                    old, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
    {
        qDebug() << "Cancel Pressed";
        return;
    }

    if (!dir.endsWith("/"))
        dir += "/";

    rootImagesCombo->setCurrentText(dir);

    config->defaultImageRoot = false;

    rootImagesChanged(dir);
}

void page_settings::rootImagesChanged(QString dir)
{
    Q_ASSERT(config->defaultImageRoot == false);

    if (dir == Sys::getRootImageDir())
    {
        return;     // not a real change
    }

    config->setAlternateRootImageDir(dir);
    config->configureImagePaths(true);
    updatePaths();
}

//////////////////////////////////////////////////
///
///  Other
///
//////////////////////////////////////////////////

void page_settings::slot_XMLToolChanged1()
{
    xmlToolCombo->select(xmlToolCombo->currentIndex());
    Sys::xmlTool = xmlToolCombo->getCurrentText();
}

void page_settings::slot_DiffToolChanged2()
{
    diffToolCombo->select(diffToolCombo->currentIndex());
    Sys::diffTool = diffToolCombo->getCurrentText();
}

void page_settings::slot_selectXMLTool()
{
    QString old = xmlToolCombo->getCurrentText();
    QString fileName = QFileDialog::getOpenFileName(this, "Select XML Editor/Viewer", old);

    xmlToolCombo->setCurrentText(fileName);
    Sys::xmlTool = fileName;
}

void page_settings::slot_selectDiffTool()
{
    QString old = diffToolCombo->getCurrentText();
    QString fileName = QFileDialog::getOpenFileName(this, "Select Difference Tool", old);

    diffToolCombo->setCurrentText(fileName);
    Sys::diffTool = fileName;
}

void page_settings::slot_darkThemeChanged(int id)
{
    config->colorTheme = eColorTheme(id);
    restartApp();
}

void page_settings::slot_mode(int id)
{
    config->pendingInsightMode = (id == 1);
    restartApp();
}

void page_settings::restartApp()
{
    QMessageBox box(panel);
    box.setIcon(QMessageBox::Critical);
    box.setText("Application restart required to apply settings");
    QPushButton * nowBtn   = box.addButton("Restart now",QMessageBox::AcceptRole);
    QPushButton * laterBtn = box.addButton("Later",QMessageBox::RejectRole);
    box.exec();

    if (box.clickedButton() == laterBtn)
        return;

    if (box.clickedButton() != nowBtn)
    {
        qWarning("Unexpected button clicked");
        return;
    }

    // restart the application
    Sys::updatePanel = false;
    viewControl->slot_unloadView();

    qInfo() << "Performing application reboot...";
    qApp->exit(TiledPatternMaker::EXIT_CODE_REBOOT);
}

void page_settings::slot_bigScreen(bool enb)
{
    config->bigScreen = enb;
    if (config->integratedScreen)
    {
        restartApp();
    }
}

void page_settings::slot_firstBirthday()
{
    QMessageBox box(panel);
    box.setIcon(QMessageBox::Critical);
    box.setWindowTitle("First Birthday TiledPatternMaker");
    box.setText("This is VERY DRASTIC - it erases all registry settings\nand sets them back to defaults.\nThere is no coming back from this.\nOK to  Proceed?");
    box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    box.setDefaultButton(QMessageBox::Cancel);
    auto rv = box.exec();
    if (rv == QMessageBox::Cancel)
        return;

    QSettings settings = Sys::settings();
    settings.remove("");

    Sys::config->firstBirthday = true;  // prevents persistence on exit

    restartApp();
}

void page_settings::slot_eraseDebugFlagHistory()
{
    QMessageBox box(panel);
    box.setIcon(QMessageBox::Critical);
    box.setText("This will erase all previous debug flag settings.\nOK to  Proceed?");
    box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    box.setDefaultButton(QMessageBox::Cancel);
    auto rv = box.exec();
    if (rv == QMessageBox::Cancel)
        return;

    Sys::flags->eraseHistory();
}

void page_settings::slot_about()
{
    DlgAbout dlg(panel);
    dlg.exec();
}

void page_settings::slot_start_options()
{
    DlgStartOptions dlg(panel);
    dlg.exec();
}

void page_settings::slot_showSupport()
{
    DlgSupport dlg(panel);
    dlg.exec();
}

void page_settings::slot_showShortcuts()
{
    DlgKbdOpts dlg(panel);
    dlg.exec();
}
