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

#include "optionsdialog.hpp"
#include "settings.hpp"

#include <QMessageBox>
#include <QColorDialog>
#include <QStyleFactory>

#include <QDebug>

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    ui.tabWidget->setCurrentIndex(0);

    // dialog buttons
    connect(ui.pushButtonDefault, SIGNAL(clicked()), this, SLOT(restoreDefaults()));
    connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(onOK()));
    connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(close()));

    // biorhythm tab
    connect(ui.labelPhy, SIGNAL(mouseClicked()), this, SLOT(changePhyColor()));
    connect(ui.checkBoxPhysical, SIGNAL(toggled(bool)), this, SIGNAL(phyBioChanged(bool)));
    connect(ui.checkBoxPhysical, SIGNAL(toggled(bool)), this, SLOT(bioChanged()));
    connect(ui.labelEmo, SIGNAL(mouseClicked()), this, SLOT(changeEmoColor()));
    connect(ui.checkBoxEmotional, SIGNAL(toggled(bool)), this, SIGNAL(emoBioChanged(bool)));
    connect(ui.checkBoxEmotional, SIGNAL(toggled(bool)), this, SLOT(bioChanged()));
    connect(ui.labelInt, SIGNAL(mouseClicked()), this, SLOT(changeIntColor()));
    connect(ui.checkBoxIntellectual, SIGNAL(toggled(bool)), this, SIGNAL(intBioChanged(bool)));
    connect(ui.checkBoxIntellectual, SIGNAL(toggled(bool)), this, SLOT(bioChanged()));
    connect(ui.labelIntu, SIGNAL(mouseClicked()), this, SLOT(changeIntuColor()));
    connect(ui.checkBoxIntuitional, SIGNAL(toggled(bool)), this, SIGNAL(intuBioChanged(bool)));
    connect(ui.checkBoxIntuitional, SIGNAL(toggled(bool)), this, SLOT(bioChanged()));
    connect(ui.labelAvg, SIGNAL(mouseClicked()), this, SLOT(changeAverageColor()));
    connect(ui.checkBoxAverage, SIGNAL(toggled(bool)), this, SIGNAL(averageBioChanged(bool)));
    connect(ui.checkBoxSelectedAverage, SIGNAL(toggled(bool)), this, SIGNAL(selectedAverageChanged(bool)));
    connect(ui.checkBoxSelectedAverage, SIGNAL(toggled(bool)), this, SLOT(bioChanged()));

    connect(ui.radioButtonNone, SIGNAL(toggled(bool)), this, SIGNAL(bioDataNone(bool)));
    connect(ui.radioButtonRight, SIGNAL(toggled(bool)), this, SIGNAL(bioDataRight(bool)));
    connect(ui.radioButtonBottom, SIGNAL(toggled(bool)), this, SIGNAL(bioDataBottom(bool)));

    // graph tab
    connect(ui.checkBoxToday, SIGNAL(toggled(bool)), this, SIGNAL(todayChanged(bool)));
    connect(ui.checkBoxToday, SIGNAL(toggled(bool)), ui.groupBoxMouseWheel, SLOT(setEnabled(bool)));
    connect(ui.checkBoxVLines, SIGNAL(toggled(bool)), this, SIGNAL(vLinesChanged(bool)));
    connect(ui.checkBoxAxisVLabels, SIGNAL(toggled(bool)), this, SIGNAL(axisVLabelsChanged(bool)));
    connect(ui.checkBoxHLines, SIGNAL(toggled(bool)), this, SIGNAL(hLinesChanged(bool)));
    connect(ui.checkBoxAxisHLabels, SIGNAL(toggled(bool)), this, SIGNAL(axisHLabelsChanged(bool)));
    connect(ui.comboBoxHLinesDelta, SIGNAL(currentIndexChanged(QString)), this, SIGNAL(axisHLinesDelta(QString)));    
    connect(ui.checkBoxZero, SIGNAL(toggled(bool)), this, SIGNAL(zeroChanged(bool)));
    connect(ui.checkBoxAntialiasing, SIGNAL(toggled(bool)), this, SIGNAL(antialiasingChanged(bool)));
    connect(ui.doubleSpinBox, SIGNAL(valueChanged(double)), this, SIGNAL(penWidthChanged(double)));
    connect(ui.checkBoxGraphBackground, SIGNAL(toggled(bool)), this, SIGNAL(graphBackgroundChanged(bool)));
    //ui.labelGraphBackground->setStyleSheet(QString("background-color: %1").arg(bio->getBackgroundColor().name()));
    connect(ui.labelGraphBackground, SIGNAL(mouseClicked()), this, SLOT(changeGraphBackgroundColor()));

    connect(ui.groupBoxMouseFollow, SIGNAL(toggled(bool)), this, SIGNAL(mouseFollowChanged(bool)));
    connect(ui.checkBoxTooltip, SIGNAL(toggled(bool)), this, SIGNAL(toolTipChanged(bool)));

    connect(ui.groupBoxMouseWheel, SIGNAL(toggled(bool)), this, SIGNAL(mouseWheelChanged(bool)));
    connect(ui.checkBoxReverseScroll, SIGNAL(toggled(bool)), this, SIGNAL(reverseScrollChanged(bool)));

    // application tab
    connect(ui.checkBoxShowTrayIcon, SIGNAL(toggled(bool)), this, SIGNAL(showTrayIcon(bool)));
    connect(ui.checkBoxAlwaysShowToolButtonMenu, SIGNAL(toggled(bool)), this, SIGNAL(showToolButtonMenu(bool)));

    //TODO menedzer
    connect(ui.comboBoxRange, SIGNAL(currentIndexChanged(int)), this, SIGNAL(rangeChanged(int)));

    //TODO zamykanie
    connect(ui.checkBoxMinimizeToTray, SIGNAL(toggled(bool)), this, SIGNAL(minimizeToTrayIconOnClose(bool)));

    ui.labelAppBackground->setStyleSheet(QString("background-color: %1").arg(QApplication::palette().background().color().name()));
    connect(ui.checkBoxAppBackground, SIGNAL(toggled(bool)), this, SIGNAL(appBackgroundChanged(bool)));
    connect(ui.labelAppBackground, SIGNAL(mouseClicked()), this, SLOT(changeAppBackgroundColor()));

    //
    originalPalette = QApplication::palette();
    ui.comboBoxStyle->addItems(QStyleFactory::keys());
    connect(ui.comboBoxStyle, SIGNAL(activated(QString)), this, SLOT(changeStyle(QString)));
    connect(ui.comboBoxColorPalette, SIGNAL(currentIndexChanged(int)), this, SLOT(setAppColorPalette(int)));
}

