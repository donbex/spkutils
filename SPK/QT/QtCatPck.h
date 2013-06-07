#ifndef __QTCATPCK_H__
#define __QTCATPCK_H__

#include <QString.h>
#include <QPtrList.h>
#include <QFile.h>
#include <QFileInfo.h>

typedef struct SCatFile {
	QString sName;
	long	iSize;
} SCatFile;

void UnpackFile ( const QString &filename, const QString &unpackedFile );
bool PackFile ( QFile *readfile, const QString &filename, bool bPCK );
bool PackFile ( unsigned char *data, size_t size, const QString &filename, bool bPCK );
bool ExtractFromCat ( const QString &filename, const QString &savefile, bool convert = false, bool plain = false );
void CopyCatFile ( const QString &from, const QString &to );
bool RemoveFromCat ( const QString &file );
QPtrList<SCatFile> GetCatList ( const QString &file );
bool ExtractFile ( const QString &dir, const QString &file, const QString &to );

#endif //__QTCATPCK_H__