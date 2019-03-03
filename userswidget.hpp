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

#ifndef USERSWIDGET_HPP
#define USERSWIDGET_HPP

#include <QWidget>
#include <QTimer>

#include "settings.hpp"
#include "userframe.hpp"

//QT_BEGIN_NAMESPACE
//class QDate;
//class QDragEnterEvent;
//class QDropEvent;
//QT_END_NAMESPACE

namespace {
    // max users on list
    //const quint8 maxUsers = 20;
    // spacing between users frame
    const quint8 spacing = 6;
}

class UsersWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UsersWidget(QWidget *parent = 0);
    ~UsersWidget();

    int getSelectedY() const;
    QString getSelectedName() const { return name; }
    QDate getSelectedDate() const { return date; }
    QString getSelectedAvatar() const { return avatar; }
    bool getSelectedDefault() const { return defaultUser; }
    quint16 getSelectedUserUID() const { return userUID; }

    void addUsers(QList<User> *usersL);
    bool getUsers(QList<User> *usersL);


    void deleteUser();
    void chooseUser(quint16 userUID);
    bool maxUsersLimit();
    bool defaultUserExist();
    void addUser(quint16 userUID, const QString &name, const QDate &date, const QString &avatar, const bool defaultUser);
    void update();
    void updateUser(const QString &name, const QDate &date, const QString &avatar, const bool defaultUser);
    quint16 getUniqueUID();

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    //virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dropEvent(QDropEvent *e);

private:
    QList<UserFrame*> usersList;

    bool defaultUser, dragStart;
    quint16 userUID;
    QString name, avatar;
    QDate date;
    QPoint dragStartPosition;

    void testGetUniqueUID();
    //QTimer dragTimer;

signals:
    void signalUserSelected(bool state);
    void addUserSignal();
    void editUser();
    void userChosen();
    void scrollToY(int y);

private slots:
    void onContextMenu(const QPoint &pos);
    //void changeDragCursor();
};

#endif // USERSWIDGET_HPP
