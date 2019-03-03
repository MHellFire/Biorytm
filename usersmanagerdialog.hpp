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

#ifndef USERSMANAGERDIALOG_HPP
#define USERSMANAGERDIALOG_HPP

#include <QKeyEvent>

#include "ui_usersmanagerdialog.h"

class UsersManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UsersManagerDialog(QList<User> *usersList = NULL, quint16 userUID = 0, QWidget *parent = 0);
    ~UsersManagerDialog();

    quint16 getUserUID() const { return userUID; }
    QString getName() const { return name; }
    QDate getDate() const { return date; }
    QString getAvatar() const { return avatar; }

    void addUserMode();
    void editUserMode();

protected:
    virtual void keyPressEvent(QKeyEvent *e);

private:
    Ui::UsersManagerDialog ui;

    UsersWidget *usersWidget;
    UserEditWidget *userEditWidget;

    QList<User> *usersList;

    QString name, avatar;
    QDate date;
    quint16 userUID;

    bool newUserEdited;

private slots:
    void onUserSelected(bool selected);

    void scrollToY(int y);

    void onChoose();
    void onExit();

    void onNew();
    void onEdit();
    void onDelete();

    void editSave();
    void editCancel();
};

#endif // USERSMANAGERDIALOG_HPP
