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

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

namespace constants {
    static const QString settingsFileName = "/settings.ini";
    static const QString userAvatar = ":/avatars/00.png";
    // TODO - change file noUserAvatar
    static const QString noUserAvatar = ":/avatars/00.png";
    static const QString daysList[7] = {"Poniedziałek", "Wtorek", "Środa", "Czwartek", "Piątek", "Sobota", "Niedziela"};
    static const quint8 maxUsers = 20;
    static const quint16 minUID = 10000;
    static const quint16 maxUID = 30000;
}

#endif // CONSTANTS_HPP
