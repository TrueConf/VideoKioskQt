#include "stdafx.h"

#include "QtCallXDemo.h"
#include "ConfigDlg.h"
#include "ImageButton.h"

/// predefined path to save logs
static const wchar_t csNotificationOutputFileNameUsingQT[] = L"Logs\\NotificationOutputUsingQT.txt";

/// registry prefix, used to save and load app settingsF
static const char csRegistryPrefix[] = "HKEY_CURRENT_USER\\Software\\TrueConf\\QtCallXDemo";

/// user status constants
static const int ciUserStatusOnline = 1;

/// log functions
static void logMsgToFileQT(const QString& aFunctionName, const QString& aMsg);
static void logMsgToFileQT(const QString& aFunctionName, const std::wstring& aMsg);
static void logMsgToFileQT(const QString& aFunctionName, const wchar_t* aMsg);

#define LOG(desc) (logMsgToFileQT(__FUNCTION__, desc))


QtCallXDemo::QtCallXDemo(bool showConfigDialog, QWidget *parent)
	: QMainWindow(parent),
	m_CallXState(ECallXState::ECXS_None),
	m_IsCallXInitialized(false),
	m_CameraDimensionsProportion(1),
	m_ShowConfigDlg(showConfigDialog)
{
	ui.setupUi(this);

	setWindowTitle("VideoKioskQt");
	
	/// you can use one of the four methods to create a TrueConfCallX COM control using axWidget in QT
	
	/// using class id
	bool bResult = ui.controlTrueConfCallX->setControl("TrueConfCallX Class");

	/// using version independent prod id
	// bResult = ui.controlTrueConfCallX->setControl("ID_TrueConfCallX");

	/// using prog id
	// bResult = ui.controlTrueConfCallX->setControl("ID_TrueConfCallX.1");

	/// using guid
	// bResult = ui.controlTrueConfCallX->setControl("{27EF4BA2-4500-4839-B88A-F2F4744FE56A}");

	this->installEventFilter(this);

	// adjust focus policy
	Qt::FocusPolicy eFP = ui.controlTrueConfCallX->focusPolicy();
	ui.controlTrueConfCallX->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	eFP = ui.controlTrueConfCallX->focusPolicy();


	/// loading setting from registry
	loadSettings();

	/// init CallX smartptr
	initCOMSupplementatryObjects();

	/// connect qt slots to active-x events
	connectQtSlotsToComObjectEvents();

	/// init buttons, status label, background and other adjustments
	initCallButtonAndStatusLabel();
	
	/// connect event processors (slots) to apropriate events
	connectButtonsAndWidgetEvents();
}

QtCallXDemo::~QtCallXDemo()
{
	/// CallX deinitialization already done in QtCallXDemo::closeEvent
	/// smartptr will decrease its ref count automatically in its destructor
}

CallXSettings& QtCallXDemo::GetCallXSettings()
{
	return m_CallXSettings;
}

TrueConf_CallXLib::ITrueConfCallXPtr QtCallXDemo::GetCallX()
{
	return m_CallXComPtr;
}

void QtCallXDemo::SaveSettings()
{
	QSettings settings(csRegistryPrefix, QSettings::NativeFormat);
	settings.setValue("ServerName", QVariant(m_CallXSettings.sServerName));
	settings.setValue("Login", QVariant(m_CallXSettings.sLogin));
	settings.setValue("Password", QVariant(m_CallXSettings.sPassword));
	settings.setValue("Camera", QVariant(m_CallXSettings.sCamera));
	settings.setValue("Speaker", QVariant(m_CallXSettings.sSpeaker));
	settings.setValue("Microphone", QVariant(m_CallXSettings.sMicrophone));

	settings.setValue("UserID", QVariant(m_CallXSettings.sUserID));
	settings.setValue("UserIDsList", QVariant(m_CallXSettings.sUserIDsList));
	settings.setValue("CallMethod", QVariant(m_CallXSettings.iCallMethod));

	settings.setValue("ManagerEmail", QVariant(m_CallXSettings.sManagerEmail));
	settings.setValue("NotifyManager", QVariant(m_CallXSettings.bNotifyManager));

	settings.setValue("ShowChatMessages", QVariant(m_CallXSettings.bShowChatMessages));
	settings.setValue("ShowMessageTime", QVariant(m_CallXSettings.iShowMessageTime));
}

