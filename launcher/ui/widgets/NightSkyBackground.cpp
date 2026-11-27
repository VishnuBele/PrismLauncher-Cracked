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

#include "NightSkyBackground.h"

#include <QBrush>
#include <QCoreApplication>
#include <QEvent>
#include <QLinearGradient>
#include <QPainter>
#include <QRadialGradient>
#include <QRandomGenerator>
#include <QtMath>

#include <algorithm>

namespace {
QColor mix(const QColor& from, const QColor& to, qreal amount)
{
    const qreal t = qBound(0.0, amount, 1.0);
    return QColor::fromRgbF(from.redF() * (1.0 - t) + to.redF() * t, from.greenF() * (1.0 - t) + to.greenF() * t,
                            from.blueF() * (1.0 - t) + to.blueF() * t);
}

qreal randomBetween(qreal from, qreal to)
{
    return from + QRandomGenerator::global()->generateDouble() * (to - from);
}
}  // namespace

NightSkyBackground::NightSkyBackground(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setFocusPolicy(Qt::NoFocus);
    setAutoFillBackground(false);

    m_timer.setInterval(33);  // ~30 fps is plenty for a slow sky
    m_timer.setTimerType(Qt::CoarseTimer);
    connect(&m_timer, &QTimer::timeout, this, &NightSkyBackground::tick);

    refreshPalette();
    rebuildNebula();
    rebuildStars();
}

void NightSkyBackground::setAnimated(bool animated)
{
    if (m_animated == animated)
        return;
    m_animated = animated;
    updateRunning();
    update();
}

void NightSkyBackground::setIntensity(qreal intensity)
{
    const qreal clamped = qBound(0.0, intensity, 1.0);
    if (qFuzzyCompare(m_intensity, clamped))
        return;
    m_intensity = clamped;
    update();
}

bool NightSkyBackground::event(QEvent* event)
{
    switch (event->type()) {
        case QEvent::PaletteChange:
        case QEvent::ApplicationPaletteChange:
        case QEvent::StyleChange:
            refreshPalette();
            rebuildNebula();
            rebuildStars();
            update();
            break;
        case QEvent::ActivationChange:
        case QEvent::WindowActivate:
        case QEvent::WindowDeactivate:
            updateRunning();
            break;
        default:
            break;
    }
    return QWidget::event(event);
}

void NightSkyBackground::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    updateRunning();
}

void NightSkyBackground::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    updateRunning();
}

void NightSkyBackground::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    m_buffer = QImage();
    ensureBuffer();
    rebuildStars();
}

void NightSkyBackground::tick()
{
    const qreal dt = m_timer.interval() / 1000.0;
    m_seconds += dt;

    // Meteors come and go: spawn one every few seconds, never too many at once.
    m_nextMeteor -= dt;
    if (m_nextMeteor <= 0.0) {
        // A bright theme gets a day sky: no shooting stars over a white window.
        if (m_meteors.size() < 3 && !m_lightSky)
            spawnMeteor();
        m_nextMeteor = randomBetween(1.8, 4.8);
    }

    for (int i = m_meteors.size() - 1; i >= 0; --i) {
        Meteor& meteor = m_meteors[i];
        meteor.age += dt;
        meteor.x += meteor.vx * dt;
        meteor.y += meteor.vy * dt;

        const bool gone = meteor.age >= meteor.life || meteor.y > height() + meteor.length || meteor.x < -meteor.length * 2;
        if (gone)
            m_meteors.remove(i);
    }

    update();
}

void NightSkyBackground::updateRunning()
{
    const bool visible = isVisible();
    const bool active = window() == nullptr || window()->isActiveWindow();
    const bool shouldRun = m_animated && visible && active && m_intensity > 0.0;

    if (shouldRun == m_running)
        return;

    m_running = shouldRun;
    if (m_running) {
        m_timer.start();
    } else {
        m_timer.stop();
    }
}

void NightSkyBackground::refreshPalette()
{
    const QPalette pal = palette();
    QColor base = pal.color(QPalette::Window);
    if (!base.isValid())
        base = QColor(14, 17, 23);

    QColor accent = pal.color(QPalette::Highlight);
    if (!accent.isValid())
        accent = QColor(76, 125, 255);

    m_lightSky = base.lightness() > 140;

    if (m_lightSky) {
        // Never paint a black sky into a white window: this is the pale dawn
        // variant, with faint dust motes instead of bright stars.
        m_skyTop = mix(base.darker(106), accent, 0.05);
        m_skyBottom = base;
        m_starColor = mix(QColor(20, 30, 50), accent, 0.25);
        m_meteorColor = accent.darker(165);
    } else {
        m_skyTop = base.darker(142);
        m_skyBottom = mix(base.lighter(124), accent, 0.10);
        m_starColor = mix(QColor(240, 246, 255), accent, 0.15);
        m_meteorColor = mix(accent.lighter(160), QColor(255, 255, 255), 0.35);
    }
}

