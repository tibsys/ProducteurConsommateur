#ifndef CONSOMMATEURCONTROLLER_H
#define CONSOMMATEURCONTROLLER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>
#include <QThread>

class BufferConsumer;

class ConsommateurController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int treatmentDuration READ treatmentDuration WRITE setTreatmentDuration)
    Q_PROPERTY(int receivedFrames READ receivedFrames)
    Q_PROPERTY(int sentAck READ sentAck)

public:
    explicit ConsommateurController(QObject *parent = nullptr);
    ConsommateurController(const ConsommateurController& orig); //Pure copie

    enum class Mode {
        SYNCHRONE,
        ASYNCHRONE,
        PRODUCTEUR_CONSOMMATEUR
    };

    void setMode(Mode mode) {
        mode_ = mode;
        qDebug() << "Nouveau mode : " << modeToString(mode_);
    }

    QString modeToString(Mode mode) const {
        switch(mode) {
        case Mode::SYNCHRONE: return "synchrone";
        case Mode::ASYNCHRONE: return "asynchrone";
        case Mode::PRODUCTEUR_CONSOMMATEUR: return "producteur-consommateur";
        }
    }

    int treatmentDuration() const { return treatmentDurationInMillis_; }
    void setTreatmentDuration(int duration);
    int receivedFrames() const { return nbReceivedFrames_; }
    int sentAck() const { return nbAckSent_; }

signals:
    void serverError();

public slots:
    void startServer();
    void setModeSynchrone();
    void setModeASynchrone();
    void setModeProducteurConsommateur();
    void sendAck();

private slots:
    void onNewConnection();
    void onDataReceived();
    void onDataSent(qint64);
    void traitementAsynchrone(int currentStep = 0);
    void reset() {
        nbReceivedFrames_ = 0;
        nbAckSent_ = 0;
    }

private:
    QTcpServer *server_ = nullptr;
    QTcpSocket *client_ = nullptr;
    Mode mode_ = Mode::SYNCHRONE;
    int treatmentDurationInMillis_ = 50;
    int nbReceivedFrames_ = 0;
    int nbAckSent_ = 0;
    QStringList buffer_;
    BufferConsumer *bufferConsumer_;
};

Q_DECLARE_METATYPE(ConsommateurController)


class BufferConsumer : public QThread {
    Q_OBJECT

public:
    BufferConsumer(QStringList &buffer, QObject* parent = nullptr)
        : QThread(parent), buffer_(buffer) {}

    void setDureeTraitement(int dureeTraitement) { treatmentDurationInMillis_ = dureeTraitement; }

public slots:
    void run();
    void stop() { canContinue_ = false; }

signals:
    void traitementTermine();

private:
    bool canContinue_ = false;
    QStringList& buffer_;
    int treatmentDurationInMillis_ = 50;
};


#endif // CONSOMMATEURCONTROLLER_H
