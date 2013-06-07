#include "QtCatPck.h"
#include "../spk.h"

/*
========================================================================
 Cat and PCK FrontEnd Libraies using QT
========================================================================
*/


void UnpackFile ( const QString &filename, const QString &unpackedFile )
{
	size_t size;

	QString newFilename = filename;
	newFilename = newFilename.replace ( '/', '\\' );
	QString newUnpackedFile = unpackedFile;
	newUnpackedFile = newUnpackedFile.replace ( '/', '\\' );

	unsigned char *buffer = UnPCKFile ( newFilename, &size );

	if ( buffer )
	{
		FILE *id = fopen ( newUnpackedFile, "wb" );

		if ( id )
		{
			fwrite ( buffer, size, 1, id );
			fclose ( id );
		}

		delete buffer;
	}
}

void CopyCatFile ( const QString &from, const QString &to )
{
	if ( !to.contains ( "::" ) )
		return;

	QString tocatfile = to.section ( "::", 0, 0 );
	QString tofile = to.section ( "::", 1, 1 );

	if ( from.contains ( "::" ) )
	{
		CCatFile tocat;
		if ( tocat.Open ( tocatfile, CATREAD_CATDECRYPT, true ) )
			return;

		tocat.AppendFile ( from, tofile );
	}
}


bool PackFile ( unsigned char *data, size_t size, const QString &filename, bool bPCK )
{
	if ( filename.find ( "::" ) == -1 )
	{
		// delete existing file first
		QString newfilename = filename;
		QFile delfile ( newfilename );
		if ( delfile.exists () )
			delfile.remove ();

		// now open file for writing
		QFile f( filename );
		if ( f.open ( IO_WriteOnly ) )
		{
			size_t newsize = 0;
			unsigned char *d = PCKData ( data, size, &newsize );
		
			bool ret = false;
			if ( d )
			{
				f.writeBlock ( (const char *)d, newsize );
				delete d;
				ret = true;
			}

			f.close ();

			return ret;
		}
	}
	else
	{
		QString catfile = filename.section ( "::", 0, 0 );
		QString file = filename.section ( "::", 1, 1 );

		CCatFile cat;
		int err = cat.Open ( catfile, CATREAD_CATDECRYPT, true );
		if ( (err != CATERR_NONE) && (err != CATERR_CREATED) )
			return false;
	
		return cat.AppendData ( data, size, file );
	}

	return false;
}

bool PackFile ( QFile *readfile, const QString &filename, bool bPCK )
{
	if ( readfile->open ( IO_ReadOnly ) )
	{
		size_t size = QFileInfo ( *readfile ).size();
		unsigned char *data = new unsigned char [size];
		readfile->readBlock ( (char *)data, size );

		readfile->close();

		bool ret = PackFile ( data, size, filename, bPCK );

		delete [] data;

		return ret;
	}

	return false;
}

bool ExtractFromCat ( const QString &filename, const QString &savefile, bool convert, bool plain )
{
	if ( filename.find ( "::" ) == -1 )
		return false;


	QString catfile = filename.section ( "::", 0, 0 );
	QString file = filename.section ( "::", 1, 1 );

	// open the cat file
	CCatFile cat;
	if ( cat.Open ( catfile, CATREAD_CATDECRYPT ) )
		return false;

	// find the file to extract
	SInCatFile *f = cat.FindData ( file );
	if ( !f )
		return false;

	// read the data to memory
	if ( !cat.ReadFileToData ( f ) )
		return false;

	// check for extract
	QString sSaveFile = savefile;
	
	unsigned char *d = f->sData;
	size_t size = f->lSize;

	if ( (plain) && (IsDataPCK ( f->sData, f->lSize )) )
	{
		if ( convert )
		{
			QString ext = QFileInfo(sSaveFile).extension(false).lower();
			if ( ext == "pbb" )
				sSaveFile = QFileInfo(sSaveFile).dirPath() + "/" + QFileInfo(sSaveFile).baseName(true) + ".bob";
			else if ( ext == "pbd" )
				sSaveFile = QFileInfo(sSaveFile).dirPath() + "/" + QFileInfo(sSaveFile).baseName(true) + ".bod";
			else if ( ext == "pck" )
			{
				if ( file.left(5).lower() == "types" )
					sSaveFile = QFileInfo(sSaveFile).dirPath() + "/" + QFileInfo(sSaveFile).baseName(true) + ".txt";
				else
					sSaveFile = QFileInfo(sSaveFile).dirPath() + "/" + QFileInfo(sSaveFile).baseName(true) + ".xml";
			}
		}

		d = UnPCKData ( f->sData, f->lSize, &size );
	}

	if ( !d )
		return false;

	// now write to file
	QFile wfile ( sSaveFile );
	if ( wfile.open ( IO_WriteOnly ) )
	{
		wfile.writeBlock ( (const char *)d, size );
		wfile.close();
	}

	return true;
}

bool RemoveFromCat ( const QString &file )
{
	if ( !file.contains ( "::" ) )
		return false;

	QString catfile = file.section ( "::", 0, 0 );
	QString infile = file.section ( "::", 1, 1 );

	CCatFile cat;
	if ( cat.Open ( catfile, CATREAD_CATDECRYPT, false ) )
		return false;

	if ( !cat.RemoveFile ( infile ) )
		return false;

	return true;
}

QPtrList<SCatFile> GetCatList ( const QString &file )
{
	QPtrList<SCatFile> list;

	CCatFile cat;
	if ( !cat.Open ( file, CATREAD_CATDECRYPT, false ) )
	{
		for ( int i = 0; i < cat.GetNumFiles(); i++ )
		{
			SInCatFile *cf = cat.GetFile (i);
			SCatFile *f = new SCatFile;
			f->iSize = cf->lSize;
			f->sName = cf->sFile.c_str();
			f->sName = f->sName.replace ( "\\", "/" );
			list.append ( f );			
		}
	}

	return list;
}


bool ExtractFile ( const QString &dir, const QString &file, const QString &to )
{
	bool done = false;

	// rmeove old file first
	if ( QFile ( to ).exists() )
		QFile ( to ).remove ();


	if ( file.contains ( "::" ) )
	{
		if ( ExtractFromCat ( file, to, true, true ) )
			done = true;
	}
	else
	{
		// find first file to check
		QString sFile;
		int num;
		for ( num = 1; num <= 99; num++ )
		{
			if ( num < 10 )
				sFile.sprintf ( "0%d.cat", num );
			else
				sFile.sprintf ( "%d.cat", num );

			if ( !QFile ( dir + "/" + sFile ).exists() )
				break;
		}

		--num;
		for ( ; num; num-- )
		{
			if ( num < 10 )
				sFile.sprintf ( "0%d", num );
			else
				sFile.sprintf ( "%d", num );

			if ( ExtractFromCat ( dir + "/" + sFile + ".cat::" + file, to, true, true ) )
			{
				done = true;
				break;
			}
		}
	}

	return done;
}


/*
  ================================================================
  =====            End of Cat/Pck Utils                      =====
  ================================================================
*/

