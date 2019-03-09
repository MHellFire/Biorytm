/************************************************************************
 * Copyright © 2018 Mariusz Helfajer                                    *
 *                                                                      *
 * This file is part of Biorytm.                                        *
 *                                                                      *
 * Biorytm is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * Biorytm is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with Biorytm.  If not, see <http://www.gnu.org/licenses/>.     *
 ************************************************************************/

#include "mainwindow.hpp"
#include "usersmanagerdialog.hpp"
#include "helpdialog.hpp"
#include "aboutdialog.hpp"
#include "constants.hpp"

#include <QMessageBox>
#include <QStandardPaths>
#include <QSettings>
#include <QCalendarWidget>
#include <QMenu>
#include <QFileDialog>
#include <QSvgGenerator>
#include <QtMath>

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    settings = new Settings;

    ui.setupUi(this);

    //
    if (Settings::readValue("MainWindow", "firstRun", true).toBool())
        Settings::writeValue("MainWindow", "firstRun", false);

    // read users from settings file
    usersList = new QList<User>;
    usersList->clear();
    Settings::readUsers(usersList);

    createTrayIcon();
    createMenu();
    createToolButtonMenu();
    createStatusBar();
    createClockTimer();
    restoreWindowState();

    ui.toolButtonLeftDay->setToolTip("Wstecz o dzień");
    ui.toolButtonRightDay->setToolTip("Naprzód o dzień");

    ui.dateEdit->calendarWidget()->setFirstDayOfWeek(Qt::Monday);

    createOptionsDialog();
    createConnections();

    // choose user
    Settings::readValue("User", "saveLastUser", true).toBool() ? currentUserUID = static_cast<quint16>(Settings::readValue("User", "lastUserUID", constants::minUID).toUInt()) : currentUserUID = 0;

    if (Settings::readValue("UsersManagerDialog", "showUsersManagerDlg", true).toBool())
        showUsersManagerDlg(1); // visible on taskbar
    else
        showUsersManagerDlg(100); // silent mode (no window, just choose user)


    // read settings
    ui.widgetBioDataBottom->setVisible(Settings::readValue("MainWindow", "widgetBioDataBottom", true).toBool());
    ui.widgetBioDataRight->setVisible(Settings::readValue("MainWindow", "widgetBioDataRight", false).toBool());

    ui.menubar->setVisible(Settings::readValue("MainWindow", "menuBar", false).toBool());
    ui.statusbar->setVisible(Settings::readValue("MainWindow", "statusBar", false).toBool());

    // 0 == ask user, 1 == app, 2 == graph
    saveScreenshotOption = static_cast<quint8>(Settings::readValue("MainWindow", "saveScreenshotOption", 0).toUInt());

    restoreBioGraph();
}

MainWindow::~MainWindow()
{
    // delete "on-line" options dialog
    optionsDialog->close();
    delete optionsDialog;

    delete settings;
}


// ****************************************************************************
//
void MainWindow::resizeEvent(QResizeEvent *e)
{
    oldSize = e->oldSize();
}

void MainWindow::moveEvent(QMoveEvent *e)
{
    oldPosition = e->oldPos();
}


// ****************************************************************************
// Minimize to Tray Icon or Quit
void MainWindow::closeEvent(QCloseEvent *e)
{
    if (minimizeToTrayIconOnClose)
    {
        if (trayIconAvailable)
        {
            if (trayIcon->isVisible())
            {
                //QMessageBox::information(this, tr("Systray"), tr("The program will keep running in the system tray. "
                //                                                 "To terminate the program choose <b>Quit</b> "
                //                                                 "in the context menu of the system tray entry or "
                //                                                 "in \"File\" menu of main window."));

                //hide();
                minimizeToTray();
                //trayIcon->showMessage(tr("LANAnalyzer tray icon"), tr("Click here to restore application"), QSystemTrayIcon::Information, 3000);

                e->ignore();
            }
            else
            {
                createTrayIcon();

                //QMessageBox::information(this, tr("Systray"), tr("The program will keep running in the system tray. "
                //                                                 "To terminate the program choose <b>Quit</b> "
                //                                                 "in the context menu of the system tray entry or "
                //                                                 "in \"File\" menu of main window."));

                //hide();
                minimizeToTray();
                e->ignore();
            }
        }
        else
        {
            //QMessageBox::critical(this, "Critical", "The system tray is not available.");
        }
    }
    else
    {
        saveSettings();

        e->accept();

        // kubuntu fix?
        QCoreApplication::quit();
    }
}

// ****************************************************************************
// keyboard shortcuts
void MainWindow::keyPressEvent(QKeyEvent *e)
{
    //Q_UNUSED(e);

    //e->ignore();

    // toggle full screen
    // F11
    if (e->key() == Qt::Key_F11)
        toggleFullScreen();

    // show options dialog
    // Ctrl + O
    if ((e->modifiers() & Qt::ControlModifier) && (e->key() == Qt::Key_O))
        showOptionsDialog();

    // show about dialog
    // Ctrl + F1
    if ((e->modifiers() & Qt::ControlModifier) && (e->key() == Qt::Key_F1))
        showAboutDialog();

    // show users manager dialog
    // Ctrl + M
    if ((e->modifiers() & Qt::ControlModifier) && (e->key() == Qt::Key_M))
        showUsersManagerDlg();

    // add new user
    // Ctrl + D
    if ((e->modifiers() & Qt::ControlModifier) && (e->key() == Qt::Key_D))
        addNewUser();

    // edit user
    // Ctrl + E
    if ((e->modifiers() & Qt::ControlModifier) && (e->key() == Qt::Key_E))
        showEditUserDlg();

    // quit the application
    // Ctrl + Q
    //if ((e->modifiers() & Qt::ControlModifier) && (e->key() == Qt::Key_Q))
    //  qApp->quit();
}


// ****************************************************************************
// save settings
void MainWindow::saveSettings()
{
    qDebug() << "I'm saving MainWindow settings!!!";

    // UsersList
    Settings::writeUsers(usersList);

    // MainWindow
    Settings::writeValue("MainWindow", "maximized", isMaximized());
    Settings::writeValue("MainWindow", "fullScreen", isFullScreen());
    if (isMaximized() || isFullScreen())
    {
         Settings::writeValue("MainWindow", "size", oldSize);
         Settings::writeValue("MainWindow", "position", oldPosition);
    }
    else
    {
        Settings::writeValue("MainWindow", "size", size());
        Settings::writeValue("MainWindow", "position", pos());
    }
    Settings::writeValue("MainWindow", "alwaysOnTop", windowFlags().testFlag(Qt::WindowStaysOnTopHint));
    Settings::writeValue("MainWindow", "minimizeToTrayIconOnClose", minimizeToTrayIconOnClose);
    Settings::writeValue("MainWindow", "showTrayIcon", trayIcon->isVisible());
    Settings::writeValue("MainWindow", "menuBar", ui.menubar->isVisible());
    Settings::writeValue("MainWindow", "statusBar", ui.statusbar->isVisible());
    Settings::writeValue("MainWindow", "widgetBioDataBottom", ui.widgetBioDataBottom->isVisible());
    Settings::writeValue("MainWindow", "widgetBioDataRight", ui.widgetBioDataRight->isVisible());
    Settings::writeValue("MainWindow", "saveScreenshotOption", saveScreenshotOption);

    // Biorhythm
    Settings::writeValue("Biorhythm", "phyBio", ui.widgetBiorhythmGraph->isPhyBio());
    Settings::writeValue("Biorhythm", "phyBioColor", ui.widgetBiorhythmGraph->getPhyBioColor());
    Settings::writeValue("Biorhythm", "emoBio", ui.widgetBiorhythmGraph->isEmoBio());
    Settings::writeValue("Biorhythm", "emoBioColor", ui.widgetBiorhythmGraph->getEmoBioColor());
    Settings::writeValue("Biorhythm", "intBio", ui.widgetBiorhythmGraph->isIntBio());
    Settings::writeValue("Biorhythm", "intBioColor", ui.widgetBiorhythmGraph->getIntBioColor());
    Settings::writeValue("Biorhythm", "intuBio", ui.widgetBiorhythmGraph->isIntuBio());
    Settings::writeValue("Biorhythm", "intuBioColor", ui.widgetBiorhythmGraph->getIntuBioColor());
    Settings::writeValue("Biorhythm", "averageBio", ui.widgetBiorhythmGraph->isAverageBio());
    Settings::writeValue("Biorhythm", "averageBioColor", ui.widgetBiorhythmGraph->getAverageBioColor());

    Settings::writeValue("Biorhythm", "showToday", ui.widgetBiorhythmGraph->isShowToday());
    Settings::writeValue("Biorhythm", "vLines", ui.widgetBiorhythmGraph->isVLines());
    Settings::writeValue("Biorhythm", "vLabels", ui.widgetBiorhythmGraph->isAxisVLabels());
    Settings::writeValue("Biorhythm", "hLines", ui.widgetBiorhythmGraph->isHLines());
    Settings::writeValue("Biorhythm", "hLabels", ui.widgetBiorhythmGraph->isAxisHLabels());
    Settings::writeValue("Biorhythm", "zeroLine", ui.widgetBiorhythmGraph->isZeroLine());

    // User
    if (Settings::readValue("User", "saveLastUser", true).toBool())
        Settings::writeValue("User", "lastUserUID", currentUserUID);

}


