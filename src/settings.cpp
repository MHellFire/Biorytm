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

#include "settings.hpp"
#include "constants.hpp"

#include <QStandardPaths>
#include <QSettings>
#include <QFile>
#include <QMessageBox>
#include <QApplication>

#include <QDebug>

Settings::Settings()
{
    errorCode = 0;

    QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (!path.isEmpty())
    {
        settingsFileName = path.append(constants::settingsFileName);

        createSettingsFile();
    }
    else
    {
        settingsFileName.clear();

        errorCode = 1;
    }
}

Settings::~Settings()
{
}

// ****************************************************************************
// TODO creating default settings file
void Settings::createSettingsFile()
{
    /*
    QSettings s(settingsFileName, QSettings::IniFormat);

    s.beginGroup("MainDialog");
    s.setValue("firstRun", defaults::firstRun);
    s.setValue("startMaximized", defaults::startMaximized);
    s.setValue("maximized", false);
    s.setValue("size", QSize((int)(QApplication::desktop()->width()/8)*6, (int)(QApplication::desktop()->height()/8)*6));
    s.setValue("position", QPoint((int)(QApplication::desktop()->width()/8), (int)(QApplication::desktop()->height()/8)));
    s.setValue("showUsersManager", true);
    s.setValue("showButtonMenu", true);
    s.endGroup();

    s.beginWriteArray("Users");
    //s.setArrayIndex(0);
    //s.setValue("uid", 0);
    //s.setValue("name", "Autor");
    //s.setValue("dateOfBirth", "27.02.1983");
    //s.setValue("avatar", "");
    //s.setValue("defaultUser", true);
    s.endArray();

    //s.beginGroup("Users");
    //s.setValue("defaultUser", 0);
    //s.endGroup();

    s.beginGroup("Biorhythm");
    s.setValue("phyColor", QColor(Qt::green).name());
    s.setValue("emoColor", QColor(Qt::red).name());
    s.setValue("intColor", QColor(Qt::blue).name());
    s.setValue("intuColor", QColor(Qt::magenta).name());
    s.setValue("averageColor", QColor(Qt::yellow).name());
    s.setValue("backgroundColor", QColor(Qt::black).name());
    s.endGroup();

    s.sync();
    */
}

// ****************************************************************************
// static read/write VALUES
QVariant Settings::readValue(const QString &group, const QString &key, const QVariant &defValue)
{
    QString settingsFileName;

    QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (!path.isEmpty())
    {
        settingsFileName = path.append(constants::settingsFileName);
    }
    else
    {
        settingsFileName.clear();
    }


    QFile file(settingsFileName);

    if (!file.exists())
    {
        QMessageBox::warning(NULL, QApplication::applicationName(), "Nie znaleziono pliku konfiguracyjnego aplikacji.");

        //error = 1;

        //create();
    }

    file.close();

    QSettings s(settingsFileName, QSettings::IniFormat);

    s.beginGroup(group);
    QVariant value = s.value(key, defValue);
    s.endGroup();

    s.sync();

    switch (s.status())
    {
        case QSettings::AccessError:
                            QMessageBox::warning(NULL, QApplication::applicationName(), "An access error occurred (trying to read the file).");
                            //return 2;
                            break;
        case QSettings::FormatError:
                            QMessageBox::warning(NULL, QApplication::applicationName(), "A format error occurred (a malformed INI file) while reading the file.");
                            //return 3;
                            break;
        default: break;
    }

    return value;
}

bool Settings::writeValue(const QString &group, const QString &key, const QVariant &value)
{
    QString settingsFileName;

    QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (!path.isEmpty())
    {
        settingsFileName = path.append(constants::settingsFileName);
    }
    else
    {
        settingsFileName.clear();
    }

    QSettings s(settingsFileName, QSettings::IniFormat);

    s.beginGroup(group);
    s.setValue(key, value);
    s.endGroup();

    s.sync();

    switch (s.status())
    {
        case QSettings::AccessError:
                            QMessageBox::warning(NULL, QApplication::applicationName(), "An access error occurred (trying to write the file). Settings not saved.");
                            return false;
                            //break;
        case QSettings::FormatError:
                            QMessageBox::warning(NULL, QApplication::applicationName(), "A format error occurred. Settings not saved.");
                            return false;
                            //break;
        default: break;
    }

    return true;
}

// ****************************************************************************
// static read/write USERS
bool Settings::readUsers(QList<User> *usersList)
{
    QString settingsFileName;

    QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (!path.isEmpty())
    {
        settingsFileName = path.append(constants::settingsFileName);
    }
    else
    {
        settingsFileName.clear();
    }

    qDebug() << "TUUUU";
    QSettings s(settingsFileName, QSettings::IniFormat);


    //QList<User> *users;
    //users = new QList<User>;
    //users->clear();

    qDebug() << " SETTTINGS FILE: " << s.fileName();
    int n = s.beginReadArray("UsersList");
    User user;
    qDebug() << "TU" << n;

    for (int i = 0; i < n; ++i)
    {

        qDebug() << "TU petla";

        s.setArrayIndex(i);
        user.uid = static_cast<quint16>(s.value("uid", 0).toUInt());
        user.name = s.value("name", "Nowy").toString();
        user.dateOfBirth = QDate::fromString(s.value("dateOfBirth", QDate::currentDate().toString("dd.MM.yyyy")).toString(), "dd.MM.yyyy");
        user.avatar = s.value("avatar", "").toString();
        user.defaultUser = s.value("defaultUser", false).toBool();
qDebug() << "TUUUU";
        usersList->append(user);
    }
    s.endArray();

    return true;
}

bool Settings::writeUsers(const QList<User> *usersList)
{
    QString settingsFileName;

    QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (!path.isEmpty())
    {
        settingsFileName = path.append(constants::settingsFileName);
    }
    else
    {
        settingsFileName.clear();
    }

    QSettings s(settingsFileName, QSettings::IniFormat);

    qDebug() << "usersList->count(): " << usersList->count();

    s.remove("UsersList");

    s.beginWriteArray("UsersList");
    for (int i = 0; i < usersList->count(); ++i)
    {
        s.setArrayIndex(i);
        s.setValue("uid", usersList->at(i).uid);
        s.setValue("name", usersList->at(i).name);
        s.setValue("dateOfBirth", usersList->at(i).dateOfBirth.toString("dd.MM.yyyy"));
        s.setValue("avatar", usersList->at(i).avatar);
        s.setValue("defaultUser", usersList->at(i).defaultUser);
    }
    s.endArray();

    s.sync();

    switch (s.status())
    {
        case QSettings::AccessError:
                            QMessageBox::warning(NULL, QApplication::applicationName(), "An access error occurred (trying to write the file). Settings not saved.");
                            return false;
                            //break;
        case QSettings::FormatError:
                            QMessageBox::warning(NULL, QApplication::applicationName(), "A format error occurred. Settings not saved.");
                            return false;
                            //break;
        case QSettings::NoError:
                            qDebug() << "UsersList zapisana";
                            return true;
        default:
            break;
    }

    return true;
}
