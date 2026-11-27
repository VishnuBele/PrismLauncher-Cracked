// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (C) 2026 Prism Launcher contributors
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "AuroraTheme.h"

#include <QFile>
#include <QObject>

#include "ThemeManager.h"

QString AuroraTheme::id()
{
    return QStringLiteral("aurora");
}

QString AuroraTheme::name()
{
    return QObject::tr("Aurora");
}

QString AuroraTheme::tooltip()
{
    return QObject::tr("Modern dark theme with an animated home background.");
}

bool AuroraTheme::hasStyleSheet()
{
    return true;
}

QString AuroraTheme::appStyleSheet()
{
    QFile styleSheet(QStringLiteral(":/themes/aurora.qss"));
    if (!styleSheet.open(QIODevice::ReadOnly | QIODevice::Text)) {
        themeWarningLog() << "Could not load the Aurora stylesheet";
        return {};
    }
    return QString::fromUtf8(styleSheet.readAll());
}

QPalette AuroraTheme::colorScheme()
{
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(14, 17, 23));
    palette.setColor(QPalette::WindowText, QColor(231, 236, 245));
    palette.setColor(QPalette::Base, QColor(21, 26, 35));
    palette.setColor(QPalette::AlternateBase, QColor(27, 33, 44));
    palette.setColor(QPalette::ToolTipBase, QColor(27, 33, 44));
    palette.setColor(QPalette::ToolTipText, QColor(231, 236, 245));
    palette.setColor(QPalette::Text, QColor(231, 236, 245));
    palette.setColor(QPalette::Button, QColor(28, 36, 49));
    palette.setColor(QPalette::ButtonText, QColor(231, 236, 245));
    palette.setColor(QPalette::BrightText, Qt::white);
    palette.setColor(QPalette::Link, QColor(143, 176, 255));
    palette.setColor(QPalette::Highlight, QColor(58, 91, 224));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::PlaceholderText, QColor(231, 236, 245, 120));
    palette.setColor(QPalette::Light, QColor(44, 54, 72));
    palette.setColor(QPalette::Midlight, QColor(36, 45, 60));
    palette.setColor(QPalette::Dark, QColor(9, 12, 17));
    palette.setColor(QPalette::Mid, QColor(18, 23, 31));
    palette.setColor(QPalette::Shadow, QColor(5, 7, 10));
    return fadeInactive(palette, fadeAmount(), fadeColor());
}

double AuroraTheme::fadeAmount()
{
    return 0.45;
}

QColor AuroraTheme::fadeColor()
{
    return QColor(14, 17, 23);
}

QString AuroraTheme::qtTheme()
{
    return QStringLiteral("Fusion");
}