// ****************************************************************************
// create application Main Menu
void MainWindow::createMenu()
{
    // application
    connect(ui.menuApplication, SIGNAL(aboutToShow()), this, SLOT(updateMenuApplicationActions()));

    connect(ui.actionSaveScreenShoot, SIGNAL(triggered(bool)), this, SLOT(saveScreenshot()));
    //connect(ui.actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui.actionQuit, SIGNAL(triggered()), this, SLOT(close()));

    // biorhythm
    connect(ui.menuBiorhythm, SIGNAL(aboutToShow()), this, SLOT(updateMenuBiorhythmActions()));

    connect(ui.actionPhy, SIGNAL(triggered(bool)), ui.widgetBiorhythmGraph, SLOT(setPhyBio(bool)));
    connect(ui.actionEmo, SIGNAL(triggered(bool)), ui.widgetBiorhythmGraph, SLOT(setEmoBio(bool)));
    connect(ui.actionInt, SIGNAL(triggered(bool)), ui.widgetBiorhythmGraph, SLOT(setIntBio(bool)));
    connect(ui.actionIntu, SIGNAL(triggered(bool)), ui.widgetBiorhythmGraph, SLOT(setIntuBio(bool)));
    connect(ui.actionAverage, SIGNAL(triggered(bool)), ui.widgetBiorhythmGraph, SLOT(setAverageBio(bool)));

    connect(ui.actionBioNone, SIGNAL(triggered(bool)), ui.widgetBioDataRight, SLOT(setHidden(bool)));
    connect(ui.actionBioNone, SIGNAL(triggered(bool)), ui.widgetBioDataBottom, SLOT(setHidden(bool)));
    connect(ui.actionBioRight, SIGNAL(triggered(bool)), ui.widgetBioDataRight, SLOT(setVisible(bool)));
    connect(ui.actionBioRight, SIGNAL(triggered(bool)), ui.widgetBioDataBottom, SLOT(setHidden(bool)));
    connect(ui.actionBioBottom, SIGNAL(triggered(bool)), ui.widgetBioDataRight, SLOT(setHidden(bool)));
    connect(ui.actionBioBottom, SIGNAL(triggered(bool)), ui.widgetBioDataBottom, SLOT(setVisible(bool)));

    // graph
    connect(ui.menuGraph, SIGNAL(aboutToShow()), this, SLOT(updateMenuGraphActions()));

    connect(ui.actionShowToday, SIGNAL(triggered(bool)), ui.widgetBiorhythmGraph, SLOT(setToday(bool)));
    connect(ui.actionVLines, SIGNAL(triggered(bool)), ui.widgetBiorhythmGraph, SLOT(setVLines(bool)));
    connect(ui.actionAxisVLabels, SIGNAL(triggered(bool)), ui.widgetBiorhythmGraph, SLOT(showAxisVLabels(bool)));
    connect(ui.actionHLines, SIGNAL(triggered(bool)), ui.widgetBiorhythmGraph, SLOT(setHLines(bool)));
    connect(ui.actionAxisHLabels, SIGNAL(triggered(bool)), ui.widgetBiorhythmGraph, SLOT(showAxisHLabels(bool)));
    connect(ui.actionZeroLine, SIGNAL(triggered(bool)), ui.widgetBiorhythmGraph, SLOT(setZero(bool)));

    // options
    connect(ui.menuOptions, SIGNAL(aboutToShow()), this, SLOT(updateMenuOptionsActions()));

    connect(ui.actionShowMenubar, SIGNAL(triggered()), this, SLOT(showMenuBar()));
    connect(ui.actionShowButtonMenu, SIGNAL(triggered()), this, SLOT(showToolButtonMenu()));
    connect(ui.actionStatusBar, SIGNAL(triggered(bool)), ui.statusbar, SLOT(setVisible(bool)));
    connect(ui.actionAlwaysOnTop, SIGNAL(triggered()), this, SLOT(toggleAlwaysOnTop()));
    connect(ui.actionFullScreen, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));
    if (trayIconAvailable)
        connect(ui.actionTrayIcon, SIGNAL(triggered(bool)), trayIcon, SLOT(setVisible(bool)));
    connect(ui.actionMinimizeToTray, SIGNAL(triggered()), this, SLOT(minimizeToTray()));
    connect(ui.actionOptionsDialog, SIGNAL(triggered()), this, SLOT(showOptionsDialog()));

    // help
    connect(ui.actionShowHelp, SIGNAL(triggered()), this, SLOT(showHelpDialog()));
    connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
}

void MainWindow::updateMenuApplicationActions()
{
    ui.actionSaveScreenShoot->setEnabled(ui.widgetBiorhythmGraph->isEnabled());
}

void MainWindow::updateMenuBiorhythmActions()
{
    // bio graph lines
    ui.actionPhy->setChecked(ui.widgetBiorhythmGraph->isPhyBio());
    ui.actionEmo->setChecked(ui.widgetBiorhythmGraph->isEmoBio());
    ui.actionInt->setChecked(ui.widgetBiorhythmGraph->isIntBio());
    ui.actionIntu->setChecked(ui.widgetBiorhythmGraph->isIntuBio());
    ui.actionAverage->setChecked(ui.widgetBiorhythmGraph->isAverageBio());

    // bio data widgets
    ui.actionBioNone->setChecked(!ui.widgetBioDataRight->isVisible() && !ui.widgetBioDataBottom->isVisible());
    ui.actionBioRight->setChecked(ui.widgetBioDataRight->isVisible());
    ui.actionBioBottom->setChecked(ui.widgetBioDataBottom->isVisible());
}

void MainWindow::updateMenuGraphActions()
{
    ui.actionShowToday->setChecked(ui.widgetBiorhythmGraph->isShowToday());
    ui.actionVLines->setChecked(ui.widgetBiorhythmGraph->isVLines());
    ui.actionAxisVLabels->setChecked(ui.widgetBiorhythmGraph->isAxisVLabels());
    ui.actionHLines->setChecked(ui.widgetBiorhythmGraph->isHLines());
    ui.actionAxisHLabels->setChecked(ui.widgetBiorhythmGraph->isAxisHLabels());
    ui.actionZeroLine->setChecked(ui.widgetBiorhythmGraph->isZeroLine());
}

void MainWindow::updateMenuOptionsActions()
{
    ui.actionShowMenubar->setChecked(ui.menubar->isVisible());
    ui.actionShowButtonMenu->setChecked(ui.toolButtonMenu->isVisible());
    ui.actionStatusBar->setChecked(ui.statusbar->isVisible());
    ui.actionAlwaysOnTop->setChecked(windowFlags().testFlag(Qt::WindowStaysOnTopHint));
    ui.actionFullScreen->setChecked(windowState() & Qt::WindowFullScreen);
    ui.actionTrayIcon->setEnabled(trayIconAvailable);
    ui.actionTrayIcon->setChecked(trayIconAvailable && trayIcon->isVisible());
    ui.actionMinimizeToTray->setEnabled(trayIconAvailable);
}

void MainWindow::showMenuBar()
{
    if (ui.toolButtonMenu->isVisible())
        ui.menubar->setVisible(!ui.menubar->isVisible());
}


// ****************************************************************************
// statusbar
void MainWindow::createStatusBar()
{
    ui.statusbar->addWidget(new QLabel("Data: "));
    ui.statusbar->addWidget(dateLabel = new QLabel(this));
    ui.statusbar->addWidget(new QLabel("Godzina: "));
    ui.statusbar->addWidget(clockLabel = new QLabel(this));
    ui.statusbar->addPermanentWidget(infoLabel = new QLabel(this));
}

