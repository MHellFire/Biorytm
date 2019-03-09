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

#include "biorhythmgraphwidget.hpp"

#include <QtMath>
#include <QMenu>

#include <QDebug>

BiorhythmGraphWidget::BiorhythmGraphWidget(QWidget *parent) :
    QWidget(parent)
{
    setMouseTracking(true);

    // set background
    setAutoFillBackground(true);
    QPalette p = palette();
    p.setColor(QPalette::Window, QColor(Qt::black));
    setPalette(p);

    //
    phyColor = Qt::green;
    emoColor = Qt::red;
    intColor = Qt::blue;
    intuColor = Qt::magenta;
    averageColor = Qt::yellow;

    phyBio = true;
    emoBio = true;
    intBio = true;
    intuBio = true;
    averageBio = true;
    zero = true;
    today = false;
    axisVLabels = true;
    axisHLabels = true;

    selectedAverage = true;

    hLinesDelta = 25;
    hLines = true;
    vLines = true;
    antialiasing = true;

    showToolTip = false;
    mouseWheel = true;
    reverseScroll = false;

    penWidth = 1.5;

    userDateOfBirth = firstDate = QDate::currentDate();
    userDaysToFirst = 0; // user days of life
    daysToShow = firstDate.daysInMonth(); // 24 == one day mode, 7 == week mode, >7 == month mode
    currentPos = QDate::currentDate().day()-1;

    calcUsersBioVal();

    currentPosChanged();

    phyList.clear();
    emoList.clear();
    intList.clear();
    intuList.clear();
    averageList.clear();

    // context menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenu(QPoint)));
}

BiorhythmGraphWidget::~BiorhythmGraphWidget()
{
}


// ****************************************************************************
// keyboard shortcuts
void BiorhythmGraphWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space)
    {
        changeView();

        event->accept();
        return;
    }

    if (!today)
    {
        //QWidget::keyPressEvent(event);
        event->ignore();
        return;
    }

    if (event->key() == Qt::Key_Left)
    {
        if (currentPos > 0)
        {
            --currentPos;
            currentPosChanged();
        }
        else
            backDay();

        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Right)
    {
        if (currentPos < daysToShow-1)
        {
            ++currentPos;
            currentPosChanged();
        }
        else
            forwardDay();

        event->accept();
        return;
    }

    // if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_Left))
    if (event->key() == Qt::Key_Down)
    {
        back();
        event->accept();
        return;
    }

    // if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_Right))
    if (event->key() == Qt::Key_Up)
    {
        forward();
        event->accept();
        return;
    }

    event->ignore();
}


// ****************************************************************************
// mouse events
void BiorhythmGraphWidget::mousePressEvent(QMouseEvent *event)
{
    if (!isActiveWindow())
    {
        event->ignore();
        return;
    }

    QPointF widgetPosition = event->localPos();

    qreal x = width()*0.038;
    x = (x > 60 ? 60 : x);
    if (!axisHLabels)
        x = 0;

    if ((widgetPosition.x() > x) && (widgetPosition.x() < width()) && (widgetPosition.y() > 0) && (widgetPosition.y() < height()))
    {
        currentPos = qFloor((widgetPosition.x()-x)/((width()-x)/daysToShow));
        currentPosChanged();

        event->accept();
        return;
    }

    event->ignore();
}

void BiorhythmGraphWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!isActiveWindow())
    {
        event->ignore();
        return;
    }

    if (event->buttons() != Qt::LeftButton)
    {
        event->ignore();
        return;
    }

    changeView();
    event->accept();
}

void BiorhythmGraphWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!isActiveWindow())
    {
        event->ignore();
        return;
    }

    QPointF widgetPosition = event->localPos();

    qreal x = width()*0.038;
    x = (x > 60 ? 60 : x);
    if (!axisHLabels)
        x = 0;

    if ((widgetPosition.x() > x) && (widgetPosition.x() < width()) && (widgetPosition.y() > 0) && (widgetPosition.y() < height()))
    {
        int d = qFloor((widgetPosition.x()-x)/((width()-x)/daysToShow));
        currentPosChanged(d);

        if (showToolTip)
        {
            QString str = QString("Fiz: %L1% Emo: %L2% Int: %L3% Intu: %L4%\nŚrednia: %L5%")
                                  .arg(phyList.at(d), 0, 'f', 2)
                                  .arg(emoList.at(d), 0, 'f', 2)
                                  .arg(intList.at(d), 0, 'f', 2)
                                  .arg(intuList.at(d), 0, 'f', 2)
                                  .arg(averageList.at(d), 0, 'f', 2);

            QToolTip::showText(event->globalPos(), str+"\r", this, this->rect());
            QToolTip::showText(event->globalPos(), str, this, this->rect());
        }

        if (event->buttons() == Qt::LeftButton)
            currentPos = d;

        event->accept();
        return;
    }

    event->ignore();
}

