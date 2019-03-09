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

#include "usersmanagerdialog.hpp"
#include "settings.hpp"

#include <QScrollBar>

#include <QDebug>
//#include <QThread>
#include <QMessageBox>

UsersManagerDialog::UsersManagerDialog(QList<User> *usersList, quint16 userUID, QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    this->usersList = usersList;

    ui.usersWidget->addUsers(usersList);

    this->userUID = userUID;

    // users widget
    // user selected
    connect(ui.usersWidget, SIGNAL(signalUserSelected(bool)), ui.pushButtonEdit, SLOT(setEnabled(bool)));
    connect(ui.usersWidget, SIGNAL(signalUserSelected(bool)), ui.pushButtonDelete, SLOT(setEnabled(bool)));
    connect(ui.usersWidget, SIGNAL(signalUserSelected(bool)), ui.pushButtonSelect, SLOT(setEnabled(bool)));
    connect(ui.usersWidget, SIGNAL(signalUserSelected(bool)), this, SLOT(onUserSelected(bool)));

    // edit user
    connect(ui.usersWidget, SIGNAL(editUser()), this, SLOT(onEdit()));
    // add user
    connect(ui.usersWidget, SIGNAL(addUserSignal()), this, SLOT(onNew()));
    // on mouse double click on user
    connect(ui.usersWidget, SIGNAL(userChosen()), this, SLOT(onChoose()));
    connect(ui.usersWidget, SIGNAL(scrollToY(int)), this, SLOT(scrollToY(int)));

    // buttons
    //connect(ui.pushButtonNew, SIGNAL(clicked()), ui.usersWidget, SLOT(addUser()));
    connect(ui.pushButtonNew, SIGNAL(clicked()), this, SLOT(onNew()));
    connect(ui.pushButtonEdit, SIGNAL(clicked()), this, SLOT(onEdit()));
    connect(ui.pushButtonDelete, SIGNAL(clicked()), this, SLOT(onDelete()));

    connect(ui.pushButtonSelect, SIGNAL(clicked()), this, SLOT(onChoose()));
    connect(ui.pushButtonExit, SIGNAL(clicked()), this, SLOT(onExit()));

    // edit widget
    connect(ui.editWidget, SIGNAL(signalCancel()), this, SLOT(editCancel()));
    connect(ui.editWidget, SIGNAL(signalSave()), this, SLOT(editSave()));

    ui.scrollArea->setWidgetResizable(true);

    // choose user
    ui.usersWidget->chooseUser(userUID);
    ui.usersWidget->setFocus();

    ui.checkBoxShow->setChecked(Settings::readValue("UsersManagerDialog", "showUsersManagerDlg", true).toBool());

    ui.pushButtonNew->setDisabled(ui.usersWidget->maxUsersLimit());

    newUserEdited = false;
}

UsersManagerDialog::~UsersManagerDialog()
{
    ui.usersWidget->getUsers(usersList);
}

void UsersManagerDialog::keyPressEvent(QKeyEvent *e)
{
    // delete user
    if (e->key() == Qt::Key_Delete)
    {
        if (ui.pushButtonDelete->isEnabled() && ui.stackedWidget->currentIndex() == 0)
            ui.pushButtonDelete->click();

        e->accept();
    }

    // add new user
    if (e->key() == Qt::Key_Insert)
    {
        if (ui.pushButtonNew->isEnabled() && ui.stackedWidget->currentIndex() == 0)
            ui.pushButtonNew->click();

        e->accept();
    }

    // edit user
    if (e->key() == Qt::Key_F2)
    {
        if (ui.pushButtonEdit->isEnabled() && (ui.stackedWidget->currentIndex() == 0))
            ui.pushButtonEdit->click();

        e->accept();
    }

    // choose user and close
    if (e->key() == Qt::Key_Enter)
    {
        if (ui.pushButtonSelect->isEnabled() && (ui.stackedWidget->currentIndex() == 0))
            ui.pushButtonSelect->click();

        e->accept();
    }

    // ignore ESCAPE key
    if (e->key() == Qt::Key_Escape)
    {
        e->accept();
    }

    /*
    Qt::KeyboardModifiers modifiers = e->modifiers();

    // Ctrl + Alt + A
    if ((modifiers & Qt::ControlModifier) && (modifiers & Qt::AltModifier) && (e->key() == Qt::Key_A))
    {
        e->accept();
    }
    */
    //QDialog::keyPressEvent(e);
    e->ignore();
}