// clock and date for statusbar
void MainWindow::createClockTimer()
{
    dateLabel->setText(QDate::currentDate().toString("dd.MM.yyyy"));
    clockLabel->setText(QTime::currentTime().toString("hh:mm:ss"));

    clockTimer = new QTimer(this);
    connect(clockTimer, SIGNAL(timeout()), this, SLOT(updateClockTimer()));
    clockTimer->start(1000);
}

void MainWindow::updateClockTimer()
{
    dateLabel->setText(QDate::currentDate().toString("dd.MM.yyyy"));
    clockLabel->setText(QTime::currentTime().toString("hh:mm:ss"));
    clockTimer->start(1000);
}


// ****************************************************************************
// restore all Saved application window states/settings/appearance
void MainWindow::restoreWindowState()
{
    // background color
    //appBackgroundColor = settings->defaultSettings.backgroundColor; //QApplication::palette().background().color();

    resize(Settings::readValue("MainWindow", "size", QSize(QApplication::desktop()->availableGeometry().width()/2, QApplication::desktop()->availableGeometry().height()/4)).toSize());
    move(Settings::readValue("MainWindow", "position", QPoint(QApplication::desktop()->availableGeometry().width()/4, QApplication::desktop()->availableGeometry().height()/4)).toPoint());

    if (Settings::readValue("MainWindow", "alwaysOnTop", false).toBool())
    {
        Qt::WindowFlags flags = windowFlags();
        flags.testFlag(Qt::WindowStaysOnTopHint) ? flags ^= Qt::WindowStaysOnTopHint : flags |= Qt::WindowStaysOnTopHint;
        setWindowFlags(flags);
    }

    if (Settings::readValue("MainWindow", "startMaximized", false).toBool() || Settings::readValue("MainWindow", "maximized", false).toBool())
    {
        setWindowState(windowState() ^ Qt::WindowMaximized);
    }

    if (Settings::readValue("MainWindow", "fullScreen", false).toBool())
    {
        ui.actionAlwaysOnTop->setDisabled(true);
        setWindowState(windowState() ^ Qt::WindowFullScreen);
        ui.actionFullScreen->setChecked(true);
    }

    statusBar()->setVisible(Settings::readValue("MainWindow", "statusBar", false).toBool());

    minimizeToTrayIconOnClose = Settings::readValue("MainWindow", "minimizeToTrayIconOnClose", false).toBool();

    if (Settings::readValue("MainWindow", "showTrayIcon", false).toBool())
        createTrayIcon();
}


// ****************************************************************************
// restore biorhythm graph saved settings
void MainWindow::restoreBioGraph()
{
    ui.widgetBiorhythmGraph->setPhyBio(Settings::readValue("Biorhythm", "phyBio", true).toBool());
    setPhyColor(Settings::readValue("Biorhythm", "phyBioColor", QColor(0, 255, 0)).value<QColor>());
    ui.widgetBiorhythmGraph->setEmoBio(Settings::readValue("Biorhythm", "emoBio", true).toBool());
    setEmoColor(Settings::readValue("Biorhythm", "emoBioColor", QColor(255, 0, 0)).value<QColor>());
    ui.widgetBiorhythmGraph->setIntBio(Settings::readValue("Biorhythm", "intBio", true).toBool());
    setIntColor(Settings::readValue("Biorhythm", "intBioColor", QColor(0, 0, 255)).value<QColor>());
    ui.widgetBiorhythmGraph->setIntuBio(Settings::readValue("Biorhythm", "intuBio", true).toBool());
    setIntuColor(Settings::readValue("Biorhythm", "intuBioColor", QColor(255, 0, 255)).value<QColor>());
    ui.widgetBiorhythmGraph->setAverageBio(Settings::readValue("Biorhythm", "averageBio", true).toBool());
    setAvgColor(Settings::readValue("Biorhythm", "averageBioColor", QColor(255, 255, 0)).value<QColor>());

    ui.widgetBiorhythmGraph->setToday(Settings::readValue("Biorhythm", "showToday", true).toBool());
    ui.widgetBiorhythmGraph->setVLines(Settings::readValue("Biorhythm", "vLines", true).toBool());
    ui.widgetBiorhythmGraph->showAxisVLabels(Settings::readValue("Biorhythm", "vLabels", true).toBool());
    ui.widgetBiorhythmGraph->setHLines(Settings::readValue("Biorhythm", "hLines", true).toBool());
    ui.widgetBiorhythmGraph->showAxisHLabels(Settings::readValue("Biorhythm", "hLabels", true).toBool());
    ui.widgetBiorhythmGraph->setZero(Settings::readValue("Biorhythm", "zeroLine", true).toBool());
}


// ****************************************************************************
// ToolButton Menu
void MainWindow::createToolButtonMenu()
{
    // Pełny ekran
    // Minimalizuj do ikonki
    // ---
    // Zapisz zrzut aplikacji...
    // ---
    // Pokaż pasek menu
    // Pokaż przycisk menu
    // Pokaż pasek stanu
    // ---
    // Opcje...
    // ---
    // Pomoc
    // O programie
    // ---
    // Zakończ

    toolButtonMenu = new QMenu("Menu", ui.toolButtonMenu);
    //toolButtonMenu->setToolTipsVisible(true);

    QAction *fullScreenA = new QAction("Pełny ekran", toolButtonMenu);
    //fullScreenA->setShortcut(Qt::Key_F11);
    fullScreenA->setCheckable(true);
    fullScreenA->setChecked(windowState() & Qt::WindowFullScreen);
    connect(fullScreenA, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));
    toolButtonMenu->addAction(fullScreenA);

    toolButtonMenu->addAction("Minimalizuj do ikonki", this, SLOT(minimizeToTray()));
    toolButtonMenu->addSeparator();

    QAction *screenShootA = new QAction("Zapisz zrzut...", toolButtonMenu);
    screenShootA->setShortcut(Qt::CTRL + Qt::Key_S);
    screenShootA->setEnabled(currentUserUID != 0);
    connect(screenShootA, SIGNAL(triggered(bool)), this, SLOT(saveScreenshot()));
    toolButtonMenu->addAction(screenShootA);
    toolButtonMenu->addSeparator();

    QAction *menuA = new QAction("Pokaż pasek menu", toolButtonMenu);
    menuA->setCheckable(true);
    menuA->setChecked(ui.menubar->isVisible());
    connect(menuA, SIGNAL(triggered(bool)), this, SLOT(showMenuBar()));
    toolButtonMenu->addAction(menuA);

    QAction *toolButtonA = new QAction("Pokaż przycisk menu", toolButtonMenu);
    toolButtonA->setCheckable(true);
    toolButtonA->setChecked(ui.toolButtonMenu->isVisible());
    connect(toolButtonA, SIGNAL(triggered(bool)), this, SLOT(showToolButtonMenu()));
    toolButtonMenu->addAction(toolButtonA);

    QAction *statusBarA = new QAction("Pokaż pasek stanu", toolButtonMenu);
    statusBarA->setCheckable(true);
    statusBarA->setChecked(ui.statusbar->isVisible());
    connect(statusBarA, SIGNAL(triggered(bool)), ui.statusbar, SLOT(setVisible(bool)));
    toolButtonMenu->addAction(statusBarA);
    toolButtonMenu->addSeparator();

    toolButtonMenu->addAction("Opcje...", this, SLOT(showOptionsDialog()), (Qt::CTRL + Qt::Key_O));
    toolButtonMenu->addSeparator();

    toolButtonMenu->addAction("Pomoc", this, SLOT(showHelpDialog()), (Qt::Key_F1));
    toolButtonMenu->addAction("O programie", this, SLOT(showAboutDialog()), (Qt::CTRL + Qt::Key_F1));
    toolButtonMenu->addSeparator();

    toolButtonMenu->addAction("Zakończ", this, SLOT(close()), (Qt::CTRL + Qt::Key_Q));

    ui.toolButtonMenu->setMenu(toolButtonMenu);
    connect(ui.toolButtonMenu->menu(), SIGNAL(aboutToShow()), this, SLOT(updateToolButtonMenuActions()));
}

