#pragma once

#include <QWidget>
#include "ui_ConfigDlg.h"

class QtCallXDemo;

/**
* ConfigDlg - Configuration dialog, showed when pressing ctrl + shift + F12 or when app launched with special key.
*	Allows user to specify server, login, password, method of call and calling user(s)
*	Allows to specify hardware using hardware dialog, showed when pressing "Hardware button"
*/
class ConfigDlg : public QDialog
{
	Q_OBJECT

public:
	ConfigDlg(QWidget *parent = Q_NULLPTR);
	~ConfigDlg();

	QtCallXDemo* GetParentApp() const;

private slots:
	/// check data and save then apply if possible
	void onAccept();

	/// cancel all changes, close dialog
	void onCancel();

	/// open hardware configuration dialog
	void onHardwareConfig();

private:
	Ui::ConfigDlg ui;

	/// store smartpointer to CallX
	TrueConf_CallXLib::ITrueConfCallXPtr callx;

	QtCallXDemo* parentApp;

	/// save dialog values to callx settings
	void saveData();

	/// fill dialog from callx settings
	void loadData();
};