void QtCallXDemo::InitHardware()
{
	/// initializes hardware
	if (!m_CallXSettings.sCamera.isEmpty())
	{
		m_CallXComPtr->Camera = QStringToWChar(m_CallXSettings.sCamera); 
	}

	if (!m_CallXSettings.sSpeaker.isEmpty())
	{
		m_CallXComPtr->Speaker = QStringToWChar(m_CallXSettings.sSpeaker);// wsSpeaker.c_str();
	}

	if (!m_CallXSettings.sMicrophone.isEmpty())
	{
		m_CallXComPtr->Microphone = QStringToWChar(m_CallXSettings.sMicrophone);
	}
}

bool QtCallXDemo::eventFilter(QObject* obj, QEvent* event)
{
	// will show config dialog
	if (processEventFilter(obj, event))
	{
		return true;
	}
	if (obj == this)
	{
		// if it is an event of a MainWindow let it process it 
		return QMainWindow::eventFilter(obj, event);
	}
	else
	{
		// it is event of a child control or other smth
		// return false so default processing of a control can occure
		return false;
	}
}

void QtCallXDemo::closeEvent(QCloseEvent *event)
{
	/// It is a good trend to deinitialize graphical component when window is closing
	if (m_CallXComPtr.GetInterfacePtr())
	{
		HRESULT hr = m_CallXComPtr->shutdown();

		/// using dipatch call by qt, provided here as an example
		//ui.controlTrueConfCallX->dynamicCall("shutdown()");
	}

	// call accept to indicate main window is ready to close
	event->accept();
}

void QtCallXDemo::ProcessOnXNotify(QString notify)
{
	LOG(notify);
}

void QtCallXDemo::ProcessOnXAfterStart()
{
	LOG(L"");
	m_IsCallXInitialized = true;

	InitHardware();

	connectToServer();

	if (m_ShowConfigDlg)
	{
		showConfigurationDlg();
	}
}

void  QtCallXDemo::ProcessOnServerConnected(QString eventDetails)
{
	LOG(eventDetails);

	if (!m_CallXSettings.sLogin.isEmpty())
	{
		m_CallXComPtr->login(QStringToWChar(m_CallXSettings.sLogin), QStringToWChar(m_CallXSettings.sPassword));
	}
}

void QtCallXDemo::ProcessOnXChangeState(int prevState, int newState)
{
	m_CallXState = newState;
	ui.labelCallXStatus->setText(QString("Current status: <%1>").arg(QString::number(newState)));

	switch (newState)
	{
	case ECallXState::ECXS_Connect:
		m_StatusIcon->setPixmap(getImage(":/QtCallXDemo/StatusIcon_Offline.png"));
		m_StatusIcon->setPixmap(getImage(":/QtCallXDemo/CallButton_Connecting.png"));
		break;

	case ECallXState::ECXS_Login:
		m_StatusIcon->setPixmap(getImage(":/QtCallXDemo/StatusIcon_Offline.png"));
		break;

	case ECallXState::ECXS_Normal:
		m_StatusIcon->setPixmap(getImage(":/QtCallXDemo/StatusIcon_Online.png"));
		m_ImageButton->setPixmap(getImage(":/QtCallXDemo/CallButton_ActionCall.png"));
		m_ImageButton->update();
		break;

	case ECallXState::ECXS_Wait:
		m_StatusIcon->setPixmap(getImage(":/QtCallXDemo/StatusIcon_Busy.png"));
		m_ImageButton->setPixmap(getImage(":/QtCallXDemo/CallButton_ActionCalling.png"));
		m_ImageButton->update();
		break;

	case ECallXState::ECXS_Conference:
		m_ImageButton->setPixmap(getImage(":/QtCallXDemo/CallButton_ActionReject.png"));
		m_ImageButton->update();
		break;
	}

	LOG(QString("prevState %1, newState = %2").arg(QString::number(prevState), QString::number(newState)));
}

