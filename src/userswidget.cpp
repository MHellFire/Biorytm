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

#include "userswidget.hpp"
#include "constants.hpp"

#include <QApplication>
#include <QtGui>
#include <QMessageBox>
#include <QMenu>
#include <QObjectList>
#include <QFileDialog>

#include <QDebug>

UsersWidget::UsersWidget(QWidget *parent) :
    QWidget(parent)
{
}

UsersWidget::~UsersWidget()
{
}

void UsersWidget::addUsers(QList<User> *usersL)
{
    usersList.clear();

    QString avatar;

    for (int i = 0; i < usersL->count(); ++i)
    {
        avatar = usersL->at(i).avatar;
        QFile avatarFile(avatar);

        avatarFile.exists() ? avatar : avatar = constants::userAvatar;

        UserFrame *frame = new UserFrame(usersL->at(i).uid,
                                         usersL->at(i).name,
                                         usersL->at(i).dateOfBirth,
                                         avatar,
                                         usersL->at(i).defaultUser,
                                         this);

        frame->setAttribute(Qt::WA_DeleteOnClose);

        frame->move(0, ((i*UserFrame::getUserFrameHeight())+(i*spacing)));
        frame->show();

        usersList.append(frame);
    }

    // minimum height, we need this for scroll area
    setMinimumHeight((usersList.count()*UserFrame::getUserFrameHeight())+(usersList.count()*spacing));

    // custom context menu setup
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenu(QPoint)));

    // drag&drop setup
    setAcceptDrops(true);
    //dragTimer.setSingleShot(true);
    //dragTimer.setInterval(QApplication::startDragTime()); // The default value is 500 ms.
    //connect(&dragTimer, SIGNAL(timeout()), this, SLOT(changeDragCursor()));
}

bool UsersWidget::getUsers(QList<User> *usersL)
{
    usersL->clear();

    User user;

    for (int i = 0; i < usersList.count(); ++i)
    {
        user.uid = usersList.at(i)->getUserUID();
        user.name = usersList.at(i)->getName();
        user.dateOfBirth = usersList.at(i)->getDate();
        user.avatar = usersList.at(i)->getAvatar();
        user.defaultUser = usersList.at(i)->isDefault();

        usersL->append(user);
    }

    return true;
}

void UsersWidget::keyPressEvent(QKeyEvent *e)
{
    // UP KEY pressed
    if (e->key() == Qt::Key_Up)
    {
        for (int i = 0; i < usersList.count(); ++i)
            if (usersList.at(i)->isSelected())
            {
                // if first user/frame on list
                if (i == 0)
                    break;

                // select user above
                usersList.at(i)->select(false);
                usersList.at(i-1)->select(true);

                userUID = usersList.at(i-1)->getUserUID();
                name = usersList.at(i-1)->getName();
                date = usersList.at(i-1)->getDate();
                avatar = usersList.at(i-1)->getAvatar();
                defaultUser = usersList.at(i-1)->isDefault();

                emit signalUserSelected(true);
                emit scrollToY(usersList.at(i-1)->pos().y()-spacing);

                break;
            }
    }

    // DOWN KEY pressed
    if (e->key() == Qt::Key_Down)
    {
        for (int i = 0; i < usersList.count(); ++i)
            if (usersList.at(i)->isSelected())
            {
                // if last user/frame on list
                if (i == usersList.count()-1)
                    break;

                // select user below
                usersList.at(i)->select(false);
                usersList.at(i+1)->select(true);

                userUID = usersList.at(i+1)->getUserUID();
                name = usersList.at(i+1)->getName();
                date = usersList.at(i+1)->getDate();
                avatar = usersList.at(i+1)->getAvatar();
                defaultUser = usersList.at(i+1)->isDefault();

                emit signalUserSelected(true);
                emit scrollToY(usersList.at(i+1)->pos().y()-UserFrame::getUserFrameHeight());

                break;
            }
    }

    QWidget::keyPressEvent(e);
}

