#pragma once
#ifndef TPM_IO_H
#define TPM_IO_H

#include <QtGlobal>

#if (QT_VERSION >= QT_VERSION_CHECK(5,15,0))
#include <QTextStream>
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
using Qt::endl;
#else
#define endl Qt::endl
#endif
#endif

#endif // TPM_IO_H