void QtCallXDemo::ProcessOnXLogin()
{
	LOG(L"");
	m_CallXComPtr->getAbook();
}

void QtCallXDemo::ProcessOnXAbookUpdate(QString eventDetails)
{
	LOG(eventDetails);
	QJsonDocument jsonDoc = QJsonDocument::fromJson(eventDetails.toUtf8());
	if (!jsonDoc.isNull())
	{
		QJsonObject json = jsonDoc.object();
		extractUsersStatusOnAbookUpdate(json);
	}
}

void QtCallXDemo::ProcessOnXLoginError(int errorCode)
{
	LOG(QString("errorCode = %1").arg(QString::number(errorCode)));
	processLoginError(errorCode);
}

void QtCallXDemo::ProcessOnInviteReceived(QString eventDetails)
{
	LOG(eventDetails);

	m_CallXComPtr->accept();
}

void QtCallXDemo::ProcessOnRecordRequest(QString eventDetails)
{
	LOG(L"");
	m_CallXComPtr->allowRecord();
}

void QtCallXDemo::ProcessOnIncomingRequestToPodiumAnswered(QString eventDetails)
{
	LOG(eventDetails);
	m_CallXComPtr->acceptPodiumInvite();
}

void QtCallXDemo::ProcessOnUpdateCameraInfo(QString eventDetails)
{
	LOG(eventDetails);
	QJsonDocument jsonDoc = QJsonDocument::fromJson(eventDetails.toUtf8());
	if (!jsonDoc.isNull())
	{
		QJsonObject json = jsonDoc.object();
		if (json.contains("event") && json["event"].isString())
		{
			QString sEventName = json["event"].toString();
			if (sEventName == "updateCameraInfo")
			{
				int newResolutionX = json["cameraWidth"].toInt();
				int newResolutionY = json["cameraHeight"].toInt();
				if (newResolutionX == 0 || newResolutionY == 0)
				{
					m_CameraDimensionsProportion = 1.0;
				}
				else
				{
					m_CameraDimensionsProportion = (double(newResolutionX)) / newResolutionY;
					ui.controlTrueConfCallX->setFixedWidth(ui.controlTrueConfCallX->height() * m_CameraDimensionsProportion);
					m_ImageButton->setFixedWidth(ui.controlTrueConfCallX->width());
				}
			}
		}
	}
}

void QtCallXDemo::ProcessOnVideoMatrixChanged(QString eventDetails)
{
	LOG(eventDetails);
}

void QtCallXDemo::ProcessOnIncomingChatMessage(QString peerId, QString peerDn, QString message, qulonglong time)
{
	LOG(
		QString("peerId=%1, peerDn=%2, message=%3, time=%4").arg(peerId, peerDn, message, QString::number(time)));

	showChatMessage(peerId, peerDn, message, time);
}

void QtCallXDemo::ProcessOnIncomingGroupChatMessage(QString peerId, QString peerDn, QString message, qulonglong time)
{
	logMsgToFileQT(__FUNCTION__,
		QString("peerId=%1, peerDn=%2, message=%3, time=%4").arg(peerId, peerDn, message, QString::number(time)));

	showChatMessage(peerId, peerDn, message, time);
}

void QtCallXDemo::OnAction()
{
	if (m_CallXState == ECallXState::ECXS_Normal)
	{
		Call();
	}
	else if (m_CallXState == ECallXState::ECXS_Conference || m_CallXState == ECallXState::ECXS_Wait)
	{
		m_CallXComPtr->hangUp();
	}
}

