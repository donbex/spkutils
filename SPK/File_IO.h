#ifndef __FILE_IO_H__
#define __FILE_IO_H__

#include "String.h"

class CFileIO
{
public:
	CFileIO ();
	CFileIO ( String filename ) { Open ( filename, true ); }

	bool Open ( String filename, bool = true );
	void ReadFileSize ();

	String GetFullFilename () { return m_sFilename; }
	String GetFilename () { return m_sFile; }
	String GetDir() { return m_sDir; }

	bool NoFile () { return m_sFilename.Empty(); }
	size_t GetFilesize () { return m_lSize; }

	char *ReadToData ( size_t *size );
	bool WriteData ( const char *data, size_t size );
	bool WriteString ( String data );

	bool TruncateFile ( size_t offset, size_t datasize );

	bool Exists ();
	bool AppendFile ( String filename );
	bool AppendData ( const char *d, size_t size );

	String GetFileExtension ();
	String ChangeFileExtension ( String ext );

private:
	String m_sFilename;
	String m_sDir;
	String m_sFile;

	bool m_bOpened;
	bool m_bBinary;

	FILE *m_fId;

	size_t m_lSize;
};

#endif //__FILE_IO_H__

