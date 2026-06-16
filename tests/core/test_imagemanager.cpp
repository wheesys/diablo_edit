#include <QtTest>
#include <QCoreApplication>
#include "ImageManager.h"
#include "test_utils.h"

/// ImageManager 单元测试
class TestImageManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testLoadImages();
    void testCodeToIndex();
    void testCodeToIndex_data();
    void testPicIndexRange();
    void testGetPixmapByCode();
    void cleanupTestCase();
};

void TestImageManager::initTestCase()
{
    auto* img = ImageManager::instance();
    QString imagePath = TestUtils::imagesDir();
    bool ok = img->loadAll(imagePath);
    QVERIFY2(ok, qPrintable("ImageManager::loadAll failed: " + imagePath));
}

void TestImageManager::testLoadImages()
{
    auto* img = ImageManager::instance();
    QVERIFY(img->isLoaded());
    QCOMPARE(img->count(), 323);
}

void TestImageManager::testCodeToIndex_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<int>("expectedIndex");

    // 杂项
    QTest::newRow("elx-elixir") << "elx" << 0;
    QTest::newRow("hpo-healing") << "hpo" << 1;
    QTest::newRow("amu-amulet") << "amu" << 12;

    // 帽子 (IDB_BITMAP139 = index 139)
    QTest::newRow("cap") << "cap" << 139;
    QTest::newRow("skp-skullcap") << "skp" << 140;

    // 盾牌 (IDB_BITMAP191 = index 191)
    QTest::newRow("buc-buckler") << "buc" << 191;
    QTest::newRow("sml-smallshield") << "sml" << 192;

    // 武器
    QTest::newRow("hax-handaxe") << "hax" << 209;
    QTest::newRow("axe") << "axe" << 210;
    QTest::newRow("wnd-wand") << "wnd" << 219;
    QTest::newRow("ssd-shortsword") << "ssd" << 234;

    // D2R 新增
    QTest::newRow("toa-token") << "toa" << 318;
    QTest::newRow("fed-essenced") << "fed" << 319;
    QTest::newRow("tes-essences") << "tes" << 322;
}

void TestImageManager::testCodeToIndex()
{
    QFETCH(QString, code);
    QFETCH(int, expectedIndex);

    auto* img = ImageManager::instance();
    QByteArray codeBytes = code.toUtf8();
    int idx = img->picIndexForCode(codeBytes.constData());
    QCOMPARE(idx, expectedIndex);
}

void TestImageManager::testPicIndexRange()
{
    auto* img = ImageManager::instance();

    // Negative → null
    QVERIFY(img->getPixmap(-1).isNull());
    // Out of range → null
    QVERIFY(img->getPixmap(999).isNull());
    // Valid index → non-null
    QVERIFY(!img->getPixmap(0).isNull());
    QVERIFY(!img->getPixmap(322).isNull());
}

void TestImageManager::testGetPixmapByCode()
{
    auto* img = ImageManager::instance();

    // 已知有效的代码
    QPixmap pix = img->getPixmapByCode("elx");
    QVERIFY(!pix.isNull());

    pix = img->getPixmapByCode("toa");
    QVERIFY(!pix.isNull());

    // 无效代码 → null
    pix = img->getPixmapByCode("zzzz");
    QVERIFY(pix.isNull());

    // 空指针 → -1
    QCOMPARE(img->picIndexForCode(nullptr), -1);
    QCOMPARE(img->picIndexForCode(""), -1);
}

void TestImageManager::cleanupTestCase()
{
    // singleton cleanup if needed
}

QTEST_MAIN(TestImageManager)
#include "test_imagemanager.moc"
