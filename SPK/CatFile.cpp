#include "CatFile.h"
#include <time.h>
#include "File.h"

CCatFile::CCatFile ()
{
	m_iDataType = CATFILE_NONE;
	m_sData = NULL;
	m_lSize = 0;
	m_bCreate = false;
}

CCatFile::~CCatFile ()
{
	for ( SInCatFile *c = m_lFiles.First(); c; c = m_lFiles.Next() )
	{
		if ( c->sData )
			delete c->sData;
		delete c;
	}
	m_lFiles.clear();
}

int CCatFile::Open ( String catfile, int readtype, bool create )
{
	m_bCreate = false;

	String datfile;
	if ( catfile.Right (4).ToLower() != ".cat" )
	{
		datfile = catfile + ".dat";
		catfile += ".cat";
	}
	else
		datfile = catfile.Left ( -4 ) + ".dat";

	// first check if the dat file exists and opens
	bool created = false;
	FILE *id = fopen ( datfile.c_str(), "rb+" );
	if ( !id )
	{
		if ( create )
			created = true;
		else
			return CATERR_NODATFILE;
	}

	if ( !created )
		fclose ( id );

	// now open the cat file to read
	FILE *cid = fopen ( catfile.c_str(), "rb" );
	if ( !cid )
	{
		if ( create )
			created = true;
		else
			return CATERR_NOCATFILE;
	}

	if ( created )
	{
		m_fCatFile.Open ( catfile );
		m_fDatFile.Open ( datfile );
		m_bCreate = true;
		return CATERR_CREATED;
	}

	// find the file size
	fseek ( cid, 0, SEEK_END );
	size_t lFileSize = ftell ( cid );
	fseek ( cid, 0, SEEK_SET );

	if ( !lFileSize )
	{
		fclose ( cid );
		return CATERR_FILEEMPTY;
	}

	// size must be multiples of 5
	size_t size = lFileSize + ((lFileSize % 5) ? 5 - (lFileSize % 5) : 0);

	// read cat to buffer
	m_sData = new unsigned char[size + 1];
	m_sData[lFileSize] = 0;
	m_lSize = size;
	fread ( m_sData, sizeof(unsigned char), m_lSize, cid );
	if ( ferror(cid) )
	{
		RemoveData ();
		fclose ( cid );
		return CATERR_READCAT;
	}

	m_iDataType = CATFILE_READ;

	fclose ( cid );

	if ( readtype != CATREAD_CAT )
	{
		if ( !DecryptData () )
			return CATERR_DECRYPT;
		m_sData[lFileSize] = 0;

		ReadFiles ();

	}

	m_fCatFile.Open ( catfile );
	m_fDatFile.Open ( datfile );

	// check the file size matches
	long compare = 0;
	id = fopen ( datfile.c_str(), "rb" );
	if ( id )
	{
		fseek ( id, 0, SEEK_END );
		compare = ftell ( id );
		fclose ( id );
	}

	/*
	SInCatFile *c = m_lFiles.Back ()->Data();
	if ( (c->lSize + c->lOffset) != compare )
		return CATERR_MISMATCH;
	*/

	if ( readtype >= CATREAD_DAT )
		LoadDatFile ();

	return CATERR_NONE;
}

bool CCatFile::RemoveFile ( String file )
{
	SInCatFile *f = FindData ( file );
	if ( !f )
	{
		m_iError = CATERR_NOFILE;
		return false;
	}

	return RemoveFile ( f );
}

void CCatFile::LoadDatFile ()
{
	if ( m_fDatFile.NoFile() )
		return;

	FILE *id = fopen ( m_fDatFile.GetFullFilename().c_str(), "rb" );
	if ( id )
	{
		for ( SInCatFile *c = m_lFiles.First(); c; c = m_lFiles.Next() )
		{
			if ( c->sData )
				delete c->sData;
			c->sData = new unsigned char[c->lSize + 1];
			fread ( c->sData, sizeof(unsigned char), c->lSize, id );
		}
		fclose ( id );
	}
}

