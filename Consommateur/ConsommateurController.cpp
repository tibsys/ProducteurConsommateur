#include "ConsommateurController.h"
#include <QHostAddress>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>


void BufferConsumer::run() {
    canContinue_ = true;

    while(canContinue_) {
        if(!buffer_.empty()) {
            //On dépile le buffer unitairement... on pourrait améliorer ca...
            buffer_.takeFirst();

            //Fait le traitement (toujours bloquant...)
            QThread::msleep(treatmentDurationInMillis_); //ATTENTION: très mauvaise pratique !
            emit traitementTermine();
        }
    }

    qDebug() << "Consommateur de buffer arrêté";

}

ConsommateurController::ConsommateurController(QObject *parent)
    : QObject(parent), server_(new QTcpServer(this)), bufferConsumer_(new BufferConsumer(buffer_, this))
{    
    connect(server_, &QTcpServer::newConnection, this, &ConsommateurController::onNewConnection);

    //Initialise le consommateur de buffer pour le mode Producteur-Consommateur
    connect(this, &QObject::destroyed, bufferConsumer_, &BufferConsumer::stop);
    connect(bufferConsumer_, &BufferConsumer::traitementTermine, this, &ConsommateurController::sendAck);    
    connect(this, &QObject::destroyed, bufferConsumer_, &QThread::quit);    
}

ConsommateurController::ConsommateurController(const ConsommateurController &orig)
    : QObject(orig.parent()), server_(orig.server_)
{}

void ConsommateurController::setTreatmentDuration(int duration)
{
    treatmentDurationInMillis_ = duration;
    if(bufferConsumer_ != nullptr) {
       bufferConsumer_->setDureeTraitement(treatmentDurationInMillis_);
    }
    qDebug() << "Durée du traitement définie à " << treatmentDurationInMillis_ << " ms";
}

void ConsommateurController::startServer()
{
    if(!server_->listen(QHostAddress::LocalHost, 12345)) {
        emit serverError();
    } else {
        qDebug() << "Consommateur démarré en mode synchrone";
    }
}

void ConsommateurController::setModeSynchrone() {
    setMode(Mode::SYNCHRONE);
    if(bufferConsumer_ != nullptr) {
        bufferConsumer_->stop();
    }
}

void ConsommateurController::setModeASynchrone() {
    setMode(Mode::ASYNCHRONE);
    if(bufferConsumer_ != nullptr) {
        bufferConsumer_->stop();
    }
}

void ConsommateurController::setModeProducteurConsommateur() {
    setMode(Mode::PRODUCTEUR_CONSOMMATEUR);
    if(bufferConsumer_ != nullptr) {
        bufferConsumer_->start();
    }
}

void ConsommateurController::onNewConnection()
{
    if(client_ != nullptr) {
        if(client_->state() == QAbstractSocket::ConnectedState) {
            qWarning() << "Un client est déjà connecté";
            client_->close();
            return;
        } else {
            reset();
        }
    }

    qDebug() << "Nouvelle connexion";
    client_ = server_->nextPendingConnection();
    connect(client_, &QTcpSocket::readyRead, this, &ConsommateurController::onDataReceived);
    connect(client_, &QTcpSocket::bytesWritten, this, &ConsommateurController::onDataSent);
}

static QMutex sendDataMutex_;
void ConsommateurController::onDataReceived()
{
    {
        //Section critique
        QMutexLocker l(&sendDataMutex_);
        nbReceivedFrames_ += 1;
        QTcpSocket *client = qobject_cast<QTcpSocket*>(QObject::sender());
        //Lecture des données
        QByteArray data = client->readAll();

        //qDebug() << data;
        if(mode_ == Mode::SYNCHRONE) {
            QThread::msleep(treatmentDurationInMillis_); //ATTENTION: très mauvaise pratique !
            sendAck();
        } else if(mode_ == Mode::ASYNCHRONE) {
            traitementAsynchrone();
        } else if(mode_ == Mode::PRODUCTEUR_CONSOMMATEUR) {
            buffer_ << data; //On stocke simplement les données
        }
    }
}

void ConsommateurController::sendAck()
{
    client_->write("ACK\r\n");
}

void ConsommateurController::onDataSent(qint64) {
    nbAckSent_ += 1;
}

void ConsommateurController::traitementAsynchrone(int currentStep)
{
    //Le traitement asynchrone est un traitement découpé en
    //plusieurs sous-traitements bloquants très peu de temps
    //La fonction doit être réentrante...
    //Nous allons simuler le nombre d'étapes en faisant boucler la
    //fonction sur elle même de manière asynchrone par étapes de 20ms
    //chacune maximum

    if(treatmentDurationInMillis_ <= 20) { //Si la durée est inférieure à 50ms, on ne boucle pas
        QThread::msleep(treatmentDurationInMillis_);
        sendAck();
        return;
    }

    //Calcul du nombre théorique d'itérations
    double nbIterations = (treatmentDurationInMillis_/20.0);
    //qDebug() << "nbIterations=" << nbIterations << ", currentStep=" << currentStep;

    if(currentStep >= nbIterations) {
        //on arrête
        sendAck();
        return;
    } else {
        //On fait le traitement
        QThread::msleep(20);

        //Puis on va à la prochaine itération en passant par la run-loop
        QMetaObject::invokeMethod(this, "traitementAsynchrone", Qt::QueuedConnection, Q_ARG(int, currentStep+1));
    }
}