void QtCallXDemo::UpdateMessagesByTime()
{
	m_ChatMessages.removeLast();
	updateMessages();
}

void QtCallXDemo::Call()
{
	switch (m_CallXSettings.iCallMethod)
	{
	case CallXSettings::ECM_UserID:
		if (!m_CallXSettings.sUserID.isEmpty())
		{
			if (checkUserStatusOnline(m_CallXSettings.sUserID))
			{
				doCall(m_CallXSettings.sUserID);
			}
			else
			{
				QMessageBox msgBox(QMessageBox::Icon::Warning, "Caution!", "Specifined user is offline!",
					QMessageBox::StandardButton::Ok, this);
				msgBox.exec();
			}
		}
		break;

	case CallXSettings::ECM_UserIDsList:
		if (!m_CallXSettings.sUserIDsList.trimmed().isEmpty())
		{
			bool onlineUserExists = false;
			QStringList lsUserIDs = m_CallXSettings.sUserIDsList.split(",");
			foreach(QString sUserID, lsUserIDs)
			{
				// trim spaces
				sUserID = sUserID.trimmed();

				if (!sUserID.isEmpty())
				{
					if (checkUserStatusOnline(sUserID))
					{
						onlineUserExists = true;
						doCall(sUserID);
					}
				}
			}
			if (!onlineUserExists)
			{
				QMessageBox msgBox(QMessageBox::Icon::Warning, "Caution!", "None of specified users is online!",
					QMessageBox::StandardButton::Ok, this);
				msgBox.exec();
			}
		}
		else
		{
			QMessageBox msgBox(QMessageBox::Icon::Warning, "Caution!", "Lisf of users to call is empty!",
				QMessageBox::StandardButton::Ok, this);
			msgBox.exec();
		}
		break;

	case CallXSettings::ECM_Phonebook:
		{
			QList<QString> onlineUsers;
			for(auto i = m_mapUserStatus.begin(); i != m_mapUserStatus.end(); ++i)
			{
				if (i.value() == ciUserStatusOnline)
				{
					onlineUsers.append(i.key());
				}
			}
			if (onlineUsers.size() > 0)
			{
				int iRandomUser = qrand() % onlineUsers.size();
				QString sUserID = onlineUsers[iRandomUser];
				doCall(sUserID);
			}
			else
			{
				QMessageBox msgBox(QMessageBox::Icon::Warning, "Caution!", "None of the addressbook user is online!",
					QMessageBox::StandardButton::Ok, this);
				msgBox.exec();
			}
		}
		break;
	

	default:
		// unknown case, do nothing
		break;
	}
}

void QtCallXDemo::loadSettings()
{
	QSettings settings(csRegistryPrefix, QSettings::NativeFormat);
	m_CallXSettings.sServerName = settings.value("ServerName", "ru11.trueconf.net").toString();
	m_CallXSettings.sLogin = settings.value("Login", "").toString();
	m_CallXSettings.sPassword = settings.value("Password", "").toString();
	m_CallXSettings.sCamera = settings.value("Camera", "").toString();
	m_CallXSettings.sSpeaker = settings.value("Speaker", "").toString();
	m_CallXSettings.sMicrophone = settings.value("Microphone", "").toString();

	m_CallXSettings.sUserID = settings.value("UserID", "").toString();
	m_CallXSettings.sUserIDsList = settings.value("UserIDsList", "").toString();
	m_CallXSettings.iCallMethod = settings.value("CallMethod", 0).toInt();

	m_CallXSettings.sManagerEmail = settings.value("ManagerEmail", "").toString();
	m_CallXSettings.bNotifyManager = settings.value("NotifyManager", false).toBool();

	m_CallXSettings.bShowChatMessages = settings.value("ShowChatMessages", false).toBool();
	m_CallXSettings.iShowMessageTime = settings.value("ShowMessageTime", 15).toInt();
}

