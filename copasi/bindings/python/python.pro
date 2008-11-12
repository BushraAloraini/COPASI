# Begin CVS Header 
#   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/bindings/python/python.pro,v $ 
#   $Revision: 1.23.6.1 $ 
#   $Name:  $ 
#   $Author: gauges $ 
#   $Date: 2008/11/12 15:18:47 $ 
# End CVS Header 

# Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
# Properties, Inc., EML Research, gGmbH, University of Heidelberg,
# and The University of Manchester.
# All rights reserved.

# Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
# Properties, Inc. and EML Research, gGmbH.
# All rights reserved.

TEMPLATE = lib
CONFIG -= qt

include(../../common.pri)

contains(BUILD_OS,WIN32){
   TARGET = _COPASI
} else {
   TARGET = CopasiPython
}

# the code generated by swig has to be compiled with -O1
# since -O2 and higher do things that might break the binary
QMAKE_CFLAGS_RELEASE -= -O3
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -O1
QMAKE_CXXFLAGS_RELEASE -= -O3
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O1


COPASI_LIBS += -L../../lib

COPASI_LIBS += -lCOPASISE 

LIBS = $$COPASI_LIBS $$LIBS

INCLUDEPATH += ../../..
contains(BUILD_OS,Linux){

  !isEmpty(PYTHON_LIB_PATH){
    LIBS += -L$$PYTHON_LIB_PATH
    LIBS += -lpython2.5
  } else {
    LIBS += `python-config --libs`
  }

  !isEmpty(PYTHON_INCLUDE_PATH){
    INCLUDEPATH += $$PYTHON_INCLUDE_PATH
  } else {
    QMAKE_CFLAGS_RELEASE += `python-config --includes` 
    QMAKE_CXXFLAGS_RELEASE += `python-config --includes` 
    QMAKE_CFLAGS_DEBUG += `python-config --includes` 
    QMAKE_CXXFLAGS_DEBUG += `python-config --includes` 
  }



 QMAKE_POST_LINK += ln -sf libCopasiPython.so _COPASI.so

}

contains(BUILD_OS, Darwin) {
    LIBS += -framework Python
    LIBS += -framework QuickTime
    LIBS += -framework Carbon
    LIBS += -framework Accelerate

    QMAKE_LFLAGS_SHLIB += -unexported_symbols_list unexported_symbols.list
    QMAKE_PRE_LINK = nm -g $$SBML_PATH/lib/libsbml.a | grep "^[0-9]" | cut -d" " -f3  > unexported_symbols.list ; nm -g $$EXPAT_PATH/lib/libexpat.a | grep "^[0-9]" | cut -d" " -f3  >> unexported_symbols.list


  !isEmpty(PYTHON_INCLUDE_PATH){
    INCLUDEPATH += $$PYTHON_INCLUDE_PATH
  }

  QMAKE_POST_LINK += ln -sf libCopasiPython.dylib _COPASI.so
}

contains(BUILD_OS, WIN32) { 
  CONFIG -= staticlib
  CONFIG += dll
  CONFIG += embed_manifest_dll
  
  release {
    QMAKE_POST_LINK = mt.exe -manifest $(TARGET).manifest -outputresource:$(TARGET);2 && ren _COPASI.dll _COPASI.pyd
  } else {
    QMAKE_POST_LINK = mt.exe -manifest $(TARGET).manifest -outputresource:$(TARGET);2 && ren _COPASI.dll _COPASI_d.pyd
  }

  !isEmpty(PYTHON_LIB_PATH){
    QMAKE_LFLAGS_WINDOWS += /LIBPATH:"$${PYTHON_LIB_PATH}"
    QMAKE_LFLAGS_CONSOLE_DLL += /LIBPATH:"$${PYTHON_LIB_PATH}"
    debug{
      LIBS += python25_d.lib
    } else { 
      LIBS += python25.lib
    }
  }

  !isEmpty(PYTHON_INCLUDE_PATH){
    INCLUDEPATH += $$PYTHON_INCLUDE_PATH
    debug{
      INCLUDEPATH += $$PYTHON_INCLUDE_PATH/../PC/
    }
  }

}


SWIG_INTERFACE_FILES=../swig/CChemEq.i \
                     ../swig/CChemEqElement.i \
                     ../swig/CCompartment.i \
                     ../swig/CCopasiContainer.i \
                     ../swig/CCopasiDataModel.i \
                     ../swig/CCopasiException.i \
		     ../swig/CCopasiMessage.i \
		     ../swig/messages.i \
                     ../swig/CCopasiMethod.i \
                     ../swig/CCopasiObject.i \
                     ../swig/CCopasiObjectName.i \
                     ../swig/CCopasiParameter.i \
                     ../swig/CCopasiParameterGroup.i \
                     ../swig/CCopasiProblem.i \
                     ../swig/CCopasiStaticString.i \
                     ../swig/CCopasiTask.i \
                     ../swig/CCopasiVector.i \
                     ../swig/CEvaluationTree.i \
                     ../swig/CFunction.i \
                     ../swig/CFunctionDB.i \
                     ../swig/CFunctionParameter.i \
                     ../swig/CFunctionParameters.i \
                     ../swig/CMatrix.i \
                     ../swig/CMetab.i \
                     ../swig/CModel.i \
                     ../swig/CModelValue.i \
                     ../swig/CMoiety.i \
		     ../swig/CNewtonMethod.i \
                     ../swig/COutputAssistant.i \
                     ../swig/COutputHandler.i \
                     ../swig/CRandom.i \
                     ../swig/CReaction.i \
                     ../swig/CReport.i \
                     ../swig/CReportDefinition.i \
                     ../swig/CReportDefinitionVector.i \
       		     ../swig/CScanMethod.i \
		     ../swig/CScanProblem.i \
		     ../swig/CScanTask.i \
                     ../swig/CState.i \
       		     ../swig/CSteadyStateMethod.i \
		     ../swig/CSteadyStateProblem.i \
		     ../swig/CSteadyStateTask.i \
                     ../swig/CTimeSeries.i \
                     ../swig/CTrajectoryMethod.i \
                     ../swig/CTrajectoryProblem.i \
                     ../swig/CTrajectoryTask.i \
                     ../swig/CLyapMethod.i \
                     ../swig/CLyapProblem.i \
                     ../swig/CLyapTask.i \
                     ../swig/COptItem.i \
                     ../swig/COptMethod.i \
                     ../swig/COptProblem.i \
                     ../swig/COptTask.i \
                     ../swig/CVersion.i \
                     ../swig/CVector.i \
                     ../swig/compare_utilities.i \
                     ../swig/copasi.i \
                     ../swig/CCopasiArray.i