void MainWindow::updateToolButtonMenuActions()
{
    // 00 Pełny ekran
    // 01 Minimalizuj do ikonki
    // 02 ---
    // 03 Zapisz zrzut aplikacji...
    // 04 ---
    // 05 Pokaż pasek menu
    // 06 Pokaż przycisk menu
    // 07 Pokaż pasek stanu
    // 08 ---
    // 09 Opcje...
    // 10 ---
    // 11 Pomoc
    // 12 O programie
    // 13 ---
    // 14 Zakończ

    // fullscreen
    ui.toolButtonMenu->menu()->actions().at(0)->setChecked(windowState() & Qt::WindowFullScreen);
    // minimize to tray icon
    ui.toolButtonMenu->menu()->actions().at(1)->setEnabled(trayIconAvailable);
    // save screenshot
    ui.toolButtonMenu->menu()->actions().at(3)->setEnabled(ui.widgetBiorhythmGraph->isEnabled());
    // menu bar
    ui.toolButtonMenu->menu()->actions().at(5)->setChecked(ui.menubar->isVisible());
    // tool button menu
    ui.toolButtonMenu->menu()->actions().at(6)->setChecked(ui.toolButtonMenu->isVisible());
    // show status bar
    ui.toolButtonMenu->menu()->actions().at(7)->setChecked(ui.statusbar->isVisible());
}

void MainWindow::showToolButtonMenu()
{
    if (ui.menubar->isVisible())
        ui.toolButtonMenu->setVisible(!ui.toolButtonMenu->isVisible());
}


// ****************************************************************************
// create all necessary connections
void MainWindow::createConnections()
{
    // widget Biorhythm Graph
    connect(ui.widgetBiorhythmGraph, SIGNAL(signalBioValues(double,bool,double,bool,double,bool,double,bool,double,bool)), this, SLOT(showBioValues(double,bool,double,bool,double,bool,double,bool,double,bool)));
    connect(ui.widgetBiorhythmGraph, SIGNAL(signalDate(QDate)), this, SLOT(showDate(QDate)));
    connect(ui.widgetBiorhythmGraph, SIGNAL(signalHour(int)), this, SLOT(showHour(int)));
    connect(ui.widgetBiorhythmGraph, SIGNAL(signalUserDays(quint64)), this, SLOT(showUserDays(quint64)));
    connect(ui.widgetBiorhythmGraph, SIGNAL(signalViewChanged(int)), this, SLOT(bioGraphViewChanged(int)));

    connect(ui.dateEdit, SIGNAL(dateChanged(QDate)), ui.widgetBiorhythmGraph, SLOT(setNewDateToShow(QDate)));

    connect(ui.toolButtonLeft, SIGNAL(clicked()), ui.widgetBiorhythmGraph, SLOT(back()));
    connect(ui.toolButtonRight, SIGNAL(clicked()), ui.widgetBiorhythmGraph, SLOT(forward()));
    connect(ui.toolButtonLeftDay, SIGNAL(clicked()), ui.widgetBiorhythmGraph, SLOT(backDay()));
    connect(ui.toolButtonRightDay, SIGNAL(clicked()), ui.widgetBiorhythmGraph, SLOT(forwardDay()));

    connect(ui.pushButtonToday, SIGNAL(clicked()), ui.widgetBiorhythmGraph, SLOT(showToday()));

    // context menu
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onDialogContextMenu(QPoint)));

    // user context menu
    connect(ui.labelUserPicture, SIGNAL(myContextMenuRequested(QPoint)), this, SLOT(onUsersContextMenu(QPoint)));
    connect(ui.labelUserName, SIGNAL(myContextMenuRequested(QPoint)), this, SLOT(onUsersContextMenu(QPoint)));
    connect(ui.labelUserDate, SIGNAL(myContextMenuRequested(QPoint)), this, SLOT(onUsersContextMenu(QPoint)));
    connect(ui.labelUser, SIGNAL(myContextMenuRequested(QPoint)), this, SLOT(onUsersContextMenu(QPoint)));

    // user double clicked
    connect(ui.labelUserPicture, SIGNAL(mouseDoubleClicked()), this, SLOT(showUsersManagerDlg()));
    connect(ui.labelUserName, SIGNAL(mouseDoubleClicked()), this, SLOT(showUsersManagerDlg()));
    connect(ui.labelUserDate, SIGNAL(mouseDoubleClicked()), this, SLOT(showUsersManagerDlg()));
    connect(ui.labelUser, SIGNAL(mouseDoubleClicked()), this, SLOT(showUsersManagerDlg()));

    // show user menu
    connect(ui.labelUserPicture, SIGNAL(mouseClicked(QPoint)), this, SLOT(onUsersMenu(QPoint)));
    connect(ui.labelUserName, SIGNAL(mouseClicked(QPoint)), this, SLOT(onUsersMenu(QPoint)));
    connect(ui.labelUserDate, SIGNAL(mouseClicked(QPoint)), this, SLOT(onUsersMenu(QPoint)));
    connect(ui.labelUser, SIGNAL(mouseClicked(QPoint)), this, SLOT(onUsersMenu(QPoint)));
}


