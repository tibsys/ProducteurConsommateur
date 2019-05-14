#include "ProducteurController.h"
#include <QDateTime>
#include <QTimer>
#include <QMutexLocker>
#include <QHostAddress>

ProducteurController::ProducteurController(QObject *parent)
    : QObject(parent), client_(new QTcpSocket(this))
{
    connect(client_, &QTcpSocket::connected, this, &ProducteurController::onClientConnected);
    connect(client_, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onClientError(QAbstractSocket::SocketError)));
    connect(client_, &QTcpSocket::readyRead, this, &ProducteurController::onClientDataReceived);
    connect(&commMonitor_, &QTimer::timeout, this, &ProducteurController::onCommMonitor);

    commMonitor_.setInterval(500);
    commMonitor_.setSingleShot(true);

    qDebug() << "Producteur instancié";
}

ProducteurController::ProducteurController(const ProducteurController &orig)
    : QObject(orig.parent()), client_(orig.client_) {}

int ProducteurController::missedFrames() const
{
    int count = 0;

    foreach(FrameStatus status, framesStatus_.values()) {
        if(status == FrameStatus::MISSED) {
            count += 1;
        }
    }

    return count;
}

void ProducteurController::connectToConsommateur()
{
    emit connectingToConsommateur();
    qDebug() << "Connection au consommateur sur le port TCP 12345";
    client_->connectToHost(QHostAddress::LocalHost, 12345);
}

void ProducteurController::stop()
{
    qDebug() << "Arrête le traitement";
    canContinue_ = false;
}

void ProducteurController::onClientConnected()
{
    qDebug() << "Nous sommes connecté au consommateur";
    canContinue_ = true;
    mustDisconnect_ = false;
    emit connectedToConsommateur();
    //lastFrameSentAt_ = QDateTime::currentMSecsSinceEpoch();
    effectiveRate_ = 1;
    currentFrameId_ = 0;
    framesStatus_.clear();    
    QTimer::singleShot(0, this, SLOT(sendNewFrame()));
}

void ProducteurController::onClientError(QAbstractSocket::SocketError)
{
    qWarning() << "L'erreur suivante s'est produite : " << client_->errorString();
}

static QMutex framesMapMutex_;
void ProducteurController::onClientDataReceived()
{
    //qDebug() << "Réponse reçue du consommateur";
    commMonitor_.start();

    {
        //Section critique
        QMutexLocker l(&framesMapMutex_);
        if(framesStatus_[currentFrameId_] == FrameStatus::SENT) {
            framesStatus_[currentFrameId_] = FrameStatus::ACK;
        }
    }

}

void ProducteurController::sendNewFrame()
{
    //Avant d'envoyer une nouvelle donnée on regarde si on a reçu l'ACK
    //de la précédente, dans le cas contraire on la compte comme perdue
    //si le contrat est du type temps réel
    { //Section critique
        if(realTime_) {
            QMutexLocker l(&framesMapMutex_);
            if(framesStatus_[currentFrameId_] == FrameStatus::SENT) {
                framesStatus_[currentFrameId_] = FrameStatus::MISSED;
            }
        }

        currentFrameId_ += 1;
    }

    //We compute the effective rate by computing an average operation
    //totalDuration_ += (QDateTime::currentMSecsSinceEpoch() - lastFrameSentAt_) +1; //+1 is to avoid division by 0
    //Ceci est un débit instantané
    double r = 1000.0/(QDateTime::currentMSecsSinceEpoch() - lastFrameSentAt_ +1); //+1 is to avoid division by 0
    effectiveRate_ = qRound(r);

    //qDebug() << "Envoi d'une trame";
    client_->write("01234567890\r\n");
    lastFrameSentAt_ = QDateTime::currentMSecsSinceEpoch();

    if(!canContinue_) {
        QTimer::singleShot(0, this, SLOT(onTraitementStoppe()));
        return;
    }

    int delay = (1000/sendRate_);
    QTimer::singleShot(delay, this, SLOT(sendNewFrame()));
    commMonitor_.start();
}

void ProducteurController::onTraitementStoppe()
{
    qDebug() << "Le traitement est stoppé. Déclenchement du timer de déconnexion.";
    mustDisconnect_ = true;
}

void ProducteurController::onCommMonitor()
{
    if(mustDisconnect_) {
        qDebug() << "Surveillance de la connexion : coupure de la connexion demandée";
        client_->disconnectFromHost();
        if(client_->state() != QAbstractSocket::UnconnectedState) {
            client_->waitForDisconnected(); //Pourrait-être géré en asynchrone
        }

        emit traitementStoppe();
    }
}