UNITTEST_FILES = unittests/Test_CChemEq.py \
                 unittests/Test_CChemEqElement.py \
                 unittests/Test_CCompartment.py \
                 unittests/Test_CCopasiContainer.py \
                 unittests/Test_CCopasiDataModel.py \
                 unittests/Test_CCopasiMethod.py \
                 unittests/Test_CCopasiObject.py \
                 unittests/Test_CCopasiObjectName.py \
                 unittests/Test_CCopasiParameter.py \
                 unittests/Test_CCopasiParameterGroup.py \
                 unittests/Test_CCopasiProblem.py \
                 unittests/Test_CCopasiStaticString.py \
                 unittests/Test_CCopasiTask.py \
                 unittests/Test_CCopasiVector.py \
                 unittests/Test_CEvaluationTree.py \
                 unittests/Test_CFunction.py \
                 unittests/Test_CFunctionDB.py \
                 unittests/Test_CFunctionParameter.py \
                 unittests/Test_CFunctionParameters.py \
                 unittests/Test_CMatrix.py \
                 unittests/Test_CMetab.py \
                 unittests/Test_CModel.py \
                 unittests/Test_CModelValue.py \
                 unittests/Test_CMoiety.py \
                 unittests/Test_COutputAssistant.py \
                 unittests/Test_CReaction.py \
                 unittests/Test_CReport.py \
                 unittests/Test_CReportDefinition.py \
                 unittests/Test_CReportDefinitionVector.py \
                 unittests/Test_CState.py \
                 unittests/Test_CTimeSeries.py \
                 unittests/Test_CTrajectoryMethod.py \
                 unittests/Test_CTrajectoryProblem.py \
                 unittests/Test_CTrajectoryTask.py \
                 unittests/Test_CVersion.py \
                 unittests/Test_CreateSimpleModel.py \
                 unittests/Test_RunSimulations.py \
                 unittests/runTests.py 
 


#DISTFILE   = $$SWIG_INTERFACE_FILES
#DISTFILES += local.cpp
#DISTFILES += python.i
#DISTFILES += $$UNITTEST_FILES

isEmpty(SWIG_PATH){
    # check if the wrapper file is there
    !exists(copasi_wrapper.cpp){
        error(Wrapper file copasi_wrapper.cpp missing. Please reconfigure with --with-swig=PATH_TO_SWIG.)
    }
}

!isEmpty(SWIG_PATH){
    # check if swig is there and create a target to run it to create
    # copasi_wrapper.cpp
    contains(BUILD_OS, WIN32){
        !exists($$SWIG_PATH/swig.exe){
        error(Unable to find swig excecutable in $$SWIG_PATH. Please use --with-swig=PATH to specify the path where PATH/swig.exe is located.) 
         }
    }
    !contains(BUILD_OS, WIN32){
      !exists($$SWIG_PATH/bin/swig){
        error(Unable to find swig excecutable in $$SWIG_PATH/bin/. Please use --with-swig=PATH to specify the path where PATH/bin/swig is located.) 
      }
    }

    DEFINE_COMMANDLINE = $$join(DEFINES," -D",-D)
    contains(BUILD_OS, WIN32){
      wrapper_source.target = copasi_wrapper.cpp
      wrapper_source.depends = $$SWIG_INTERFACE_FILES python.i local.cpp
      wrapper_source.commands = $(DEL_FILE) $$wrapper_source.target && $$SWIG_PATH\swig.exe $$DEFINE_COMMANDLINE -I..\.. -c++ -python -o $$wrapper_source.target python.i
      QMAKE_EXTRA_WIN_TARGETS += wrapper_source
      PRE_TARGETDEPS += ..\..\lib\COPASISE.lib
    }
    !contains(BUILD_OS, WIN32){
      wrapper_source.target = copasi_wrapper.cpp
      wrapper_source.depends = $$SWIG_INTERFACE_FILES python.i local.cpp
      wrapper_source.commands = $(DEL_FILE) $$wrapper_source.target ; $$SWIG_PATH/bin/swig $$DEFINE_COMMANDLINE -classic -I../.. -c++ -python -o $$wrapper_source.target python.i
  
      QMAKE_EXTRA_UNIX_TARGETS += wrapper_source
      PRE_TARGETDEPS += ../../lib/libCOPASISE.a
    }
    PRE_TARGETDEPS += copasi_wrapper.cpp
}

QMAKE_CLEAN += copasi_wrapper.cpp 
QMAKE_CLEAN += COPASI.py 

SOURCES += copasi_wrapper.cpp
# under windows qmake seems to ignore the last line of progject files


