#-------------------------------------------------
#
# Project created by QtCreator 2012-11-07T11:35:26
#
#-------------------------------------------------

QT += svg
QT += serialport


DEFINES += _BUILDING_ARMOR_CONFIGURATOR_
#DEFINES += QT_NO_DEBUG_OUTPUT

win32:RC_FILE = resources/appicon.rc
DEFINES += _APP_NAME=\\\"Simple_ARMOR_configurator\\\"
DEFINES += _APP_VERSION=\\\"0.1.0.141117a\\\"

TRANSLATIONS = app_ru.ts


#################################################################################
win32 {

include($$(SYSTEMDRIVE)/QxOrm/QxOrm.pri)

QXORM_LIB_PATH = $$(SYSTEMDRIVE)/QxOrm/lib
QXORM_INCLUDE_PATH = $$(SYSTEMDRIVE)/QxOrm/include

exists($$PWD/_laptop_win32) {
HEADERS += $$(SYSTEMDRIVE)/Dvlp/OpenCPU_GS2_SDK_V5.0/custom/common/configurator_protocol.h
INCLUDEPATH += $$(SYSTEMDRIVE)/Dvlp/OpenCPU_GS2_SDK_V5.0/custom/common
} # _laptop_win32

exists($$PWD/_laptop_victor.txt) {
HEADERS += E:/Armor/OpenCPU_GS2_SDK_V5.0/custom/common/configurator_protocol.h
INCLUDEPATH += E:/Armor/OpenCPU_GS2_SDK_V5.0/custom/common
} # _laptop_victor.txt

} # win32
#################################################################################

#################################################################################
unix {

# **************** laptop ***************
exists($$PWD/_laptop_linux) {
include($$(HOME)/LIBS/QxOrm/QxOrm.pri)

QXORM_LIB_PATH =$$(HOME)/LIBS/QxOrm/lib
QXORM_INCLUDE_PATH =$$(HOME)/LIBS/QxOrm/include

HEADERS += $$(HOME)/Dvlp/OpenCPU_GS2_SDK_V5.0/custom/common/configurator_protocol.h
INCLUDEPATH += $$(HOME)/Dvlp/OpenCPU_GS2_SDK_V5.0/custom/common

# =======================================
# special paths for libGL by NVidia
#INCLUDEPATH += /usr/include/nvidia-319-updates
#LIBS += -L/usr/lib/nvidia-319-updates
#QMAKE_LFLAGS += -Wl,-rpath,/usr/lib/nvidia-319-updates
# =======================================
} # _laptop_linux


# suppress warnings of GCC 4.8
QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

QMAKE_LFLAGS += -Wl,-rpath,$$[QT_INSTALL_LIBS]
QMAKE_LFLAGS += -Wl,-rpath,$$QX_BOOST_LIB_PATH
QMAKE_LFLAGS += -Wl,-rpath,$$QXORM_LIB_PATH
QMAKE_LFLAGS += -Wl,-rpath,.

} # unix
#################################################################################


CONFIG += precompile_header
CONFIG += c++11

INCLUDEPATH += $${QXORM_INCLUDE_PATH}
LIBS += -L$${QXORM_LIB_PATH}

INCLUDEPATH += $${QX_BOOST_INCLUDE_PATH}
LIBS += -L$${QX_BOOST_LIB_PATH}


CONFIG(debug, debug|release) {
    LIBS += -lQxOrmd
    LIBS += -l$${QX_BOOST_LIB_SERIALIZATION_DEBUG}
    OUTPUT_DIR = $$PWD/build/debug
    TARGET = armor_configurator_d
} else {
    LIBS += -lQxOrm
    LIBS += -l$${QX_BOOST_LIB_SERIALIZATION_RELEASE}
    OUTPUT_DIR = $$PWD/build/release
    TARGET = armor_configurator
}

DESTDIR = $$PWD/build

RCC_DIR = $$OUTPUT_DIR
MOC_DIR = $$OUTPUT_DIR
OBJECTS_DIR = $$OUTPUT_DIR
UI_DIR = $$OUTPUT_DIR
PRECOMPILED_DIR = $$OUTPUT_DIR
#OUT_PWD = $$OUTPUT_DIR

INCLUDEPATH += $$PWD/src
INCLUDEPATH += $$PWD/src/bo
INCLUDEPATH += $$PWD/src/managerform
INCLUDEPATH += $$PWD/src/communicator
INCLUDEPATH += $$OUTPUT_DIR

DEPENDPATH += $$INCLUDEPATH


win32 {
CONFIG(debug, debug|release) {
} else {
DEFINES += NDEBUG
win32-msvc2005: QMAKE_LFLAGS += /OPT:NOREF
win32-msvc2008: QMAKE_LFLAGS += /OPT:NOREF
win32-msvc2010: QMAKE_LFLAGS += /OPT:NOREF
win32-msvc2012: QMAKE_LFLAGS += /OPT:NOREF
} # CONFIG(debug, debug|release)
win32-g++: QMAKE_LFLAGS += -Wl,--export-all-symbols -Wl,-enable-auto-import
} # win32


TEMPLATE = app
PRECOMPILED_HEADER = src/precompiled.h

SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/logconsole.cpp \
    src/progressdialog.cpp \
    src/qled.cpp \
    src/HierarchicalHeaderView.cpp \
    src/xmlsettingsprovider.cpp \
    src/adccalibrator.cpp \
    src/fotauploader.cpp \
    \
    src/communicator/abstractcommunicator.cpp \
    src/communicator/localcommunicator.cpp \
    src/communicator/remotecommunicator.cpp \
    src/communicator/bridgecommunicator.cpp \
    \
    src/bo/commontrigger.cpp \
    src/bo/t_zone.cpp \
    src/bo/t_arminggroup.cpp \
    src/bo/t_commonsettings.cpp \
    src/bo/t_expander.cpp \
    src/bo/t_phone.cpp \
    src/bo/t_ipaddress.cpp \
    src/bo/t_simcard.cpp \
    src/bo/t_systeminfo.cpp \
    src/bo/t_auxphone.cpp \
    src/bo/t_key.cpp \
    src/bo/t_systemboard.cpp \
    src/bo/t_etr.cpp \
    src/bo/t_relay.cpp \
    src/bo/t_button.cpp \
    src/bo/t_bell.cpp \
    src/bo/t_led.cpp \
    src/bo/t_event.cpp \
    src/bo/t_reaction.cpp \
    src/bo/t_behaviorpreset.cpp \
    src/bo/s_parentunit.cpp \
    src/bo/s_relationetrgroup.cpp \
    \
    src/managerform/arminggroupsmanager.cpp \
    src/managerform/expandersmanager.cpp \
    src/managerform/commonsettingsmanager.cpp \
    src/managerform/loopsmanager.cpp \
    src/managerform/keysmanager.cpp \
    src/managerform/relaysmanager.cpp \
    src/managerform/systemboardmanager.cpp \
    src/managerform/reactionsmanager.cpp \
    src/managerform/etrmanager.cpp \
    src/managerform/eventsmanager.cpp \
    src/managerform/behaviorpresetsmanager.cpp \
    src/managerform/abstractmanager.inl \
    src/managerform/ledsmanager.cpp \
    src/managerform/bellsmanager.cpp \
    src/managerform/buttonsmanager.cpp \
    src/managerform/keyrequestdialog.cpp


HEADERS += \
    src/export.h \
    src/precompiled.h \
    src/mainwindow.h \
    src/crc.h \
    src/logconsole.h \
    src/progressdialog.h \
    src/qled.h \
    src/ledindicator.h \
    src/HierarchicalHeaderView.h \
    src/headerviewmodel.h \
    src/xmlsettingsprovider.h \
    src/adccalibrator.h \
    src/fotauploader.h \
    \
    src/communicator/abstractcommunicator.h \
    src/communicator/localcommunicator.h \
    src/communicator/remotecommunicator.h \
    src/communicator/communicatorfactory.h \
    src/communicator/bridgecommunicator.h \
    \
    src/serialization/persistqflags.h \
    \
    src/bo/commontrigger.h \
    src/bo/t_zone.h \
    src/bo/t_arminggroup.h \
    src/bo/t_commonsettings.h \
    src/bo/t_expander.h \
    src/bo/t_phone.h \
    src/bo/t_ipaddress.h \
    src/bo/t_simcard.h \
    src/bo/t_systeminfo.h \
    src/bo/t_auxphone.h \
    src/bo/t_key.h \
    src/bo/t_systemboard.h \
    src/bo/t_etr.h \
    src/bo/t_relay.h \
    src/bo/t_button.h \
    src/bo/t_bell.h \
    src/bo/t_led.h \
    src/bo/t_event.h \
    src/bo/t_reaction.h \
    src/bo/t_behaviorpreset.h \
    src/bo/s_parentunit.h \
    src/bo/s_relationetrgroup.h \
    src/bo/bo.h \
    src/bo/textcodec.h \
    \
    src/managerform/arminggroupsmanager.h \
    src/managerform/expandersmanager.h \
    src/managerform/commonsettingsmanager.h \
    src/managerform/loopsmanager.h \
    src/managerform/keysmanager.h \
    src/managerform/relaysmanager.h \
    src/managerform/systemboardmanager.h \
    src/managerform/reactionsmanager.h \
    src/managerform/etrmanager.h \
    src/managerform/eventsmanager.h \
    src/managerform/behaviorpresetsmanager.h \
    src/managerform/abstractmanager.h \
    src/managerform/ledsmanager.h \
    src/managerform/bellsmanager.h \
    src/managerform/buttonsmanager.h \
    src/managerform/keyrequestdialog.h

RESOURCES += \
    resources/resources.qrc \
    resources/qled.qrc

FORMS += \
    src/mainwindow.ui \
    src/logconsole.ui \
    src/adccalibrator.ui \
    src/fotauploaderform.ui \
    \
    src/managerform/arminggroupsmanager.ui \
    src/managerform/expandersmanager.ui \
    src/managerform/protectionloopsmanager.ui \
    src/managerform/commonsettingsmanager.ui \
    src/managerform/keysmanager.ui \
    src/managerform/systemboardmanager.ui \
    src/managerform/relaysmanager.ui \
    src/managerform/reactionsmanager.ui \
    src/managerform/etrmanager.ui \
    src/managerform/behaviorpresetssmanager.ui \
    src/managerform/eventsmanager.ui \
    src/managerform/ledsmanager.ui \
    src/managerform/buttonsmanager.ui \
    src/managerform/bellsmanager.ui \
    src/managerform/keyrequestdialog.ui

OTHER_FILES +=
