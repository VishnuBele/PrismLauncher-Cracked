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

#include <QAbstractListModel>
#include <QTest>

#include "ui/instanceview/InstanceNameFilter.h"
#include "ui/instanceview/InstanceProxyModel.h"

/*! Stand-in for InstanceList: DisplayRole is the instance name, as it is there. */
class FakeInstanceModel : public QAbstractListModel {
   public:
    explicit FakeInstanceModel(QStringList names) : m_names(std::move(names)) {}

    int rowCount(const QModelIndex& parent = QModelIndex()) const override { return parent.isValid() ? 0 : int(m_names.size()); }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= m_names.size())
            return {};
        if (role == Qt::DisplayRole)
            return m_names.at(index.row());
        return {};
    }

   private:
    QStringList m_names;
};

class InstanceFilterTest : public QObject {
    Q_OBJECT

   private slots:
    void predicate()
    {
        QVERIFY(instanceNameMatchesFilter(QStringLiteral("SkyFactory 5"), QString()));
        QVERIFY(instanceNameMatchesFilter(QStringLiteral("SkyFactory 5"), QStringLiteral("sky")));
        QVERIFY(instanceNameMatchesFilter(QStringLiteral("SkyFactory 5"), QStringLiteral("  SKY  ")));
        QVERIFY(instanceNameMatchesFilter(QStringLiteral("Fabulously Optimized"), QStringLiteral("optim")));
        QVERIFY(!instanceNameMatchesFilter(QStringLiteral("Vanilla"), QStringLiteral("sky")));
        QVERIFY(!instanceNameMatchesFilter(QStringLiteral("Vanilla"), QStringLiteral("vanilla1")));
    }

    void proxyFiltersRows()
    {
        FakeInstanceModel model({ QStringLiteral("SkyFactory 5"), QStringLiteral("All the Mods 10"), QStringLiteral("Vanilla"),
                                  QStringLiteral("skyblock") });
        InstanceProxyModel proxy;
        proxy.setSourceModel(&model);

        QCOMPARE(proxy.rowCount(), 4);

        proxy.setInstanceFilter(QStringLiteral("sky"));
        QCOMPARE(proxy.rowCount(), 2);

        proxy.setInstanceFilter(QStringLiteral("  SKY  "));
        QCOMPARE(proxy.rowCount(), 2);

        proxy.setInstanceFilter(QStringLiteral("vanilla"));
        QCOMPARE(proxy.rowCount(), 1);
        QCOMPARE(proxy.index(0, 0).data(Qt::DisplayRole).toString(), QStringLiteral("Vanilla"));

        proxy.setInstanceFilter(QStringLiteral("nomatch"));
        QCOMPARE(proxy.rowCount(), 0);

        proxy.setInstanceFilter(QString());
        QCOMPARE(proxy.rowCount(), 4);
    }
};

QTEST_GUILESS_MAIN(InstanceFilterTest)

#include "InstanceFilter_test.moc"