void UsersWidget::mousePressEvent(QMouseEvent *e)
{
    UserFrame *child = qobject_cast<UserFrame*>(childAt(e->pos()));

    // if no user clicked
    if (!child)
    {
        for (int i = 0; i < usersList.count(); ++i)
        {
            usersList.at(i)->select(false);
        }

        emit signalUserSelected(false);

        dragStart = false;

        return;
    }

    if (e->type() == QEvent::MouseButtonPress)
    {
        // select current user (select clicked user)
        for (int i = 0; i < usersList.count(); ++i)
        {
            usersList.at(i)->select(false);
        }

        child->select(true);

        userUID = child->getUserUID();
        name = child->getName();
        date = child->getDate();
        avatar = child->getAvatar();
        defaultUser = child->isDefault();

        emit signalUserSelected(true);
    }

    // if left mouse button clicked prepare to drag
    if (e->button() == Qt::LeftButton)
    {
        dragStart = true;
        dragStartPosition = e->pos();
        //dragTimer.start();
    }
}
/*
void UsersWidget::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);

    if (dragTimer.isActive())
    {
        dragTimer.stop();

        return;
    }
}
*/
void UsersWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->type() == QEvent::MouseButtonDblClick)
    {
        UserFrame *child = qobject_cast<UserFrame*>(childAt(e->pos()));

        if (!child)
            return;

        emit userChosen();
    }
}

void UsersWidget::mouseMoveEvent(QMouseEvent *e)
{
    // ! mouse move event is only for drag&drop

    if (!(e->buttons() & Qt::LeftButton))
        return;

    if (!dragStart)
        return;

    /*
     *  QApplication::startDragDistance()
     *  The default value (if the platform doesn't provide a different default) is 10 pixels.
     */

    if ((e->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
        return;

    UserFrame *child = qobject_cast<UserFrame*>(childAt(dragStartPosition));

    if (!child)
        return;

    /*
     *  QPoint hotSpot = event->pos() - child->pos();
     *  QMimeData *mimeData = new QMimeData;
     *  mimeData->setText(child->text());
     *  mimeData->setData("application/x-hotspot",
     *                    QByteArray::number(hotSpot.x()) + " " + QByteArray::number(hotSpot.y()));
     */

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-biorhythmitemdata", QByteArray());

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);

    // make pixmap for drag
    UserFrame tmpFrame(child->getUserUID(), child->getName(), child->getDate(), child->getAvatar(), child->isDefault(), this);
    QPixmap tmpPixmap(tmpFrame.size());
    tmpPixmap.fill(Qt::transparent);
    QPainter painter(&tmpPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(0.8);
    tmpFrame.paint(painter);
    painter.end();

    drag->setPixmap(tmpPixmap);
    drag->setHotSpot(QPoint(dragStartPosition - child->pos())); // OR CENTER tmpPixmap.width()*0.5, tmpPixmap.height()*0.5));

    child->hide();

    if (drag->exec() == Qt::MoveAction)
    {
        // at users list
        child->show();
    }
    else
    {
        // outside users list
        deleteUser();
        child->show();
    }

    // unsetCursor();
}

void UsersWidget::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasFormat("application/x-biorhythmitemdata"))
    {
        if (e->source() == this)
        {
            e->setDropAction(Qt::MoveAction);
            e->accept();
        }
        else
        {
            e->acceptProposedAction();
        }
    }
    else
    {
        e->ignore();
    }
}

