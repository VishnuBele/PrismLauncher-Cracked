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

#include <QColor>
#include <QImage>
#include <QTimer>
#include <QVector>
#include <QWidget>

/*!
 * An animated night sky for the home screen: a vertical sky gradient, a field of
 * twinkling stars, and meteors that streak past every few seconds, with a very
 * faint nebula wash for depth.
 *
 * Every colour comes from the current theme palette. On a light theme the sky
 * becomes a pale dawn instead of a black night, so the widget never turns a
 * white window into an unreadable dark one.
 *
 * The sky and nebula are rendered into a half resolution buffer and upscaled
 * (they are soft gradients, so the upscale is invisible); the stars and meteors
 * are drawn at full resolution afterwards so they stay crisp. Animation stops
 * completely when the widget is hidden or the window is not active.
 */
class NightSkyBackground : public QWidget {
    Q_OBJECT

   public:
    explicit NightSkyBackground(QWidget* parent = nullptr);
    ~NightSkyBackground() override = default;

    bool isAnimated() const { return m_animated; }
    void setAnimated(bool animated);

    /*! 0..1 multiplier for the stars and meteors. */
    qreal intensity() const { return m_intensity; }
    void setIntensity(qreal intensity);

    bool event(QEvent* event) override;

   protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

   private slots:
    void tick();

   private:
    struct Star {
        qreal x;              //!< fraction of the width
        qreal y;              //!< fraction of the height
        qreal radius;         //!< pixels, full resolution
        qreal brightness;     //!< 0..1
        qreal twinkleSpeed;   //!< radians per second
        qreal twinklePhase;   //!< radians
    };

    struct Meteor {
        qreal x;         //!< head position, pixels
        qreal y;
        qreal vx;        //!< pixels per second
        qreal vy;
        qreal length;    //!< tail length in pixels
        qreal width;     //!< stroke width in pixels
        qreal age;       //!< seconds
        qreal life;      //!< seconds
    };

    struct Nebula {
        QColor color;
        qreal radius;    //!< fraction of the diagonal
        qreal phase;
        qreal speed;     //!< radians per second
        qreal driftX;
        qreal driftY;
        qreal x;         //!< rest position, fractions
        qreal y;
    };

    void refreshPalette();
    void rebuildNebula();
    void rebuildStars();
    void spawnMeteor();
    void ensureBuffer();
    void updateRunning();

    QVector<Star> m_stars;
    QVector<Meteor> m_meteors;
    QVector<Nebula> m_nebula;

    QTimer m_timer;
    QImage m_buffer;
    QImage m_noise;

    QColor m_skyTop;
    QColor m_skyBottom;
    QColor m_starColor;
    QColor m_meteorColor;

    qreal m_seconds = 0;
    qreal m_nextMeteor = 3.0;
    qreal m_intensity = 1.0;
    bool m_animated = true;
    bool m_running = false;
    bool m_lightSky = false;
};
