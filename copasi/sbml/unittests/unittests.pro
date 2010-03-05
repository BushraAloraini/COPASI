# Begin CVS Header 
#   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/sbml/unittests/unittests.pro,v $ 
#   $Revision: 1.29.2.4 $ 
#   $Name:  $ 
#   $Author: gauges $ 
#   $Date: 2010/03/05 11:34:27 $ 
# End CVS Header 

# Copyright (C) 2010 by Pedro Mendes, Virginia Tech Intellectual 
# Properties, Inc., University of Heidelberg, and The University 
# of Manchester. 
# All rights reserved. 

# Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual 
# Properties, Inc., EML Research, gGmbH, University of Heidelberg, 
# and The University of Manchester. 
# All rights reserved. 

TEMPLATE = app

CONFIG -= qt

include(../../common.pri)
include(../../app.pri)

COPASI_LIBS += $${COPASI_LIBS_SE}

INCLUDEPATH += ../../..

contains(BUILD_OS, WIN32) {
#  LIBS += $$join(COPASI_LIBS, ".lib  ../lib/", ../lib/, .lib)

#  TARGETDEPS += $$join(COPASI_LIBS, ".lib  ../lib/", ../lib/, .lib)
}

contains(BUILD_OS, Linux) {
!isEmpty(CPPUNIT_PATH) {
  LIBS += -L$${CPPUNIT_PATH}/lib -lcppunit
  INCLUDEPATH += $${CPPUNIT_PATH}/include

  LIBS = -L../lib \
         $$join(COPASI_LIBS, " -l", -l) \
         $${LIBS}
}

}

contains(BUILD_OS, SunOS) {
  QMAKE_LFLAGS += -z rescan
  LIBS += -lsocket
  LIBS += -lnsl

!isEmpty(CPPUNIT_PATH) {
  LIBS += -L$${CPPUNIT_PATH}/lib -lcppunit
  INCLUDEPATH += $${CPPUNIT_PATH}/include

  LIBS = $$join(COPASI_LIBS, ".a  ../../lib/lib", ../../lib/lib, .a) \
         $${LIBS}
}

}  

contains(BUILD_OS, Darwin){
  QMAKE_LFLAGS += -Wl,-search_paths_first

  LIBS = $$join(COPASI_LIBS, ".a  ../../lib/lib", ../../lib/lib, .a) \
         $${LIBS}
   
  TARGETDEPS += $$join(COPASI_LIBS, ".a  ../../lib/lib", ../../lib/lib, .a)
  
!isEmpty(CPPUNIT_PATH) {
  LIBS += -L$${CPPUNIT_PATH}/lib -lcppunit
  INCLUDEPATH += $${CPPUNIT_PATH}/include

}

}


# Input
HEADERS += utilities.hpp \
           test000001.hpp \
           test000002.hpp \
           test000003.hpp \
           test000004.hpp \
           test000005.hpp \
           test000006.hpp \
           test000007.hpp \
           test000008.hpp \
           test000009.hpp \
           test000010.hpp \
           test000011.hpp \
           test000012.hpp \
           test000013.hpp \
           test000014.hpp \
           test000015.hpp \
           test000016.hpp \
           test000017.hpp \
           test000018.hpp \
           test000019.hpp \
           test000020.hpp \
           test000021.h \
           test000022.h \
           test000023.h \
           test000024.h \
           test000025.h \
           test000026.h \
           test000027.h \
           test000028.h \
           test000029.h \
           test000030.h \
           test000031.h \
           test000032.h \
           test000033.h \
           test000034.h \
           test000035.h \
           test000036.h \
           test000037.h \
           test000038.h \
           test000039.h \
           test000040.h \
           test000041.h \
           test000042.h \
           test000043.h \
           test000044.h \
           test000045.h \
           test000046.h \
           test000047.h \
           test000048.h \
           test000049.h \
           test000050.h \
           test000051.h \
           test000052.h \
           test000053.h \
           test000054.h \
           test000055.h \
           test000056.h \
           test000057.h \
           test000058.h \
           test000059.h \
           test000060.h \
           test000061.h \
           test000062.h \
           test000063.h \
           test000064.h \
           test000065.h \
           test000066.h \
           test000067.h \
           test000068.h \
           test000069.h \
           test000070.h \
           test000071.h \
           test000072.h \
           test000073.h \
           test000074.h \
           test000075.h \
           test000076.h \
           test000077.h \
           test000078.h \
           test000079.h \
           test000080.h \
           test000081.h \
           test000082.h \
           test000083.h \
           test000084.h \
           test000085.h \
           test000086.h \
           test000087.h \
           test000088.h \
           test000089.h \
           test000090.h \
           test000091.h 


SOURCES += utilities.cpp \
           test000001.cpp \ 
           test000002.cpp \ 
           test000003.cpp \ 
           test000004.cpp \ 
           test000005.cpp \ 
           test000006.cpp \ 
           test000007.cpp \ 
           test000008.cpp \ 
           test000009.cpp \ 
           test000010.cpp \ 
           test000011.cpp \ 
           test000012.cpp \ 
           test000013.cpp \ 
           test000014.cpp \ 
           test000015.cpp \ 
           test000016.cpp \ 
           test000017.cpp \ 
           test000018.cpp \ 
           test000019.cpp \ 
           test000020.cpp \ 
           test000021.cpp \ 
           test000022.cpp \ 
           test000023.cpp \ 
           test000024.cpp \ 
           test000025.cpp \ 
           test000026.cpp \ 
           test000027.cpp \ 
           test000028.cpp \ 
           test000029.cpp \ 
           test000030.cpp \ 
           test000031.cpp \ 
           test000032.cpp \ 
           test000033.cpp \ 
           test000034.cpp \ 
           test000035.cpp \ 
           test000036.cpp \ 
           test000037.cpp \ 
           test000038.cpp \ 
           test000039.cpp \ 
           test000040.cpp \ 
           test000041.cpp \ 
           test000042.cpp \ 
           test000043.cpp \ 
           test000044.cpp \ 
           test000045.cpp \ 
           test000046.cpp \ 
           test000047.cpp \ 
           test000048.cpp \ 
           test000049.cpp \ 
           test000050.cpp \ 
           test000051.cpp \ 
           test000052.cpp \ 
           test000053.cpp \ 
           test000054.cpp \ 
           test000055.cpp \ 
           test000056.cpp \ 
           test000057.cpp \ 
           test000058.cpp \ 
           test000059.cpp \ 
           test000060.cpp \ 
           test000061.cpp \ 
           test000062.cpp \ 
           test000063.cpp \ 
           test000064.cpp \ 
           test000065.cpp \ 
           test000066.cpp \ 
           test000067.cpp \ 
           test000068.cpp \ 
           test000069.cpp \ 
           test000070.cpp \ 
           test000071.cpp \ 
           test000072.cpp \ 
           test000073.cpp \ 
           test000074.cpp \ 
           test000075.cpp \ 
           test000076.cpp \ 
           test000077.cpp \ 
           test000078.cpp \ 
           test000079.cpp \ 
           test000080.cpp \ 
           test000081.cpp \ 
           test000082.cpp \ 
           test000083.cpp \ 
           test000084.cpp \ 
           test000085.cpp \ 
           test000086.cpp \ 
           test000087.cpp \ 
           test000088.cpp \ 
           test000089.cpp \ 
           test000090.cpp \ 
           test000091.cpp \ 
           test.cpp


#The following line was inserted by qt3to4
QT += xml  opengl qt3support 
