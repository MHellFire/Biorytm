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

#include "helpdialog.hpp"

HelpDialog::HelpDialog(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    // tree widget - help contents
    ui.treeWidget->setCurrentItem(ui.treeWidget->topLevelItem(0));
    connect(ui.treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(onTreeCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));

    // buttons
    connect(ui.pushButtonOK, SIGNAL(clicked(bool)), this, SLOT(close()));
}

void HelpDialog::onTreeCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous);

    ui.textBrowser->scrollToAnchor(current->text(0));
}
