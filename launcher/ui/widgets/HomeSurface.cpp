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

#include "HomeSurface.h"

#include <QAbstractScrollArea>
#include <QEvent>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QResizeEvent>
#include <QToolButton>
#include <QVBoxLayout>

#include "NightSkyBackground.h"

HomeSurface::HomeSurface(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("homeSurface"));

    m_background = new NightSkyBackground(this);
    m_background->setObjectName(QStringLiteral("homeBackground"));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(26, 20, 26, 18);
    root->setSpacing(16);

    // ---- header -------------------------------------------------------
    m_hero = new QWidget(this);
    m_hero->setObjectName(QStringLiteral("homeHeader"));
    auto* heroLayout = new QHBoxLayout(m_hero);
    heroLayout->setContentsMargins(0, 0, 0, 0);
    heroLayout->setSpacing(12);

    auto* titles = new QVBoxLayout();
    titles->setContentsMargins(0, 0, 0, 0);
    titles->setSpacing(2);

    m_title = new QLabel(tr("Instances"), m_hero);
    m_title->setObjectName(QStringLiteral("homeTitle"));
    m_subtitle = new QLabel(tr("No instances yet"), m_hero);
    m_subtitle->setObjectName(QStringLiteral("homeSubtitle"));

    titles->addWidget(m_title);
    titles->addWidget(m_subtitle);
    heroLayout->addLayout(titles);
    heroLayout->addStretch(1);

    m_search = new QLineEdit(m_hero);
    m_search->setObjectName(QStringLiteral("homeSearch"));
    m_search->setPlaceholderText(tr("Search instances"));
    m_search->setClearButtonEnabled(true);
    m_search->setFixedWidth(220);
    m_search->setToolTip(tr("Filter the instance list by name"));
    heroLayout->addWidget(m_search);

    m_account = new QToolButton(m_hero);
    m_account->setObjectName(QStringLiteral("homeAccount"));
    m_account->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_account->setText(tr("Accounts"));
    m_account->setIcon(QIcon::fromTheme(QStringLiteral("noaccount")));
    m_account->setIconSize(QSize(18, 18));
    m_account->setToolTip(tr("Manage accounts"));
    m_account->setCursor(Qt::PointingHandCursor);
    heroLayout->addWidget(m_account);

    m_add = new QPushButton(tr("Add Instance"), m_hero);
    m_add->setObjectName(QStringLiteral("homeAdd"));
    m_add->setIcon(QIcon::fromTheme(QStringLiteral("new")));
    m_add->setToolTip(tr("Create a new instance"));
    m_add->setCursor(Qt::PointingHandCursor);
    heroLayout->addWidget(m_add);

    m_settings = new QToolButton(m_hero);
    m_settings->setObjectName(QStringLiteral("homeSettings"));
    m_settings->setIcon(QIcon::fromTheme(QStringLiteral("settings")));
    m_settings->setIconSize(QSize(18, 18));
    // An icon-only button collapses to zero width when the icon theme is
    // missing an entry, so keep a clickable floor.
    m_settings->setMinimumSize(34, 34);
    m_settings->setToolTip(tr("Settings"));
    m_settings->setCursor(Qt::PointingHandCursor);
    heroLayout->addWidget(m_settings);

    root->addWidget(m_hero);

    // ---- card area ----------------------------------------------------
    m_cardsHost = new QWidget(this);
    m_cardsHost->setObjectName(QStringLiteral("homeCards"));
    m_cardsLayout = new QVBoxLayout(m_cardsHost);
    m_cardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cardsLayout->setSpacing(0);
    root->addWidget(m_cardsHost, 1);

    m_search->setMinimumHeight(32);
    m_account->setMinimumHeight(32);
    m_add->setMinimumHeight(32);
    m_add->setMinimumWidth(130);
    polishSelf();

    connect(m_search, &QLineEdit::textChanged, this, &HomeSurface::searchChanged);
    connect(m_add, &QPushButton::clicked, this, &HomeSurface::addInstanceRequested);
    connect(m_account, &QToolButton::clicked, this, &HomeSurface::accountsRequested);
    connect(m_settings, &QToolButton::clicked, this, &HomeSurface::settingsRequested);
}

