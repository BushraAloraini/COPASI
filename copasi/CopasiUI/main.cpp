// Copyright (C) 2017 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2002 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include <stdexcept>
#include <locale.h>

#include <QApplication>

#define COPASI_MAIN

#include "copasi.h"

#include "UI/CQCopasiApplication.h"
#include "UI/copasiui3window.h"
#include "UI/DataModelGUI.h"
#include "UI/CQMessageBox.h"
#include  <copasi/UI/CQUndoTester.h>

#include "CopasiDataModel/CDataModel.h"
#include "copasi/core/CRootContainer.h"
#include "function/CFunctionDB.h"
#include "function/CFunction.h"
#include "commandline/COptionParser.h"
#include "commandline/COptions.h"

#ifdef COPASI_SBW_INTEGRATION
// SBW includes
#include "SBW/SBW.h"
#include <sstream>
#endif // COPASI_SBW_INTEGRATION

#ifdef Darwin
# include <QtCore/QDir>
# include "UI/qtUtilities.h"
#endif // Darwin

int main(int argc, char *argv[])
{
  // Fix for Issue 1377, if locale is not set to the default one, some
  // numbers will be displayed in the systems default locale:
  setlocale(LC_ALL, "C");

  // Fix of Issue with Text centering on OSX Mavericks
#ifdef Q_OS_MACX

  if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_8)
    {
      // fix Mac OS X 10.9 (mavericks) font issue
      // https://bugreports.qt-project.org/browse/QTBUG-32789
      QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    }

#endif

  CQCopasiApplication a(argc, argv);

  a.setAttribute(Qt::AA_DontShowIconsInMenus, false);

  Q_INIT_RESOURCE(copasi);

  // Parse the commandline options
  try
    {
      // Create the root container.
      CRootContainer::init(argc, argv, true);
    }
  catch (copasi::option_error & msg)
    {
      CQMessageBox::critical(NULL, "Initialization Error",
                             msg.what(),
                             QMessageBox::Ok , QMessageBox::Ok);

      return 1;
    }

#ifdef Darwin
  std::string PluginDir;

  COptions::getValue("CopasiDir", PluginDir);
  PluginDir += "/Contents/plugins";

  QApplication::setLibraryPaths(QStringList(FROM_UTF8(PluginDir)));
#endif // Darwin

  // Create the main application window.
  CopasiUI3Window *pWindow = CopasiUI3Window::create();

#ifdef COPASI_SBW_INTEGRATION

  if (COptions::compareValue("SBWRegister", true))
    goto finish;

#endif // COPASI_SBW_INTEGRATION

  if (pWindow != NULL)
    {
      a.setMainWindow(pWindow);

      if (!COptions::getEnvironmentVariable("COPASI_TEST_UNDO").empty())
        {
          // test undo if environment variable is present
          CQUndoTester*  tester = new CQUndoTester(&a, NULL, pWindow->getUndoStack());
          tester->startTestIn(1000);
        }

      a.exec();
    }

finish:

  try // To suppress any access violations during destruction works only under Windows
    {
      pdelete(pWindow);
      CRootContainer::destroy();
    }
  catch (...)
    {}

  return 0;
}
