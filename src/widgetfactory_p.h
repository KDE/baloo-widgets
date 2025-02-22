/*
    SPDX-FileCopyrightText: 2012-2014 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>

    Code largely copied/adapted from KFileMetadataProvider
    SPDX-FileCopyrightText: 2010 Peter Penz <peter.penz@gmx.at>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef WIDGETFACTORY_P_H
#define WIDGETFACTORY_P_H

#include <QDateTime>
#include <QTimeZone>

#include <KFormat>

static QString formatDateTime(const QVariant &value, QLocale::FormatType dateFormat, const QTimeZone &localTimeZone)
{
    const QString valueString = value.toString();
    QDateTime dt;
    KFormat form;
    if (value.metaType().id() == QMetaType::QDateTime) {
        dt = value.toDateTime();
    } else if (value.metaType().id() == QMetaType::QDate) {
        return form.formatRelativeDate(value.toDate(), dateFormat);
    } else if (value.metaType().id() == QMetaType::QString) {
        dt = QDateTime::fromString(valueString, Qt::ISODate);
    }

    if (dt.isValid()) {
        KFormat form;
        // Check if Date/DateTime
        QTime time = dt.time();
        if (!time.isValid()) {
            return form.formatRelativeDate(dt.toTimeZone(localTimeZone).date(), dateFormat);
        } else {
            return form.formatRelativeDateTime(dt.toTimeZone(localTimeZone), dateFormat);
        }
    }

    return valueString;
}

#endif // WIDGETFACTORY_P_H