void NightSkyBackground::rebuildNebula()
{
    const QPalette pal = palette();
    QColor accent = pal.color(QPalette::Highlight);
    if (!accent.isValid())
        accent = QColor(76, 125, 255);

    QColor cool = accent;
    cool.setHsv((accent.hsvHue() + 26) % 360, qBound(0, int(accent.hsvSaturation() * 0.8), 255), qMin(255, int(accent.value() * 1.04)));
    QColor warm = accent;
    warm.setHsv((accent.hsvHue() + 330) % 360, qBound(0, int(accent.hsvSaturation() * 0.75), 255), qMin(255, int(accent.value())));
    m_nebula = {
        { mix(accent, m_skyBottom, 0.80), 0.95, 0.0, 0.030, 0.10, 0.06, 0.72, 0.24 },
        { mix(cool, m_skyBottom, 0.82), 0.80, 2.1, 0.024, 0.12, 0.05, 0.22, 0.66 },
        { mix(warm, m_skyTop, 0.86), 1.05, 4.2, 0.017, 0.08, 0.07, 0.48, 0.42 },
    };
}

void NightSkyBackground::rebuildStars()
{
    const int wanted = qBound(70, int(width() * qreal(height()) / 3800.0), 320);
    if (m_stars.size() == wanted && !m_stars.isEmpty())
        return;

    auto* rng = QRandomGenerator::global();
    m_stars.clear();
    m_stars.reserve(wanted);
    for (int i = 0; i < wanted; ++i) {
        Star star;
        star.x = rng->generateDouble();
        // Weight the field slightly towards the top, where the sky is darkest.
        star.y = std::pow(rng->generateDouble(), 1.35);
        star.radius = randomBetween(0.6, 1.7);
        star.brightness = randomBetween(0.30, 1.0);
        star.twinkleSpeed = randomBetween(0.25, 1.10);
        star.twinklePhase = randomBetween(0.0, 6.28318);
        m_stars.append(star);
    }
}

void NightSkyBackground::spawnMeteor()
{
    Meteor meteor;
    const qreal speed = randomBetween(760.0, 1180.0);
    const bool leftwards = QRandomGenerator::global()->bounded(100) < 70;
    // 20-40 degrees below the horizon line.
    const qreal angle = qDegreesToRadians(randomBetween(20.0, 40.0));

    meteor.vx = (leftwards ? -1.0 : 1.0) * qCos(angle) * speed;
    meteor.vy = qSin(angle) * speed;
    meteor.length = randomBetween(90.0, 180.0);
    meteor.width = randomBetween(1.2, 2.2);
    meteor.age = 0.0;
    meteor.life = randomBetween(1.3, 2.1);

    // Enter from just outside the top edge so nothing pops in mid-sky.
    const qreal column = leftwards ? randomBetween(0.30, 1.02) : randomBetween(-0.02, 0.70);
    meteor.x = column * width();
    meteor.y = -randomBetween(10.0, 90.0);
    m_meteors.append(meteor);
}

void NightSkyBackground::ensureBuffer()
{
    if (m_buffer.isNull() || m_buffer.size() != QSize(qMax(1, width() / 2), qMax(1, height() / 2))) {
        // The sky and nebula are soft gradients, so half resolution is invisible
        // once upscaled and keeps the paint cost four times lower.
        m_buffer = QImage(QSize(qMax(1, width() / 2), qMax(1, height() / 2)), QImage::Format_ARGB32_Premultiplied);
    }
}