void UsersManagerDialog::onUserSelected(bool selected)
{
    ui.pushButtonSelect->setDefault(selected);
    ui.pushButtonNew->setDefault(!selected);
}

void UsersManagerDialog::scrollToY(int y)
{
    if (y > ui.scrollArea->verticalScrollBar()->value())
        ui.scrollArea->verticalScrollBar()->setMaximum(y);
    //ui.scrollArea->ensureVisible(0, y);
    ui.scrollArea->verticalScrollBar()->setValue(y);
}

// add new user
void UsersManagerDialog::onNew()
{
    if (ui.usersWidget->maxUsersLimit())
    {
        QApplication::beep();
        QMessageBox::warning(this,
                             QCoreApplication::applicationName(),
                             QString("Osiągnięto limit użytkowników (%1 użytkowników). Aby dodać nowego użytkownika proszę usunąć któregoś z istniejących.").arg(constants::maxUsers));


        ui.pushButtonNew->setDisabled(true);

        return;
    }

    ui.editWidget->setData("", QDate::currentDate(), "", !ui.usersWidget->defaultUserExist(), ui.usersWidget->getUniqueUID());
    ui.stackedWidget->setCurrentIndex(1);

    ui.pushButtonSelect->setDisabled(true);
    ui.pushButtonExit->setDisabled(true);

    // edit mode with new user "flag"
    newUserEdited = true;
}

void UsersManagerDialog::onDelete()
{
    ui.usersWidget->deleteUser();

    if (!ui.usersWidget->maxUsersLimit())
        ui.pushButtonNew->setEnabled(true);
}

// switch to editing
void UsersManagerDialog::onEdit()
{
    ui.editWidget->setData(ui.usersWidget->getSelectedName(), ui.usersWidget->getSelectedDate(), ui.usersWidget->getSelectedAvatar(), ui.usersWidget->getSelectedDefault(), ui.usersWidget->getSelectedUserUID());

    ui.stackedWidget->setCurrentIndex(1);

    ui.pushButtonSelect->setDisabled(true);
    ui.pushButtonExit->setDisabled(true);
}

// back from editing
void UsersManagerDialog::editSave()
{
    ui.stackedWidget->setCurrentIndex(0);

    ui.pushButtonExit->setEnabled(true);

    // if new user "flag" selected
    if (newUserEdited)
    {
        ui.usersWidget->addUser(ui.editWidget->getUserUID(), ui.editWidget->getName(), ui.editWidget->getDate(), ui.editWidget->getAvatar(), ui.editWidget->getDefaultUser());

        newUserEdited = false;

        if (ui.usersWidget->maxUsersLimit())
        {
            QMessageBox::warning(this,
                                 QCoreApplication::applicationName(),
                                 QString("Osiągnięto limit użytkowników (%1 użytkowników).").arg(constants::maxUsers));

            ui.pushButtonNew->setDisabled(true);
        }
    }
    else
    {
        ui.usersWidget->updateUser(ui.editWidget->getName(), ui.editWidget->getDate(), ui.editWidget->getAvatar(), ui.editWidget->getDefaultUser());
    }
}

void UsersManagerDialog::editCancel()
{
    ui.stackedWidget->setCurrentIndex(0);

    ui.pushButtonExit->setEnabled(true);

    ui.usersWidget->update();
}

// OK
void UsersManagerDialog::onChoose()
{
    name = ui.usersWidget->getSelectedName();
    date = ui.usersWidget->getSelectedDate();
    avatar = ui.usersWidget->getSelectedAvatar();
    userUID = ui.usersWidget->getSelectedUserUID();

    Settings::writeValue("UsersManagerDialog", "showUsersManagerDlg",ui.checkBoxShow->isChecked());

    close();
}

// exit users manager dialog
void UsersManagerDialog::onExit()
{
    if (ui.usersWidget->getSelectedUserUID() == userUID)
    {
        onChoose();
    }
    else
    {
        Settings::writeValue("UsersManagerDialog", "showUsersManagerDlg", ui.checkBoxShow->isChecked());

        close();
    }
}

void UsersManagerDialog::addUserMode()
{
    onNew();
}

void UsersManagerDialog::editUserMode()
{
    onEdit();
}