void HomeSurface::setContent(QWidget* content)
{
    if (!content)
        return;

    content->setParent(m_cardsHost);
    m_cardsLayout->addWidget(content);

    // Let the animated backdrop show through around and between the cards. The
    // selectors are scoped on purpose: a bare "background: transparent" would
    // cascade into the view's scrollbars and scroll-area frame.
    content->setObjectName(QStringLiteral("homeContent"));
    if (auto* area = qobject_cast<QAbstractScrollArea*>(content)) {
        area->setFrameShape(QFrame::NoFrame);
        area->setAttribute(Qt::WA_NoSystemBackground, true);
        if (auto* viewport = area->viewport()) {
            viewport->setObjectName(QStringLiteral("qt_scrollarea_viewport"));
            viewport->setAutoFillBackground(false);
            viewport->setAttribute(Qt::WA_NoSystemBackground, true);
        }
    }
    content->setStyleSheet(QStringLiteral("QAbstractScrollArea#homeContent, QWidget#qt_scrollarea_viewport {"
                                          " background: transparent; border: none; }"));
}

void HomeSurface::setSummary(int instanceCount, const QString& playtime)
{
    m_instanceCount = instanceCount;
    m_playtime = playtime;

    QString instances = instanceCount == 1 ? tr("1 instance") : tr("%1 instances").arg(instanceCount);
    if (playtime.isEmpty())
        m_subtitle->setText(instances);
    else
        m_subtitle->setText(tr("%1  -  %2").arg(instances, playtime));
}

void HomeSurface::setAccount(const QString& name, const QIcon& icon)
{
    m_accountName = name;
    m_account->setText(name.isEmpty() ? tr("Accounts") : name);
    m_account->setIcon(icon.isNull() ? QIcon::fromTheme(QStringLiteral("noaccount")) : icon);
}

void HomeSurface::setBackgroundAnimated(bool animated)
{
    m_background->setAnimated(animated);
}

bool HomeSurface::isBackgroundAnimated() const
{
    return m_background->isAnimated();
}

void HomeSurface::setSearchText(const QString& text)
{
    m_search->setText(text);
}

void HomeSurface::retranslateUi()
{
    m_title->setText(tr("Instances"));
    m_search->setPlaceholderText(tr("Search instances"));
    m_search->setToolTip(tr("Filter the instance list by name"));
    m_add->setText(tr("Add Instance"));
    m_add->setToolTip(tr("Create a new instance"));
    m_account->setToolTip(tr("Manage accounts"));
    m_settings->setToolTip(tr("Settings"));
    m_account->setText(m_accountName.isEmpty() ? tr("Accounts") : m_accountName);

    setSummary(m_instanceCount, m_playtime);
}

void HomeSurface::focusSearch()
{
    m_search->setFocus(Qt::ShortcutFocusReason);
    m_search->selectAll();
}

void HomeSurface::polishSelf()
{
    // Themes without a stylesheet (Bright, for one) get no typography at all, so
    // the hierarchy has to be built from the palette and fonts instead of QSS.
    const QPalette pal = palette();

    QFont titleFont = font();
    titleFont.setPointSizeF(font().pointSizeF() + 7.5);
    titleFont.setWeight(QFont::DemiBold);
    m_title->setFont(titleFont);

    QFont subtitleFont = font();
    subtitleFont.setPointSizeF(qMax(7.0, font().pointSizeF() - 0.5));
    m_subtitle->setFont(subtitleFont);
    QPalette subtitlePalette = m_subtitle->palette();
    QColor muted = pal.color(QPalette::WindowText);
    muted.setAlpha(170);
    subtitlePalette.setColor(QPalette::WindowText, muted);
    m_subtitle->setPalette(subtitlePalette);

    QPalette primary = m_add->palette();
    primary.setColor(QPalette::Button, pal.color(QPalette::Highlight));
    primary.setColor(QPalette::ButtonText, pal.color(QPalette::HighlightedText));
    primary.setColor(QPalette::WindowText, pal.color(QPalette::HighlightedText));
    m_add->setPalette(primary);
    m_add->setAutoFillBackground(true);
}

void HomeSurface::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::StyleChange)
        polishSelf();
}

void HomeSurface::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    if (m_background) {
        m_background->setGeometry(rect());
        m_background->lower();
    }
}
