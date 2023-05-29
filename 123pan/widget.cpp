#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    init();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::init()
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/favicon.ico"));
    setWindowTitle(tr("123盘-直链解析器"));
    resize(600,300);
    mNetAccessManager = new QNetworkAccessManager;
    connect(mNetAccessManager, &QNetworkAccessManager::finished, this, &Widget::onReplied);
}

void Widget::parseJson(QByteArray data)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    if(jsonDoc.isNull()){
        QMessageBox::warning(this, tr("获取数据失败"), tr("没有获取到预期中的数据！"));
        return;
    }
    if(!jsonDoc.isObject()){
        QMessageBox::warning(this, tr("数据类型错误"), tr("没有获取到预期的数据类型"));
        return;
    }
    QJsonObject jsonObj = jsonDoc.object();
    int code = jsonObj["code"].toInt();
    QString message = jsonObj["message"].toString();
    if(code != 0){
        QMessageBox::warning(this, tr("提示"), message);
    }
    if(jsonObj.contains("data")){
        QJsonObject dataObj = jsonObj["data"].toObject();
        if(dataObj.contains("InfoList")){
            QJsonArray InfoListArray = dataObj["InfoList"].toArray();
            if(!InfoListArray.isEmpty()){
                QJsonObject firstObj = InfoListArray.first().toObject();
                int type = firstObj["type"].toInt();
                if(type){
                    QMessageBox::warning(this, tr("非单文件"), tr("暂不支持文件夹下载！"));
                    return;
                }
                mFileId = firstObj["FileId"].toInt();
                mS3KeyFlag = firstObj["S3KeyFlag"].toString();
                mSize = firstObj["Size"].toInt();
                mEtag = firstObj["Etag"].toString();
                secondGetData();
            }
        }else if(dataObj.contains("DownloadURL")){
            mDownloadUrl = dataObj["DownloadURL"].toString();
            thirdGetData();
        }else if(dataObj.contains("redirect_url")){
            QString url = dataObj["redirect_url"].toString();
            ui->downloadTextBrowser->setText(url);
        }
    }
}

void Widget::firstGetData()
{
    QUrl url(tr("https://www.123pan.com/b/api/share/get?limit=100&next=1&orderBy=share_id&orderDirection=desc&shareKey=%1&SharePwd=%2&ParentFileId=0&Page=1").arg(mShareKey,mSharePwd));
    mNetAccessManager->get(QNetworkRequest(url));
}

void Widget::secondGetData()
{
    QUrl url("https://www.123pan.com/b/api/share/download/info");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json;charset=UTF-8");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/113.0.0.0 Safari/537.36");

    QJsonObject jsonData;
    jsonData["ShareKey"] = mShareKey;
    jsonData["FileID"] = mFileId;
    jsonData["S3keyFlag"] = mS3KeyFlag;
    jsonData["Size"] = mSize;
    jsonData["Etag"] = mEtag;

    QJsonDocument jsonDoc(jsonData);
    QByteArray postData = jsonDoc.toJson();
    mNetAccessManager->post(request, postData);
}

void Widget::thirdGetData()
{
    QRegularExpression regexDownloadUrl("params=([^&]+)");
    QRegularExpressionMatch matchDownloadUrl = regexDownloadUrl.match(mDownloadUrl);
    if (matchDownloadUrl.hasMatch()) {
        QString encodedString = matchDownloadUrl.captured(1);
        QByteArray byteArray = QByteArray::fromBase64(encodedString.toUtf8());
        QString decodedString = QString::fromUtf8(byteArray);
        QString strUrl = tr("%1&auto_redirect=0").arg(decodedString);
        QUrl url(strUrl);
        mNetAccessManager->get(QNetworkRequest(url));
    }
}