// ****************************************************************************
// o
void MainWindow::createOptionsDialog()
{
    // create "on-line" options window/dialog
    optionsDialog = new OptionsDialog(this);

    // options dialog to graph widget connections
    // biorhythm tab
    connect(optionsDialog, SIGNAL(phyBioChanged(bool)), ui.widgetBiorhythmGraph, SLOT(setPhyBio(bool)));
    connect(optionsDialog, SIGNAL(phyBioChanged(bool)), ui.framePhy1, SLOT(setVisible(bool)));
    connect(optionsDialog, SIGNAL(phyBioChanged(bool)), ui.framePhy2, SLOT(setVisible(bool)));
    connect(optionsDialog, SIGNAL(phyColorChanged(QColor)), this, SLOT(setPhyColor(QColor)));

    connect(optionsDialog, SIGNAL(emoBioChanged(bool)), ui.widgetBiorhythmGraph, SLOT(setEmoBio(bool)));
    connect(optionsDialog, SIGNAL(emoBioChanged(bool)), ui.frameEmo1, SLOT(setVisible(bool)));
    connect(optionsDialog, SIGNAL(emoBioChanged(bool)), ui.frameEmo2, SLOT(setVisible(bool)));
    connect(optionsDialog, SIGNAL(emoColorChanged(QColor)),this, SLOT(setEmoColor(QColor)));

    connect(optionsDialog, SIGNAL(intBioChanged(bool)), ui.widgetBiorhythmGraph, SLOT(setIntBio(bool)));
    connect(optionsDialog, SIGNAL(intBioChanged(bool)), ui.frameInt1, SLOT(setVisible(bool)));
    connect(optionsDialog, SIGNAL(intBioChanged(bool)), ui.frameInt2, SLOT(setVisible(bool)));
    connect(optionsDialog, SIGNAL(intColorChanged(QColor)), this, SLOT(setIntColor(QColor)));

    connect(optionsDialog, SIGNAL(intuBioChanged(bool)), ui.widgetBiorhythmGraph, SLOT(setIntuBio(bool)));
    connect(optionsDialog, SIGNAL(intuBioChanged(bool)), ui.frameIntu1, SLOT(setVisible(bool)));
    connect(optionsDialog, SIGNAL(intuBioChanged(bool)), ui.frameIntu2, SLOT(setVisible(bool)));
    connect(optionsDialog, SIGNAL(intuColorChanged(QColor)), this, SLOT(setIntuColor(QColor)));

    connect(optionsDialog, SIGNAL(averageBioChanged(bool)), ui.widgetBiorhythmGraph, SLOT(setAverageBio(bool)));
    connect(optionsDialog, SIGNAL(averageBioChanged(bool)), ui.frameAvg1, SLOT(setVisible(bool)));
    connect(optionsDialog, SIGNAL(averageBioChanged(bool)), ui.frameAvg2, SLOT(setVisible(bool)));
    connect(optionsDialog, SIGNAL(averageColorChanged(QColor)), this, SLOT(setAvgColor(QColor)));

    connect(optionsDialog, SIGNAL(selectedAverageChanged(bool)), ui.widgetBiorhythmGraph, SLOT(setSelectedAverageChanged(bool)));

    connect(optionsDialog, SIGNAL(bioDataNone(bool)), ui.widgetBioDataRight, SLOT(setHidden(bool)));
    connect(optionsDialog, SIGNAL(bioDataNone(bool)), ui.widgetBioDataBottom, SLOT(setHidden(bool)));
    connect(optionsDialog, SIGNAL(bioDataRight(bool)), ui.widgetBioDataRight, SLOT(setVisible(bool)));
    connect(optionsDialog, SIGNAL(bioDataRight(bool)), ui.widgetBioDataBottom, SLOT(setHidden(bool)));
    connect(optionsDialog, SIGNAL(bioDataBottom(bool)), ui.widgetBioDataRight, SLOT(setHidden(bool)));
    connect(optionsDialog, SIGNAL(bioDataBottom(bool)), ui.widgetBioDataBottom, SLOT(setVisible(bool)));

    // graph tab
    connect(optionsDialog, SIGNAL(todayChanged(bool)), ui.widgetBiorhythmGraph, SLOT(setToday(bool)));
    connect(optionsDialog, SIGNAL(vLinesChanged(bool)), ui.widgetBiorhythmGraph, SLOT(setVLines(bool)));
    connect(optionsDialog, SIGNAL(axisVLabelsChanged(bool)), ui.widgetBiorhythmGraph, SLOT(showAxisVLabels(bool)));
    connect(optionsDialog, SIGNAL(hLinesChanged(bool)), ui.widgetBiorhythmGraph, SLOT(setHLines(bool)));
    connect(optionsDialog, SIGNAL(axisHLabelsChanged(bool)), ui.widgetBiorhythmGraph, SLOT(showAxisHLabels(bool)));
    connect(optionsDialog, SIGNAL(axisHLinesDelta(QString)), ui.widgetBiorhythmGraph, SLOT(setHLinesDelta(QString)));
    connect(optionsDialog, SIGNAL(zeroChanged(bool)), ui.widgetBiorhythmGraph, SLOT(setZero(bool)));
    connect(optionsDialog, SIGNAL(antialiasingChanged(bool)), ui.widgetBiorhythmGraph, SLOT(setAntialiasing(bool)));
    connect(optionsDialog, SIGNAL(penWidthChanged(double)), ui.widgetBiorhythmGraph, SLOT(setPenWidth(double)));
    connect(optionsDialog, SIGNAL(graphBackgroundChanged(bool)), ui.widgetBiorhythmGraph, SLOT(setBackground(bool)));
    connect(optionsDialog, SIGNAL(graphBackgroundChanged(QColor)), ui.widgetBiorhythmGraph, SLOT(setBackground(QColor)));

    connect(optionsDialog, SIGNAL(mouseFollowChanged(bool)), ui.widgetBiorhythmGraph, SLOT(setMouseFollow(bool)));
    connect(optionsDialog, SIGNAL(toolTipChanged(bool)), ui.widgetBiorhythmGraph, SLOT(setShowToolTip(bool)));

    connect(optionsDialog, SIGNAL(mouseWheelChanged(bool)), ui.widgetBiorhythmGraph, SLOT(setMouseWheel(bool)));
    connect(optionsDialog, SIGNAL(reverseScrollChanged(bool)), ui.widgetBiorhythmGraph, SLOT(setReverseScroll(bool)));

    // application tab
    connect(optionsDialog, SIGNAL(showTrayIcon(bool)), trayIcon, SLOT(setVisible(bool)));
    connect(optionsDialog, SIGNAL(showToolButtonMenu(bool)), ui.toolButtonMenu, SLOT(setVisible(bool)));

    // TODO menedzer
    connect(optionsDialog, SIGNAL(rangeChanged(int)), ui.widgetBiorhythmGraph, SLOT(changeView(int)));

    connect(optionsDialog, SIGNAL(minimizeToTrayIconOnClose(bool)), this, SLOT(setMinimizeToTrayIconOnClose(bool)));

    //TODO wyglad
    connect(optionsDialog, SIGNAL(appBackgroundChanged(bool)), this, SLOT(setAppBackgroundColor(bool)));
    connect(optionsDialog, SIGNAL(appBackgroundChanged(QColor)), this, SLOT(setAppBackgroundColor(QColor)));
}


// ****************************************************************************
// update Biorhythm Data values
void MainWindow::showBioValues(double phyB, bool phyArrowUp, double emoB, bool emoArrowUp, double intB, bool intArrowUp, double intuB, bool intuArrowUp, double averageB, bool avgArrowUp)
{
    if (ui.widgetBioDataRight->isEnabled())// && ui.frameBioDataRight->isVisible())
    {
        QPixmap up(":/images/arrow-up.png");
        QPixmap down(":/images/arrow-down.png");

        ui.labelPhy2_2->setText(QLocale(QLocale::Polish).toString(phyB, 'f', 2));
        ui.labelPhy2_Arrow->setPixmap(phyArrowUp ? up : down);
        ui.labelEmo2_2->setText(QLocale(QLocale::Polish).toString(emoB, 'f', 2));
        ui.labelEmo2_Arrow->setPixmap(emoArrowUp ? up : down);
        ui.labelInt2_2->setText(QLocale(QLocale::Polish).toString(intB, 'f', 2));
        ui.labelInt2_Arrow->setPixmap(intArrowUp ? up : down);
        ui.labelIntu2_2->setText(QLocale(QLocale::Polish).toString(intuB, 'f', 2));
        ui.labelIntu2_Arrow->setPixmap(intuArrowUp ? up : down);
        ui.labelAvg2_2->setText(QLocale(QLocale::Polish).toString(averageB, 'f', 2));
        ui.labelAvg2_Arrow->setPixmap(avgArrowUp ? up : down);

        //return;
    }
    if (ui.widgetBioDataBottom->isEnabled())// && ui.widgetBioDataBottom->isVisible())
    {
        QPixmap up(":/images/arrow-up.png");
        QPixmap down(":/images/arrow-down.png");

        ui.labelPhy1_2->setText(QLocale(QLocale::Polish).toString(phyB, 'f', 2));
        ui.labelPhy1_Arrow->setPixmap(phyArrowUp ? up : down);
        ui.labelEmo1_2->setText(QLocale(QLocale::Polish).toString(emoB, 'f', 2));
        ui.labelEmo1_Arrow->setPixmap(emoArrowUp ? up : down);
        ui.labelInt1_2->setText(QLocale(QLocale::Polish).toString(intB, 'f', 2));
        ui.labelInt1_Arrow->setPixmap(intArrowUp ? up : down);
        ui.labelIntu1_2->setText(QLocale(QLocale::Polish).toString(intuB, 'f', 2));
        ui.labelIntu1_Arrow->setPixmap(intuArrowUp ? up : down);
        ui.labelAvg1_2->setText(QLocale(QLocale::Polish).toString(averageB, 'f', 2));
        ui.labelAvg1_Arrow->setPixmap(avgArrowUp ? up : down);

        //return;
    }
}


// ****************************************************************************
// update Current Date/Hour and user days
void MainWindow::showDate(const QDate &date)
{
    Q_ASSERT(date.isValid());

    if (!date.isValid())
        return;

    // enable/disable push button TODAY
    ui.pushButtonToday->setEnabled(date != QDate::currentDate());

    // BOLD day of week name if date == today date
    QFont font = ui.labelDay->font();
    font.setBold(date == QDate::currentDate());
    ui.labelDay->setFont(font);
    ui.labelDay->setText(constants::daysList[date.dayOfWeek()-1]);

    // change date in DATE EDIT
    disconnect(ui.dateEdit, SIGNAL(dateChanged(QDate)), ui.widgetBiorhythmGraph, SLOT(setNewDateToShow(QDate)));
    ui.dateEdit->setDate(date);
    connect(ui.dateEdit, SIGNAL(dateChanged(QDate)), ui.widgetBiorhythmGraph, SLOT(setNewDateToShow(QDate)));

    // calculate and show the age of a user in years, months, days
    int y, m, d;

    int t0 = currentUserBirthDate.year()*12 + currentUserBirthDate.month() - 1;  // total months for birthdate
    int t1 = date.year()*12 + date.month() - 1;                                  // total months for now
    int dm = t1 - t0;                                                            // delta months

    if(date.day() >= currentUserBirthDate.day())
    {
        y = qFloor(dm/12);
        m = dm%12;
        d = date.day() - currentUserBirthDate.day();
    }
    else
    {
        dm--;

        y = qFloor(dm/12);
        m = dm%12;
        d = currentUserBirthDate.day() > date.addMonths(-1).daysInMonth() ? date.day() : date.addMonths(-1).daysInMonth() - currentUserBirthDate.day() + date.day();

        // v2 ok
        //t1--;
        //int day = currentUserBirthDate.day() > QDate(qFloor(t1/12), (t1%12)+1, 1).daysInMonth() ? QDate(qFloor(t1/12), (t1%12)+1, 1).daysInMonth() : currentUserBirthDate.day();
        //d = QDate(qFloor(t1/12), (t1%12)+1, day).daysTo(date);
    }

    ui.labelUserYears->setText(QString::number(y));
    ui.labelUserMonths->setText(QString::number(m));
    ui.labelUserDays->setText(QString::number(d));
}