void UsersWidget::dragMoveEvent(QDragMoveEvent *e)
{
    if (e->mimeData()->hasFormat("application/x-biorhythmitemdata"))
    {
        if (e->source() == this)
        {
            // search index of hidden user (empty frame == hidden user == moved user)
            int dragUserIndex = 0;
            bool foundDragUserIndex = false;
            for (; dragUserIndex < usersList.count(); ++dragUserIndex)
                if (usersList.at(dragUserIndex)->isHidden())
                {
                    foundDragUserIndex = true;
                    break;
                }
            if (!foundDragUserIndex)
                return;

            /*
            // search index of mouse pointed user, inclusive hidden user, for scrollArea
            int indexAll = 0;
            bool foundIndexAll = false;
            for (; indexAll < usersList.count(); ++indexAll)
            {
                if (usersList.at(indexAll)->frameGeometry().contains(e->pos()))
                {
                    foundIndexAll = true;
                    break;
                }
            }
            if (foundIndexAll)
            */

            // search index of mouse pointed user
            int index = 0;
            bool foundIndex = false;
            for (; index < usersList.count(); ++index)
                if (usersList.at(index)->isVisible())
                {
                    if (usersList.at(index)->frameGeometry().contains(e->pos()))
                    {
                        foundIndex = true;
                        break;
                    }
                }
            if (!foundIndex)
                return;

            //
            if ((e->pos().y()) < (usersList.at(index)->pos().y()+(UserFrame::getUserFrameHeight()/2)))
            {
                if (index == 0)
                {
                    UserFrame *frame = usersList.takeAt(dragUserIndex);
                    usersList.insert(index, frame);
                }
                else
                {
                    if (usersList.at(index-1)->isVisible())
                    {
                        UserFrame *frame = usersList.takeAt(dragUserIndex);
                        usersList.insert(index, frame);
                    }
                }
            }
            else if ((e->pos().y()) > (usersList.at(index)->pos().y()+(UserFrame::getUserFrameHeight()/2)))
            {
                if (index == usersList.count()-1)
                {
                    UserFrame *frame = usersList.takeAt(dragUserIndex);
                    usersList.insert(index, frame);
                }
                else
                {
                    if (usersList.at(index+1)->isVisible())
                    {
                        UserFrame *frame = usersList.takeAt(dragUserIndex);
                        usersList.insert(index, frame);
                    }
                }
            }

            for (int i = 0; i < usersList.count(); ++i)
                usersList.at(i)->move(0, (i*UserFrame::getUserFrameHeight())+(i*spacing));

            e->setDropAction(Qt::MoveAction);
            e->accept();
        }
        else
        {
            e->acceptProposedAction();
        }
    }
    else
    {
        e->ignore();
    }
}

void UsersWidget::dropEvent(QDropEvent *e)
{
    if (e->mimeData()->hasFormat("application/x-biorhythmitemdata"))
    {
        /*
         *  if (event->mimeData()->hasText())
         *  {
         *      const QMimeData *mime = event->mimeData();
         *      QStringList pieces = mime->text().split(QRegExp("\\s+"), QString::SkipEmptyParts);
         *      QPoint position = event->pos();
         *      QPoint hotSpot;
         *
         *      QList<QByteArray> hotSpotPos = mime->data("application/x-hotspot").split(' ');
         *      if (hotSpotPos.size() == 2)
         *      {
         *          hotSpot.setX(hotSpotPos.first().toInt());
         *          hotSpot.setY(hotSpotPos.last().toInt());
         *      }
         *  ...
         */

        if (e->source() == this)
        {
            e->setDropAction(Qt::MoveAction);
            e->accept();
        }
        else
        {
            e->acceptProposedAction();
        }
    }
    else
    {
        e->ignore();
    }
}

void UsersWidget::deleteUser()
{
    // do you want to delete user?
    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                              QCoreApplication::applicationName(),
                                                              "Czy na pewno chcesz usunąć użytkownika?",
                                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (reply == QMessageBox::No)
        return;

    // search selected user
    int i = 0;
    bool foundUser = false;

    for (; i < usersList.count(); ++i)
        if (usersList.at(i)->isSelected())
        {
            foundUser = true;
            break;
        }

    // if no user selected
    if (!foundUser)
        return;

    // delete avatar used by this user
    if ((!usersList.at(i)->getAvatar().isEmpty()) && (usersList.at(i)->getAvatar() != constants::userAvatar))
        for(;;)
        {
            if (!QFile::remove(usersList.at(i)->getAvatar()))
            {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::critical(this,
                                              QCoreApplication::applicationName(),
                                              QString("Nie można było usunąć pliku avatara:\n%1").arg(QDir::toNativeSeparators(usersList.at(i)->getAvatar())),
                                              QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore);

                if (reply == QMessageBox::Abort)
                    return;
                else if (reply == QMessageBox::Retry)
                    continue;
                else
                    break;  // if (replay == QMessageBox::Ignore)
            }

            break;
        }

    // delete user
    usersList.at(i)->close();
    usersList.removeAt(i);

    // redraw users
    for (int i = 0; i < usersList.count(); ++i)
    {
        usersList.at(i)->move(0, (i*UserFrame::getUserFrameHeight())+(i*spacing));
        usersList.at(i)->show();
    }

    setMinimumHeight((usersList.count()*UserFrame::getUserFrameHeight())+(usersList.count()*spacing));

    emit signalUserSelected(false);

    // then select next user
    if (usersList.count() == 0)
    {
        userUID = 0;

        return;
    }

    i == usersList.count() ? --i : i;

    usersList.at(i)->select(true);

    userUID = usersList.at(i)->getUserUID();
    name = usersList.at(i)->getName();
    date = usersList.at(i)->getDate();
    avatar = usersList.at(i)->getAvatar();
    defaultUser = usersList.at(i)->isDefault();

    emit signalUserSelected(true);
}

