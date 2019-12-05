#include <QtWidgets>
#include <QMessageBox>
#include <QPalette>

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// This will generate trueconf_callx.tlh and trueconf_callx.tli in output directory
/// We use standart microsoft compiler directive #import since it generates code for active-x events also
/// and it makes a bit better demonstration of how you can use active-x control and how it works
/// Either way you can use dumpcpp utility from Qt since it'll provide more qt friendly wrappers, 
/// though without events declarations
/// For processing of events see QtCallXDemo::connectQtSlotsToComObjectEvents()
/// Note that TrueConfCallX.html with Qt compatible html description of active-x will be generated 
/// at the very start of build process in pre-built event
////////////////////////////////////////////////////////////////////////////////////////////////////////
#import "progid:ID_TrueConfCallX" named_guids
// Both lines will do the same though uncommented line uses progid and works if one does not know where active-x is installed
//#import <C:\Program Files (x86)\TrueConf\CallX\TrueConf_CallX.ocx> named_guids

#define MacroStr(x)   #x
#define MacroStr2(x)  MacroStr(x)
#define Message(desc) __pragma(message(__FILE__ "(" MacroStr2(__LINE__) ") :" #desc))

