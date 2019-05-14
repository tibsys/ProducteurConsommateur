#ifndef PRODUCTEURCONTROLLER_H
#define PRODUCTEURCONTROLLER_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>

class ProducteurController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int frameRate READ frameRate WRITE setFrameRate NOTIFY frameRateChanged)
    Q_PROPERTY(int sentFrames READ sentFrames)
    Q_PROPERTY(int missedFrames READ missedFrames)
    Q_PROPERTY(int effectiveRate READ effectiveFrameRate)
    Q_PROPERTY(bool realTime READ realTime WRITE setRealTime NOTIFY contractChanged)

public:
    explicit ProducteurController(QObject *parent = nullptr);
    //Pure copy
    ProducteurController(const ProducteurController& orig);
    void setFrameRate(const int rateInFramePerSec) {
        sendRate_ = rateInFramePerSec;
        //totalDuration_ = 0;
        qDebug() << "Nouveau débit : " << sendRate_ << " fps";
        framesStatus_.clear();
    }
    int frameRate() const { return sendRate_; }
    int effectiveFrameRate() const { return effectiveRate_; }
    int missedFrames() const;
    int sentFrames() const { return currentFrameId_; }
    bool realTime() const { return realTime_; }
    void setRealTime(const bool realTime) {
        realTime_ = realTime;
        qDebug() << "Contrat modifié";
    }

    enum class FrameStatus {
        SENT,
        ACK,
        MISSED
    };

signals:
    void connectingToConsommateur();
    void connectedToConsommateur();
    void traitementStoppe();
    void frameRateChanged(int);
    void frameSent();
    void contractChanged();

public slots:
    void connectToConsommateur();
    void stop();

private slots:
    void onClientConnected();
    void onClientError(QAbstractSocket::SocketError);
    void sendNewFrame();
    void onClientDataReceived();
    void onTraitementStoppe();
    void onCommMonitor();

private:
    QTcpSocket *client_ = nullptr;
    int sendRate_ = 1; //In frames per second
    bool canContinue_ = false;
    bool acqReceived_ = false;
    QMap<int /* frameId */, FrameStatus> framesStatus_;
    int currentFrameId_ = 0;
    qint64 lastFrameSentAt_ = 0;
    int effectiveRate_ = 1;
    bool realTime_ = true;
    QTimer commMonitor_;
    bool mustDisconnect_ = false;
};

Q_DECLARE_METATYPE(ProducteurController)

#endif // PRODUCTEURCONTROLLER_H
