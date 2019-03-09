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

#include "usereditwidget.hpp"
#include "examplesavatarsdialog.hpp"

#include <QStandardPaths>
#include <QCalendarWidget>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>

#include <QDebug>

UserEditWidget::UserEditWidget(QWidget *parent) :
    QWidget(parent)
{
    ui.setupUi(this);

    ui.dateEdit->setMaximumDate(QDate::currentDate());
    ui.calendarWidget->setMaximumDate(QDate::currentDate());

    //QTextCharFormat format = ui.calendarWidget->weekdayTextFormat(Qt::Saturday);
    //format.setForeground(QBrush(Qt::black, Qt::SolidPattern));
    //ui.calendarWidget->setWeekdayTextFormat(Qt::Saturday, format);

    connect(ui.label, SIGNAL(mouseClicked()), this, SLOT(onImport()));
    connect(ui.pushButtonImport, SIGNAL(clicked()), this, SLOT(onImport()));
    connect(ui.pushButtonDelete, SIGNAL(clicked()), this, SLOT(onDelete()));
    connect(ui.labelAvatars, SIGNAL(mouseClicked()), this, SLOT(onExamplesAvatars()));
    connect(ui.checkBox, SIGNAL(toggled(bool)), ui.labelStar, SLOT(setVisible(bool)));
    connect(ui.pushButtonSave, SIGNAL(clicked()), this, SLOT(onSave()));
    connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(onCancel()));

    connect(ui.dateEdit, SIGNAL(dateChanged(QDate)), ui.calendarWidget, SLOT(setSelectedDate(QDate)));
    connect(ui.calendarWidget, SIGNAL(activated(QDate)), ui.dateEdit, SLOT(setDate(QDate)));
    connect(ui.calendarWidget, SIGNAL(clicked(QDate)), ui.dateEdit, SLOT(setDate(QDate)));

    pictureFile = constants::userAvatar;
    newPictureImported = false;
    newPictureFile.clear();
    oldPictureDelete = false;

    homePath = QDir::homePath();
}

void UserEditWidget::setData(const QString &name, const QDate &date, const QString &avatar, const bool defaultUser, const quint16 userUID)
{
    if (userUID == 0)
        onCancel();

    ui.lineEdit->setText(name);//name.isEmpty() ? "Użytkownik" : name);
    ui.dateEdit->setDate(date);
    ui.checkBox->setChecked(defaultUser);
    ui.labelStar->setVisible(defaultUser);
    ui.label->setPixmap(QPixmap(avatar.isEmpty() ? constants::userAvatar : avatar));

    if (avatar == constants::userAvatar || avatar.isEmpty())
        ui.pushButtonDelete->setDisabled(true);
    else
        ui.pushButtonDelete->setEnabled(true);

    pictureFile = avatar;
    oldPictureDelete = false;
    newPictureImported = false;
    newPictureFile.clear();

    this->userUID = userUID;

    ui.lineEdit->setFocus();
    ui.pushButtonSave->setDefault(true);
}