void BiorhythmGraphWidget::wheelEvent(QWheelEvent *event)
{
    if (!mouseWheel || !today)
    {
        event->ignore();
        return;
    }

    QPoint numDegrees = event->angleDelta() / 8;
    if (numDegrees.isNull())
        return;

    QPoint numSteps = numDegrees / 15;
    int delta = reverseScroll ? -numSteps.y() : numSteps.y();

    if (delta > 0)
    {
        // left
        if (currentPos > 0)
        {
            --currentPos;
            currentPosChanged();
        }
        else
            backDay();

        event->accept();
        return;
    }
    else if (delta < 0)
    {
        // right
        if (currentPos < daysToShow-1)
        {
            ++currentPos;
            currentPosChanged();
        }
        else forwardDay();

        event->accept();
        return;
    }

    event->ignore();
}


// ****************************************************************************
// PAINTER
void BiorhythmGraphWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    paint(painter);
}

void BiorhythmGraphWidget::paint(QPainter &painter)
{
    if (antialiasing)
        painter.setRenderHint(QPainter::Antialiasing, true);

    //
    qreal x = width()*0.038;
    x = (x > 60 ? 60 : x);
    qreal y = 10;//height()*0.01;

    if (!axisHLabels)
    {
        x = 0;
        y = 2;
    }

    qreal w = width()-x;
    qreal h = (height()-y)*0.90;
    h = (height()-h > 25 ? height()-25 : h);

    if (!axisVLabels)
    {
        h = (height()-y);
    }

    qreal dX = 0.25;
    qreal ax = w/static_cast<qreal>(daysToShow);
    qreal ay = (h-y)/2.0; // (1.0 - (-1.0));

    // graph
    if (hLines)
    {
        painter.save();

        QPen pen;
        pen.setStyle(Qt::DotLine);
        pen.setColor(Qt::darkGray);
        painter.setPen(pen);
        qreal ay=(h-y)/200.0*hLinesDelta;

        for (qreal i=y; i < qFloor(y+(200/hLinesDelta+1)*ay); i+=ay)
            painter.drawLine(QPointF(x, i), QPointF(w+x, i));

        painter.restore();
    }

    if (axisHLabels)
    {
        painter.save();

        qreal ay = (h-y)/200.0*hLinesDelta;

        QFont font;

        for (qreal fontSize=6; fontSize <= 12; fontSize+=0.5)
        {
            font.setPointSizeF(fontSize);
            QFontMetrics fm(font);
            if (fm.width("-100%") > x*0.8)
                break;
        }

        painter.setFont(font);
        painter.setPen(Qt::white);

        QFontMetrics fm(font);
        int j=100;
        for (qreal i=y; i < qFloor(y+(200/hLinesDelta+1)*ay); i+=ay, j-=hLinesDelta)
            painter.drawText(QPointF(x-fm.width(QString("%1%").arg(j)), i+(fm.ascent()*0.3)), QString("%1%").arg(j));

        painter.restore();
    }

    if (vLines)
    {
        painter.save();

        QPen pen;
        pen.setStyle(Qt::DotLine);
        pen.setColor(Qt::darkGray);
        painter.setPen(pen);
        for (qreal i=x; i < qFloor(x+ax*daysToShow); i+=ax)
            painter.drawLine(QPointF(i, h), QPointF(i, y));

        painter.restore();
    }

    if (axisVLabels)
    {
        painter.save();

        QFont font;

        for (qreal fontSize=6; fontSize <= 12; fontSize+=0.5)
        {
            font.setPointSizeF(fontSize);
            QFontMetrics fm(font);
            if (fm.width("44:44") > ax*0.8)
                break;
        }

        painter.setFont(font);
        painter.setPen(Qt::white);

        QFontMetrics fm(font);
        qreal offset = x + (ax-static_cast<qreal>(fm.width("44:44")))/2.0;

        if (daysToShow == 24)
            for (int i=0; i < daysToShow; ++i)
                painter.drawText(QPointF(i*ax+offset, h+(fm.height()*0.8)), QString("%1:00").arg(i+1, 2, 10, QChar('0')));
        else
            for (int i=0; i < daysToShow; ++i)
                painter.drawText(QPointF(i*ax+offset, h+(fm.height()*0.8)), QString("%1.%2").arg(firstDate.addDays(i).day(),2, 10, QChar('0')).arg(firstDate.addDays(i).month(), 2, 10, QChar('0')));

        painter.restore();
    }

    if (zero)
    {
        painter.save();

        QPen pen;
        pen.setColor(Qt::white);
        pen.setWidthF(1.5);
        painter.setPen(pen);

        painter.drawLine(QPointF(x, y+ay), QPointF(x+w, y+ay));

        painter.restore();
    }

    painter.save();

    QPen pen;
    pen.setWidthF(penWidth);
    pen.setCapStyle(Qt::RoundCap);

    if (daysToShow == 24)
    {
        ax = w/1.0;
    }

    if (phyBio)
    {
        QPainterPath path;

        pen.setColor(phyColor);
        painter.setPen(pen);

        path.moveTo(x, y+ay*(1.0-getBioValue(23, userDaysToFirst)));

        for (qreal X = static_cast<double>(userDaysToFirst)+dX; X <= static_cast<double>(userDaysToFirst)+daysToShow+dX; X+=dX)
            path.lineTo(x+ax*(X-static_cast<double>(userDaysToFirst)), y+ay*(1.0-getBioValue(23, X)));

        painter.drawPath(path);
    }

    if (emoBio)
    {
        QPainterPath path;

        pen.setColor(emoColor);
        painter.setPen(pen);

        path.moveTo(x, y+ay*(1.0-getBioValue(28, userDaysToFirst)));

        for (qreal X = static_cast<double>(userDaysToFirst)+dX; X <= static_cast<double>(userDaysToFirst)+daysToShow+dX; X+=dX)
            path.lineTo(x+ax*(X-static_cast<double>(userDaysToFirst)), y+ay*(1.0-getBioValue(28, X)));

        painter.drawPath(path);
    }

    if (intBio)
    {
        QPainterPath path;

        pen.setColor(intColor);
        painter.setPen(pen);

        path.moveTo(x, y+ay*(1.0-getBioValue(33, userDaysToFirst)));

        for (qreal X = static_cast<double>(userDaysToFirst)+dX; X <= static_cast<double>(userDaysToFirst)+daysToShow+dX; X+=dX)
            path.lineTo(x+ax*(X-static_cast<double>(userDaysToFirst)), y+ay*(1.0-getBioValue(33, X)));

        painter.drawPath(path);
    }

    if (intuBio)
    {
        QPainterPath path;

        pen.setColor(intuColor);
        painter.setPen(pen);

        path.moveTo(x, y+ay*(1.0-getBioValue(38, userDaysToFirst)));

        for (qreal X = static_cast<double>(userDaysToFirst)+dX; X <= static_cast<double>(userDaysToFirst)+daysToShow+dX; X+=dX)
            path.lineTo(x+ax*(X-static_cast<double>(userDaysToFirst)), y+ay*(1.0-getBioValue(38, X)));

        painter.drawPath(path);
    }

    if (averageBio)
    {
        QPainterPath path;

        pen.setColor(averageColor);
        painter.setPen(pen);

        path.moveTo(x, y+ay*(1.0-(getBioValue(23, userDaysToFirst)+getBioValue(28, userDaysToFirst)+getBioValue(33, userDaysToFirst)+getBioValue(38, userDaysToFirst))/4.0));

        for (qreal X = static_cast<double>(userDaysToFirst)+dX; X <= static_cast<double>(userDaysToFirst)+daysToShow+dX; X+=dX)
            path.lineTo(x+ax*(X-static_cast<double>(userDaysToFirst)), y+ay*(1.0-(getBioValue(23, X)+getBioValue(28, X)+getBioValue(33, X)+getBioValue(38, X))/4.0));

        painter.drawPath(path);
    }

    painter.restore();

    if (today)
    {
        ax = w/static_cast<qreal>(daysToShow);

        painter.setOpacity(0.8);

        QDate d = QDate::currentDate();
        if(firstDate.addDays(currentPos) == d || (daysToShow == 24 && firstDate == d))
            painter.fillRect(QRectF(((currentPos)*ax)+x, y, ax, h-y), Qt::white);
        else
            painter.fillRect(QRectF(((currentPos)*ax)+x, y, ax, h-y), Qt::lightGray);
    }
}


