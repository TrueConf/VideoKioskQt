#include "stdafx.h"
#include "HardwareDlg.h"
#include "ConfigDlg.h"
#include "QtCallXDemo.h"

static void initCombo(QComboBox *combo, const _bstr_t& values, const QString& initValue);

HardwareDlg::HardwareDlg(QWidget *parent)
	: QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	parentApp(NULL)
{
	ui.setupUi(this);

	connect(ui.buttonHardwareApply, SIGNAL(pressed()), this, SLOT(onHardwareApply()));

	ConfigDlg* parentDlg = dynamic_cast<ConfigDlg*>(parent);
	if (parentDlg)
	{
		parentApp = parentDlg->GetParentApp();
	}

	if (parentApp)
	{
		CallXSettings& settings = parentApp->GetCallXSettings();
		callx = parentApp->GetCallX();

		//init camera combo
		initCombo(ui.comboCamera, callx->XGetCameraList(), settings.sCamera);

		// init speaker combo
		initCombo(ui.comboSpeaker, callx->XGetSpeakerList(), settings.sSpeaker);

		// init microphone combo
		initCombo(ui.comboMicrophone, callx->XGetMicList(), settings.sMicrophone);
	}
}

HardwareDlg::~HardwareDlg()
{
}

/**
* check and apply hardware changes
*/
void HardwareDlg::onHardwareApply()
{
	CallXSettings& settings = parentApp->GetCallXSettings();

	int iCameraIndex = ui.comboCamera->currentIndex();
	if (ui.comboCamera->currentIndex() <= 0 && !settings.sCamera.isEmpty())
	{
		callx->XDeselectCamera();
	}

	if (ui.comboSpeaker->currentIndex() <= 0 && !settings.sSpeaker.isEmpty())
	{
		callx->XDeselectSpeaker();
	}

	if (ui.comboMicrophone->currentIndex() <= 0 && !settings.sMicrophone.isEmpty())
	{
		callx->XDeselectMic();
	}

	settings.sCamera = ui.comboCamera->currentText();
	settings.sSpeaker = ui.comboSpeaker->currentText();
	settings.sMicrophone = ui.comboMicrophone->currentText();

	parentApp->InitHardware();
	close();
}

/** 
* load values in the combo box, add default one, that means nothing to use
*/
static void initCombo(QComboBox *combo, const _bstr_t& values, const QString& initValue)
{
	const QString csDeviceNone("None");

	combo->clear();
	int iCurrentIndex = combo->currentIndex();
	
	// case "None" will be first with id = 0
	combo->addItem(csDeviceNone);

	QStringList qsCameraStringList = QStringList(QString::fromWCharArray(values.operator LPCWSTR()).split("\r\n"));
	// qt split may produce an empty string, 'll remove it
	qsCameraStringList.removeAll("");
	combo->addItems(qsCameraStringList);

	combo->setCurrentText(initValue);
	iCurrentIndex = combo->currentIndex();
	if (iCurrentIndex < 0 || combo->itemText(iCurrentIndex) != initValue)
	{
		// if initValue is empty or initValue not in the list of avaliable choises
		combo->setCurrentIndex(0);
	}
}