void UserEditWidget::onImport()
{
    QString filter = "Pliki obrazów (*.png *.jpg *.jpeg *.gif *.bmp *.tif *.tiff)",
            selectedFilter = filter,
            newFile;

    // obtain new avatar file
    for (;;)
    {
        newFile = QFileDialog::getOpenFileName(this, "Wybierz plik awatara",
                                                     homePath,
                                                     filter,
                                                     &selectedFilter,
                                                     QFileDialog::ReadOnly);

        // Cancel
        if (newFile.isEmpty())
            return;

        QFileInfo fInfo(newFile);

        if (!fInfo.exists())
        {
            QMessageBox::warning(this,
                                 QApplication::applicationName(),
                                 "Plik nie istnieje?",
                                 QMessageBox::Ok);

            homePath = fInfo.path();

            continue;
        }

        if ((fInfo.suffix().compare("png", Qt::CaseInsensitive) != 0) && (fInfo.suffix().compare("jpg", Qt::CaseInsensitive) != 0) &&
            (fInfo.suffix().compare("jpeg", Qt::CaseInsensitive) != 0) && (fInfo.suffix().compare("gif", Qt::CaseInsensitive) != 0) &&
            (fInfo.suffix().compare("bmp", Qt::CaseInsensitive) != 0) && (fInfo.suffix().compare("tif", Qt::CaseInsensitive) != 0) &&
            (fInfo.suffix().compare("tiff", Qt::CaseInsensitive) != 0))
        {
            QMessageBox::warning(this,
                                 QApplication::applicationName(),
                                 "Wybrany plik nie jest jednym z dozwolonych typów.",
                                 QMessageBox::Ok);

            homePath = fInfo.path();

            continue;
        }

        if (fInfo.size() == 0)
        {
            QMessageBox::warning(this,
                                 QApplication::applicationName(),
                                 "Wybrany plik jest pusty.",
                                 QMessageBox::Ok);

            homePath = fInfo.path();

            continue;
        }

        //if (fInfo.size() > 48*1024)
        //{
        //    QMessageBox::warning(this,
        //                         QApplication::applicationName(),
        //                         "Wybrany plik jest za duży. Proszę wybrać mniejszy. Maksymalny rozmiar to 48KB.",
        //                         QMessageBox::Ok);
        //    homePath = fInfo.path();
        //    continue;
        //}

        QPixmap pixmap(newFile);
        if (pixmap.isNull())
        {
            QMessageBox::warning(this,
                                 QApplication::applicationName(),
                                 "Wybrany plik nie jest prawidłowym plikiem graficznym.",
                                 QMessageBox::Ok);

            homePath = fInfo.path();

            continue;
        }

        if (pixmap.height() != 48 || pixmap.width() != 48)
        {
            homePath = fInfo.path();

            QMessageBox::StandardButton reply;

            reply = QMessageBox::warning(this,
                                         QApplication::applicationName(),
                                         "Wybrany plik posiada nieodpowiednią rozdzielczość. Zalecana rozdzielczość to 48x48 pikseli.",
                                         QMessageBox::Retry | QMessageBox::Ignore | QMessageBox::Cancel);

            if (reply == QMessageBox::Retry)
                continue;
            else if (reply == QMessageBox::Ignore)
                break;
            else
                return;  // reply == QMessageBox::Cancel
        }

        break;
    }

    newPictureImported = true;
    newPictureFile = newFile;
    ui.label->setPixmap(QPixmap(newPictureFile));

    ui.pushButtonDelete->setEnabled(true);
}

void UserEditWidget::onExamplesAvatars()
{
    ExamplesAvatarsDialog dlg(newPictureImported ? newPictureFile : pictureFile, this);
    dlg.exec();

    // if new avatar selected
    if (!dlg.getAvatarFileName().isEmpty())
    {
        newPictureImported = true;
        newPictureFile = dlg.getAvatarFileName();

        if (dlg.getAvatarFileName() == constants::userAvatar)
            ui.pushButtonDelete->setDisabled(true);
        else
            ui.pushButtonDelete->setEnabled(true);

        ui.label->setPixmap(QPixmap(newPictureFile));
    }
}

void UserEditWidget::onDelete()
{
    oldPictureDelete = true;

    ui.label->setPixmap(QPixmap(constants::userAvatar));

    newPictureImported = false;
    newPictureFile.clear();

    ui.pushButtonDelete->setDisabled(true);
}

void UserEditWidget::onCancel()
{
    emit signalCancel();
}