// ****************************************************************************
// context menu
void BiorhythmGraphWidget::customContextMenu(const QPoint &pos)
{
    QMenu menu("Menu kontekstowe", this);

    QAction *phyA = new QAction("Fizyczny", &menu);
    phyA->setCheckable(true);
    phyA->setChecked(phyBio);
    connect(phyA, SIGNAL(toggled(bool)), this, SLOT(setPhyBio(bool)));
    menu.addAction(phyA);

    QAction *emoA = new QAction("Emocjonalny", &menu);
    emoA->setCheckable(true);
    emoA->setChecked(emoBio);
    connect(emoA, SIGNAL(toggled(bool)), this, SLOT(setEmoBio(bool)));
    menu.addAction(emoA);

    QAction *intA = new QAction("Intelektualny", &menu);
    intA->setCheckable(true);
    intA->setChecked(intBio);
    connect(intA, SIGNAL(toggled(bool)), this, SLOT(setIntBio(bool)));
    menu.addAction(intA);

    QAction *intuA = new QAction("Intuicyjny", &menu);
    intuA->setCheckable(true);
    intuA->setChecked(intuBio);
    connect(intuA, SIGNAL(toggled(bool)), this, SLOT(setIntuBio(bool)));
    menu.addAction(intuA);

    QAction *averageA = new QAction("Średnia", &menu);
    averageA->setCheckable(true);
    averageA->setChecked(averageBio);
    connect(averageA, SIGNAL(toggled(bool)), this, SLOT(setAverageBio(bool)));
    menu.addAction(averageA);

    menu.addSeparator();

    QActionGroup *group = new QActionGroup(&menu);
    group->setExclusive(true);

    QAction *mA = new QAction("Miesiąc", group);
    mA->setCheckable(true);
    menu.addAction(mA);

    QAction *wA = new QAction("Tydzień", group);
    wA->setCheckable(true);
    menu.addAction(wA);

    QAction *dA = new QAction("Dzień", group);
    dA->setCheckable(true);
    menu.addAction(dA);

    if (daysToShow == 24)
        dA->setChecked(true);
    else if (daysToShow == 7)
        wA->setChecked(true);
    else
        mA->setChecked(true);

    menu.addSeparator();

    QAction *mouseA = new QAction("Śledź myszkę", &menu);
    mouseA->setCheckable(true);
    mouseA->setChecked(hasMouseTracking());
    connect(mouseA, SIGNAL(triggered(bool)), this, SLOT(setMouseFollow(bool)));
    menu.addAction(mouseA);

    //if (hasMouseTracking())
    {
        QAction *toolTipA = new QAction("Pokaż dymek", &menu);
        toolTipA->setCheckable(true);
        toolTipA->setChecked(showToolTip);
        toolTipA->setEnabled(hasMouseTracking());
        connect(toolTipA, SIGNAL(triggered(bool)), this, SLOT(setShowToolTip(bool)));
        menu.addAction(toolTipA);
    }

    QAction *a = menu.exec(mapToGlobal(pos));

    if (a == 0)
        return;

    if (a->text().remove("&") == "Miesiąc")
    {
//        currentPos = qFloor(pos.x()/(static_cast<qreal>(width())/daysToShow));;
//        repaint();
        changeView(MONTH);//mouseDoubleClickEvent(new QMouseEvent(QEvent::MouseButtonDblClick, QPoint(pos), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));
    }
    else if (a->text().remove("&") == "Tydzień")
    {
//        currentPos = qFloor(pos.x()/(static_cast<qreal>(width())/daysToShow));;
//        repaint();
        changeView(WEEK);//mouseDoubleClickEvent(new QMouseEvent(QEvent::MouseButtonDblClick, QPoint(pos), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));
    }
    else if (a->text().remove("&") == "Dzień")
    {
//        currentPos = qFloor(pos.x()/(static_cast<qreal>(width())/daysToShow));;
//        repaint();
        changeView(DAY);//mouseDoubleClickEvent(new QMouseEvent(QEvent::MouseButtonDblClick, QPoint(pos), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));
    }
}


