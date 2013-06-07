#include "File_IO.h"

CFileIO::CFileIO ()
{
	m_bOpened = false;
	m_fId = 0;
	m_lSize = 0;
	m_bBinary = true;
}

bool CFileIO::Open ( String filename, bool binary )
{
	m_bBinary = binary;
	m_sFilename = filename;
	m_sFilename = m_sFilename.FindReplace ( "\\", "/" );

	m_sDir = filename.GetToken ( "/", 1, filename.NumToken ( "/" ) - 1 );
	m_sFile = filename.GetToken ( "/", filename.NumToken ( "/" ) );

	ReadFileSize();
	return true;
}

void CFileIO::ReadFileSize ()
{
	FILE *id = fopen ( m_sFilename.c_str(), (m_bBinary) ? "rb" : "r" );
	if ( !id )
	{
		m_lSize = 0;
		return;
	}
	fseek ( id, 0, SEEK_END );
	m_lSize = ftell ( id );
	fseek ( id, 0, SEEK_SET );
	fclose ( id );
}

bool CFileIO::TruncateFile ( size_t offset, size_t datasize )
{
	if ( NoFile() )
		return false;

	FILE *id = fopen ( m_sFilename.c_str(), (m_bBinary) ? "rb+" : "r+" );
	if ( !id )
		return false;

	// first find file size
	fseek ( id, 0, SEEK_END );
	size_t fullsize = ftell ( id ), size = fullsize;
	fseek ( id, 0, SEEK_SET );

	char data[500000];

#ifdef _WIN32
	FILE *writeId = fopen ( "temp.tmp", (m_bBinary) ? "wb" : "w" );
	if ( !writeId )
	{
		fclose ( id );
		return false;
	}

	// first write the data before the file
	size_t tosize = offset;
	while ( tosize )
	{
		int read = 500000;
		if ( read > tosize )
			read = tosize;

		fread ( data, sizeof(unsigned char), read, id );
		fwrite ( data, sizeof(unsigned char), read, writeId );

		tosize -= read;
	}

	// next fseek after and write
	fseek ( id, datasize, SEEK_CUR );
	size = fullsize - offset - datasize;
	while ( size )
	{
		int read = 500000;
		if ( read > size )
			read = size;

		fread ( data, sizeof(unsigned char), read, id );
		fwrite ( data, sizeof(unsigned char), read, writeId );

		size -= read;
	}

	fclose ( writeId );
	fclose ( id );

	// now copy to original file
	remove ( m_sFilename.c_str() );
	rename ( "temp.tmp", m_sFilename.c_str() );

#else
	// move to beginning of file data to remove
	fseek ( id, offset, SEEK_SET );

	size_t writepos = offset;
	size_t readpos = offset + datasize;

	size -= readpos;

	while ( size > 0 )
	{
		int read = 500000;
		if ( read > size )
			read = size;

		// read data
		fseek ( id, readpos, SEEK_SET );
		fread ( data, sizeof(unsigned char), read, id );
		size -= read;
		readpos += read;

		// now seek back and write
		fseek ( id, writepos, SEEK_SET );
		fwrite ( data, sizeof(unsigned char), read, id );
		writepos += read;

	}

	truncate ( m_sFilename.c_str(), fullsize - datasize );
	fclose ( id );
#endif

	return true;
}


char *CFileIO::ReadToData ( size_t *size )
{
	*size = 0;

	if ( NoFile() )
		return NULL;

	if ( !m_lSize )
		ReadFileSize();

	if ( !m_lSize )
		return NULL;

	FILE *id = fopen ( m_sFilename.c_str(), (m_bBinary) ? "rb" : "r" );
	if ( !id )
		return NULL;

	char *data = new char[m_lSize];

	fread ( data, sizeof(char), m_lSize, id );
	if ( ferror (id) )
	{
		fclose ( id );
		return NULL;
	}

	*size = m_lSize;
	fclose ( id );

	return data;
}

bool CFileIO::WriteData ( const char *data, size_t size )
{
	if ( NoFile() )
		return false;

	FILE *id = fopen ( m_sFilename.c_str(), (m_bBinary) ? "wb" : "w" );
	if ( !id )
		return false;
	
	fwrite ( data, size, sizeof(char), id );
	fclose ( id );

	ReadFileSize();

	return true;
}

bool CFileIO::WriteString ( String data )
{
	return WriteData ( data.c_str(), data.Length() );
}

bool CFileIO::Exists ()
{
	FILE *id = fopen ( m_sFilename.c_str(), (m_bBinary) ? "rb" : "r" );
	if ( !id )
		return false;

	fclose ( id );
	return true;
}

bool CFileIO::AppendFile ( String filename )
{
	FILE *id = fopen ( filename.c_str(), (m_bBinary) ? "rb" : "r" );
	if ( !id )
		return false;

	FILE *aid = fopen ( m_sFilename.c_str(), (m_bBinary) ? "ab" : "a" );
	if ( !aid )
	{
		return false;
		fclose ( id );
	}

	// move to the end of the file
	fseek ( aid, 0, SEEK_END );

	// get size of file
	fseek ( id, 0, SEEK_END );
	size_t size = ftell ( id );
	fseek ( id, 0, SEEK_SET );

	char data[500000];
	while ( size > 0 )
	{
		size_t read = 500000;
		if ( read > size )
			read = size;

		size -= read;

		fread ( data, sizeof(char), read, id );
		fwrite ( data, sizeof(char), read, aid );
	}

	fclose ( aid );
	fclose ( id );
	return true;
}


bool CFileIO::AppendData ( const char *d, size_t size )
{
	FILE *aid = fopen ( m_sFilename.c_str(), (m_bBinary) ? "ab" : "a" );
	if ( !aid )
		return false;

	// move to the end of the file
	fseek ( aid, 0, SEEK_END );

	char *pos = (char *)d;
	while ( size > 0 )
	{
		size_t read = 500000;
		if ( read > size )
			read = size;

		size -= read;

		fwrite ( pos, sizeof(char), read, aid );
		pos += read;
	}

	fclose ( aid );
	return true;
}

String CFileIO::GetFileExtension ()
{
	if ( m_sFilename.Empty() )
		return NullString;

	String ext = m_sFilename.GetToken ( ".", m_sFilename.NumToken ( "." ) );
	return ext;
}

String CFileIO::ChangeFileExtension ( String ext )
{
	if ( m_sFilename.Empty() )
		return NullString;

	String noext = m_sFilename.GetToken ( ".", 1, m_sFilename.NumToken ( "." ) - 1 );
	return noext + "." + ext;
}