void UsersWidget::onContextMenu(const QPoint &pos)
{
    UserFrame *child = qobject_cast<UserFrame*>(childAt(pos));

    // if no user selected
    if (!child)
    {
        QMenu menu("Menu", this);
        menu.addAction("Dodaj...", this, SIGNAL(addUserSignal()), Qt::Key_Insert);
        menu.exec(mapToGlobal(pos));

        return;
    }

    // if user selected
    QMenu menu("Menu", this);

    QAction *a = new QAction(QString("Domyślny"), &menu);
    a->setCheckable(true);
    a->setChecked(child->isDefault());
    menu.addAction(a);
    menu.addSeparator();

    menu.addAction("Edytuj...", this, SIGNAL(editUser()), Qt::Key_F2);
    menu.addAction("Usuń", this, SLOT(deleteUser()), Qt::Key_Delete);

    QAction *action = menu.exec(mapToGlobal(pos));

    if (action == 0)
    {
        return;
    }

    // kubuntu 17.10 - KDE Plasma? fix
    // remove("&")
    if (action->text().remove("&") == "Domyślny")
    {
        if (!child->isDefault())
        {
            for (int i = 0; i < usersList.count(); ++i)
                usersList.at(i)->setDefault(false);

            child->setDefault(true);
        }
        else
        {
            child->setDefault(false);
        }

        userUID = child->getUserUID();
        name = child->getName();
        date = child->getDate();
        avatar = child->getAvatar();
        defaultUser = child->isDefault();
    }
}

int UsersWidget::getSelectedY() const
{
    for (int i = 0; i < usersList.count(); ++i)
        if (usersList.at(i)->isSelected())
            return usersList.at(i)->pos().y()-UserFrame::getUserFrameHeight()-spacing;

    return 0;
}

void UsersWidget::addUser(quint16 userUID, const QString &name, const QDate &date, const QString &avatar, const bool defaultUser)
{
    // check if users limit reached (MAX_USERS)
    if (maxUsersLimit())
        return;

    // deselecting current user
    for (int i = 0; i < usersList.count(); ++i)
        usersList.at(i)->select(false);

    // remove the "default"
    if (defaultUser)
        for (int i = 0; i < usersList.count(); ++i)
            usersList.at(i)->setDefault(false);

    // adding new user
    UserFrame *newUserFrame = new UserFrame(userUID, name, date, avatar, defaultUser, this);
    newUserFrame->setAttribute(Qt::WA_DeleteOnClose);
    newUserFrame->select(true);

    usersList.append(newUserFrame);

    // redraw users
    for (int i = 0; i < usersList.count(); ++i)
    {
        usersList.at(i)->move(0, (i*UserFrame::getUserFrameHeight())+(i*spacing));
        usersList.at(i)->show();
    }

    setMinimumHeight((usersList.count()*UserFrame::getUserFrameHeight())+(usersList.count()*spacing));

    this->userUID = userUID;
    this->name = name;
    this->date = date;
    this->avatar = avatar;
    this->defaultUser = defaultUser;

    emit signalUserSelected(true);
    emit scrollToY(usersList.at(usersList.count()-1)->pos().y());
}

bool UsersWidget::maxUsersLimit()
{
    // check number of users

    if (usersList.count() >= constants::maxUsers)
        return true;

    return false;
}

void UsersWidget::update()
{
    for (int i = 0; i < usersList.count(); ++i)
    {
        if (usersList.at(i)->isSelected())
        {
            emit signalUserSelected(true);
            return;
        }
    }

    emit signalUserSelected(false);
}

