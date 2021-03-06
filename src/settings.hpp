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

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <QString>
#include <QDate>
#include <QVariant>

// structure for users list (QList<User>)
struct User
{
    quint16 uid;
    QString name;
    QDate dateOfBirth;
    QString avatar;
    bool defaultUser;
};

class Settings
{
public:
    explicit Settings();
    ~Settings();

    quint8 getErrorCode() const { return errorCode; }

    // static functions
    static QVariant readValue(const QString &group, const QString &key, const QVariant &defValue);
    static bool writeValue(const QString &group, const QString &key, const QVariant &value);

    // static functions for read/save users list
    static bool readUsers(QList<User> *usersList);
    static bool writeUsers(const  QList<User> *usersList);

private:
    QString settingsFileName; 
    quint8 errorCode;

    void createSettingsFile();
};

#endif // SETTINGS_HPP