void NightSkyBackground::paintEvent(QPaintEvent*)
{
    ensureBuffer();
    if (m_buffer.isNull())
        return;

    const QSize half = m_buffer.size();
    const QRectF canvas(0, 0, half.width(), half.height());

    QPainter buffer(&m_buffer);
    buffer.setRenderHint(QPainter::Antialiasing, true);

    QLinearGradient sky(canvas.topLeft(), canvas.bottomLeft());
    sky.setColorAt(0.0, m_skyTop);
    sky.setColorAt(1.0, m_skyBottom);
    buffer.fillRect(canvas, sky);

    buffer.setPen(Qt::NoPen);
    const qreal diagonal = qSqrt(qreal(half.width()) * half.width() + qreal(half.height()) * half.height());
    for (const Nebula& nebula : std::as_const(m_nebula)) {
        const qreal angle = m_seconds * nebula.speed * 2.0 * M_PI + nebula.phase;
        const qreal cx = (nebula.x + qSin(angle) * nebula.driftX) * half.width();
        const qreal cy = (nebula.y + qCos(angle * 0.87) * nebula.driftY) * half.height();
        const qreal radius = qMax(1.0, nebula.radius * diagonal);

        QRadialGradient gradient(QPointF(cx, cy), radius);
        const qreal glowAlpha = (m_lightSky ? 0.045 : 0.10) * m_intensity;
        QColor inner = nebula.color;
        inner.setAlphaF(qBound(0.0, glowAlpha, 1.0));
        QColor middle = nebula.color;
        middle.setAlphaF(qBound(0.0, glowAlpha * 0.3, 1.0));
        QColor outer = nebula.color;
        outer.setAlphaF(0.0);
        gradient.setColorAt(0.0, inner);
        gradient.setColorAt(0.5, middle);
        gradient.setColorAt(1.0, outer);

        // Filling the canvas rather than an ellipse: a drawn circle would cut a
        // visible arc wherever its radius ended inside the widget.
        buffer.setBrush(gradient);
        buffer.fillRect(canvas, gradient);
    }

    // A faint static grain hides the contour rings an 8-bit gradient leaves on
    // a dark background; on a light one it just looks like a dirty screen.
    if (!m_lightSky && m_noise.isNull()) {
        m_noise = QImage(64, 64, QImage::Format_ARGB32);
        auto* rng = QRandomGenerator::global();
        for (int y = 0; y < m_noise.height(); ++y) {
            for (int x = 0; x < m_noise.width(); ++x) {
                m_noise.setPixel(x, y, qRgba(128, 128, 128, int(rng->bounded(9))));
            }
        }
    }
    if (!m_lightSky)
        buffer.fillRect(canvas, QBrush(m_noise));
    buffer.end();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.drawImage(rect(), m_buffer);

    if (m_intensity <= 0.0 || m_lightSky)
        return;

    // ---- stars (full resolution, so they stay crisp) --------------------
    const qreal starLevel = m_intensity * (m_lightSky ? 0.45 : 1.0);
    painter.setPen(Qt::NoPen);
    for (const Star& star : std::as_const(m_stars)) {
        const qreal twinkle = 0.55 + 0.45 * qSin(m_seconds * star.twinkleSpeed + star.twinklePhase);
        QColor color = m_starColor;
        color.setAlphaF(qBound(0.0, star.brightness * twinkle * starLevel, 1.0));

        const QPointF center(star.x * width(), star.y * height());
        painter.setBrush(color);
        painter.drawEllipse(center, star.radius, star.radius);

        if (star.brightness > 0.86) {
            QColor halo = color;
            halo.setAlphaF(color.alphaF() * 0.22);
            painter.setBrush(halo);
            painter.drawEllipse(center, star.radius * 2.8, star.radius * 2.8);
        }
    }

    // ---- meteors -------------------------------------------------------
    for (const Meteor& meteor : std::as_const(m_meteors)) {
        const qreal progress = meteor.age / meteor.life;
        const qreal fade = qBound(0.0, 1.0 - progress, 1.0) * m_intensity;
        if (fade <= 0.0)
            continue;

        const QPointF head(meteor.x, meteor.y);
        const qreal speed = qSqrt(meteor.vx * meteor.vx + meteor.vy * meteor.vy);
        if (speed < 1.0)
            continue;
        const QPointF direction(meteor.vx / speed, meteor.vy / speed);
        const QPointF tail = head - direction * meteor.length;

        QLinearGradient streak(tail, head);
        QColor cold = m_meteorColor;
        cold.setAlphaF(0.0);
        QColor warm = m_meteorColor;
        warm.setAlphaF(0.55 * fade);
        QColor hot = m_meteorColor;
        hot.setAlphaF(0.95 * fade);
        streak.setColorAt(0.0, cold);
        streak.setColorAt(0.65, warm);
        streak.setColorAt(1.0, hot);

        painter.setPen(QPen(QBrush(streak), meteor.width, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(tail, head);

        const qreal glowRadius = meteor.width * 3.6;
        QRadialGradient glow(head, glowRadius);
        QColor core = hot;
        core.setAlphaF(0.85 * fade);
        QColor edge = hot;
        edge.setAlphaF(0.0);
        glow.setColorAt(0.0, core);
        glow.setColorAt(1.0, edge);
        painter.setPen(Qt::NoPen);
        painter.setBrush(glow);
        painter.drawEllipse(head, glowRadius, glowRadius);
    }
}