void UsersWidget::updateUser(const QString &name, const QDate &date, const QString &avatar, const bool defaultUser)
{
    this->name = name;
    this->date = date;
    this->avatar = avatar;
    this->defaultUser = defaultUser;

    for (int i = 0; i < usersList.count(); ++i)
    {
        if (defaultUser)
            usersList.at(i)->setDefault(false);

        if (usersList.at(i)->isSelected())
        {
            usersList.at(i)->setName(name);
            usersList.at(i)->setDate(date);
            usersList.at(i)->setAvatar(avatar);
            usersList.at(i)->setDefault(defaultUser);

            // if the edited user is not default user we can break "clearing" the default variable
            if (!defaultUser)
                break;
        }
    }

    emit signalUserSelected(true);
}

void UsersWidget::chooseUser(quint16 userUID)
{
    if (usersList.count() == 0)
    {
        emit signalUserSelected(false);

        return;
    }

    /*
     * 1. choose requested user
     * 2. choose default user
     * 3. choose first user on list
     * 4. no user choosen (userSelected == false)
     */

    bool userSelected = false;

    // 1. choose requested user
    if ((userUID >= 10000) && (userUID <=30000))
    {
        for (int i = 0; i < usersList.count(); ++i)
            if (usersList.at(i)->getUserUID() == userUID)
            {
                usersList.at(i)->select(true);
                userSelected = true;

                this->userUID = usersList.at(i)->getUserUID();
                name = usersList.at(i)->getName();
                date = usersList.at(i)->getDate();
                avatar = usersList.at(i)->getAvatar();
                defaultUser = usersList.at(i)->isDefault();

                emit scrollToY(usersList.at(i)->pos().y());

                break;
            }
    }
    else
    {
        // 2. choose default user
        for (int i = 0; i < usersList.count(); ++i)
            if (usersList.at(i)->isDefault())
            {
                usersList.at(i)->select(true);
                userSelected = true;

                this->userUID = usersList.at(i)->getUserUID();
                name = usersList.at(i)->getName();
                date = usersList.at(i)->getDate();
                avatar = usersList.at(i)->getAvatar();
                defaultUser = usersList.at(i)->isDefault();

                emit scrollToY(usersList.at(i)->pos().y());

                break;
            }

        // 3. choose first user on list (if default user doesn't exist)
        if (!userSelected)
        {
            usersList.at(0)->select(true);
            userSelected = true;

            this->userUID = usersList.at(0)->getUserUID();
            name = usersList.at(0)->getName();
            date = usersList.at(0)->getDate();
            avatar = usersList.at(0)->getAvatar();
            defaultUser = usersList.at(0)->isDefault();

            emit scrollToY(usersList.at(0)->pos().y());
        }
    }

    // 4. if no user choosen, userSelected == false

    emit signalUserSelected(userSelected);
}

bool UsersWidget::defaultUserExist()
{
    for (int i = 0; i < usersList.count(); ++i)
        if (usersList.at(i)->isDefault())
            return true;

    return false;
}

quint16 UsersWidget::getUniqueUID()
{
    // returns random UniqueUID <constants::minUID;constants::maxUID>

    quint16 userUID;
    bool repeat;

    QTime time = QTime::currentTime();
    qsrand(time.hour() + time.minute() + time.second() + static_cast<int>(qPow(time.msec(), 2)));

    for (;;)
    {
        repeat = false;
        userUID = static_cast<quint16>(constants::minUID + qrand() / (RAND_MAX + 1.0) * (constants::maxUID-constants::minUID+1));

        Q_ASSERT(userUID >= constants::minUID && userUID <= constants::maxUID);

        for (int i = 0; i < usersList.count(); ++i)
            if (userUID == usersList.at(i)->getUserUID())
            {
                repeat = true;
                break;
            }

        if (!repeat)
            break;
    }

    return userUID;
}

/*
void UsersWidget::changeDragCursor()
{
    setCursor(Qt::ClosedHandCursor);
}
*/


// for testing
void UsersWidget::testGetUniqueUID()
{
    QFile file(QFileDialog::getSaveFileName(this));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QVector<quint16> list;
    list.clear();

    QTime time = QTime::currentTime();
    qsrand(time.hour() + time.minute() + time.second() + static_cast<int>(qPow(time.msec(), 2)));

    for (int i = 0; i < 10000000; ++i)
        list += static_cast<quint16>(constants::minUID + qrand() / (RAND_MAX + 1.0) * (constants::maxUID-constants::minUID+1));

    qSort(list.begin(), list.end());

    QTextStream out(&file);

    for (int i = 0; i < list.size(); ++i)
        out << list.at(i) << "\n";

    file.close();
}