bool CCatFile::ReadFiles ()
{
	int num = 0;
	String s((const char*)m_sData);
	String *lines = s.SplitToken ( '\n', &num );

	size_t offset = 0;

	for ( int i = 1; i < num; i++ )
	{
		String l = lines[i];
		if ( l.NumToken ( ' ' ) <= 1 )
			continue;

		SInCatFile *catfile = new SInCatFile;

		catfile->lSize = l.GetToken ( l.NumToken ( ' ' ), ' ' ).ToLong();
		catfile->sFile = l.GetToken ( 0, l.NumToken ( ' ' ) - 1, ' ' );
		catfile->lOffset = offset;

		offset += catfile->lSize;

		m_lFiles.push_back ( catfile );
	}

	return true;
}

bool CCatFile::DecryptData ( unsigned char *data, size_t size )
{
	unsigned char cl=0xDB, dh=0xDC, dl=0xDD, ah=0xDE, ch=0xDF, al;
	unsigned char *ptr = data, *end = data + size;
	size_t i = 0;

	for ( ptr = data; ptr < end; ptr += 5)
	{
		al=ch;
		ch+=5;
		*(ptr + 4) ^= al;
		al=ah;
		ah+=5;
		*(ptr + 3) ^= al;
		al=dl;
		dl+=5;
		*(ptr + 2) ^= al;
		al=dh;
		dh+=5;
		*(ptr + 1) ^= al;
		al=cl;
		cl+=5;
		*(ptr + 0) ^= al;
	}

	return true;
}

bool CCatFile::DecryptData ()
{
	if ( !DecryptData ( m_sData, m_lSize ) )
		return false;

	m_iDataType = CATERR_DECRYPT;

	return true;
}

void CCatFile::RemoveData ()
{
	if ( m_sData )
		delete m_sData;
	m_sData = NULL;
	m_iDataType = CATFILE_NONE;
	m_lSize = 0;
}

bool CCatFile::ReadFileToData ( String filename )
{
	SInCatFile *c = FindData ( filename );
	return ReadFileToData ( c );
}

bool CCatFile::ReadFileToData ( SInCatFile *c )
{
	if ( !c )
		return false;
	size_t size = 0;
	c->sData = ReadData ( c, &size );

	if ( c->sData )
		return true;

	return false;
}

unsigned char *CCatFile::ReadData ( SInCatFile *c, size_t *size )
{
	*size = c->lSize;

	FILE *id = fopen ( m_fDatFile.GetFullFilename().c_str(), "rb" );
	if ( id )
	{
		fseek ( id, c->lOffset, SEEK_SET );
		unsigned char *data = new unsigned char[c->lSize];
		fread ( data, sizeof(unsigned char), c->lSize, id );
		*size = c->lSize;
		fclose ( id );

		return data;
	}

	return NULL;
}

unsigned char *CCatFile::ReadData ( String filename, size_t *size )
{
	*size = 0;

	if ( !m_fDatFile.NoFile() )
	{
		SInCatFile *c = FindData ( filename );
		if ( c )
			return ReadData ( c, size );
	}

	return NULL;
}

SInCatFile *CCatFile::FindData ( String filename )
{
	if ( m_lFiles.empty() )
		return NULL;

	String check = filename;
	check.ToLower();
	check = check.FindReplace ( "\\", "/" );
	for ( SInCatFile *c = m_lFiles.First(); c; c = m_lFiles.Next() )
	{
		String f = c->sFile;
		f = f.FindReplace ( "\\", "/" );
		if ( f.lower() == check.lower() )
			return c;
	}
	return NULL;
}


unsigned char *CCatFile::UnpackFile ( SInCatFile *c, size_t *size )
{
	*size = 0;
	if ( !c )
		return NULL;

	if ( IsDataPCK ( (const unsigned char *)c->sData, c->lSize ) )
		return UnPCKData ( c->sData, c->lSize, size );

	*size = c->lSize;
	return c->sData;
}

bool CCatFile::RemoveFile ( SInCatFile *f )
{
	if ( (m_bCreate) || (m_lFiles.empty()) )
		return false;

	if ( !f )
		return false;

	if ( !m_fDatFile.TruncateFile ( f->lOffset, f->lSize ) )
		return false;

	// now just write the new cat file
	m_lFiles.remove ( f );
	WriteCatFile ();

	return true;
}

bool CCatFile::WriteCatFile ()
{
	if ( (m_bCreate) || (m_lFiles.empty()) )
		return false;

	String cat = m_fCatFile.GetFilename() + "\n";
	
	for ( SInCatFile *f = m_lFiles.First(); f; f = m_lFiles.Next() )
		cat += f->sFile + " " + (long)f->lSize + "\n";

	unsigned char *data = new unsigned char[cat.Length() + 1];
	memcpy ( data, cat.c_str(), cat.Length() );

	DecryptData ( data, cat.Length() );

	m_fCatFile.WriteData ( (const char *)data, cat.Length() );

	return true;
}

