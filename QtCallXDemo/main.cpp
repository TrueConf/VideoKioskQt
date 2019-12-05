#include "stdafx.h"
#include "QtCallXDemo.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	// Warning!! if you are not using QT not to forget to use CoInitializeEx() or OleInitialize() 
	// before creating or working with COM objects (active-x controls etc.)

	// if you are not using qt you can use CAxWindow (ATL) to host active-x control

	// Here Qt is used so there is no need for this

	// note that second call to CoInitializeEx() with incompatible params to that 
	// of a previous call will fail (for each thread)

	bool showConfigDialog = false;
	if (argc == 2)
	{
		if (QString(argv[1]).compare("-config") == 0)
		{
			showConfigDialog = true;
		}
	}

	QApplication a(argc, argv);
	QtCallXDemo w(showConfigDialog);

	w.show();

	/// start fullscreen	
	/// this will also remove system's window title
	w.setWindowState(Qt::WindowFullScreen);
	return a.exec();
}