void QtCallXDemo::initCOMSupplementatryObjects()
{
	// init callx smartpointer
	HRESULT hr = ui.controlTrueConfCallX->queryInterface(TrueConf_CallXLib::IID_ITrueConfCallX, (void**)&m_CallXComPtr.GetInterfacePtr());
}

void QtCallXDemo::connectQtSlotsToComObjectEvents()
{
	// Note that slot name better be different from active-x event name, or it may not work!
	// Also note that the signal name must be equal to active-x event name (case-sensitive)
	// !! Very important. See TrueConfCallX.html for accurate active-x event processors (slots) parameters
	// If you write long instead of int event processor (slot) wont be called! You can see error of connect method
	// if you start application in debug mode and check output
	connect(ui.controlTrueConfCallX, SIGNAL(OnXNotify(QString)), this, SLOT(ProcessOnXNotify(QString)));
	connect(ui.controlTrueConfCallX, SIGNAL(OnXAfterStart()), this, SLOT(ProcessOnXAfterStart()));
	connect(ui.controlTrueConfCallX, SIGNAL(OnServerConnected(QString)), this, SLOT(ProcessOnServerConnected(QString)));
	connect(ui.controlTrueConfCallX, SIGNAL(OnXChangeState(int, int)), this, SLOT(ProcessOnXChangeState(int, int)));
	connect(ui.controlTrueConfCallX, SIGNAL(OnXLoginError(int)), this, SLOT(ProcessOnXLoginError(int)));
	connect(ui.controlTrueConfCallX, SIGNAL(OnXLogin()), this, SLOT(ProcessOnXLogin()));
	connect(ui.controlTrueConfCallX, SIGNAL(OnAbookUpdate(QString)), this, SLOT(ProcessOnXAbookUpdate(QString)));
	connect(ui.controlTrueConfCallX, SIGNAL(OnInviteReceived(QString)), this, SLOT(ProcessOnInviteReceived(QString)));
	connect(ui.controlTrueConfCallX, SIGNAL(OnRecordRequest(QString)), this, SLOT(ProcessOnRecordRequest(QString)));
	connect(ui.controlTrueConfCallX, SIGNAL(OnIncomingRequestToPodiumAnswered(QString)), this, SLOT(ProcessOnIncomingRequestToPodiumAnswered(QString)));
	connect(ui.controlTrueConfCallX, SIGNAL(OnUpdateCameraInfo(QString)), this, SLOT(ProcessOnUpdateCameraInfo(QString)));
	connect(ui.controlTrueConfCallX, SIGNAL(OnVideoMatrixChanged(QString)), this, SLOT(ProcessOnVideoMatrixChanged(QString)));
	connect(ui.controlTrueConfCallX, SIGNAL(OnIncomingChatMessage(QString, QString, QString, qulonglong)),
		this, SLOT(ProcessOnIncomingChatMessage(QString, QString, QString, qulonglong)));
	connect(ui.controlTrueConfCallX, SIGNAL(OnIncomingGroupChatMessage(QString, QString, QString, qulonglong)),
		this, SLOT(ProcessOnIncomingGroupChatMessage(QString, QString, QString, qulonglong)));
}

void QtCallXDemo::connectButtonsAndWidgetEvents()
{
	connect(m_ImageButton.get(), SIGNAL(pressed()), this, SLOT(OnAction()));
}

bool QtCallXDemo::processEventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::WindowActivate)
	{
		if (m_ChatFrame.get() != NULL)
		{
			m_ChatFrame->show();
		}
	}
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == ::Qt::Key_F12)
		{
			if (keyEvent->modifiers() & (::Qt::ShiftModifier | ::Qt::ControlModifier))
			{
				if (!m_IsCallXInitialized)
				{
					/// callx not ready, showing options dlg is disabled then, 
					/// because methods querying hardware may return wrong values
					return false;
				}

				showConfigurationDlg();
				return true;
			}
		}
	}
	return false;
}

