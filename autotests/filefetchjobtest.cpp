/*
    SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "filefetchjob.h"

#include <KFileMetaData/Properties>

#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>

using Baloo::FileFetchJob;

class FileFetchJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanup();

    void testExtractedProperties();
    void testEventLoopRunsDuringExtraction();
    void testKilledJobReportsNothing();
    void testMissingExtractorFinishes();

private:
    // Puts an extractor that runs the given shell commands first in PATH.
    void useFakeExtractor(const QByteArray &commands);

    QByteArray m_path;
    QTemporaryDir m_fakeBin;
};

void FileFetchJobTest::initTestCase()
{
    m_path = qgetenv("PATH");
    QVERIFY(m_fakeBin.isValid());
}

void FileFetchJobTest::cleanup()
{
    qputenv("PATH", m_path);
}

void FileFetchJobTest::useFakeExtractor(const QByteArray &commands)
{
    QFile script(m_fakeBin.filePath(QStringLiteral("baloo_filemetadata_temp_extractor")));
    QVERIFY(script.open(QIODevice::WriteOnly | QIODevice::Truncate));
    script.write("#!/bin/sh\n" + commands + "\n");
    script.close();
    QVERIFY(script.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
    qputenv("PATH", QByteArray(QFile::encodeName(m_fakeBin.path()) + ':' + m_path));
}

void FileFetchJobTest::testExtractedProperties()
{
    const QString file = QFINDTESTDATA("samplefiles/test.mp3");
    auto job = new FileFetchJob({file}, false, FileFetchJob::UseRealtimeIndexing::Only);
    QSignalSpy result(job, &KJob::result);
    job->start();
    QVERIFY(result.wait(10000));

    QCOMPARE(job->data().size(), 1);
    QCOMPARE(job->data().first().value(QStringLiteral("sampleRate")).toInt(), 44100);
}

void FileFetchJobTest::testEventLoopRunsDuringExtraction()
{
    useFakeExtractor("sleep 1");

    int ticks = 0;
    QTimer timer;
    timer.setInterval(50);
    connect(&timer, &QTimer::timeout, this, [&ticks] {
        ++ticks;
    });

    auto job = new FileFetchJob({QFINDTESTDATA("samplefiles/test.mp3")}, false, FileFetchJob::UseRealtimeIndexing::Only);
    QSignalSpy result(job, &KJob::result);
    job->start();
    timer.start();
    QVERIFY(result.wait(10000));

    // The extractor took a second, so the timer ticked about 20 times while it ran.
    QVERIFY2(ticks >= 10, qPrintable(QStringLiteral("only %1 ticks").arg(ticks)));
    QCOMPARE(job->data().size(), 1);
}

void FileFetchJobTest::testKilledJobReportsNothing()
{
    useFakeExtractor("sleep 2");

    auto job = new FileFetchJob({QFINDTESTDATA("samplefiles/test.mp3")}, false, FileFetchJob::UseRealtimeIndexing::Only);
    QSignalSpy result(job, &KJob::result);
    QSignalSpy destroyed(job, &QObject::destroyed);
    job->start();
    QTest::qWait(200);

    QVERIFY(job->kill());
    QVERIFY(destroyed.wait(5000));
    QCOMPARE(result.count(), 0);
}

void FileFetchJobTest::testMissingExtractorFinishes()
{
    qputenv("PATH", QFile::encodeName(m_fakeBin.path() + QStringLiteral("/missing")));

    auto job = new FileFetchJob({QFINDTESTDATA("samplefiles/test.mp3")}, false, FileFetchJob::UseRealtimeIndexing::Only);
    QSignalSpy result(job, &KJob::result);
    job->start();
    QVERIFY(result.wait(5000));
    QCOMPARE(job->data().size(), 1);
}

QTEST_GUILESS_MAIN(FileFetchJobTest)

#include "filefetchjobtest.moc"
