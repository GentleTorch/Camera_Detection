// Qt lib import
#include <QPointer>
#include <QThread>
#include <QtCore>
#include <QtGlobal>
#include <iostream>

// JQLibrary import
#include "JQNet.h"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    qsrand(static_cast<uint>(QTime(0, 0, 0).secsTo(QTime::currentTime())));
    QString arr[2] = { "0", "1" };

    for (int i = 0; i < 100; i++) {
        QJsonObject obj;

        obj.insert("CameraSpin", arr[qrand()%2]);
        obj.insert("CameraDetect", arr[qrand()%2]);
        obj.insert("AutoFocus", arr[qrand()%2]);

        const auto&& reply = JQNet::HTTP::post("https://192.168.100.88:24684/pc_vas/CruiseControl", QJsonDocument(obj).toJson());

        qDebug() << i << ": " << reply.first << reply.second;

        //            QThread::msleep(1000);
        //            QString cur = QDateTime::currentDateTime().toLocalTime().toString();
        //            qDebug() << cur;
    }

    return 0;
}
