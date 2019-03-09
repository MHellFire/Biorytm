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

#ifndef MYLABEL_HPP
#define MYLABEL_HPP

#include <QLabel>
#include <QTimer>
#include <QMouseEvent>
#include <QContextMenuEvent>

class MyLabel : public QLabel
{
    Q_OBJECT

public:
    explicit MyLabel(QWidget *parent = 0);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);

    bool hitLabel(const QPoint &pos);
    //bool mPressed;
    QTimer timer;
    bool mDblClicked;
    QPoint pos;

signals:
    void myContextMenuRequested(const QPoint &pos);
    void mouseClicked();
    void mouseClicked(const QPoint &pos);
    void mouseDoubleClicked();
    void mousePressed();
    void mouseReleased();

protected slots:
    void timeout();
};

#endif // MYLABEL_HPP
