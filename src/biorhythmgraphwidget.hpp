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

#ifndef BIORHYTHMGRAPHWIDGET_HPP
#define BIORHYTHMGRAPHWIDGET_HPP

#include <QWidget>

#include <QMouseEvent>
#include <QPainter>
#include <QDate>
#include <QToolTip>

#define MONTH 0
#define WEEK 1
#define DAY 2

class BiorhythmGraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BiorhythmGraphWidget(QWidget *parent = 0);
    ~BiorhythmGraphWidget();

    void newDateOfBirth(QDate userDateOfBirth);

    QColor getBackgroundColor() const { return palette().background().color(); }

    // for menus actions
    bool isPhyBio() const { return phyBio; }
    bool isEmoBio() const { return emoBio; }
    bool isIntBio() const { return intBio; }
    bool isIntuBio() const { return intuBio; }
    bool isAverageBio() const { return averageBio; }

    QColor getPhyBioColor() const { return phyColor; }
    QColor getEmoBioColor() const { return emoColor; }
    QColor getIntBioColor() const { return intColor; }
    QColor getIntuBioColor() const { return intuColor; }
    QColor getAverageBioColor() const { return averageColor; }

    bool isShowToday() const { return today; }
    bool isVLines() const { return vLines; }
    bool isAxisVLabels() const { return axisVLabels; }
    bool isHLines() const { return hLines; }
    bool isAxisHLabels() const { return axisHLabels; }
    bool isZeroLine() const { return zero; }

    void paint(QPainter &painter);

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void paintEvent(QPaintEvent *event);

private:
    QList<double> phyList, emoList, intList, intuList, averageList;
    qint64 userDaysToFirst;
    int daysToShow;
    int currentPos;
    QDate userDateOfBirth, firstDate;
    quint32 hLinesDelta;

    QColor phyColor, emoColor, intColor, intuColor, averageColor;
    double penWidth;
    bool phyBio, emoBio, intBio, intuBio, averageBio, zero, today, hLines, vLines, axisVLabels, axisHLabels, antialiasing, showToolTip, mouseWheel, reverseScroll, selectedAverage;

    inline qreal getBioValue(quint8 cycle, qreal day);
    inline qreal getBioValue(quint8 cycle, qint64 day);
    void emitBioValues(int pos);

    void calcUsersBioVal();
    void currentPosChanged();
    void currentPosChanged(int pos);


signals:
    void signalBioValues(double phyB, bool phyArrowUp, double emoB, bool emoArrowUp, double intB, bool intArrowUp, double intuB, bool intuArrowUp, double average, bool avgArrowUp);
    void signalUserDays(const quint64 userDays);
    void signalDate(const QDate &date);
    void signalHour(const int hour);
    void signalViewChanged(int newView);

public slots:
    //void changeView();
    void changeView(int newView = -1);
    void setNewDateToShow(const QDate &date);

    void showToday();

    void setPhyBio(bool state);
    void setEmoBio(bool state);
    void setIntBio(bool state);
    void setIntuBio(bool state);
    void setAverageBio(bool state);

    void setSelectedAverageChanged(bool state);

    void setZero(bool state);
    void setToday(bool state);
    void showAxisVLabels(bool state);
    void showAxisHLabels(bool state);

    void setHLinesDelta(QString delta);
    void back();
    void forward();
    void backDay();
    void forwardDay();

    void setHLines(bool state);
    void setVLines(bool state);
    void setAntialiasing(bool state);
    void setBackground(const QColor &color);
    void setBackground(bool state);
    void setPenWidth(double width);

    void setShowToolTip(bool state);
    void setMouseFollow(bool state);
    void setMouseWheel(bool state);
    void setReverseScroll(bool state);

    void setPhyColor(QColor color);
    void setEmoColor(QColor color);
    void setIntColor(QColor color);
    void setIntuColor(QColor color);
    void setAverageColor(QColor color);

private slots:
    void customContextMenu(const QPoint &pos);
};

#endif // BIORHYTHMGRAPHWIDGET_HPP