void MainWindow::showHour(const int hour)
{
    if (hour > 0)
    {
        ui.labelHour->setText(QString("%1:00").arg(hour, 2, 10, QChar('0')));
        ui.labelClock->show();
        ui.labelHour->show();
    }
    else
    {
        ui.labelClock->hide();
        ui.labelHour->hide();
    }
}

void MainWindow::showUserDays(const quint64 userDays)
{
    ui.labelUserDay->setText(QString::number(userDays));
}


// ****************************************************************************
// set Biorhythm Data Colors
void MainWindow::setPhyColor(QColor color)
{
    ui.labelPhy1_1->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelPhy1_2->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelPhy1_3->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelPhy2_1->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelPhy2_2->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelPhy2_3->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.widgetBiorhythmGraph->setPhyColor(color);
}

void MainWindow::setEmoColor(QColor color)
{
    ui.labelEmo1_1->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelEmo1_2->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelEmo1_3->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelEmo2_1->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelEmo2_2->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelEmo2_3->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.widgetBiorhythmGraph->setEmoColor(color);
}

void MainWindow::setIntColor(QColor color)
{
    ui.labelInt1_1->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelInt1_2->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelInt1_3->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelInt2_1->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelInt2_2->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelInt2_3->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.widgetBiorhythmGraph->setIntColor(color);
}

void MainWindow::setIntuColor(QColor color)
{
    ui.labelIntu1_1->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelIntu1_2->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelIntu1_3->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelIntu2_1->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelIntu2_2->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelIntu2_3->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.widgetBiorhythmGraph->setIntuColor(color);
}

void MainWindow::setAvgColor(QColor color)
{
    ui.labelAvg1_1->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelAvg1_2->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelAvg1_3->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelAvg2_1->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelAvg2_2->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.labelAvg2_3->setStyleSheet(QString("background-color: %1").arg(color.name()));
    ui.widgetBiorhythmGraph->setAverageColor(color);
}


// ****************************************************************************
// enable application "background color"
void MainWindow::setAppBackgroundColor(bool enable)
{
    QPalette p = palette();
    p.setColor(QPalette::Window, enable ? appBackgroundColor : QApplication::palette().background().color());
    setPalette(p);

    //ui.pushButtonToday->setStyleSheet(QString("background-color:" + (enable ? appBackgroundColor.name() : QApplication::palette().background().color().name())));
}


// ****************************************************************************
// change application "background color"
void MainWindow::setAppBackgroundColor(const QColor &color)
{
    appBackgroundColor = color;

    QPalette p = palette();
    p.setColor(QPalette::Window, color);
    setPalette(p);

    //ui.pushButtonToday->setStyleSheet(QString("background-color:" + color.name()));
}


// ****************************************************************************
// users menu
void MainWindow::onUsersMenu(const QPoint &pos)
{
    // if no users
    if (usersList->isEmpty())
    {
        QMenu menu("Menu", this);
        menu.addAction("Dodaj użytkownika", this, SLOT(addNewUser()), (Qt::CTRL + Qt::Key_D));
        menu.exec(pos);

        return;
    }

    QMenu menu("Menu", this);
    menu.setToolTipsVisible(true);

    QActionGroup *group = new QActionGroup(&menu);
    group->setExclusive(true);

    signalMapper = new QSignalMapper(this);
    QPixmap overlay(":/images/default.png"); // :/images/default.png = 16x16

    for (int i = 0; i < usersList->count(); ++i)
    {
        QPixmap base(usersList->at(i).avatar),
                result(48, 48); //base.width(), base.height());

        /* Returns true if this is a null pixmap; otherwise returns false.
         * A null pixmap has zero width, zero height and no contents. You cannot draw in a null pixmap.
         */
        if (base.isNull())
            base.load(constants::userAvatar);

        result.fill(Qt::transparent); // force alpha channel

        QPainter painter(&result);
        painter.drawPixmap(0, 0, base.scaled(48, 48));
        if (usersList->at(i).defaultUser)
            painter.drawPixmap(result.height()-32, result.width()-32, overlay.scaled(32, 32));

        QAction *a = new QAction(QIcon(result), QString(usersList->at(i).name), group);
        a->setToolTip("Data urodzenia: " + usersList->at(i).dateOfBirth.toString("dd.MM.yyyy"));
        a->setCheckable(true);
        a->setChecked(usersList->at(i).uid == currentUserUID);

        connect(a, SIGNAL(triggered()), signalMapper, SLOT(map()));
        signalMapper->setMapping(a, usersList->at(i).uid);

        menu.addAction(a);
    }

    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(newUserChoosen(int)));

    menu.exec(pos);

    signalMapper->disconnect();
    delete signalMapper;

}

void MainWindow::addNewUser()
{
    showUsersManagerDlg(3);
}


// ****************************************************************************
// context menus
void MainWindow::onUsersContextMenu(const QPoint &pos)
{
    QMenu menu("Menu kontekstowe", this);
    menu.setToolTipsVisible(true);

    menu.addAction("Menedżer", this, SLOT(showUsersManagerDlg()), (Qt::CTRL + Qt::Key_M));
    if (currentUserUID != 0)
        menu.addAction("Edytuj", this, SLOT(showEditUserDlg()), (Qt::CTRL + Qt::Key_E));
    menu.addAction("Dodaj użytkownika", this, SLOT(addNewUser()), (Qt::CTRL + Qt::Key_D));
    menu.addSeparator();

    menu.exec(pos);
}

void MainWindow::onDialogContextMenu(const QPoint &pos)
{
    QMenu menu("Menu kontekstowe", this);

    QAction *fullScreenAction = new QAction("Pełny ekran", &menu);
    fullScreenAction->setShortcut(Qt::Key_F11);
    fullScreenAction->setCheckable(true);
    fullScreenAction->setChecked(windowState() & Qt::WindowFullScreen);
    connect(fullScreenAction, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));
    menu.addAction(fullScreenAction);
    menu.addSeparator();

    menu.addAction("Opcje...", this, SLOT(showOptionsDialog()), (Qt::CTRL + Qt::Key_O));
    menu.addSeparator();

    menu.addAction("Pomoc", this, SLOT(showHelpDialog()), (Qt::Key_F1));
    menu.addAction("O programie", this, SLOT(showAboutDialog()), (Qt::CTRL + Qt::Key_F1));
    menu.addSeparator();

    menu.addAction("Zakończ", this, SLOT(close()), (Qt::CTRL + Qt::Key_Q));

    menu.exec(mapToGlobal(pos));
}


