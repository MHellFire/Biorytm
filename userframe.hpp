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

#ifndef USERFRAME_HPP
#define USERFRAME_HPP

#include <QFrame>
#include <QDate>

#include "constants.hpp"

class UserFrame : public QFrame
{
    Q_OBJECT

public:
    explicit UserFrame(const quint16 &userUID = 0, const QString &name = "Użytkownik", const QDate &date = QDate::currentDate(), const QString &avatar = constants::userAvatar, bool defaultUser = false, QWidget *parent = 0);

    void select(const bool selected);
    void setDefault(const bool defaultUser);
    void setName(const QString &name);
    void setDate(const QDate &date);
    void setAvatar(const QString &avatar);

    static quint8 getUserFrameHeight() { return userFrameHeight; }
    static quint8 getUserFrameWidth() { return userFrameWidth; }

    quint16 getUserUID() const { return userUID; }
    QString getName() const { return name; }
    QDate getDate() const { return date; }
    QString getAvatar() const { return avatar; }
    bool isDefault() const { return defaultUser; }
    bool isSelected() const { return selected; }

    void paint(QPainter &painter);

protected:
    virtual void paintEvent(QPaintEvent *e);

private:
    static const quint8 userFrameHeight;
    static const quint8 userFrameWidth;

    quint16 userUID;
    QString name;
    QDate date;
    QString avatar;
    bool defaultUser, selected;
};

#endif // USERFRAME_HPP
