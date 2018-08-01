// https://github.com/KubaO/QtDataStream
#include <QDataStream>

class DataStream : public QDataStream {
   struct Proxy {
      QScopedPointer<QDataStreamPrivate> d;
      QIODevice *dev;
      bool owndev;
      bool noswap;
      QDataStream::ByteOrder byteorder;
      int ver;
      QDataStream::Status q_status;
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
      virtual ~Proxy();
#endif
   };
   static Proxy *p(QDataStream *ds) { return reinterpret_cast<Proxy *>(ds); }
   static const Proxy *p(const QDataStream *ds) {
      return reinterpret_cast<const Proxy *>(ds);
   }
#if defined(QT_TESTLIB_LIB) || defined(QT_MODULE_TEST)
   friend class DataStreamTest;
#endif
  public:
   DataStream() = default;
   using QDataStream::QDataStream;
   DataStream(DataStream &&other) : DataStream(static_cast<QDataStream &&>(other)) {}
   DataStream(QDataStream &&other) {
      using std::swap;
      Proxy &o = *p(&other);
      Proxy &t = *p(this);
      swap(t.d, o.d);
      swap(t.dev, o.dev);
      swap(t.owndev, o.owndev);
      swap(t.noswap, o.noswap);
      swap(t.byteorder, o.byteorder);
      swap(t.ver, o.ver);
      swap(t.q_status, o.q_status);
   }
   DataStream &operator=(DataStream &&other) {
      return *this = static_cast<QDataStream &&>(other);
   }
   DataStream &operator=(QDataStream &&other) {
      this->~DataStream();
      new (this) DataStream(std::move(other));
      return *this;
   }
   void setOwnedDevice(QIODevice *dev) {
      setDevice(dev);
      p(this)->owndev = true;
   }
   bool ownsDevice() const { return p(this)->owndev; }
   static bool ownsDevice(const QDataStream *ds) { return p(ds)->owndev; }
};
