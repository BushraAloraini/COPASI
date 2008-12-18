# Begin CVS Header
#   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/CopasiUI/CopasiUI.pro,v $
#   $Revision: 1.146 $
#   $Name:  $
#   $Author: shoops $
#   $Date: 2008/12/18 17:29:16 $
# End CVS Header

# Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
# Properties, Inc., EML Research, gGmbH, University of Heidelberg,
# and The University of Manchester.
# All rights reserved.

# Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
# Properties, Inc. and EML Research, gGmbH.
# All rights reserved.

######################################################################
# $Revision: 1.146 $ $Author: shoops $ $Date: 2008/12/18 17:29:16 $
######################################################################

TEMPLATE = app

SRC_TARGET = CopasiUI

include(../common.pri)

DEPENDPATH += ..
INCLUDEPATH += ..

COPASI_LIBS += COPASIUI
COPASI_LIBS += COPASISE

contains(BUILD_OS, WIN32) {
  RC_FILE = CopasiUI.rc

  LIBS += $$join(COPASI_LIBS, ".lib  ../lib/", ../lib/, .lib)

  TARGETDEPS += $$join(COPASI_LIBS, ".lib  ../lib/", ../lib/, .lib)

  release {
    distribution.extra = C:\cygwin\bin\bash ../../admin/mkbuild.sh $${BUILD_OS}
  }
 
!isEmpty(QWT_PATH) {
    QMAKE_CXXFLAGS   += -I"$${QWT_PATH}\include"
    QMAKE_LFLAGS += /LIBPATH:"$${QWT_PATH}\lib"
  }


!isEmpty(QWT3D_PATH) {
    QMAKE_CXXFLAGS   += -I"$${QWT3D_PATH}\include"
    QMAKE_LFLAGS += /LIBPATH:"$${QWT3D_PATH}\lib"
  }
}

contains(BUILD_OS, Linux) {
  CONFIG += opengl

  LIBS += -lfreetype

  LIBS = -L../lib \
         $$join(COPASI_LIBS, " -l", -l) \
         $${LIBS}

  TARGETDEPS += $$join(COPASI_LIBS, ".a  ../lib/lib", ../lib/lib, .a)

  contains(QMAKE_LFLAGS, -static) {
   LIBS += -Wl,-lqt-mt \
           -Wl,-lGL \
           -Wl,-lXxf86vm \
           -Wl,-lXcursor \
           -Wl,-lXft \
           -Wl,-lfontconfig \
           -Wl,-lpthread
  }

  release {
    dynamic_LFLAGS = $${QMAKE_LFLAGS}
    dynamic_LFLAGS -= -static

    dynamic_LIBS = -Wl,-Bstatic $${LIBS} -Wl,-Bdynamic
    dynamic_LIBS -= -Wl,-lqt-mt
    dynamic_LIBS -= -Wl,-lXcursor
    dynamic_LIBS -= -Wl,-lXft
    dynamic_LIBS -= -Wl,-lfontconfig
    dynamic_LIBS -= -Wl,-lpthread

    dynamic.target   = CopasiUI-dynamic
    dynamic.depends  = $(OBJECTS) $(OBJMOC) $(OBJCOMP) $${TARGETDEPS}
    dynamic.commands = \
      $(LINK) $${dynamic_LFLAGS} -L$(QTDIR)/lib -L/usr/X11R6/lib \
              -o $@ $(OBJECTS) $(OBJMOC) $(OBJCOMP) $${dynamic_LIBS} \
              -Wl,--start-group \
              -lqt-mt -lXrender -lXrandr -lXcursor -lXinerama -lXft \
              -lfreetype -lfontconfig -lSM -lICE -lXext -lX11 -lm \
              -Wl,--end-group \
              -ldl -lpthread && \
              strip $@

    QMAKE_EXTRA_UNIX_TARGETS += dynamic

#    distribution.extra = make $${dynamic.target}; \
    distribution.extra = ../../admin/mkbuild.sh $${BUILD_OS}

  }
}

contains(BUILD_OS, SunOS) {
  QMAKE_LFLAGS += -z rescan

  LIBS = -L../lib \
         $$join(COPASI_LIBS, " -l", -l) \
         $${LIBS}

  TARGETDEPS += $$join(COPASI_LIBS, ".a  ../lib/lib", ../lib/lib, .a)

  LIBS += -lICE -ldl

  release {
    distribution.extra = ../../admin/mkbuild.sh $${BUILD_OS}
  }
}

contains(BUILD_OS, Darwin){
  QMAKE_LFLAGS += -Wl,-search_paths_first

  CONFIG += opengl

  LIBS = $$join(COPASI_LIBS, ".a  ../lib/lib", ../lib/lib, .a) \
         $$LIBS

  TARGETDEPS += $$join(COPASI_LIBS, ".a  ../lib/lib", ../lib/lib, .a)

  release {
    distribution.extra = ../../admin/mkbuild.sh $${BUILD_OS}
  }
}

# Input
HEADERS += resource.h

SOURCES += main.cpp

release {
  distribution.path = .
  distribution.file = CopasiUI

  INSTALLS += distribution
}

DISTFILES += CopasiUI.vcproj \
             CopasiUI.rc

#The following line was inserted by qt3to4
QT += xml  opengl qt3support 
