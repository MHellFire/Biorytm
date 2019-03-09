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

#include "aboutdialog.hpp"
#include "constants.hpp"

#include <QDate>
#include <QStandardPaths>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    // version number of Qt at run-time
    ui.lineEditQtVersion->setText(qVersion()); 
    ui.lineEditQtVersion->setCursorPosition(0);

    // "more widget"
    connect(ui.pushButtonMore, SIGNAL(toggled(bool)), this, SLOT(onMoreToggled(bool)));
    connect(ui.pushButtonMore, SIGNAL(toggled(bool)), ui.widgetMore, SLOT(setVisible(bool)));
    ui.pushButtonMore->setIcon(QIcon(":/images/more_down.png"));
    ui.pushButtonMore->setToolTip("Więcej");
    ui.widgetMore->hide();

    // buttons
    connect(ui.pushButtonLicense, SIGNAL(clicked()), this, SLOT(onShowLicense()));
    connect(ui.pushButtonAboutQt, SIGNAL(clicked()), qApp, SLOT(aboutQt()));
    connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(close()));

    // edit main label text
    QString str = ui.label->text();
    // from main.cpp
    str.replace("__appversion__", QApplication::applicationVersion());
    // c++ preprocessor macros
    str.replace("__date__", QLatin1String(__DATE__));
    str.replace("__time__", QLatin1String(__TIME__));
    // this is the version against which the application is compiled
    str.replace("__qtversion__", QT_VERSION_STR);
    // current year
    str.replace("__year__", QString::number(QDate::currentDate().year()));
    ui.label->setText(str);

    // edit settings file
    ui.lineEditSettingsFile->setText(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + constants::settingsFileName);
    ui.lineEditSettingsFile->setCursorPosition(0);

    // hide testowanie tab (for future usage)
    ui.tabWidget->removeTab(2);

    // TODO check update
}

void AboutDialog::onMoreToggled(bool checked)
{
    ui.pushButtonMore->setIcon(checked ? QIcon(":/images/more_up.png") : QIcon(":/images/more_down.png"));
    ui.pushButtonMore->setToolTip(checked ? "Mniej" : "Więcej");
}

void AboutDialog::onShowLicense()
{
    LicenseDialog dlg(this);
    dlg.exec();
}
