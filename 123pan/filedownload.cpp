#include "filedownload.h"


FileDownload::FileDownload(const QString &url, const QString &filePath)
{
    mUrl = url;
    mFilePath = filePath;
}

void FileDownload::run()
{
    QNetworkAccessManager manager;
    QUrl url(mUrl);
    QNetworkRequest request(url);
    QNetworkReply* reply = manager.get(request);

    QObject::connect(reply, &QNetworkReply::downloadProgress,this, [&](qint64 bytesReceived, qint64 bytesTotal) {
        emit downloadProgress(bytesReceived, bytesTotal);
    });
    QObject::connect(reply, &QNetworkReply::finished,this, [&]() {
        if (reply->error() == QNetworkReply::NoError) {
            QFile file(mFilePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reply->readAll());
                file.close();
                emit downloadFinished(true);
            } else {
                emit downloadFinished(false);
            }
        } else {
            emit downloadFinished(false);
        }
        reply->deleteLater();
    });
    exec();
}
