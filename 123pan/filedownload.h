#ifndef FILEDOWNLOAD_H
#define FILEDOWNLOAD_H

#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>

class FileDownload : public QThread
{
    Q_OBJECT
public:
    FileDownload(const QString& url, const QString& filePath);
    void run();

signals:
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished(bool success);

private:
    QString mUrl;
    QString mFilePath;
};

#endif // FILEDOWNLOAD_H