bool CCatFile::CheckExtensionPck ( String filename )
{
	String ext = filename.GetToken ( ".", filename.NumToken  ( "." ) ).lower();
	
	if ( ext == "xml" )
		return true;
	else if ( ext == "txt" )
		return true;
	else if ( ext == "bob" )
		return true;
	else if ( ext == "bod" )
		return true;

	return false;
}

bool CCatFile::CheckPackedExtension ( String filename )
{
	String ext = filename.GetToken ( ".", filename.NumToken  ( "." ) ).lower();
	
	if ( ext == "pck" )
		return true;
	else if ( ext == "pbb" )
		return true;
	else if ( ext == "pbd" )
		return true;

	return false;
}

String CCatFile::PckChangeExtension ( String f )
{
	CFileIO fo ( f );
	String ext = fo.GetFileExtension ().lower();
	if ( ext == "txt" )
		return fo.ChangeFileExtension ( "pck" );
	else if ( ext == "xml" )
		return fo.ChangeFileExtension ( "pck" );
	else if ( ext == "bob" )
		return fo.ChangeFileExtension ( "pbb" );
	else if ( ext == "bod" )
		return fo.ChangeFileExtension ( "pbd" );

	return f;
}

bool CCatFile::AppendFile ( String filename, String to, bool pck )
{
	if ( (!m_bCreate) && (!m_fCatFile.Exists ()) )
		return false;

	if ( filename.IsIn ( "::" ) )
		return WriteFromCat ( filename.GetToken ( "::", 1, 1 ), filename.GetToken ( "::", 2, 2 ) );

	// first open the file to check it exists
	FILE *id = fopen ( filename.c_str(), "rb" );
	if ( !id )
		return false;
	fclose ( id );

	// then check if the file already exists
	if ( !m_lFiles.empty() )
	{
		SInCatFile *f = FindData ( to );
		if ( f )
		{
			if ( !RemoveFile ( f ) )
				return false;
		}
	}

	bool append = false;

	SInCatFile *f = new SInCatFile;
	if ( m_lFiles.empty() )
		f->lOffset = 0;
	else
		f->lOffset = m_lFiles.Back()->Data()->lOffset + m_lFiles.Back()->Data()->lSize;

	bool dofile = true;
	if ( ((pck) && (CheckExtensionPck ( filename ))) || ((CheckPackedExtension ( to ))) )
	{
		to = PckChangeExtension ( to );

		if ( !m_lFiles.empty() )
		{
			SInCatFile *checkf = FindData ( to );
			if ( checkf )
			{
				if ( !RemoveFile ( checkf ) )
					return false;
			}
		}

		FILE *id = fopen ( filename.c_str(), "rb" );
		if ( !id )
			return false;
		fseek ( id, 0, SEEK_END );
		size_t size = ftell ( id );
		fseek ( id, 0, SEEK_SET );
		unsigned char *data = new unsigned char[size];
		fread ( data, sizeof(unsigned char), size, id );
		fclose ( id );

		if ( !IsDataPCK ( data, size ) )	
		{
			dofile = false;

			size_t newsize = 0;
			unsigned char *newdata = PCKData ( data, size, &newsize );

			f->lSize = newsize;
			append = m_fDatFile.AppendData ( (const char *)newdata, newsize );

			delete [] newdata;
		}

		delete [] data;
	}

	if ( dofile )
	{
		FILE *id = fopen ( filename.c_str(), "rb" );
		if ( !id )
			return false;
		fseek ( id, 0, SEEK_END );
		f->lSize = ftell ( id );
		fclose ( id );
		append = m_fCatFile.AppendFile ( filename );
	}

	if ( append )
	{
		m_bCreate = false;
		f->sFile = to;
		m_lFiles.push_back ( f );
		WriteCatFile ();
	}
	else
		delete f;

	return true;
}

bool CCatFile::AddData ( String catfile, unsigned char *data, size_t size, String to, bool pck, bool create )
{
	int err = Open ( catfile, CATREAD_CATDECRYPT, create );
	if ( (err != CATERR_NONE) && (err != CATERR_CREATED) )
		return false;

	return AppendData ( data, size, to, pck );
}

