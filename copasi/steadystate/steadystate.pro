######################################################################
# $Revision: 1.7 $ $Author: shoops $ $Date: 2006/06/20 13:19:55 $  
######################################################################

LIB = steadystate

include(../lib.pri)
include(../common.pri)

# Input
HEADERS += CEigen.h \
           CMCAMethod.h \
           CMCAProblem.h \
	   CMCATask.h \
       	   CNewtonMethod.h \
           CSteadyStateMethod.h \
           CSteadyStateProblem.h \
           CSteadyStateTask.h

SOURCES += CEigen.cpp \
           CMCAMethod.cpp \
           CMCAProblem.cpp \
	   CMCATask.cpp \
           CNewtonMethod.cpp \
           CSteadyStateMethod.cpp \
           CSteadyStateProblem.cpp \
           CSteadyStateTask.cpp

DISTFILES +=steadystate.dsp