// ****************************************************************************
// calculate BIORHYTHM values
inline qreal BiorhythmGraphWidget::getBioValue(quint8 cycle, qreal day)
{
    // cycle
    // 23 Physical cycle
    // 28 Emotional cycle
    // 33 Intellectual cycle
    // 38 Intuition cycle
    // day - day of life

    return sin(2.0*M_PI*day/cycle);
}

inline qreal BiorhythmGraphWidget::getBioValue(quint8 cycle, qint64 day)
{
    // cycle
    // 23 Physical cycle
    // 28 Emotional cycle
    // 33 Intellectual cycle
    // 38 Intuition cycle
    // day - day of life

    return sin(2.0*M_PI*static_cast<double>(day)/cycle);
}

void BiorhythmGraphWidget::calcUsersBioVal()
{
    phyList.clear();
    emoList.clear();
    intList.clear();
    intuList.clear();
    averageList.clear();

    qreal sum;
    int div;

    if (daysToShow == 24)
    {
        // "<=" because we need one more position for showing bio up/down arrows
        // check void BiorhythmGraphWidget::emitBioValues(int pos)
        for (int i = 0; i <= daysToShow; ++i)
        {
            phyList.append(getBioValue(23, userDaysToFirst+i/24)*100);
            emoList.append(getBioValue(28, userDaysToFirst+i/24)*100);
            intList.append(getBioValue(33, userDaysToFirst+i/24)*100);
            intuList.append(getBioValue(38, userDaysToFirst+i/24)*100);

            sum = 0;
            div = 0;

            if (selectedAverage)
            {
                if(phyBio)
                {
                    div++;
                    sum += getBioValue(23, userDaysToFirst+i/24);
                }
                if(emoBio)
                {
                    div++;
                    sum += getBioValue(28, userDaysToFirst+i/24);
                }
                if(intBio)
                {
                    div++;
                    sum += getBioValue(33, userDaysToFirst+i/24);
                }
                if(intuBio)
                {
                    div++;
                    sum += getBioValue(38, userDaysToFirst+i/24);
                }

                sum = (sum / static_cast<qreal>(div))*100;

                averageList.append(sum);
            }
            else
                averageList.append(((getBioValue(23, userDaysToFirst+i/24) + getBioValue(28, userDaysToFirst+i/24) + getBioValue(33, userDaysToFirst+i/24) + getBioValue(38, userDaysToFirst+i/24)) / 4.0)*100);
        }
    }
    else
    {
        // "<=" because we need one more position for showing bio up/down arrows
        // check void BiorhythmGraphWidget::emitBioValues(int pos)
        for (int i = 0; i <= daysToShow; ++i)
        {
            phyList.append(getBioValue(23, userDaysToFirst+i)*100);
            emoList.append(getBioValue(28, userDaysToFirst+i)*100);
            intList.append(getBioValue(33, userDaysToFirst+i)*100);
            intuList.append(getBioValue(38, userDaysToFirst+i)*100);

            sum = 0;
            div = 0;

            if (selectedAverage)
            {
                if(phyBio)
                {
                    div++;
                    sum += getBioValue(23, userDaysToFirst+i);
                }
                if(emoBio)
                {
                    div++;
                    sum += getBioValue(28, userDaysToFirst+i);
                }
                if(intBio)
                {
                    div++;
                    sum += getBioValue(33, userDaysToFirst+i);
                }
                if(intuBio)
                {
                    div++;
                    sum += getBioValue(38, userDaysToFirst+i);
                }

                sum = (sum / static_cast<qreal>(div))*100;

                averageList.append(sum);
            }
            else
                averageList.append(((getBioValue(23, userDaysToFirst+i) + getBioValue(28, userDaysToFirst+i) + getBioValue(33, userDaysToFirst+i) + getBioValue(38, userDaysToFirst+i)) / 4.0)*100);
        }
    }
}

