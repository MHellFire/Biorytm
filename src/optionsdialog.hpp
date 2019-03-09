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

#ifndef OPTIONSDIALOG_HPP
#define OPTIONSDIALOG_HPP

#include "ui_optionsdialog.h"

#include "biorhythmgraphwidget.hpp"

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = 0);
    ~OptionsDialog();

private:
    Ui::OptionsDialog ui;

    QPalette originalPalette;
    QColor getColorDialog(const QWidget *w);

    void restoreSettings();

private slots:
    void bioChanged();
    void changePhyColor();
    void changeEmoColor();
    void changeIntColor();
    void changeIntuColor();
    void changeAverageColor();

    void changeGraphBackgroundColor();

    void changeAppBackgroundColor();
    void setAppColorPalette(int index);
    void changeStyle(const QString &styleName);
    void changePalette();

    void onOK();
    void restoreDefaults();
    void saveSettings();

public slots:

    /*void rangeChanged(int range);
    void phyBioChanged(bool);
    void emoBioChanged(bool);
    void intBioChanged(bool);
    void intuBioChanged(bool);
    void averageBioChanged(bool);
    void selectedAverageChanged(bool);
    void zeroChanged(bool);
    void todayChanged(bool);
    void antialiasingChanged(bool);
    void graphBackgroundChanged(bool);
    void graphBackgroundChanged(QColor);
    void penWidthChanged(double);
    void mouseFollowChanged(bool);
    void vLinesChanged(bool);
    void phyColorChanged(QColor);
    void emoColorChanged(QColor);
    void intColorChanged(QColor);
    void intuColorChanged(QColor);
    void averageColorChanged(QColor);
    void appBackgroundChanged(bool);
    void appBackgroundChanged(QColor);
*/
signals:
    // biorhythm tab
    void phyColorChanged(QColor);
    void phyBioChanged(bool);
    void emoColorChanged(QColor);
    void emoBioChanged(bool);
    void intColorChanged(QColor);
    void intBioChanged(bool);
    void intuColorChanged(QColor);
    void intuBioChanged(bool);
    void averageColorChanged(QColor);
    void averageBioChanged(bool);
    void selectedAverageChanged(bool);
    void bioDataNone(bool);
    void bioDataRight(bool);
    void bioDataBottom(bool);

    // graph tab
    void todayChanged(bool);
    void vLinesChanged(bool);
    void axisVLabelsChanged(bool);
    void hLinesChanged(bool);
    void axisHLabelsChanged(bool);
    void axisHLinesDelta(QString delta);
    void zeroChanged(bool);
    void antialiasingChanged(bool);
    void penWidthChanged(double);
    void graphBackgroundChanged(bool);
    void graphBackgroundChanged(QColor);

    void mouseFollowChanged(bool);
    void toolTipChanged(bool);

    void mouseWheelChanged(bool);
    void reverseScrollChanged(bool);

    // application tab
    void showTrayIcon(bool);
    void showToolButtonMenu(bool);

    void minimizeToTrayIconOnClose(bool);

    void rangeChanged(int range);

    void appBackgroundChanged(bool);
    void appBackgroundChanged(QColor);
};

#endif // OPTIONSDIALOG_HPP