void UserEditWidget::onSave()
{
    if (newPictureImported)
    {
        // if avatar is from program resources
        if (newPictureFile.startsWith(":/avatars/"))
        {
            pictureFile = newPictureFile;
            emit signalSave();
            return;
        }

        // prepare destination folder

        QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/avatars/");

        // if destination folder doesn't exist then create
        if (!dir.exists())
            // if can't create destination folder
            if (!dir.mkdir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/avatars/")) //QApplication::applicationDirPath()+"/avatars/")) for portable version?
            {
                QMessageBox::critical(this,
                                      QApplication::applicationName(),
                                      QString("Nie mogę utworzyć folderu dla avatarów: \n%1\n\nSprawdź czy posiadasz odpowiednie prawa do zapisu.").arg(QDir::toNativeSeparators(dir.absolutePath())));
                return;
            }

        // if destination folder is not readable
        if (!dir.isReadable())
        {
            QMessageBox::critical(this,
                                  QApplication::applicationName(),
                                  QString("Brak dostępu do folderu dla avatarów: \n%1\n\nSprawdź czy posiadasz odpowiednie prawa do zapisu.").arg(QDir::toNativeSeparators(dir.absolutePath())));
            return;
        }

        // if destination folder is OK
        // copy source (new) file

        // create new file name from UID (user ID)
        QFile sourceFile(newPictureFile);
        QFileInfo fileInfo(sourceFile);
        QString destinationFileName(dir.absolutePath()+"/"+QString::number(userUID)+"."+fileInfo.suffix());

        /*
         * bool QFile::copy(const QString & newName)
         *
         * Copies the file currently specified by fileName() to a file called newName.
         * Returns true if successful; otherwise returns false.
         *
         * Note that if a file with the name newName already exists, copy() returns false (i.e. QFile will not overwrite it).
         *
         * The source file is closed before it is copied.
         */

        // if destination file doesn't exist then copy new file
        if (!QFile(destinationFileName).exists())
        {
            if (!sourceFile.copy(destinationFileName))
            {
                QMessageBox::critical(this,
                                      QApplication::applicationName(),
                                      QString("Nie mogę skopiować pliku avatara:\n%1").arg(QDir::toNativeSeparators(destinationFileName)));
                return;
            }

        }
        // if destination file exists try to delete them
        else
        {
            for (;;)
            {
                // if deleting fails
                if (!QFile::remove(destinationFileName))
                {
                    QMessageBox::StandardButton reply;
                    reply = QMessageBox::critical(this,
                                                  QApplication::applicationName(),
                                                  QString("Nie mogę usunąć poprzedniego pliku avatara:\n%1").arg(QDir::toNativeSeparators(destinationFileName)),
                                                  QMessageBox::Retry | QMessageBox::Abort);

                    if (reply == QMessageBox::Retry)
                        continue; // try one more time to delete
                    else if (reply == QMessageBox::Abort)
                        break;
                }
                // if destination file was deleted then copy new file
                else
                {
                    if (!sourceFile.copy(destinationFileName))
                    {
                        QMessageBox::critical(this,
                                              QApplication::applicationName(),
                                              QString("Nie mogę skopiować pliku avatara:\n%1").arg(QDir::toNativeSeparators(destinationFileName)));
                        return;
                    }
                    break;
                }
            }
        }

        pictureFile = destinationFileName;
    }
    else
    {
        // if restore default avatar

        if (oldPictureDelete && pictureFile.startsWith(":/avatars/"))
        {
            pictureFile = newPictureFile;
            emit signalSave();
            return;
        }

        if (oldPictureDelete && (!pictureFile.isEmpty()))
        {
            for (;;)
            {
                if (!QFile::remove(pictureFile))
                {
                    QMessageBox::StandardButton reply;
                    reply = QMessageBox::critical(this,
                                                  QApplication::applicationName(),
                                                  QString("Nie można było skasować pliku avatara:\n%1").arg(QDir::toNativeSeparators(pictureFile)),
                                                  QMessageBox::Retry | QMessageBox::Abort | QMessageBox::Ignore);

                    if (reply == QMessageBox::Abort)
                    {
                        emit signalSave();

                        return;
                    }
                    else if (reply == QMessageBox::Retry)
                        continue;
                }

                // if reply == QMessageBox::Ignore
                // or file successful removed
                pictureFile = constants::userAvatar;
                break;
            }
        }
    }

    emit signalSave();
}