void QtCallXDemo::processLoginError(int loginResult)
{
	const int iUSER_LOGGEDIN_OK = 0;// (login successful, otherwise error code)
	const int iUSER_ALREADY_LOGGEDIN = 1;// (answer on CheckUserLoginStatus_Method, if current CID is already authorized at TransportRouter)
	const int iNO_USER_LOGGEDIN = 2;// (answer on CheckUserLoginStatus_Method, if current CID is not authorized at TransportRouter - can try to login)
	const int iACCESS_DENIED = 3;// (incorrect password or other problems with DB)
	const int iSILENT_REJECT_LOGIN = 4;// (client shouldn't show error to user (example: incorrect AutoLoginKey))
	const int iLICENSE_USER_LIMIT = 5;// (license restriction of online users reached, server cannot login you)
	const int iUSER_DISABLED = 6;// (user exist, but he is disabled to use this server)
	const int iRETRY_LOGIN = 7;// (client should retry login after timeout (value in container or default), due to server busy or other server problems)
	const int iINVALID_CLIENT_TYPE = 8;// (user cannot login using this client app (should use other type of client app))

	// most of cases must be processed, but not shown to user
	// though in our demo app we'll show everythings as it is 
	static const wchar_t* csMessages[] = { 
		L"", 
		L"Current CID is already authorized at TransportRouter",
		L"Current CID is not authorized at TransportRouter - can try to login",
		L"Incorrect password or other problems with DB",
		L"Problems with login (silent reject login)", 
		L"License restriction of online users reached, server cannot login you",
		L"Supplied user exists, but is disabled on this server",
		L"You should retry login after timeout, server is busy or other server problems",
		L"Unable to login using this client application" };

	// find out number of elements in array, this'll work in current arch
	int arraySize = (*(&csMessages + 1) - csMessages);
	if (iUSER_LOGGEDIN_OK == loginResult)
	{
		/// state must be changed via XChangeState event
		//m_CallXState = ECallXState::ECXS_LoggedIn;
	}
	else if(loginResult >= 0 && loginResult < arraySize)
	{
		QMessageBox msgBox(QMessageBox::Icon::Warning, "Login error!", QString::fromWCharArray(csMessages[loginResult]),
			QMessageBox::StandardButton::Ok, this);
		//msgBox.show();
		msgBox.exec();
	}
	else 
	{
		QMessageBox msgBox(QMessageBox::Icon::Warning, "Error!", QString("Error, code = %1").arg(loginResult),
			QMessageBox::StandardButton::Ok, this);
		//msgBox.show();
		msgBox.exec();
	}

}

void QtCallXDemo::extractUsersStatusOnAbookUpdate(const QJsonObject& json)
{
	if (json.contains("event") && json["event"].isString())
	{
		QString sEventName = json["event"].toString();
		if (sEventName == "onAbookUpdate")
		{
			QJsonArray jsonUserStatuses = json["abook"].toArray();

			extractUsersStatus(jsonUserStatuses);
		}
	}
}

void QtCallXDemo::extractUsersStatus(const QJsonArray& jsonUserStatuses)
{
	foreach(auto jsonUserStatus, jsonUserStatuses)
	{
		if (jsonUserStatus.isObject())
		{
			QJsonObject jsonObjectUserStatus = jsonUserStatus.toObject();
			if (jsonObjectUserStatus.contains("peerId") && jsonObjectUserStatus["peerId"].isString())
			{
				m_mapUserStatus[jsonObjectUserStatus["peerId"].toString()] = jsonObjectUserStatus["status"].toInt();
			}
		}
	}
}

bool QtCallXDemo::checkUserStatusOnline(const QString& sUserID)
{
	auto result = m_mapUserStatus.find(sUserID);
	if (result != m_mapUserStatus.end())
	{
		// key exists, check value
		return result.value() == ciUserStatusOnline;
	}
	return false;
}

