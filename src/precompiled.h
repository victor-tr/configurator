#ifndef PRECOMPILED_H
#define PRECOMPILED_H


#ifdef Q_OS_WIN
#ifndef Q_MOC_RUN   // that's a workaround for "Warning: Macro argument mismatch." MSVC-2010
#include <QxOrm.h>
#endif // Q_MOC_RUN
#else
#include <QxOrm.h>
#endif

#include "export.h"
#include "textcodec.h"


#endif // PRECOMPILED_H