void BiorhythmGraphWidget::emitBioValues(int pos)
{
    emit signalBioValues(phyList.at(pos),
                         phyList.at(pos) < phyList.at(pos+1) ? true: false,
                         emoList.at(pos),
                         emoList.at(pos) < emoList.at(pos+1) ? true: false,
                         intList.at(pos),
                         intList.at(pos) < intList.at(pos+1) ? true: false,
                         intuList.at(pos),
                         intuList.at(pos) < intuList.at(pos+1) ? true: false,
                         averageList.at(pos),
                         averageList.at(pos) < averageList.at(pos+1) ? true: false
                         );
}

void BiorhythmGraphWidget::newDateOfBirth(QDate userDateOfBirth)
{
    this->userDateOfBirth = userDateOfBirth;

    showToday();
}

void BiorhythmGraphWidget::setNewDateToShow(const QDate &date)
{
    // one day
    if (daysToShow == 24)
    {
        firstDate = date;
        userDaysToFirst = userDateOfBirth.daysTo(firstDate);
        currentPos = 11;
    }
    else if (daysToShow == 7)
    {
        firstDate = date.addDays(-date.dayOfWeek() +1);;
        userDaysToFirst = userDateOfBirth.daysTo(firstDate);
        currentPos = date.dayOfWeek()-1;
    }
    else if (daysToShow > 24)
    {
        firstDate = QDate(date.year(), date.month(), 1);
        userDaysToFirst =userDateOfBirth.daysTo(firstDate);
        daysToShow = firstDate.daysInMonth();
        currentPos = date.day()-1;
    }

    calcUsersBioVal();

    currentPosChanged();
}


