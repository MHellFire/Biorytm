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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "ui_mainwindow.h"

#include <QMainWindow>
#include <QSignalMapper>
#include <QSystemTrayIcon>
#include <QTimer>

#include "settings.hpp"
#include "optionsdialog.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void resizeEvent(QResizeEvent *e);
    virtual void moveEvent(QMoveEvent *e);
    virtual void closeEvent(QCloseEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);

private:
    Ui::MainWindow ui;

    QSize oldSize;
    QPoint oldPosition;
    bool minimizeToTrayIconOnClose;
    quint8 saveScreenshotOption;

    QList<User> *usersList;

    quint16 currentUserUID;
    QDate currentUserBirthDate;
    QColor appBackgroundColor;

    OptionsDialog *optionsDialog;
    Settings *settings;

    QSignalMapper *signalMapper;

    // toolButtonMenu
    QMenu *toolButtonMenu;

    // statusbar
    QLabel *dateLabel, *clockLabel, *infoLabel;
    QTimer *clockTimer;

    // tray icon
    QMenu *trayIconMenu;
    QSystemTrayIcon *trayIcon;
    bool trayIconAvailable;

    void createMenu();
    void createStatusBar();
    void createToolButtonMenu();
    void createClockTimer();
    void createTrayIcon();
    void createConnections();
    void createOptionsDialog();
    void restoreWindowState();
    void restoreBioGraph();
    void disableBioGraph();
    void enableBioGraph();

    void saveSettings();

private slots:
    void showBioValues(double phyB, bool phyArrowUp, double emoB, bool emoArrowUp, double intB, bool intArrowUp, double intuB, bool intuArrowUp, double averageB, bool avgArrowUp);

    void showDate(const QDate &date);
    void showHour(const int hour);
    void showUserDays(const quint64 userDays);

    void setPhyColor(QColor color);
    void setEmoColor(QColor color);
    void setIntColor(QColor color);
    void setIntuColor(QColor color);
    void setAvgColor(QColor color);

    void setAppBackgroundColor(bool enable);
    void setAppBackgroundColor(const QColor &color);

    void onUsersMenu(const QPoint &pos);
    void onUsersContextMenu(const QPoint &pos);
    void onDialogContextMenu(const QPoint &pos);

    void showUsersManagerDlg();
    void showUsersManagerDlg(quint8 mode);
    void showEditUserDlg();
    void showOptionsDialog();
    void showHelpDialog();
    void showAboutDialog();

    void newUserChoosen(const int userUID);

    void addNewUser();

    void bioGraphViewChanged(int newView);

    void saveScreenshot();
    void toggleAlwaysOnTop();
    void toggleFullScreen();
    void setMinimizeToTrayIconOnClose(bool state);

    void updateClockTimer();

    // menu
    void updateMenuApplicationActions();
    void updateMenuBiorhythmActions();
    void updateMenuGraphActions();
    void updateMenuOptionsActions();
    void showMenuBar();

    // tool button menu
    void updateToolButtonMenuActions();
    void updateTrayIconContextMenuActions();
    void showToolButtonMenu();

    // tray icon
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void messageClicked();
    void minimizeToTray();
};

#endif // MAINWINDOW_HPP
