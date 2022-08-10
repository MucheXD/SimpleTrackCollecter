#include "trackersCollecter.h"

trackersCollecter::trackersCollecter(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
	refresh();
	connect(ui.pushButton_copy, &QPushButton::clicked, this, &trackersCollecter::copy);
	connect(ui.pushButton_refresh, &QPushButton::clicked, this, &trackersCollecter::refresh);
	connect(ui.pushButton_editLocalTrackers, &QPushButton::clicked, this, &trackersCollecter::editLocalTrackers);
	connect(ui.pushButton_editOnlineSources, &QPushButton::clicked, this, &trackersCollecter::editOnlineSources);
}

void trackersCollecter::editLocalTrackers()
{
	QFile file;
	file.setFileName(QCoreApplication::applicationDirPath() + "/trackers.txt");
	if (file.exists() == false)
	{
		file.open(QIODevice::ReadWrite);
		file.close();
	}
	QString a = QString("explorer.exe \"%1\"").arg(file.fileName());
	QProcess::execute(QString("explorer.exe \"%1\"").arg(file.fileName()));
}

void trackersCollecter::editOnlineSources()
{
	QFile file;
	file.setFileName(QCoreApplication::applicationDirPath() + "/sources.txt");
	if (file.exists() == false)
	{
		file.open(QIODevice::ReadWrite);
		file.close();
	}
	QString a = QString("explorer.exe \"%1\"").arg(file.fileName());
	QProcess::execute(QString("explorer.exe \"%1\"").arg(file.fileName()));
}

void trackersCollecter::copy()
{
	QString text;
	int nAp = 0;
	while (nAp < ui.listTable->rowCount())
	{
		text += ui.listTable->item(nAp, 0)->text();
		text += "\n";
		nAp += 1;
	}
	QClipboard* cb = QApplication::clipboard();
	cb->setText(text);
}

void trackersCollecter::refresh()
{
	while (ui.listTable->rowCount() != 0)
		ui.listTable->removeRow(0);
	getLocalTrackers();
	getOnlineTrackers();
}

void trackersCollecter::getLocalTrackers()
{
    QFile file;
    file.setFileName(QCoreApplication::applicationDirPath() + "/trackers.txt");
    if (file.exists() == false)
        return;
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    while (!file.atEnd())
    {
		QString line = file.readLine().replace("\n", "");
        addTrackers(line, "Local");
    }
}

void trackersCollecter::getOnlineTrackers()
{
	QFile file;
	file.setFileName(QCoreApplication::applicationDirPath() + "/sources.txt");
	if (file.exists() == false)
		return;
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	waitingOnlineSources.clear();
	while (!file.atEnd())
	{
		QString line = file.readLine().replace("\n", "");
		
		waitingOnlineSources.push_back(line);
	}
	nextOnlineSource();
}

void trackersCollecter::nextOnlineSource()
{
	ui.pushButton_refresh->setEnabled(false);
	if (waitingOnlineSources.isEmpty() == false)
	{
		ui.title_state->setText(QString("Collecting from online source... %1 more sources waiting").arg(waitingOnlineSources.count()));
		thread_onlineCollector* thread_otc = new thread_onlineCollector;
		thread_otc->url = waitingOnlineSources.at(0);
		waitingOnlineSources.removeAt(0);
		connect(thread_otc, &thread_onlineCollector::trackerRet, this, &trackersCollecter::addTrackers);
		connect(thread_otc, &thread_onlineCollector::finishWork, this, &trackersCollecter::nextOnlineSource);
		thread_otc->start();
	}
	else
	{
		ui.title_state->setText(QString("Ready"));
		ui.pushButton_refresh->setEnabled(true);
		return;
	}
}

void trackersCollecter::addTrackers(QString name, QString from)
{
    int nCheck = 0;
    while (nCheck < ui.listTable->rowCount())
    {
        if (name == ui.listTable->item(nCheck, 0)->text())
            return;
        nCheck += 1;
    }
    const int nRC = ui.listTable->rowCount();
    ui.listTable->insertRow(nRC);
    ui.listTable->setItem(nRC, 0, new QTableWidgetItem(name));
    ui.listTable->setItem(nRC, 1, new QTableWidgetItem(from));
    ui.val_collectedTrackers->setText(QString::number(ui.listTable->rowCount()));
}

trackersCollecter::~trackersCollecter()
{}

void thread_onlineCollector::run(void)
{
	Network network;
	int code = network.sendHttpRequest("GET", url);
	if (code == 200)
	{
		QString responseText = network.getHttpData() + "\n";
		while (responseText.indexOf("\n") != -1)
		{
			const QString line = responseText.left(responseText.indexOf("\n"));
			if (line != "" && line != "\n")
				emit trackerRet(line.toUtf8(), url);
			responseText.remove(0, responseText.indexOf("\n") + 1);
			//Sleep(1);
		}
		emit finishWork();
	}
	else
		emit finishWork();
}

Network::Network()
{
	sendBody = NULL;
	HRESULT isSuccessedCheck = CoInitialize(NULL);
	pHttpWork.CreateInstance(__uuidof(WinHttp::WinHttpRequest));
}

int Network::sendHttpRequest(QString method, QString url)
{
	HRESULT isSuccessedCheck;
	//DEBUG 此处的代理仅供调试使用
	isSuccessedCheck = pHttpWork->Open(method.toStdWString().c_str(), url.toStdWString().c_str());
	if (FAILED(isSuccessedCheck))
	{
		return -3;
	}
	int nAddHeader = 0;
	while (headers[0].size() > nAddHeader)
	{
		pHttpWork->SetRequestHeader(headers[0].at(nAddHeader).toStdWString().c_str(), headers[1].at(nAddHeader).toStdWString().c_str());;
		nAddHeader += 1;
	}
	try
	{
		isSuccessedCheck = pHttpWork->Send(sendBody.toStdWString().c_str());
	}
	catch (...)
	{
		return -4;
	}
	return pHttpWork->Status;
}

QByteArray Network::getHttpData()
{
	_variant_t body = pHttpWork->GetResponseBody();
	ULONG dataLen = body.parray->rgsabound[0].cElements;
	QByteArray data((char*)body.parray->pvData, dataLen);
	return data;
}

QString Network::getHttpHeader(QString header)
{
	if (header == "")
	{
		_variant_t header = pHttpWork->GetAllResponseHeaders();
		ULONG dataLen = header.parray->rgsabound[0].cElements;
		QByteArray headerData((char*)header.parray->pvData, dataLen);
		return headerData;
	}
	else
	{
		return (LPCSTR)pHttpWork->GetResponseHeader(header.toStdWString().c_str());
	}
}