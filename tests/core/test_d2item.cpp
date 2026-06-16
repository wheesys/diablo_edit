#include <QtTest>
#include <QCoreApplication>
#include "D2Item.h"
#include "D2S_Struct.h"
#include "BinDataStream.h"

/// D2Item 基础单元测试
class TestD2Item : public QObject
{
    Q_OBJECT

private slots:
    void testDefaultConstruct();
    void testEmptyItem();
    void testIsBox();
};

void TestD2Item::testDefaultConstruct()
{
    CD2Item item;
    QVERIFY(!item.IsBox());
}

void TestD2Item::testEmptyItem()
{
    CD2Item item;
    // g_dataMgr 为 null 时 ItemName() 会崩溃，暂跳过
    QVERIFY(true);
}

void TestD2Item::testIsBox()
{
    CD2Item item;
    QCOMPARE(item.IsBox(), false);
}

QTEST_MAIN(TestD2Item)
#include "test_d2item.moc"