void QtCallXDemo::connectToServer()
{
	if (m_CallXState >= ECallXState::ECXS_Login)
	{
		LOG(L"Already connected!");
	}
	if (!m_CallXSettings.sServerName.isEmpty())
	{
		m_CallXComPtr->connectToServer(QStringToWChar(m_CallXSettings.sServerName));
	}
}

QPixmap& QtCallXDemo::getImage(QString path)
{
	auto i = m_pixmaps.find(path);
	if (i != m_pixmaps.end())
	{
		return i.value();
	}
	i = m_pixmaps.insert(path, QPixmap(path));
	return i.value();
}

void QtCallXDemo::initCallButtonAndStatusLabel()
{
	/// Creating custom call button
	m_ImageButton.reset(new ImageButton(ui.centralWidget, true));

	QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
	// using this button will be in vertical proportion of 2 to 15 to callx control 
	sizePolicy1.setVerticalStretch(2); 
	m_ImageButton->setSizePolicy(sizePolicy1);
	m_ImageButton->setObjectName(QStringLiteral("imageButton"));

	// set initial picture for call button
	QPixmap pixmap(":/QtCallXDemo/CallButton_ActionConnecting.png");
	m_ImageButton->setPixmap(pixmap);

	// set initial picture for icon label
	QPixmap pixmap2(":/QtCallXDemo/StatusIcon_NotConnected.png");
	m_StatusIcon = ui.labelCallXState;
	m_StatusIcon->setPixmap(pixmap2);
	m_StatusIcon->setScaledContents(false);

	// add our button to layout and set its alignment on the center
	QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(ui.centralWidget->layout());
	Q_ASSERT(layout != NULL);
	layout->addWidget(m_ImageButton.get());
	layout->setAlignment(m_ImageButton.get(), Qt::AlignHCenter);

	layout->addWidget(ui.labelCallXStatus);
	layout->addWidget(ui.labelLowerLogo);

	/// set background 
	QPixmap pixmapBackground;
	pixmapBackground.load(":/QtCallXDemo/Background.png");

	QPalette palette;
	palette.setBrush(QPalette::Window, pixmapBackground);
	this->setPalette(palette);

	/// logo, load initial image, it will change with callx status change
	QPixmap logo;
	logo.load(":/QtCallXDemo/YourLogo.png");

	// will set initial width, it will be anyway reset, when video is turned on
	// since app start fullscreen so half of one will be good initial value
	ui.controlTrueConfCallX->setFixedWidth(qApp->screens()[0]->size().width() / 2); 
	m_ImageButton->setFixedWidth(ui.controlTrueConfCallX->width());
}

void QtCallXDemo::showConfigurationDlg()
{
	/// show options dialog
	auto dialog = new ConfigDlg(this);
	dialog->exec();
}

void QtCallXDemo::doCall(QString userID)
{
	LOG(userID);
	m_CallXComPtr->call(QStringToWChar(userID));
}

