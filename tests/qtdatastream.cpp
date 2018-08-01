// https://github.com/KubaO/QtDataStream
#include "QtDataStream"
#include <QtTest>

class DataStreamTest : public QObject {
   Q_OBJECT
   static QObjectData *getD(QObject *obj) {
      return static_cast<DataStreamTest *>(obj)->d_ptr.data();
   }
   static bool wasDeleted(QObject *obj) { return getD(obj)->wasDeleted; }
   template <typename T, typename... Args>
   DataStream make_stream(Args &&... args) {
      return T(std::forward<Args>(args)...);
   }
   static QDataStream::ByteOrder flipped(QDataStream::ByteOrder o) {
      return (o == QDataStream::BigEndian) ? QDataStream::LittleEndian
                                           : QDataStream::BigEndian;
   }
   Q_SLOT void isBinaryCompatible() {
      QCOMPARE(sizeof(DataStream), sizeof(QDataStream));
      QCOMPARE(sizeof(DataStream::Proxy), sizeof(QDataStream));
      struct Test {
         QByteArray data;
         QDataStream ds{&data, QIODevice::ReadWrite};
         void check(int loc = 0) {
            if (!loc) {
               check(1);
               ds.setDevice(nullptr);
               check(1);
            }
            QCOMPARE(!!ds.device(), DataStream::ownsDevice(&ds));
            QCOMPARE(ds.device(), DataStream::p(&ds)->dev);

            if (!loc) check(2);
            bool noswap = DataStream::p(&ds)->noswap;
            QCOMPARE(noswap, DataStream::p(&ds)->noswap);
            QCOMPARE(ds.byteOrder(), DataStream::p(&ds)->byteorder);
            if (loc != 2) {
               ds.setByteOrder(flipped(ds.byteOrder()));
               noswap = !noswap;
            }
            if (!loc) check(2);
            QCOMPARE(noswap, DataStream::p(&ds)->noswap);

            if (!loc) check(3);
            QCOMPARE(ds.version(), DataStream::p(&ds)->ver);
            if (loc != 3) ds.setVersion(QDataStream::Qt_4_0);
            if (!loc) check(3);

            if (!loc) check(4);
            QCOMPARE(ds.status(), DataStream::p(&ds)->q_status);
            if (loc != 4) ds.setStatus(QDataStream::ReadPastEnd);
            if (!loc) check(4);
         }
      } test;
      test.check();
   }
   Q_SLOT void streams() {
      QString str{"Hello, world"};
      QVector<uint> ints{44, 0xDEADBEEF, 1};
      QByteArray data;
      DataStream ds(&data, QIODevice::ReadWrite);
      ds << str << ints;
      ds.device()->reset();
      QString str2;
      QVector<uint> ints2;
      ds >> str2 >> ints2;
      QCOMPARE(str2, str);
      QCOMPARE(ints2, ints);
   }
   Q_SLOT void movesFromNotOwnedQDataStream() {
      QBuffer buf;
      QDataStream ds(&buf);
      QVERIFY(ds.device() == &buf);
      DataStream ds2(std::move(ds));
      QVERIFY(!ds.device());
      QVERIFY(ds2.device() == &buf);
      QVERIFY(!wasDeleted(&buf));
   }
   Q_SLOT void movesFromNotOwnedDataStream() {
      QBuffer buf;
      DataStream ds(&buf);
      QVERIFY(ds.device() == &buf);
      DataStream ds2(std::move(ds));
      QVERIFY(!ds.device());
      QVERIFY(ds2.device() == &buf);
      QVERIFY(!wasDeleted(&buf));
   }
   Q_SLOT void assignsFromNotOwnedQDataStream() {
      QBuffer buf;
      QDataStream ds(&buf);
      QVERIFY(ds.device() == &buf);
      DataStream ds2;
      ds2 = std::move(ds);
      QVERIFY(!ds.device());
      QVERIFY(ds2.device() == &buf);
      QVERIFY(!wasDeleted(&buf));
   }
   Q_SLOT void assignsFromNotOwnedDataStream() {
      QBuffer buf;
      DataStream ds(&buf);
      QVERIFY(ds.device() == &buf);
      DataStream ds2;
      ds2 = std::move(ds);
      QVERIFY(!ds.device());
      QVERIFY(ds2.device() == &buf);
      QVERIFY(!wasDeleted(&buf));
   }
   Q_SLOT void returnsFromNotOwnedQDataStream() {
      QBuffer buf;
      {
         auto ds = make_stream<QDataStream>(&buf);
         QVERIFY(ds.device());
         QVERIFY(!ds.ownsDevice());
      }
      QVERIFY(!wasDeleted(&buf));
   }
   Q_SLOT void returnsFromNotOwnedDataStream() {
      QBuffer buf;
      buf.open(QIODevice::ReadWrite);
      {
         auto ds = make_stream<DataStream>(&buf);
         QVERIFY(ds.device());
         QVERIFY(!ds.ownsDevice());
      }
      QVERIFY(!wasDeleted(&buf));
   }
   Q_SLOT void movesFromOwnedQDataStream() {
      QPointer<QIODevice> buf;
      {
         QByteArray data;
         QDataStream ds(&data, QIODevice::ReadWrite);
         QVERIFY(DataStream::ownsDevice(&ds));
         buf = ds.device();
         DataStream ds2(std::move(ds));
         QVERIFY(!ds.device());
         QVERIFY(ds2.device() == buf);
         QVERIFY(buf);
      }
      QVERIFY(!buf);
   }
   Q_SLOT void moveFromOwnedDataStream() {
      QPointer<QBuffer> buf(new QBuffer);
      {
         DataStream ds;
         ds.setOwnedDevice(buf);
         QVERIFY(ds.device() == buf);
         DataStream ds2(std::move(ds));
         QVERIFY(!ds.device());
         QVERIFY(ds2.device() == buf);
         QVERIFY(buf);
      }
      QVERIFY(!buf);
   }
   Q_SLOT void assignsFromOwnedQDataStream() {
      QPointer<QIODevice> buf;
      {
         QByteArray data;
         QDataStream ds(&data, QIODevice::ReadWrite);
         QVERIFY(DataStream::ownsDevice(&ds));
         buf = ds.device();
         DataStream ds2;
         ds2 = std::move(ds);
         QVERIFY(!ds.device());
         QVERIFY(ds2.device() == buf);
         QVERIFY(buf);
      }
      QVERIFY(!buf);
   }
   Q_SLOT void assignsFromOwnedDataStream() {
      QPointer<QBuffer> buf(new QBuffer);
      {
         DataStream ds;
         ds.setOwnedDevice(buf);
         QVERIFY(ds.device() == buf);
         DataStream ds2;
         ds2 = std::move(ds);
         QVERIFY(!ds.device());
         QVERIFY(ds2.device() == buf);
         QVERIFY(buf);
      }
      QVERIFY(!buf);
   }
   Q_SLOT void returnsFromOwnedQDataStream() {
      QPointer<QIODevice> dev;
      QByteArray data;
      {
         auto ds = make_stream<QDataStream>(&data, QIODevice::ReadWrite);
         dev = ds.device();
         QVERIFY(ds.device());
         QVERIFY(ds.ownsDevice());
      }
      QVERIFY(!dev);
   }
   Q_SLOT void returnsFromOwnedDataStream() {
      QPointer<QIODevice> dev;
      QByteArray data;
      {
         auto ds = make_stream<DataStream>(&data, QIODevice::ReadWrite);
         dev = ds.device();
         QVERIFY(ds.device());
         QVERIFY(ds.ownsDevice());
      }
      QVERIFY(!dev);
   }
};

QTEST_MAIN(DataStreamTest)
#include "qtdatastream.moc"
