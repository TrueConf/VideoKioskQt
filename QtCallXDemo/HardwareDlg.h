#pragma once

#include <QWidget>
#include "ui_HardwareDlg.h"

class QtCallXDemo;


/**
* HardwareDlg - Hardware dialog, allows user to choose camera, speker and mic for CallX Active-X
*	shows avaliable choises in appropriate combo boxes, applies changes on exit
*/
class HardwareDlg : public QDialog
{
	Q_OBJECT

public:
	HardwareDlg(QWidget *parent = Q_NULLPTR);
	~HardwareDlg();

private slots:
	void onHardwareApply();

private:
	Ui::HardwareDlg ui;

	/// holds smartpointer to CallX
	TrueConf_CallXLib::ITrueConfCallXPtr callx;

	QtCallXDemo* parentApp;
};