bool CCatFile::AppendData ( unsigned char *data, size_t size, String to, bool pck )
{
	if ( (!m_bCreate) && (!m_fCatFile.Exists ()) )
		return false;

	if ( (size <= 0) || (!data) )
		return false;

	// then check if the file already exists
	if ( !m_lFiles.empty() )
	{
		SInCatFile *f = FindData ( to );
		if ( f )
		{
			if ( !RemoveFile ( f ) )
				return false;
		}
	}

	bool append = false;

	SInCatFile *f = new SInCatFile;
	if ( m_lFiles.empty() )
		f->lOffset = 0;
	else
		f->lOffset = m_lFiles.Back()->Data()->lOffset + m_lFiles.Back()->Data()->lSize;

	// if file extension is packed but not the file, then pack it, "pck" forces it to be packed unless it already is

	f->lSize = size;
	if ( (((pck) && (CheckExtensionPck (to))) || ((CheckPackedExtension ( to )))) && (!IsDataPCK ( data, size )) )
	{
		to = PckChangeExtension ( to );

		if ( !m_lFiles.empty() )
		{
			SInCatFile *f = FindData ( to );
			if ( f )
			{
				if ( !RemoveFile ( f ) )
					return false;
			}
		}
		size_t newsize = 0;
		unsigned char *d = PCKData ( data, size, &newsize );

		f->lSize = newsize;
		append = m_fDatFile.AppendData ( (const char *)d, newsize );

		delete [] d;
	}
	else
		append = m_fDatFile.AppendData ( (const char *)data, size );

	if ( append )
	{
		m_bCreate = false;
		f->sFile = to;
		m_lFiles.push_back ( f );
		WriteCatFile ();
	}
	else
		delete f;

	return true;
}


bool CCatFile::ExtractFile ( String filename, String to )
{
	if ( m_lFiles.empty() )
		return false;

	// check if file exists
	SInCatFile *f = FindData ( filename );
	if ( !f )
	{
		m_iError = CATERR_NOFILE;
		return false;
	}

	unsigned char *data = 0;
	size_t size = 0;

	// load the data from file
	ReadFileToData ( f );

	data = UnpackFile ( f, &size );
	if ( !data )
	{
		m_iError = CATERR_CANTREAD;
		return false;
	}

	String tofile = to;
	if ( !tofile.Empty() )
		tofile += "/";
	tofile += filename;
	tofile = tofile.FindReplace ( "/", "\\" );
	String todir = tofile.GetToken ( "\\", 1, tofile.NumToken('\\') - 1 );

	// create the directory to extract to
	if ( !CreateDirectory ( todir ) )
		m_iError = CATERR_CANTCREATEDIR;
	else
	{
		FILE *id = fopen ( tofile.c_str(), "wb" );
		if ( !id )
			m_iError = CATERR_INVALIDDEST;
		else
		{
			fwrite ( data, sizeof(char), size, id );
			fclose ( id );

			ClearError ();

			delete data;

			return true;
		}
	}

	delete data;

	return false;
}

String CCatFile::GetErrorString ()
{
	switch ( m_iError )
	{
		case CATERR_NONE:
			return NullString;
		case CATERR_NODATFILE:
			return "Unable to open Dat file";
		case CATERR_NOCATFILE:
			return "Unable to open Cat file";
		case CATERR_FILEEMPTY:
			return "Cat file is empty";
		case CATERR_READCAT:
			return "Unable to read from cat file";
		case CATERR_DECRYPT:
			return "Unable to decrypt cat file";
		case CATERR_MISMATCH:
			return "File size mismatch with Dat file";
		case CATERR_NOFILE:
			return "Unable to find file in archive";
		case CATERR_CANTREAD:
			return "Unable to read file in archive";
		case CATERR_CANTCREATEDIR:
			return "Unable to create destiantion directory";
		case CATERR_INVALIDDEST:
			return "Unable to write to destiantion file";
	}

	return "Invalid";
}


