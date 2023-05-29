#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QUrl>
#include <QDesktopServices>
#include <QFileDialog>
#include "filedownload.h"
#include <QSystemTrayIcon>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void init();
    void parseJson(QByteArray data);
    void firstGetData();
    void secondGetData();
    void thirdGetData();

private slots:
    void onReplied(QNetworkReply *reply);

    void on_sharedContentClearBtn_clicked();

    void on_clearDownloadTextBtn_clicked();

    void on_sharedContentAnalysisBtn_clicked();

    void on_browserDownloadBtn_clicked();

    void on_downloadBtn_clicked();

    void handleDownloadFinished();

    void isFileExists(quint8 v);


private:
    Ui::Widget *ui;
    QNetworkAccessManager *mNetAccessManager;
    int mFileId;
    QString mS3KeyFlag;
    int mSize;
    QString mEtag;
    QString mDownloadUrl;
    QString mShareKey;
    QString mSharePwd;
    QString mFileName;
    QString mFileDir;
    QString mFilePath;
};
#endif // WIDGET_H
