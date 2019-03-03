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

#include "mainwindow.hpp"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(images);
    Q_INIT_RESOURCE(avatars);
    Q_INIT_RESOURCE(other);

    // Sets whether Qt should use the system's standard colors, fonts, etc., to on.
    // By default, this is true.
    //QApplication::setDesktopSettingsAware(false);

    QApplication a(argc, argv);

    QApplication::setQuitOnLastWindowClosed(false);

    QCoreApplication::setOrganizationName("Helfajer");
    QCoreApplication::setOrganizationDomain("helfajer.info");
    QCoreApplication::setApplicationName("Biorytm");
    QCoreApplication::setApplicationVersion("0.0.1");

    MainWindow w;
    w.show();

    return a.exec();
}
