#include "stdafx.h"
#include "ConfigDlg.h"
#include "QtCallXDemo.h"
#include "HardwareDlg.h"

static void initCombo(QComboBox *combo, const _bstr_t& values, const QString& initValue);


ConfigDlg::ConfigDlg(QWidget *parent)
	: QDialog(parent, Qt::WindowTitleHint),
	parentApp(NULL)
{
	ui.setupUi(this);

	connect(ui.buttonHardwareConfig, SIGNAL(pressed()), this, SLOT(onHardwareConfig()));
	connect(ui.buttonOK, SIGNAL(pressed()), this, SLOT(onAccept()));
	connect(ui.buttonCancel, SIGNAL(pressed()), this, SLOT(onCancel()));
	
	parentApp = dynamic_cast<QtCallXDemo*>(parent);
	if (parentApp)
	{
		callx = parentApp->GetCallX();
	}

	loadData();
}

ConfigDlg::~ConfigDlg()
{
}

QtCallXDemo* ConfigDlg::GetParentApp() const
{
	return parentApp;
}

void ConfigDlg::onAccept()
{
	if (ui.editPassword->text() != ui.editConfirmPassword->text())
	{
		QMessageBox msgBox(QMessageBox::Icon::Warning, "Error!", "Passwords do not match!", 
			QMessageBox::StandardButton::Ok, this);
		msgBox.exec();
		ui.editPassword->setFocus();
		return;
	}
	saveData();
	ConfigDlg::accept();
}

void ConfigDlg::onCancel()
{
	ConfigDlg::reject();
}

void ConfigDlg::onHardwareConfig()
{
	auto dialog = new HardwareDlg(this);
	dialog->exec();
}

void ConfigDlg::saveData()
{
	if (parentApp)
	{
		CallXSettings& settings = parentApp->GetCallXSettings();
		bool reconnect = false;
		if ((ui.editLogin->text() != settings.sLogin) ||
			(settings.sServerName != ui.editServerName->text()) ||
			(settings.sPassword != ui.editPassword->text()))
		{
			reconnect = true;
		}

		settings.sServerName = ui.editServerName->text();
		settings.sLogin = ui.editLogin->text();
		settings.sPassword = ui.editPassword->text();

		// call tab
		settings.sUserID = ui.editCallUserID->text();
		settings.sUserIDsList = ui.editCallUserIDsList->text();

		if (ui.radioCallUserID->isChecked())
		{
			settings.iCallMethod = CallXSettings::ECM_UserID;
		}
		else if (ui.radioCallUserIDsList->isChecked())
		{
			settings.iCallMethod = CallXSettings::ECM_UserIDsList;
		}
		else if (ui.radioCallUserFromPhonebook->isChecked())
		{
			settings.iCallMethod = CallXSettings::ECM_UserIDsList;
		}

		settings.bNotifyManager = ui.checkboxNotifyManager->isChecked();
		settings.sManagerEmail = ui.editManagerEMAIL->text();

		// chat messages tab
		settings.bShowChatMessages = ui.checkboxShowChatMessages->isChecked();
		settings.iShowMessageTime = ui.spinboxShowMessageTime->value();

		// write new settings to registry
		parentApp->SaveSettings();

		if (reconnect)
		{
			// reconnect disabled in this version of demo application, close and lauch again (or uncomment code below)
			//if (settings.sServerName != ui.editServerName->text())
			//{
			//	// will login after successfull connection to server in method ProcessOnServerConnected
			//	callx->connectToServer(QStringToWChar(settings.sServerName));
			//} 
			//else
			//{
			//	// server is the same so it means login data must be different
			//	callx->login(QStringToWChar(settings.sLogin), QStringToWChar(settings.sPassword));
			//}
		}
	}
}

void ConfigDlg::loadData()
{
	if (parentApp)
	{
		CallXSettings& settings = parentApp->GetCallXSettings();

		// authorization tab
		ui.editServerName->setText(settings.sServerName);
		ui.editLogin->setText(settings.sLogin);
		ui.editPassword->setText(settings.sPassword);
		ui.editConfirmPassword->setText(settings.sPassword);

		// call tab
		switch (settings.iCallMethod)
		{
		case CallXSettings::ECM_UserIDsList:
			ui.radioCallUserIDsList->setChecked(true);
			break;

		case CallXSettings::ECM_Phonebook:
			ui.radioCallUserFromPhonebook->setChecked(true);
			break;

		case CallXSettings::ECM_UserID:
		default:
			ui.radioCallUserID->setChecked(true);
			break;
		}
		ui.checkboxNotifyManager->setChecked(settings.bNotifyManager);

		ui.editManagerEMAIL->setText(settings.sManagerEmail);
		ui.editCallUserID->setText(settings.sUserID);
		ui.editCallUserIDsList->setText(settings.sUserIDsList);

		// messages tab
		ui.checkboxShowChatMessages->setChecked(settings.bShowChatMessages);
		ui.spinboxShowMessageTime->setValue(settings.iShowMessageTime);
	}
}