// ****************************************************************************
// new User choosen
void MainWindow::newUserChoosen(const int userUID)
{
    qDebug() << "newUserChoosen USER UID: " << userUID;
    for (int i = 0; i < usersList->count(); ++i)
    {
        // if current user is on users list
        if (usersList->at(i).uid == static_cast<quint16>(userUID))
        {
            if (usersList->at(i).dateOfBirth > QDate::currentDate())
            {
                QMessageBox::warning(this,
                                     "Zła data",
                                     QString("Data urodzenia (%1 r.) jest nieprawidłowa.\n"
                                             "Dziś jest: %2 r.\n\n"
                                             "Sprawdź poprawność daty w systemie lub zmień datę urodzenia na prawidłową.")
                                     .arg(usersList->at(i).dateOfBirth.toString("dd.MM.yyyy"))
                                     .arg(QDate::currentDate().toString("dd.MM.yyyy")));

                return;
            }

            qDebug() << "newUserChoosen staticv cast" << static_cast<quint16>(userUID) << "newUserChoosen  UID: " << userUID;
            currentUserUID = static_cast<quint16>(userUID);

            QPixmap base(usersList->at(i).avatar),
                    result(48, 48); //base.width(), base.height());

            /* Returns true if this is a null pixmap; otherwise returns false.
             * A null pixmap has zero width, zero height and no contents. You cannot draw in a null pixmap.
             */
            if (base.isNull())
                base.load(constants::userAvatar);

            result.fill(Qt::transparent); // force alpha channel

            QPainter painter(&result);
            painter.drawPixmap(0, 0, base.scaled(48, 48));
            if (usersList->at(i).defaultUser)
                painter.drawPixmap(32, 32, QPixmap(":/images/default.png")); // :/images/default.png = 16x16
            ui.labelUserPicture->setPixmap(result);

            ui.labelUserName->setText(usersList->at(i).name);
            ui.labelUserDate->setText(usersList->at(i).dateOfBirth.toString("dd.MM.yyyy"));
            setWindowTitle("Biorytm - " + ui.labelUserName->text() + " - " + ui.labelUserDate->text());

            ui.dateEdit->setMinimumDate(usersList->at(i).dateOfBirth);

            // if user has a birthday show birthday picture
            currentUserBirthDate = usersList->at(i).dateOfBirth;
            if (currentUserBirthDate.month() == QDate::currentDate().month() && currentUserBirthDate.day() == QDate::currentDate().day())
                ui.labelBirthday->show();
            else
                ui.labelBirthday->hide();

            if (trayIconAvailable)
                trayIcon->setToolTip("Biorytm\n" + usersList->at(i).name + "\n" + usersList->at(i).dateOfBirth.toString("dd.MM.yyyy"));

            enableBioGraph();

            ui.widgetBiorhythmGraph->newDateOfBirth(currentUserBirthDate);

            return;
         }
    }

    // if current user not found in users list

    currentUserUID = 0;
    ui.labelUserPicture->setPixmap(QPixmap(constants::noUserAvatar));
    ui.labelUserName->setText("");
    ui.labelUserDate->setText("");
    setWindowTitle("Biorytm");
    currentUserBirthDate = QDate::currentDate();

    ui.labelBirthday->hide();

    if (trayIconAvailable)
        trayIcon->setToolTip("Biorytm");

    disableBioGraph();
}


// ****************************************************************************
// users Manager and Edit Dialog
void MainWindow::showUsersManagerDlg()
{
    // mode == 2 not visible on taskbar

    showUsersManagerDlg(2);
}

void MainWindow::showUsersManagerDlg(quint8 mode)
{
    // mode == 1 visible on taskbar, necessary at application startup
    // mode == 2 not visible on taskbar
    // mode == 3 users menager in edit mode (add new user)
    // or silent mode, at application startup

    UsersManagerDialog *dlg;

    qDebug() << "showUsersManagerDlg(quint8 mode) mode: " << mode << "currentUserUID" << currentUserUID;

    // mode == 1 visible on taskbar, necessary at application startup
    if (mode == 1)
    {
        dlg = new UsersManagerDialog(usersList, currentUserUID, 0);
        dlg->setWindowTitle(QString("%1 - Menedżer użytkowników").arg(QCoreApplication::applicationName()));
        dlg->exec();
    }
    // mode == 2 not visible on taskbar
    else if (mode == 2)
    {
        dlg = new UsersManagerDialog(usersList, currentUserUID, this);
        dlg->setWindowTitle("Menedżer użytkowników");
        dlg->exec();
    }
    // mode == 3 users menager in edit mode (add new user)
    else if (mode == 3)
    {
        dlg = new UsersManagerDialog(usersList, currentUserUID, this);
        dlg->setWindowTitle("Menedżer użytkowników");
        dlg->addUserMode();
        dlg->exec();
    }
    else
    // silent mode, at application startup
    {
        dlg = new UsersManagerDialog(usersList, currentUserUID, this);

    }

    quint16 tmp = dlg->getUserUID();

    delete dlg;

    newUserChoosen(tmp);

    // save users in to settings file
    Settings::writeUsers(usersList);
}

void MainWindow::showEditUserDlg()
{
    if (currentUserUID == 0)
        return;

    int i;

    for (i = 0; i < usersList->count(); ++i)
        if (usersList->at(i).uid == currentUserUID)
            break;

    // create dialog window for user's edit widget
    QDialog dlg(this);
    dlg.setWindowTitle("Edycja danych");
    dlg.setWindowIcon(QIcon(":/images/pencil.png"));
    UserEditWidget w(&dlg);
    connect(&w, SIGNAL(signalCancel()), &dlg, SLOT(reject()));
    connect(&w, SIGNAL(signalSave()), &dlg, SLOT(accept()));
    QGridLayout  *layout = new QGridLayout;
    layout->setContentsMargins(9, 9, 9, 0);
    layout->addWidget(&w);
    dlg.setLayout(layout);

    w.setData(usersList->at(i).name, usersList->at(i).dateOfBirth, usersList->at(i).avatar, usersList->at(i).defaultUser, currentUserUID);

    if (dlg.exec())
    {
        User user;
        user.name = w.getName();
        user.dateOfBirth = w.getDate();
        user.avatar = w.getAvatar();
        user.defaultUser = w.getDefaultUser();
        user.uid = currentUserUID;

        usersList->replace(i, user);

        newUserChoosen(currentUserUID);

        // save users in to settings file
        Settings::writeUsers(usersList);
    }    
}


// ****************************************************************************
// ON/OFF biorhythm GRAPH
void MainWindow::disableBioGraph()
{
    // biorhythm graph
    ui.widgetBiorhythmGraph->setDisabled(true);
    ui.widgetBiorhythmGraph->hide();

    showBioValues(static_cast<qreal>(0), false, static_cast<qreal>(0), false, static_cast<qreal>(0), false, static_cast<qreal>(0), false, static_cast<qreal>(0), false);

    ui.widgetBioDataBottom->setDisabled(true);
    ui.widgetBioDataRight->setDisabled(true);

    ui.toolButtonLeft->setDisabled(true);
    ui.toolButtonLeftDay->setDisabled(true);
    ui.toolButtonRight->setDisabled(true);
    ui.toolButtonRightDay->setDisabled(true);

    ui.label_1->setDisabled(true);
    ui.label_2->setDisabled(true);
    ui.label_3->setDisabled(true);
    ui.label_4->setDisabled(true);
    ui.label_5->setDisabled(true);
    ui.labelUserYears->setDisabled(true);
    ui.labelUserMonths->setDisabled(true);
    ui.labelUserDays->setDisabled(true);
    ui.labelUserDay->setDisabled(true);

    ui.labelUserYears->setText("0");
    ui.labelUserMonths->setText("0");
    ui.labelUserDays->setText("0");
    ui.labelUserDay->setText("0");
    ui.dateEdit->setDate(QDate::currentDate());
    ui.dateEdit->setDisabled(true);
    ui.pushButtonToday->setDisabled(true);
    ui.labelClock->setDisabled(true);
    ui.labelHour->setText("12:00");
    ui.labelHour->setDisabled(true);
    ui.labelDay->setDisabled(true);
}

void MainWindow::enableBioGraph()
{
    // biorhythm graph
    ui.widgetBiorhythmGraph->setEnabled(true);
    ui.widgetBiorhythmGraph->show();

    ui.widgetBioDataBottom->setEnabled(true);
    ui.widgetBioDataRight->setEnabled(true);

    ui.toolButtonLeft->setEnabled(true);
    ui.toolButtonLeftDay->setEnabled(true);
    ui.toolButtonRight->setEnabled(true);
    ui.toolButtonRightDay->setEnabled(true);

    ui.label_1->setEnabled(true);
    ui.label_2->setEnabled(true);
    ui.label_3->setEnabled(true);
    ui.label_4->setEnabled(true);
    ui.label_5->setEnabled(true);
    ui.labelUserYears->setEnabled(true);
    ui.labelUserMonths->setEnabled(true);
    ui.labelUserDays->setEnabled(true);
    ui.labelUserDay->setEnabled(true);

    ui.dateEdit->setEnabled(true);
    ui.labelDay->setEnabled(true);

    ui.labelClock->setEnabled(true);
    ui.labelHour->setEnabled(true);
}


