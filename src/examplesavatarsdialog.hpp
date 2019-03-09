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

#ifndef EXAMPLESAVATARSDIALOG_HPP
#define EXAMPLESAVATARSDIALOG_HPP

#include "ui_examplesavatarsdialog.h"

#include <QListWidgetItem>

class ExamplesAvatarsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExamplesAvatarsDialog(const QString &avatarName = "", QWidget *parent = 0);

    QString getAvatarFileName() const { return fileName; }

private:
    Ui::ExamplesAvatarsDialog ui;

    QString fileName;

private slots:
    void onItemChanged();
    void onItemDoubleClicked(QListWidgetItem *item);
    void onOK();
};

#endif // EXAMPLESAVATARSDIALOG_HPP
