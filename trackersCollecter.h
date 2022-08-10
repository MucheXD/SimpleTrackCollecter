#pragma once

#include <QtWidgets/QWidget>
#include <QFile>
#include <QString>
#include <QThread>
#include <QClipBoard>
#include <QProcess>
#include <Windows.h>
#include <QDebug>
#include "ui_trackersCollecter.h"
#import "winhttpcom.dll"

class trackersCollecter : public QWidget
{
    Q_OBJECT

public:
    trackersCollecter(QWidget *parent = nullptr);
    ~trackersCollecter();

private:
    void getLocalTrackers();
	void getOnlineTrackers();
	void nextOnlineSource();
	void copy();
	void refresh();
	void editLocalTrackers();
	void editOnlineSources();
    void addTrackers(QString name, QString form);

	QStringList waitingOnlineSources;
    Ui::trackersCollecterClass ui;
};

class thread_onlineCollector :
	public QThread
{
	Q_OBJECT
public:
	QString url;
	void run();
signals:
	void trackerRet(QString tracker, QString from);
	void finishWork();
};

class Network
{
public:
	WinHttp::IWinHttpRequestPtr pHttpWork;
	QString sendBody;
	QStringList headers[2];//0位存储标头 1位存储头内容
	Network();
	int sendHttpRequest(QString method, QString url);
	QByteArray getHttpData();//先发送请求
	QString getHttpHeader(QString header);//先发送请求
	/*结束代码
	* 1=成功
	* 0=未调用
	* -3=无法初始化连接
	* -4=无法获取数据
	*/
private:
};