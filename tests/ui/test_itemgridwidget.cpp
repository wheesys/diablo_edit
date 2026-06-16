#include <QtTest>
#include <QApplication>
#include "ui/ItemGridWidget.h"
#include "data/DataManager.h"
#include "test_utils.h"
#include "ImageManager.h"

/// ItemGridWidget 可视化测试
/// 运行方式：QT_QPA_PLATFORM=offscreen ./test_itemgridwidget
class TestItemGridWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testCreateWidget();
    void testReset();
    void testLayout();
    void cleanupTestCase();

private:
    ItemGridWidget* m_widget = nullptr;
    DataManager* m_dataMgr = nullptr;
};

void TestItemGridWidget::initTestCase()
{
    m_dataMgr = new DataManager;
    g_dataMgr = m_dataMgr;
    QString dataPath = TestUtils::dataDir();
    bool ok = m_dataMgr->loadAll(dataPath);
    QVERIFY2(ok, qPrintable("DataManager::loadAll failed: " + dataPath));

    ImageManager::instance()->loadAll(TestUtils::imagesDir());

    m_widget = new ItemGridWidget;
}

void TestItemGridWidget::testCreateWidget()
{
    QVERIFY(m_widget != nullptr);
}

void TestItemGridWidget::testReset()
{
    m_widget->resetAll();
    QVERIFY(true);
}

void TestItemGridWidget::testLayout()
{
    QSize minSize = m_widget->minimumSizeHint();
    QVERIFY(minSize.width() > 0);
    QVERIFY(minSize.height() > 0);
}

void TestItemGridWidget::cleanupTestCase()
{
    delete m_widget;
    m_widget = nullptr;
    delete m_dataMgr;
    m_dataMgr = nullptr;
    g_dataMgr = nullptr;
}

QTEST_MAIN(TestItemGridWidget)
#include "test_itemgridwidget.moc"