void QtCallXDemo::updateMessages()
{
	const int ciOneMessageMaxHeight = 100;
	if (m_ChatFrame.get() == NULL)
	{
		QWidget* parent = this;
		/// the best way to draw over a window dispalying a video is to create popup window and place it over this window (control)
		// if you create child window even if you put it over window with video it may produce drawing artifacts
		// well, that have been tested on Windows 10 and Qt 5.9.1
		// for more information about popup windows and window styles you can read msdn section describing windows (popup, child etc.)
		// note that qt styles does not directly map to that of windows window styles
		//m_ChatFrame.reset(new QWidget(parent, Qt::Popup | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus/* | Qt::FramelessWindowHint */));
		m_ChatFrame.reset(new QWidget(NULL, Qt::ToolTip | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus/* | Qt::FramelessWindowHint */));
		
		m_ChatFrame->move(ui.controlTrueConfCallX->pos());
		m_ChatFrame->setFixedWidth(ui.controlTrueConfCallX->width());
		m_ChatFrame->show();
		m_ChatFrame->setWindowOpacity(0.65);
		auto verticalLayout = new QVBoxLayout(m_ChatFrame.get());
		verticalLayout->setSpacing(0);
		verticalLayout->setContentsMargins(2, 2, 2, 2);
		verticalLayout->setObjectName(QStringLiteral("frameVerticalLayout"));
	}

	bool needRecomposition = m_ChatMessagesLabels.size() != m_ChatMessages.size();
	if (m_ChatMessagesLabels.size() < m_ChatMessages.size())
	{
		// message's been added, add new message label
		std::shared_ptr<QLabel> labelMessage(new QLabel(m_ChatFrame.get()));
		labelMessage->setWordWrap(true);
		labelMessage->setText(m_ChatMessages[0]);
		labelMessage->setFixedHeight(ciOneMessageMaxHeight);
		m_ChatMessagesLabels.append(labelMessage);

		QVBoxLayout* verticalLayout = dynamic_cast<QVBoxLayout*>(m_ChatFrame->layout());
		verticalLayout->insertWidget(0, labelMessage.get(), 0, Qt::AlignTop);
	}
	if (m_ChatMessagesLabels.size() > m_ChatMessages.size())
	{
		// message's been deleted, delete existing message label
		std::shared_ptr<QLabel> labelMessage = m_ChatMessagesLabels[0];
		m_ChatMessagesLabels.removeAt(0);
		labelMessage->close();
	}
	if (needRecomposition)
	{
		int newDesiredHeight = m_ChatMessagesLabels.size() * ciOneMessageMaxHeight;
		newDesiredHeight = newDesiredHeight < ui.controlTrueConfCallX->height() ?
			newDesiredHeight : ui.controlTrueConfCallX->height();
		m_ChatFrame->setFixedHeight(newDesiredHeight);
	}
	if (m_ChatMessages.size() == 0)
	{
		m_ChatFrame->setFixedSize(0, 0);
		m_ChatFrame->close();
		m_ChatFrame.reset(NULL);
	}
}

void QtCallXDemo::showChatMessage(QString peerId, QString peerDn, QString message, qulonglong time)
{
	/// check options
	if (m_CallXSettings.bShowChatMessages)
	{
		/// insert new message text
		m_ChatMessages.insert(m_ChatMessages.begin(), peerDn + ":\n\t" + message);

		/// will show new messages
		updateMessages();

		/// sets timer for iShowMessageTime seconds
		QTimer::singleShot(m_CallXSettings.iShowMessageTime * 1000, this, SLOT(UpdateMessagesByTime()));
	}
}

static void logMsgToFile(const std::wstring& aFileName, const wchar_t* aFunctionName, const wchar_t* aMsg);

void logMsgToFileQT(const QString& aFunctionName, const QString& aMsg)
{
	logMsgToFile(csNotificationOutputFileNameUsingQT, QStringToWChar(aFunctionName), QStringToWChar(aMsg));
}

void logMsgToFileQT(const QString& aFunctionName, const std::wstring& aMsg)
{
	logMsgToFile(csNotificationOutputFileNameUsingQT, QStringToWChar(aFunctionName), aMsg.c_str());
}

void logMsgToFileQT(const QString& aFunctionName, const wchar_t* aMsg)
{
	logMsgToFile(csNotificationOutputFileNameUsingQT, QStringToWChar(aFunctionName), aMsg);
}

void logMsgToFile(const std::wstring& aFileName, const wchar_t* aFunctionName, const wchar_t* aMsg)
{
	FILE* f = ::_wfopen(aFileName.c_str(), L"a+, ccs=UTF-16LE");
	if (f)
	{
		::SYSTEMTIME time;
		::GetSystemTime(&time);
		int res = ::fwprintf(f, L"%i.%i.%i %i:%i:%i %s\n%s\r\n", time.wYear, time.wMonth, time.wDay, 
			time.wHour, time.wMinute, time.wSecond, aFunctionName, aMsg);
		::fflush(f);
		::fclose(f);
	}
}


