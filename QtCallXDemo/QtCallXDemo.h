#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtCallXDemo.h"

#ifdef _DEBUG
#include "Debug\trueconf_callx.tlh"
#else
#include "Release\trueconf_callx.tlh"
#endif


// we assume that target platform must be windows win32 or x64, in this case sizeof(QCHAR)==sizeof(wchar_t)==2 
// and windows encodes Utf16 by default so that casts below are correct 
inline wchar_t* QStringToWChar(QString& str)
{
	return (wchar_t*)str.data();
}

inline const wchar_t* QStringToWChar(const QString& str)
{
	return (const wchar_t*)str.data();
}

#define IDC_TRUECONFEVENTPROCESSOR 1019

class CallXSettings {
public:

	/// Enum specifies which type of call will be used 
	enum ECallMethod {
		ECM_UserID = 0, ///< call user by user id
		ECM_UserIDsList = 1, ///< call first availiable user from list of users (users have to be in phonebook)
		ECM_Phonebook = 2 ///< call random user from a phonebook
	};

	CallXSettings() :
		iCallMethod(ECM_UserID),
		bNotifyManager(false),
		bShowChatMessages(false),
		iShowMessageTime(15)
	{
	}

	QString sServerName;
	QString sLogin;
	QString sPassword;

	QString sCamera;
	QString sSpeaker;
	QString sMicrophone;

	QString sUserID;
	QString sUserIDsList;
	int		iCallMethod;

	bool	bNotifyManager;
	QString sManagerEmail;

	bool	bShowChatMessages;
	int		iShowMessageTime;
};

class ImageButton;

/**
* QtCallXDemo - main window class, holds axWidget with TrueConfCallX active-x control inside,
*	perfoms management of smartpointer to CallX active-x,
*	establishes link to active-x events, 
*	manages graphical representantion of application
*/
class QtCallXDemo : public QMainWindow
{
	Q_OBJECT

public:
	/// Enum that defines CallX states
	enum ECallXState {
		ECXS_None, ///< none, no server connection and application does not try to connect to server
		ECXS_Connect, ///< connect, application tries to connect to server
		ECXS_Login, ///< login, login is in process
		ECXS_Normal, ///< normal, application connected and logged in
		ECXS_Wait, ///< establishing a call link or terminating a call
		ECXS_Conference, ///< talking (p2p or conference, application is in call)
		ECXS_Close ///< close, CallX is terminating
	};

public:
	QtCallXDemo(bool showConfigDialog, QWidget *parent = Q_NULLPTR);
	~QtCallXDemo();

	CallXSettings& GetCallXSettings();

	/// return smartptr to CallX
	TrueConf_CallXLib::ITrueConfCallXPtr GetCallX();

	/// Write settings to registry
	void SaveSettings();

	/// set camera, speaker, mic
	void InitHardware();

protected:
	/// process events (needed for processing keyboard events)
	bool eventFilter(QObject* obj, QEvent* event) override;

	/// process close event to handle deinitialization of CallX
	void closeEvent(QCloseEvent *event) override;

protected slots:
	/**
	* Processing of event ProcessOnXNotify, CallX fires this event together with almost any other event
	* To handle specific type of information you should process substantial event
	* e.g. to control status of contacts you should process OnAbookUpdate
	* though processing of this notification is very usefull for debug purposes
	* @param[in] notify contains json describing what is happening
	*/
	void ProcessOnXNotify(QString notify);

	/**
	* Processing of event OnXAfterStart, CallX fires this event when initialization is complete
	* and it is ready to connect to server and to initialize hardware
	*/
	void ProcessOnXAfterStart();

	/**
	* Processing of event OnServerConnected, CallX fires this event when connection to server succeded
	* and it is ready to login
	* @param[in] eventDetails Json that contains server name, port and service parameter
	*/
	void ProcessOnServerConnected(QString eventDetails);

	/**
	* Processing of event OnXChangeState, CallX fires this event when CallX state is changed
	* Depending on the state of CallX (in a call, waiting, "logged in" etc.) behaviour of app changes
	* Use this event to know and track current CallX state
	* @param[in] prevState Previous state of CallX
	* @param[in] newState Current state of CallX
	*/
	void ProcessOnXChangeState(int prevState, int newState);

	/**
	* Processing of event OnXLoginError, CallX fires this event when login failed
	* Use this event to show user an error message or to try login once more depending on the errorCode
	* @param[in] errorCode Login error
	*/
	void ProcessOnXLoginError(int errorCode);

	/**
	* Processing of event OnXLogin, CallX fires this event when login succeeded
	*/
	void ProcessOnXLogin();