// ****************************************************************************
void BiorhythmGraphWidget::setPhyBio(bool state)
{
    phyBio = state;
    if (selectedAverage)
    {
        calcUsersBioVal();
        currentPosChanged();
    }
    else
        update();
}

void BiorhythmGraphWidget::setEmoBio(bool state)
{
    emoBio = state;
    if (selectedAverage)
    {
        calcUsersBioVal();
        currentPosChanged();
    }
    else
        update();
}

void BiorhythmGraphWidget::setIntBio(bool state)
{
    intBio = state;
    if (selectedAverage)
    {
        calcUsersBioVal();
        currentPosChanged();
    }
    else
        update();
}

void BiorhythmGraphWidget::setIntuBio(bool state)
{
    intuBio = state;
    if (selectedAverage)
    {
        calcUsersBioVal();
        currentPosChanged();
    }
    else
        update();
}

void BiorhythmGraphWidget::setAverageBio(bool state)
{
    averageBio = state;

    calcUsersBioVal();
    currentPosChanged();
}

void BiorhythmGraphWidget::setSelectedAverageChanged(bool state)
{
    selectedAverage = state;
    calcUsersBioVal();
    update();
}


// ****************************************************************************
void BiorhythmGraphWidget::showToday()
{
    // "reset" to months view

    //today = true;

    // first day in month
    firstDate = QDate::currentDate().addDays(-QDate::currentDate().day()+1);
    Q_ASSERT(firstDate.isValid());
    qDebug() << "showToday(); firstDate:" << firstDate.toString("dd.MM.yyyy");
    currentPos = QDate::currentDate().day()-1;

    // start from "date of birth"
    if ((userDateOfBirth.daysTo(QDate::currentDate()) < firstDate.daysInMonth()) && (userDateOfBirth.month() == firstDate.month()))
    {
        firstDate = userDateOfBirth;
        currentPos = QDate::currentDate().day()-firstDate.day();
    }

    daysToShow = firstDate.daysInMonth();
    userDaysToFirst = userDateOfBirth.daysTo(firstDate);

    calcUsersBioVal();
    currentPosChanged();
    emit signalViewChanged(MONTH);
}

void BiorhythmGraphWidget::back()
{
    if (daysToShow == 24)
    {
        if (firstDate.addDays(-1) < userDateOfBirth)
            return;

        firstDate = firstDate.addDays(-1);
        daysToShow = 24;
    }
    else if (daysToShow == 7)
    {
        if (firstDate.addDays(-7) < userDateOfBirth)
            firstDate = userDateOfBirth;
        else
            firstDate = firstDate.addDays(-7);

        daysToShow = 7;
    }
    else if (daysToShow > 24)
    {
        if (firstDate.addMonths(-1) < userDateOfBirth)
            firstDate = userDateOfBirth;
        else
            firstDate = firstDate.addMonths(-1);

        if (currentPos+1  > firstDate.daysInMonth())
            currentPos = firstDate.daysInMonth()-1;

        daysToShow = firstDate.daysInMonth();
    }

    userDaysToFirst = userDateOfBirth.daysTo(firstDate);

    calcUsersBioVal();
    currentPosChanged();
}