void Widget::onReplied(QNetworkReply *reply)
{
    quint8 status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt();
    if(reply->error() != QNetworkReply::NoError || status_code != 200){
        QMessageBox::warning(this, tr("网络错误"), tr("请求失败，没有网络或123盘服务失效！"));
        return;
    }else{
        QByteArray reply_data = reply->readAll();
        QByteArray data  = QString(reply_data).toUtf8();
        parseJson(data);
    }
    reply->deleteLater();
}

void Widget::on_sharedContentClearBtn_clicked()
{
    ui->sharedLineEdit->clear();
    ui->sharedLineEdit->setFocus();
}

void Widget::on_clearDownloadTextBtn_clicked()
{
    ui->downloadTextBrowser->clear();
    ui->sharedLineEdit->setFocus();
}

void Widget::on_sharedContentAnalysisBtn_clicked()
{
    QString strText = ui->sharedLineEdit->text();
    if(strText.isEmpty()){
        QMessageBox::warning(this, tr("文本为空"), tr("请输入分享的链接和提取码"));
        return;
    }

    QRegularExpression regexUrl("/s/(.*?)\\.");
    QRegularExpressionMatch matchUrl = regexUrl.match(strText);
    if(matchUrl.hasMatch()){
        mShareKey = matchUrl.captured(1);
    }else{
        QMessageBox::warning(this, tr("链接错误"), tr("链接非123盘分享链接！"));
        return;
    }

    QRegularExpression regexPwd("提取码:([A-Za-z_0-9]{4}?)");
    QRegularExpressionMatch matchPwd = regexPwd.match(strText);
    if(matchPwd.hasMatch()){
        mSharePwd = matchPwd.captured(1);
    }else{
        mSharePwd = "";
    }
    firstGetData();
}

void Widget::on_browserDownloadBtn_clicked()
{
    QString strText = ui->downloadTextBrowser->toPlainText();
    if(strText.isEmpty()){
        QMessageBox::warning(this, tr("错误"), tr("暂无解析后的链接"));
        return;
    }
    QDesktopServices::openUrl(QUrl(strText));
}

void Widget::on_downloadBtn_clicked()
{
    QString strText = ui->downloadTextBrowser->toPlainText();
    if(strText.isEmpty()){
        QMessageBox::warning(this, tr("错误"), tr("暂无解析后的链接"));
        return;
    }

    QRegularExpression regexUrl("&filename=(.*?)&x-mf-biz-cid=");
    QRegularExpressionMatch matchUrl = regexUrl.match(strText);
    if(matchUrl.hasMatch()){
        mFileName = matchUrl.captured(1);
    }

    mFileDir = QFileDialog::getExistingDirectory(nullptr, tr("为 %1 -选择下载目录").arg(mFileName), QDir::homePath());
    if(mFileDir.isEmpty()){
        return;
    }
    mFilePath = QDir(mFileDir).filePath(mFileName);
    isFileExists(0);

    FileDownload *fileDownload = new FileDownload(strText, mFilePath);
    connect(fileDownload, &FileDownload::downloadFinished, this, &Widget::handleDownloadFinished);
    fileDownload->start();
}

void Widget::handleDownloadFinished()
{
    QSystemTrayIcon trayIcon;
    trayIcon.setIcon(QIcon(":/logo.png"));
    trayIcon.show();
    trayIcon.showMessage(tr("%1 下载完成").arg(mFileName), tr("下载位置：%1").arg(mFilePath), QSystemTrayIcon::Information, 5000);
}

void Widget::isFileExists(quint8 v)
{
    if(QFile::exists(mFilePath)){
        v++;
        QString fileFirstName,fileSecondName;
        int lastDotIndex = mFileName.lastIndexOf(".");
        if (lastDotIndex != -1 && lastDotIndex < mFileName.length() - 1) {
            fileFirstName = mFileName.left(lastDotIndex)+"("+QString::number(v)+")";
            fileSecondName = mFileName.mid(lastDotIndex + 1);
        }
        mFilePath = QDir(mFileDir).filePath(fileFirstName+"."+fileSecondName);
        isFileExists(v);
    }
    return;
}

