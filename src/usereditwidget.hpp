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

#ifndef USEREDITWIDGET_HPP
#define USEREDITWIDGET_HPP

#include "ui_usereditwidget.h"

#include "constants.hpp"

class UserEditWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UserEditWidget(QWidget *parent = 0);

    void setData(const QString &name = "", const QDate &date = QDate::currentDate(), const QString &avatar = constants::userAvatar, const bool defaultUser = false, const quint16 userUID = 0);

    QString getName() const { return ui.lineEdit->text().isEmpty() ? QString("Użytkownik") : ui.lineEdit->text(); }
    QDate getDate() const { return ui.dateEdit->date(); }
    QString getAvatar() const { return pictureFile; }
    bool getDefaultUser() const { return ui.checkBox->isChecked(); }
    quint16 getUserUID() const { return userUID; }

private:
    Ui::UserEditWidget ui;

    bool newPictureImported, oldPictureDelete;
    QString pictureFile, newPictureFile, homePath;
    quint16 userUID;

signals:
    void signalCancel();
    void signalSave();

private slots:
    void onImport();
    void onExamplesAvatars();
    void onDelete();
    void onCancel();
    void onSave();
};

#endif // USEREDITWIDGET_HPP