void BiorhythmGraphWidget::forward()
{
    if (daysToShow == 24)
    {
        firstDate = firstDate.addDays(1);
        daysToShow = 24;
    }
    else if (daysToShow == 7)
    {
        firstDate = firstDate.addDays(7);
        daysToShow = 7;
    }
    else if (daysToShow > 24)
    {
        firstDate = firstDate.addMonths(1);
        if (currentPos+1  > firstDate.daysInMonth())
            currentPos = firstDate.daysInMonth()-1;
        daysToShow = firstDate.daysInMonth();
    }

    userDaysToFirst = userDateOfBirth.daysTo(firstDate);

    calcUsersBioVal();
    currentPosChanged();
}

void BiorhythmGraphWidget::backDay()
{
    if (firstDate.addDays(-1) < userDateOfBirth)
        return;

    firstDate = firstDate.addDays(-1);
    userDaysToFirst = userDateOfBirth.daysTo(firstDate);

    calcUsersBioVal();
    currentPosChanged();
}

void BiorhythmGraphWidget::forwardDay()
{
    firstDate = firstDate.addDays(1);
    userDaysToFirst = userDateOfBirth.daysTo(firstDate);

    calcUsersBioVal();
    currentPosChanged();
}


// ****************************************************************************

/*
void BiorhythmGraphWidget::changeView()
{
    if (daysToShow == 24)
        changeView(MONTH);
    else if (daysToShow == 7)
        changeView(DAY);
    else if (daysToShow > 24)
        changeView(WEEK);
}
*/

// change current biorhythm view to newView
void BiorhythmGraphWidget::changeView(int newView) // default = -1
{
    // if default, if function was called without arguments change view to next view "day -> month -> week"
    if (newView == -1)
    {
        if (daysToShow == 24)
            newView = MONTH;
        else if (daysToShow == 7)
            newView = DAY;
        else if (daysToShow > 24)
            newView = WEEK;
    }

    // block "changes" to self (month -> month, week -> week, day -> day)
    if ((newView == MONTH && daysToShow > 24) || (newView == WEEK && daysToShow == 7) || (newView == DAY && daysToShow == 24))
        return;

    qDebug() << "changeView to newView:" << newView;

    // day -> month
    // week -> month
    // day -> week
    // month -> week
    // week -> day
    // month -> day

    // to month
    if (newView == MONTH)
    {
        // start from "date of birth"
        if (userDateOfBirth.daysTo(firstDate.addDays(currentPos)) <= firstDate.addDays(currentPos).daysInMonth())
        {
            firstDate = userDateOfBirth;
            currentPos = 0;
        }
        // day to month
        else if (daysToShow == 24)
        {
            currentPos = firstDate.day()-1;
            firstDate = firstDate.addDays(-firstDate.day()+1);
        }
        // week to month
        else if (daysToShow == 7)
        {
            int tmp = currentPos;
            currentPos = firstDate.addDays(currentPos).day()-1;
            firstDate = QDate(firstDate.addDays(tmp).year(), firstDate.addDays(tmp).month(), 1);
        }

        daysToShow = firstDate.daysInMonth();
        userDaysToFirst = userDateOfBirth.daysTo(firstDate);

        calcUsersBioVal();
        currentPosChanged();
        emit signalViewChanged(MONTH);
    }
    // to week
    else if (newView == WEEK)
    {
        // start from "date of birth"
        if (userDateOfBirth.daysTo(firstDate.addDays(currentPos)) < firstDate.addDays(currentPos).dayOfWeek())
        {
            firstDate = userDateOfBirth;
            currentPos = 0;
        }
        // day to week
        else if (daysToShow == 24)
        {
            currentPos = firstDate.dayOfWeek()-1;
            firstDate = firstDate.addDays(-firstDate.dayOfWeek() +1);
        }
        // month to week
        else if (daysToShow > 24)
        {
            int tmp = currentPos;
            currentPos = firstDate.addDays(tmp).dayOfWeek()-1;
            firstDate = firstDate.addDays(tmp - firstDate.addDays(tmp).dayOfWeek() +1);
        }

        daysToShow = 7;
        userDaysToFirst = userDateOfBirth.daysTo(firstDate);

        calcUsersBioVal();
        currentPosChanged();
        emit signalViewChanged(WEEK);
    }
    // to day
    else if (newView == DAY)
    {
        // if week or month change to day
        if (daysToShow == 7 || daysToShow > 24)
        {
            firstDate = firstDate.addDays(currentPos);
            currentPos = 11;
            daysToShow = 24;
            userDaysToFirst = userDateOfBirth.daysTo(firstDate);

            calcUsersBioVal();
            currentPosChanged();
            emit signalViewChanged(DAY);
        }
    }
}


