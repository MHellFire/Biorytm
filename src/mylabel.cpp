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

#include "mylabel.hpp"

#include <QApplication>
#include <QDebug>

MyLabel::MyLabel(QWidget *parent) :
    QLabel(parent)
{
    setContextMenuPolicy(Qt::DefaultContextMenu);

    //mPressed = false;

    //timer.setInterval(QApplication::doubleClickInterval());
    timer.setInterval(150);
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
    mDblClicked = false;
}

void MyLabel::contextMenuEvent(QContextMenuEvent *e)
{
    emit myContextMenuRequested(mapToGlobal(e->pos()));
}

void MyLabel::mousePressEvent(QMouseEvent *e)
{
    /*
    if (e->button() == Qt::LeftButton)
        mPressed = true;
    else
        mPressed = false;
    */

    emit mousePressed();

    QLabel::mousePressEvent(e);
}

void MyLabel::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && hitLabel(e->pos()))
        pos = e->pos();

    if (mDblClicked)
        mDblClicked = false;
    else if (!timer.isActive())
        timer.start();

    /*
    if (mPressed && (e->button() == Qt::LeftButton) && hitLabel(e->pos()))
    {
        emit mouseClicked();
        emit mouseClicked(mapToGlobal(e->pos()));
    }

    mPressed = false;

    emit mouseReleased();
    */

    QLabel::mouseReleaseEvent(e);
}

void MyLabel::mouseDoubleClickEvent(QMouseEvent *e)
{
    timer.stop();
    mDblClicked = true;

    if (e->button() == Qt::LeftButton)
        emit mouseDoubleClicked();

    QLabel::mouseDoubleClickEvent(e);
}

void MyLabel::timeout()
{
    emit mouseClicked();
    emit mouseClicked(mapToGlobal(pos));
}

bool MyLabel::hitLabel(const QPoint &pos)
{
    return rect().contains(pos);
}