unsigned char *CompressPCKData ( unsigned char *buffer, size_t size, size_t *retsize, size_t mtime )
{
	size_t newsize = (size * 2) + 20;
	unsigned char *data = (unsigned char *)malloc ( sizeof(unsigned char) * newsize );

	z_stream zs;
	char flags=0;
	
//	error(0);
	
	unsigned char *d = data;
//	if(m_pszComment && strlen(m_pszComment) > 0) flags|=GZ_F_COMMENT;
//	if(m_pszFileName && strlen(m_pszFileName) > 0) flags|=GZ_F_FILENAME;	
	
	int pos = 6 + sizeof(mtime);
	*d = 0x1F; 		d++;
	*d = 0x8B; 		d++;
	*d = 8;			d++;
	*d = flags; 	d++;
	memcpy(d, &mtime, sizeof(mtime));
	d += sizeof(mtime);
	*d = 0; 		d++;
	*d = 11; 		d++;
//	if(flags & GZ_F_FILENAME) put((const char *)m_pszFileName);
//	if(flags & GZ_F_COMMENT) put((const char *)m_pszComment);
	
	memset(&zs, 0, sizeof(zs));
	zs.next_in=buffer;
	zs.avail_in=size;

	int ret;
	unsigned long ubound;
	ret=deflateInit2(&zs, 9, Z_DEFLATED, -15, 9, Z_DEFAULT_STRATEGY);
	if(ret!=Z_OK) 
		return NULL;

	ubound=deflateBound(&zs, size);
	if ( newsize < ubound) 
	{
		newsize += ubound;
		data = (unsigned char *)realloc ( data, sizeof(unsigned char) * newsize );
	}

	
	zs.next_out=d;
	zs.avail_out=newsize - pos;

	while((ret=deflate(&zs, Z_FINISH))==Z_OK)
	{
		newsize += 1024;
		data = (unsigned char *)realloc ( data, sizeof(unsigned char) * newsize );
		zs.next_out=data + zs.total_out;
		zs.avail_out=newsize - zs.total_out;
	}
	pos += (size_t)zs.total_out;

	deflateEnd(&zs);

	unsigned long crc=crc32(0, NULL, 0);
	crc=crc32(crc, buffer, size);

	int s = sizeof(crc) + sizeof(size);
	if ( newsize < (s + pos) )
	{
		newsize += (s + pos) - newsize;
		data = (unsigned char *)realloc ( data, sizeof(unsigned char) * newsize );
	}

	memcpy(&data[pos], &crc, sizeof(crc));
	pos += sizeof(crc);
	memcpy(&data[pos], &size, sizeof(size));
	pos += sizeof(size);

	newsize = pos;

	unsigned char *retdata = NULL;
	if ( ret == Z_STREAM_END )
	{
		*retsize = newsize;
		retdata = new unsigned char[newsize];
		memcpy ( retdata, data, newsize );
	}
	free ( data );

	return retdata;
}

unsigned char *PCKData ( unsigned char *data, size_t oldsize, size_t *newsize )
{
	unsigned char *newdata = CompressPCKData ( data, oldsize, newsize, time(NULL) );
	if ( newdata )
	{
		char magic = (char)clock(), m;
		m=magic ^ 0xC8;

		size_t *uncomprLenSize = (size_t*)(newdata + (*newsize - 4));

		unsigned char *ptr = newdata, *end = newdata + *newsize;
		// XOR encryption
		for ( ; ptr < end; ptr++ )
			(*ptr)^=magic;

		unsigned char *finalData = new unsigned char[*newsize + 1];
		finalData[0] = m;
		memcpy ( finalData + 1, newdata, *newsize );
		delete [] newdata;
		(*newsize)++;
		return finalData;
	}

	return NULL;
}

bool CCatFile::WriteFromCat ( String catfile, String file )
{
	CCatFile fcat;
	if ( fcat.Open ( catfile, CATREAD_CATDECRYPT, false ) )
		return false;

	// now find the file in the cat file
	SInCatFile *getfile = fcat.FindData ( file );
	if ( !getfile )
		return false;

	// read the dat from the cat file
	size_t size = 0;
	unsigned char *data = fcat.ReadData ( getfile, &size );
	if ( !data )
		return false;

	// now check if it exists in this file
	RemoveFile ( file );

	// now write to the new file
	if ( !m_fDatFile.AppendData ( (const char *)data, size ) )
		return false;

	// finally add to the list
	SInCatFile *f = new SInCatFile;
	if ( m_lFiles.empty() )
		f->lOffset = 0;
	else
		f->lOffset = m_lFiles.Back()->Data()->lOffset + m_lFiles.Back()->Data()->lSize;
	f->sFile = file;
	f->lSize = size;
	m_lFiles.push_back ( f );
	WriteCatFile ();

	return true;
}