	/**
	* Processing of event OnXAbookUpdate, CallX fires this event in two cases 
	* first, when login is completed to send book of contacts of current user
	* second, when status of a contact from a book is changed
	* Process this event to track user status to know who is online 
	* @param[in] eventDetails Json describing users status
	*/
	void ProcessOnXAbookUpdate(QString eventDetails);

	/**
	* Processing of event OnInviteReceived, CallX fires this event when you are called 
	* Process this event to start a conversation or join a conference
	* @param[in] eventDetails Json describing user that calls you, contains conference id if you are invited to conference
	*/
	void ProcessOnInviteReceived(QString eventDetails);

	/**
	* Processing of event OnRecordRequest, CallX fires this event when someone wants to start recording
	* You receive this notification when it is necessary to ask you about recording
	* @param[in] eventDetails Json with substantial info
	*/
	void ProcessOnRecordRequest(QString);

	/**
	* Processing of event OnIncomingRequestToPodiumAnswered
	* CallX fires this event when you are invited to podium in a conference
	* Process this event to accept or decline an invitation
	* @param[in] eventDetails Json describing user that calls you, contains conference id if you are invited to conference
	*/
	void ProcessOnIncomingRequestToPodiumAnswered(QString eventDetails);

	/**
	* Processing of event OnUpdateCameraInfo, CallX fires this event when camera settings's been changed
	* Process this event to know CallX video resolulion to adjust its size if needed
	* @param[in] eventDetails Json containing camera information such as resolution, framerate and other
	*/
	void ProcessOnUpdateCameraInfo(QString eventDetails);

	/**
	* Processing of event OnVideoMatrixChanged, CallX fires this event when camera settings's been changed
	* Process this event to know CallX video resolulion in a call to adjust its size if needed
	* In call or especially in a conference mode when you can see not only yourself and your opponent 
	* but you can see more than one conference's user video inside CallX 
	* this event tells you correct size of "table of users videos" and size of each displayed users rectangle
	* @param[in] eventDetails Json containing information about current video table size and sizes of inside rectangles
	*/
	void ProcessOnVideoMatrixChanged(QString eventDetails);

	/**
	* Processing of event OnIncomingChatMessage, CallX fires this event when you receive a chat message
	* Process this event to display chat messages
	* @param[in] peerId User id
	* @param[in] peerDn User display name
	* @param[in] message Message text
	* @param[in] time Time
	*/
	void ProcessOnIncomingChatMessage(QString peerId, QString peerDn, QString message, qulonglong time);

	/**
	* Processing of event OnIncomingChatMessage, CallX fires this event when you receive a chat message
	* Process this event to display chat messages
	* @param[in] peerId User id
	* @param[in] peerDn User display name
	* @param[in] message Message text
	* @param[in] time Time
	*/
	void ProcessOnIncomingGroupChatMessage(QString peerId, QString peerDn, QString message, qulonglong time);

	/// process action button click (call or hang up, accept an incoming call or accept an invitation to join a conference)
	void OnAction();

	void UpdateMessagesByTime();

	/// make a call, calling user depends on choosed settings
	void Call();


private:
	Ui::QtCallXDemoClass ui;

	/// smartpointer to callx active-x object
	TrueConf_CallXLib::ITrueConfCallXPtr m_CallXComPtr;
	
	/// CallX settings
	CallXSettings m_CallXSettings;

	/// adress book and status monitor
	QMap<QString, int> m_mapUserStatus;

	/// holds current callx state
	/*ECallXState*/int m_CallXState;

	/// determines if callx's been successfully initialized
	bool m_IsCallXInitialized;

	/// indicates that app was launched with key -config, so ConfigDlg will be show, after CallX is initialized
	bool m_ShowConfigDlg;

	QMap<QString, QPixmap> m_pixmaps;

	std::unique_ptr<ImageButton> m_ImageButton;
	QLabel* m_StatusIcon;

	double m_CameraDimensionsProportion;

	QList<QString> m_ChatMessages;
	std::unique_ptr<QWidget> m_ChatFrame;
	QList<std::shared_ptr<QLabel>> m_ChatMessagesLabels;

	/// read CallX settings from the registry
	void loadSettings();

	void initCOMSupplementatryObjects();
	void connectQtSlotsToComObjectEvents();
	void connectButtonsAndWidgetEvents();
	bool processEventFilter(QObject* obj, QEvent* event);

	/// determines error, shows appropriate messagebox
	void processLoginError(int loginError);

	void extractUsersStatusOnAbookUpdate(const QJsonObject& json);
	void extractUsersStatus(const QJsonArray& jsonUserStatuses);
	bool checkUserStatusOnline(const QString& sUserID);
	void connectToServer();
	QPixmap& getImage(QString path);
	void initCallButtonAndStatusLabel();
	void showConfigurationDlg();
	void doCall(QString user);
	void updateMessages();
	void showChatMessage(QString peerId, QString peerDn, QString message, qulonglong time);
};


