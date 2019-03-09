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

#include "examplesavatarsdialog.hpp"

#include <QDir>

#include <QDebug>

ExamplesAvatarsDialog::ExamplesAvatarsDialog(const QString &avatarName, QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    fileName.clear();

    ui.pushButtonOK->setDisabled(true);

    connect(ui.listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(onItemChanged()));
    connect(ui.listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onItemDoubleClicked(QListWidgetItem*)));

    QDir dir(":/avatars/");
    QStringList files = dir.entryList(QStringList("*.png"));

    for (int i = 0; i < files.count(); ++i)
    {
        QListWidgetItem *item = new QListWidgetItem(ui.listWidget);
        item->setIcon(QIcon(dir.absoluteFilePath(files.at(i))));
        item->setWhatsThis(dir.absoluteFilePath(files.at(i)));
        ui.listWidget->addItem(item);

        // select current user avatar if exists
        if (item->whatsThis() == avatarName)
            ui.listWidget->setCurrentItem(item);
    }

    // if no avatar found select first image
    if (ui.listWidget->selectedItems().isEmpty())
        ui.listWidget->setCurrentItem(ui.listWidget->item(0));

    // buttons
    connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(onOK()));
    connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(close()));
}

void ExamplesAvatarsDialog::onItemChanged()
{
    ui.pushButtonOK->setDisabled(ui.listWidget->selectedItems().isEmpty());
}

void ExamplesAvatarsDialog::onItemDoubleClicked(QListWidgetItem *item)
{
    fileName = item->whatsThis();

    close();
}

void ExamplesAvatarsDialog::onOK()
{
    fileName = ui.listWidget->currentItem()->whatsThis();

    close();
}