OptionsDialog::~OptionsDialog()
{
    //Settings::users.saveLastUser = ui.checkBoxSaveLastUser->isChecked();
}


// ****************************************************************************
//
void OptionsDialog::restoreSettings()
{

}


// ****************************************************************************
// dialog buttons
void OptionsDialog::restoreDefaults()
{
    /* 18.11.2018
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this,
                                 QApplication::applicationName(),
                                 "Czy na pewno przywrócić domyślne ustawienia dla tej zakładki?",
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (reply == QMessageBox::No)
        return;
    else if (reply == QMessageBox::Yes)
    {
        switch (ui.tabWidget->currentIndex())
        {
            case 0: // biorytm

                    ui.labelPhy->setStyleSheet(QString("background-color: %1").arg(QColor(Qt::green).name()));
                    emit phyColorChanged(Qt::green);
                    ui.checkBoxPhysical->setChecked(true);
                    ui.labelEmo->setStyleSheet(QString("background-color: %1").arg(QColor(Qt::red).name()));
                    emit emoColorChanged(Qt::red);
                    ui.checkBoxEmotional->setChecked(true);
                    ui.labelInt->setStyleSheet(QString("background-color: %1").arg(QColor(Qt::blue).name()));
                    emit intColorChanged(Qt::blue);
                    ui.checkBoxIntellectual->setChecked(true);
                    ui.labelIntu->setStyleSheet(QString("background-color: %1").arg(QColor(Qt::magenta).name()));
                    emit intuColorChanged(Qt::magenta);
                    ui.checkBoxIntuitional->setChecked(true);
                    ui.labelAvg->setStyleSheet(QString("background-color: %1").arg(QColor(Qt::yellow).name()));
                    emit averageColorChanged(Qt::yellow);
                    ui.checkBoxAverage->setChecked(true);

                    ui.checkBoxSelectedAverage->setChecked(true);

                    ui.radioButtonBottom->toggle();

                    break;

            case 1: // wykres

                    ui.comboBoxRange->setCurrentIndex(0);
                    ui.checkBoxToday->setChecked(true);
                    ui.checkBoxZero->setChecked(true);
                    ui.checkBoxVLines->setChecked(true);
                    //ui.checkBoxAxisLabels->setChecked(false);
                    ui.checkBoxAntialiasing->setChecked(true);
                    ui.checkBoxGraphBackground->setChecked(true);
                    ui.labelGraphBackground->setStyleSheet(QString("background-color: %1").arg(QColor(Qt::black).name()));
                    emit graphBackgroundChanged(QColor(Qt::black));
                    ui.doubleSpinBox->setValue(0.0);

                    ui.groupBoxMouseFollow->setChecked(true);

                    break;

            case 2: // aplikacja

                    // main dialog background color
                    ui.labelAppBackground->setStyleSheet(QString("background-color: %1").arg(QApplication::palette().background().color().name()));
                    emit appBackgroundChanged(QApplication::palette().background().color());
                    ui.checkBoxAppBackground->setChecked(false);

                    break;

            default: break;
        }
    }
    */
}