// ****************************************************************************
void BiorhythmGraphWidget::currentPosChanged(int pos)
{
    if (daysToShow == 24)
    {
        emitBioValues(pos);
        emit signalDate(firstDate);
        emit signalHour(pos+1);
        emit signalUserDays(userDaysToFirst);
    }
    else
    {
        emitBioValues(pos);
        emit signalDate(firstDate.addDays(pos));
        emit signalHour(0);
        emit signalUserDays(userDaysToFirst + pos);
    }

    update();
}

void BiorhythmGraphWidget::currentPosChanged()
{
    if (daysToShow == 24)
    {
        emitBioValues(currentPos);
        emit signalDate(firstDate);
        emit signalHour(currentPos+1);
        emit signalUserDays(userDaysToFirst);
    }
    else
    {
        emitBioValues(currentPos);
        emit signalDate(firstDate.addDays(currentPos));
        emit signalHour(0);
        emit signalUserDays(userDaysToFirst + currentPos);
    }

    update();
}


// ****************************************************************************
// "interface"
void BiorhythmGraphWidget::setZero(bool state)
{
    zero = state;
    update();
}

void BiorhythmGraphWidget::setToday(bool state)
{
    today = state;
    update();
}

void BiorhythmGraphWidget::setHLines(bool state)
{
    hLines = state;
    update();
}

void BiorhythmGraphWidget::setVLines(bool state)
{
    vLines = state;
    update();
}

void BiorhythmGraphWidget::showAxisVLabels(bool state)
{
    axisVLabels = state;
    update();
}

void BiorhythmGraphWidget::showAxisHLabels(bool state)
{
    axisHLabels = state;
    update();
}

void BiorhythmGraphWidget::setHLinesDelta(QString delta)
{
    bool ok;
    quint32 tmp = delta.toUInt(&ok, 10);

    if (ok && (5<=tmp) && (tmp<=100))
        hLinesDelta = tmp;//static_cast<quint8>(tmp);
    else
        hLinesDelta = 25;

    update();
}

void BiorhythmGraphWidget::setAntialiasing(bool state)
{
    antialiasing = state;
    update();
}

void BiorhythmGraphWidget::setBackground(bool state)
{
    setAutoFillBackground(state);
}

void BiorhythmGraphWidget::setBackground(const QColor &color)
{
    QPalette p = palette();
    p.setColor(QPalette::Window, color);
    setPalette(p);
}

void BiorhythmGraphWidget::setPenWidth(double width)
{
    if (width == 0) // if "Default" width
        penWidth = 1.5; // 1.5 - default width
    else
        penWidth = width;
    update();
}

void BiorhythmGraphWidget::setShowToolTip(bool state)
{
    showToolTip = state;
}

void BiorhythmGraphWidget::setMouseFollow(bool state)
{
    setMouseTracking(state);
}

void BiorhythmGraphWidget::setMouseWheel(bool state)
{
    mouseWheel = state;
}

void BiorhythmGraphWidget::setReverseScroll(bool state)
{
    reverseScroll = state;
}

void BiorhythmGraphWidget::setPhyColor(QColor color)
{
    phyColor = color;
    update();
}

void BiorhythmGraphWidget::setEmoColor(QColor color)
{
    emoColor = color;
    update();
}

void BiorhythmGraphWidget::setIntColor(QColor color)
{
    intColor = color;
    update();
}

void BiorhythmGraphWidget::setIntuColor(QColor color)
{
    intuColor = color;
    update();
}

void BiorhythmGraphWidget::setAverageColor(QColor color)
{
    averageColor = color;
    update();
}
