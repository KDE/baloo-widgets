/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _BALOO_METADATAFILTER_H
#define _BALOO_METADATAFILTER_H

#include <QVariant>

namespace Baloo
{
class Variant;

class MetadataFilter : public QObject
{
public:
    explicit MetadataFilter(QObject *parent = nullptr);
    ~MetadataFilter() override;

    /**
     * Takes all the data by the provider and filters the data
     * according to 'baloofileinformationrc' config
     * This acts as a filter and a data aggregator
     */
    QVariantMap filter(const QVariantMap &data);

private:
    /**
     * Initializes the configuration file "kmetainformationrc"
     * with proper default settings for the first start in
     * an uninitialized environment.
     */
    void initMetaInformationSettings();
};
}

#endif // _BALOO_METADATAFILTER_H