// ****************************************************************************
// tray icon
void MainWindow::createTrayIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        //QMessageBox::information(0, "Biorytm - informacja", "No system tray detected on this system.\n"
        //                                            "The tray icon option will be disabled.");

        //ui.actionTrayIcon->setDisabled(true);
        //ui.actionMinimizeToTray->setDisabled(true);

        trayIconAvailable = false;
    }
    else
    {
        trayIcon = new QSystemTrayIcon(this);

        // On Windows, the system tray icon size is 16x16; on X11, the preferred size is 22x22.
        // The icon will be scaled to the appropriate size as necessary.
        trayIcon->setIcon(QIcon(":/images/biorhythm_16.png"));
        trayIcon->setToolTip("Biorytm");

        // Tray Icon Context Menu
        // Minimalizuj do ikonki OR Przywróć aplikację
        // ---
        // Wyjdź
        trayIconMenu = new QMenu(this);
        trayIconMenu->addAction("Minimalizuj do ikonki", this, SLOT(minimizeToTray()));
        trayIconMenu->addAction("Przywróć aplikację", this, SLOT(messageClicked()));
        trayIconMenu->addSeparator();
        trayIconMenu->addAction("Wyjdź", this, SLOT(close()));

        connect(trayIconMenu, SIGNAL(aboutToShow()), this, SLOT(updateTrayIconContextMenuActions()));

        trayIcon->setContextMenu(trayIconMenu);

        connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
        connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));

        trayIconAvailable = true;
    }
}

void MainWindow::updateTrayIconContextMenuActions()
{
    // tray icon context menu
    // Minimalizuj do ikonki
    trayIconMenu->actions().at(0)->setVisible(isVisible());
    // Przywróć aplikację
    trayIconMenu->actions().at(1)->setVisible(!isVisible());
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::Trigger: messageClicked(); break;
        case QSystemTrayIcon::DoubleClick: break;
        case QSystemTrayIcon::MiddleClick: break;
        case QSystemTrayIcon::Context: break;
        default: break;
    }
}

void MainWindow::messageClicked()
{
    if (!isVisible())
    {
        show();
        raise();
    }
    else
    {
        hide();
    }
}

void MainWindow::minimizeToTray()
{
    // first show tray icon
    if (trayIconAvailable && !trayIcon->isVisible())
        trayIcon->setVisible(true);

    //
    if (trayIcon->isVisible())
    {
        hide();
        //trayIcon->showMessage("Biorytm", "Kliknij, aby powrócić do aplikacji", QSystemTrayIcon::Information, 3000);
    }

    // QSystemTrayIcon::NoIcon       0   No icon is shown.
    // QSystemTrayIcon::Information  1   An information icon is shown.
    // QSystemTrayIcon::Warning      2   A standard warning icon is shown.
    // QSystemTrayIcon::Critical     3   A critical warning icon is shown.
}


// ****************************************************************************
// GRAPH VIEW CHANGED
void MainWindow::bioGraphViewChanged(int newView)
{
    switch (newView)
    {
        case MONTH: ui.toolButtonLeft->setToolTip("Wstecz o miesiąc");
                    ui.toolButtonRight->setToolTip("Naprzód o miesiąc");
                    infoLabel->setText("Miesiąc");
                    break;

        case WEEK: ui.toolButtonLeft->setToolTip("Wstecz o tydzień");
                   ui.toolButtonRight->setToolTip("Naprzód o tydzień");
                   infoLabel->setText("Tydzień");
                   break;

        case DAY: ui.toolButtonLeft->setToolTip("Wstecz o dzień");
                  ui.toolButtonRight->setToolTip("Naprzód o dzień");
                  infoLabel->setText("Dzień");
                  break;

        default: break;
    }
}


// ****************************************************************************
// screenshot
void MainWindow::saveScreenshot()
{
    // saveScreenshotOption
    // 0 == ask user
    // 1 == app
    // 2 == graph

    int msgBoxReply = 0;

    if (saveScreenshotOption == 0)
    {
        QMessageBox msgBox(this);
        QPushButton *graphButton = msgBox.addButton("Wykres", QMessageBox::ActionRole);
        QPushButton *appButton = msgBox.addButton("Aplikacja", QMessageBox::ActionRole);
        QPushButton *abortButton = msgBox.addButton("Anuluj", QMessageBox::RejectRole);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setText("Zapisz zrzut wykresu lub całej aplikacji.");
        msgBox.setInformativeText("Proszę wybrać wersję:");
        msgBox.setDefaultButton(graphButton);

        msgBox.exec();

        if (msgBox.clickedButton() == abortButton)
            return;
        else if (msgBox.clickedButton() == appButton)
            msgBoxReply = 1;
        else if (msgBox.clickedButton() == graphButton)
            msgBoxReply = 2;
    }

    QString initialPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (initialPath.isEmpty())
        initialPath = QDir::currentPath();

    QString fileName = QDateTime::currentDateTime().toString("yyyy-MM-dd") + " - " + ui.labelUserName->text();
    QString types = ("Joint Photographic Experts Group (*.jpeg *.jpg);;"
                     "Portable Network Graphics (*.png);;"
                     "Scalable Vector Graphics (*.svg);;"
                     "Tagged Image File Format (*.tiff *.tif);;"
                     "Windows Bitmap (*.bmp);;");
    QString filter = "Portable Network Graphics (*.png)";

    QString newFileName = QFileDialog::getSaveFileName(this, "Zapisz zrzut jako...", initialPath + "/" + fileName, types, &filter);

    if (newFileName.isEmpty())
        return;

    QFileInfo fi(newFileName);

    // saveScreenshotOption == 0 ask user && msgBoxReply == 2 graphButton || saveScreenshotOption == 2 graph screenshot
    if ((saveScreenshotOption == 0 && msgBoxReply == 2) || saveScreenshotOption == 2)
    {
        // save ONLY biorhythm graph

        if (fi.suffix().contains("svg", Qt::CaseInsensitive))
        {
            QPainter painter(this);
            QSvgGenerator generator;
            generator.setFileName(newFileName);
            generator.setSize(ui.widgetBiorhythmGraph->size());
            generator.setViewBox(QRect(0, 0, ui.widgetBiorhythmGraph->width(), ui.widgetBiorhythmGraph->height()));
            generator.setTitle(fileName);
            //generator.setDescription("");
            painter.begin(&generator);
            ui.widgetBiorhythmGraph->paint(painter);
            painter.end();
        }
        else
        {
            QImage image(ui.widgetBiorhythmGraph->size(), QImage::Format_RGB32);
            //image.invertPixels(QImage::InvertRgb);
            QPainter painter(&image);
            //painter.begin(&image);
            ui.widgetBiorhythmGraph->paint(painter);
            //painter.end();

            if (!image.save(newFileName, fi.suffix().toUtf8().constData(), -1))
            {
                QMessageBox::critical(this, QApplication::applicationName(), QString("Nie można zapisać pliku: %1").arg(newFileName));
            }
        }
    }
    // saveScreenshotOption == 0 ask user && msgBoxReply == 1 appButton || saveScreenshotOption == 1 app screenshot
    else if ((saveScreenshotOption == 0 && msgBoxReply == 1) || saveScreenshotOption == 1)
    {
        // save application screenshot

        if (fi.suffix().contains("svg", Qt::CaseInsensitive))
        {
            QSvgGenerator generator;
            generator.setFileName(newFileName);
            generator.setSize(size());
            generator.setViewBox(rect());
            generator.setTitle(fileName);
            //generator.setDescription("");
            render(&generator, QPoint(), QRegion(rect()));
        }
        else
        {
            QPixmap pixmap(rect().size());
            render(&pixmap, QPoint(), QRegion(rect()));

            if (!pixmap.save(newFileName, fi.suffix().toUtf8().constData(), -1))
                QMessageBox::warning(this, "Błąd zapisu", QString("Nie można zapisać pliku: \"%1\"").arg(QDir::toNativeSeparators(newFileName)));
        }
    }
}



// ****************************************************************************
// slot minimize to tray icon on close for settings dialog
void MainWindow::setMinimizeToTrayIconOnClose(bool state)
{
    minimizeToTrayIconOnClose = state;
}


// ****************************************************************************
// always on top
void MainWindow::toggleAlwaysOnTop()
{
    setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint);
    show();
    raise();
}


// ****************************************************************************
// full screen
void MainWindow::toggleFullScreen()
{
    setWindowState(windowState() ^ Qt::WindowFullScreen);
}


// ****************************************************************************
// options dialog
void MainWindow::showOptionsDialog()
{
    optionsDialog->show();
    optionsDialog->raise();
    optionsDialog->activateWindow();
}


// ****************************************************************************
// help dialog
void MainWindow::showHelpDialog()
{
    HelpDialog dlg(this);
    dlg.exec();
}


// ****************************************************************************
// about dialog
void MainWindow::showAboutDialog()
{
    AboutDialog dlg(this);
    dlg.exec();
}