void OptionsDialog::onOK()
{
    saveSettings();

    close();
}

void OptionsDialog::saveSettings()
{
    QString s = ui.labelPhy->styleSheet();

    //Settings::biorhythm.phyColor = QColor(s.mid(s.indexOf("#"), 7));
}


// ****************************************************************************
// biorhythm tab
void OptionsDialog::bioChanged()
{
    if (!ui.checkBoxPhysical->isChecked() && !ui.checkBoxEmotional->isChecked() && !ui.checkBoxIntellectual->isChecked() && !ui.checkBoxIntuitional->isChecked() && ui.checkBoxSelectedAverage->isChecked())
    {
        emit averageBioChanged(false);
        ui.checkBoxAverage->setDisabled(true);
    }
    else
    {
        ui.checkBoxAverage->setEnabled(true);
        if (ui.checkBoxAverage->isChecked())
            emit averageBioChanged(true);
    }
}

void OptionsDialog::changePhyColor()
{
    QColor color = getColorDialog(ui.labelPhy);

    if (color.isValid())
    {
        ui.labelPhy->setStyleSheet(QString("background-color: %1").arg(color.name()));

        emit (phyColorChanged(color));
    }
}

void OptionsDialog::changeEmoColor()
{
    QColor color = getColorDialog(ui.labelEmo);

    if (color.isValid())
    {
        ui.labelEmo->setStyleSheet(QString("background-color: %1").arg(color.name()));

        emit (emoColorChanged(color));
    }
}

void OptionsDialog::changeIntColor()
{
    QColor color = getColorDialog(ui.labelInt);

    if (color.isValid())
    {
        ui.labelInt->setStyleSheet(QString("background-color: %1").arg(color.name()));

        emit (intColorChanged(color));
    }
}

void OptionsDialog::changeIntuColor()
{
    QColor color = getColorDialog(ui.labelIntu);

    if (color.isValid())
    {
        ui.labelIntu->setStyleSheet(QString("background-color: %1").arg(color.name()));

        emit (intuColorChanged(color));
    }
}

void OptionsDialog::changeAverageColor()
{
    QColor color = getColorDialog(ui.labelAvg);

    if (color.isValid())
    {
        ui.labelAvg->setStyleSheet(QString("background-color: %1").arg(color.name()));

        emit (averageColorChanged(color));
    }
}


// ****************************************************************************
// graph tab
void OptionsDialog::changeGraphBackgroundColor()
{
    QColor color = getColorDialog(ui.labelGraphBackground);

    if (color.isValid())
    {
        ui.labelGraphBackground->setStyleSheet(QString("background-color: %1").arg(color.name()));

        emit (graphBackgroundChanged(color));
    }
}


// ****************************************************************************
// application tab

// app background color
void OptionsDialog::changeAppBackgroundColor()
{
    QColor color = getColorDialog(ui.labelAppBackground);

    if (color.isValid())
    {
        ui.labelAppBackground->setStyleSheet(QString("background-color: %1").arg(color.name()));

        emit (appBackgroundChanged(color));
    }
}

// app dark style
void OptionsDialog::setAppColorPalette(int index)
{
    // dark "theme"
    if (index == 2)
    {
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53,53,53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25,25,25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
        darkPalette.setColor(QPalette::ToolTipBase, QColor(53,53,53));
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53,53,53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);

        QApplication::setPalette(darkPalette);

        //qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    }
    // style "theme"
    else if (index == 1)
        changePalette();
    // default "theme"
    else if (index == 0)
        changePalette();
}

void OptionsDialog::changeStyle(const QString &styleName)
{
    QApplication::setStyle(QStyleFactory::create(styleName));

    changePalette();
}

void OptionsDialog::changePalette()
{
    // style "theme"
    if (ui.comboBoxColorPalette->currentIndex() == 1)
    {
        QApplication::setPalette(QApplication::style()->standardPalette());
    }
    // default "theme"
    else if (ui.comboBoxColorPalette->currentIndex() == 0)
        QApplication::setPalette(originalPalette);
}


// ****************************************************************************
//
QColor OptionsDialog::getColorDialog(const QWidget *w)
{
    QString style = w->styleSheet();

    if (!style.isEmpty())
    {
        style = style.right(7);

        return QColorDialog::getColor(QColor(style), this);
    }
    else
    {
        return QColorDialog::getColor(Qt::white, this);
    }
}
