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

#pragma once

#include <QIcon>
#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QToolButton;
class QVBoxLayout;
class QHBoxLayout;
class NightSkyBackground;

/*!
 * The launcher's main surface: an animated backdrop, a header with the
 * instance overview, search and the primary actions, and the instance cards
 * below it. The instance view itself is supplied by MainWindow so the existing
 * model, drag and drop and context menu behaviour keeps working unchanged.
 */
class HomeSurface : public QWidget {
    Q_OBJECT

   public:
    explicit HomeSurface(QWidget* parent = nullptr);
    ~HomeSurface() override = default;

    /*! Places the instance list in the card area. Takes ownership. */
    void setContent(QWidget* content);

    void setSummary(int instanceCount, const QString& playtime);
    void setAccount(const QString& name, const QIcon& icon);
    void setBackgroundAnimated(bool animated);
    bool isBackgroundAnimated() const;

    void setSearchText(const QString& text);
    void focusSearch();

    /*! Re-applies the translated strings (called on a language change). */
    void retranslateUi();

    /*! Re-applies the palette/font driven look, for themes without a stylesheet. */
    void polishSelf();

   signals:
    void searchChanged(const QString& text);
    void addInstanceRequested();
    void accountsRequested();
    void settingsRequested();

   protected:
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;

   private:
    NightSkyBackground* m_background = nullptr;
    QWidget* m_hero = nullptr;
    QWidget* m_cardsHost = nullptr;
    QVBoxLayout* m_cardsLayout = nullptr;
    QLabel* m_title = nullptr;
    QLabel* m_subtitle = nullptr;
    QLineEdit* m_search = nullptr;
    QToolButton* m_account = nullptr;
    QPushButton* m_add = nullptr;
    QToolButton* m_settings = nullptr;
    int m_instanceCount = 0;
    QString m_playtime;
    QString m_accountName;
};
