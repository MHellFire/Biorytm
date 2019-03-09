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

#include "userframe.hpp"

#include <QPainter>

#include <QDebug>

const quint8 UserFrame::userFrameHeight = 60;
const quint8 UserFrame::userFrameWidth = 250;

UserFrame::UserFrame(const quint16 &userUID, const QString &name, const QDate &date, const QString &avatar, bool defaultUser, QWidget *parent) :
    QFrame(parent), userUID(userUID), name(name), date(date), avatar(avatar), defaultUser(defaultUser)
{
    setMinimumSize(userFrameWidth, userFrameHeight);
    setMaximumSize(userFrameWidth, userFrameHeight);

    selected = false;
}

void UserFrame::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QPainter painter(this);
    paint(painter);
}

void UserFrame::paint(QPainter &painter)
{
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen; // constructs a default black solid line pen with 1 width.

    //pen.setWidthF(4.0);

    //pen.setColor(selected ? QColor(102, 153, 255) : QColor(227, 227, 227));   // light
    //pen.setColor(selected ? QColor(49, 128, 203) : QColor(138, 138, 138));   // medium
    pen.setColor(selected ? QColor(49, 128, 203) : QColor(120, 120, 120));    // dark

    painter.setPen(pen);//Qt::NoPen); // no line at all
    //painter.setBrush(selected ? QColor(102, 153, 255) : QColor(227, 227, 227));   // light
    painter.setBrush(selected ? QColor(49, 128, 203) : QColor(138, 138, 138));   // medium
    //painter.setBrush(selected ? QColor(49, 128, 203) : QColor(120, 120, 120));    // dark

    //painter.drawRoundedRect(0, 0, userFrameWidth, userFrameHeight, 8.0, 8.0);
    painter.drawRect(0, 0, userFrameWidth, userFrameHeight);

    if (defaultUser)
        painter.drawPixmap(214, 22, 16, 16, QPixmap(":/images/default.png"));

    painter.drawPixmap(10, 6, 48, 48, QPixmap(avatar.isEmpty() ? constants::userAvatar : avatar));

    QFont font;
    font.setPixelSize(14);
    painter.setPen(Qt::black);
    painter.setFont(font);
    painter.drawText(64, 20, name);
    font.setPixelSize(13);
    painter.setFont(font);
    painter.drawText(64, 42, date.toString("dd.MM.yyyy"));

    // draw userUID
    #ifdef QT_DEBUG
        font.setPixelSize(8);
        painter.setFont(font);
        painter.drawText(64, 54, QString("UID: %1").arg(userUID));
    #endif
}

void UserFrame::setDefault(const bool defaultUser)
{
    this->defaultUser = defaultUser;
    update();
}

void UserFrame::select(const bool selected)
{
    this->selected = selected;
    update();
}

void UserFrame::setName(const QString &name)
{
    this->name = name;
}

void UserFrame::setDate(const QDate &date)
{
    this->date = date;
}

void UserFrame::setAvatar(const QString &avatar)
{
    this->avatar = avatar;
}
