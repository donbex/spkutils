#include "MultiSpkFile.h"
#include "File.h"

bool CMultiSpkFile::AddFile ( String file, bool on )
{
	// multipack
	if ( file.IsIn ( "::" ) )
	{
		String mfile = file.Left ( file.FindPos("::") );
		CMultiSpkFile spkfile;
		if ( spkfile.ReadFile ( mfile, false ) )
		{
			SMultiSpkFile *ms = new SMultiSpkFile;
			ms->sName = file.Right ( (file.Length() - file.FindPos("::")) - 2 );
			ms->sData = NULL;
			if ( spkfile.ExtractData ( ms ) )
			{
				ms->bOn = on;
				m_lFiles.push_back ( ms );
				return true;
			}
			delete ms;
		}
		return false;
	}

	// check its a valid file
	if ( CBaseFile::CheckFile ( file ) == SPKFILE_MULTI )
	{
		CMultiSpkFile spkfile;
		if ( spkfile.ReadFile ( file, false ) )
		{
			CLinkList<SMultiSpkFile> *list = spkfile.GetFileList();
			for ( SMultiSpkFile *it = list->First(); it; it = list->Next() )
			{
				SMultiSpkFile *ms = new SMultiSpkFile;
				ms->sName = it->sName;
				ms->sData = NULL;
				if ( spkfile.ExtractData ( ms ) )
				{
					ms->bOn = on;
					m_lFiles.push_back ( ms );
				}
				else
					delete ms;
			}
			return true;
		}
		return false;
	}
	else if ( CBaseFile::CheckFile ( file ) != SPKFILE_SINGLE )
		return false;

	FILE *id = fopen ( file.c_str(), "rb" );
	if ( !id )
		return false;

	// create entry
	SMultiSpkFile *ms = new SMultiSpkFile;
	ms->sName = file;
	ms->sName = ms->sName.FindReplace ( "\\", "/" );
	ms->sName = ms->sName.GetToken ( ms->sName.NumToken('/'), '/' );
	ms->bOn = on;

	// find file size
	fseek ( id, 0, SEEK_END );
	ms->lSize = ftell ( id );
	fseek ( id, 0, SEEK_SET );

	// read data
	ms->sData = new char[ms->lSize];
	fread ( ms->sData, sizeof(char), ms->lSize, id );

	fclose ( id );

	CSpkFile spkfile;
	if ( spkfile.ReadFile ( file, SPKREAD_VALUES ) )
	{
		ms->sScriptAuthor	= spkfile.GetAuthor();
		ms->sScriptName		= spkfile.GetName();
		ms->sScriptVersion	= spkfile.GetVersion();
	}

	// find if theres any files with the same filename
	SMultiSpkFile *it;
	for ( it = m_lFiles.First(); it; it = m_lFiles.Next() )
	{
		if ( it->sName.ToLower() == ms->sName.ToLower() )
		{
			m_lFiles.remove ( it );
			delete it;
			break;
		}
	}

	// now find if theres a file with the same script/author
	for ( it = m_lFiles.First(); it; it = m_lFiles.Next() )
	{
		if ( (it->sScriptAuthor.ToLower() == ms->sScriptAuthor.ToLower()) && (it->sScriptName.ToLower() == ms->sScriptName.ToLower()) )
		{
			m_lFiles.remove ( it );
			delete it;
			break;
		}
	}

	m_lFiles.push_back ( ms );

	return true;
}

SMultiSpkFile *CMultiSpkFile::AddFileEntry ( String filename )
{
	SMultiSpkFile *ms = new SMultiSpkFile;
	ms->sName = filename;
	ms->bOn = true;

	m_lFiles.push_back ( ms );

	return ms;
}

String CMultiSpkFile::CreateData ()
{
	String ret;
	if ( !m_sName.Empty() )
		ret = String("Name: ") + m_sName + "\n";
	for ( SMultiSpkFile *ms = m_lFiles.First(); ms; ms = m_lFiles.Next() )
	{
		ret += "SpkFile: ";
		if ( ms->bOn )
			ret += "1:";
		else
			ret += "0:";
		ret += ms->lSize;
		ret += ":";
		ret += ms->sName;
		if ( !ms->sScriptName.Empty() )
			ret += (String(":") + ms->sScriptName + "|" + ms->sScriptAuthor + "|" + ms->sScriptVersion);
		ret += "\n";
	}

	return ret;
}

bool CMultiSpkFile::WriteFile ( String filename )
{
	FILE *id = fopen ( filename.c_str(), "wb" );
	if ( !id )
		return false;

	String data = CreateData ();

	int comprLen = data.Length(), uncomprLen = comprLen;;
	unsigned char *compr = NULL;

	bool compressed = false;
	int valueheader = m_SHeader.iCompression;
#ifndef _WIN32
	if ( valueheader == SPKCOMPRESS_7ZIP )
		valueheader = SPKCOMPRESS_ZLIB;
#endif
	if ( valueheader == SPKCOMPRESS_ZLIB )
	{
		comprLen = uncomprLen;	
		if ( comprLen < 100 )
			comprLen = 200;
		else if ( comprLen < 1000 )
			comprLen *= 2;

		compr = (unsigned char *)calloc((unsigned int)comprLen, 1);
		int err = compress ( (unsigned char *)compr, (unsigned long *)&comprLen, (const unsigned char *)data.c_str(), data.Length() );
		if ( err == Z_OK )
			compressed = true;
	}
#ifdef _WIN32
	else if ( valueheader == SPKCOMPRESS_7ZIP )
	{
		long len = data.Length();
		compr = LZMAEncodeData ( (unsigned char *)data.c_str(), len, len );
		if ( compr )
		{
			comprLen = len;
			compressed = true;
		}
	}
#endif
	if ( !compressed )
	{
		comprLen = uncomprLen;
		compr = (unsigned char *)calloc((unsigned int)comprLen, 1);
		memcpy ( compr, data.c_str(), comprLen );
		valueheader = SPKCOMPRESS_NONE;
	}

	// write the main header to the file
	fprintf ( id, "MSPKCycrow;%.2f;%d;%d;%d;%d\n", FILEVERSION, valueheader, data.Length(), comprLen, (m_SHeader.bSelection) ? 1 : 0 );
	if ( ferror(id) )
	{
		fclose ( id );
		return false;
	}

	// write the compressed data to file
	fputc ( (unsigned char)(uncomprLen >> 24), id ); 
	fputc ( (unsigned char)(uncomprLen >> 16), id ); 
	fputc ( (unsigned char)(uncomprLen >> 8), id ); 
	fputc ( (unsigned char)uncomprLen, id ); 
	fwrite ( compr, sizeof(char), comprLen, id );

	free ( compr );

	for ( SMultiSpkFile *ms = m_lFiles.First(); ms; ms = m_lFiles.Next() )
	{
		if ( (!ms->sData) && (ms->pFile) )
			ms->pFile->WriteData ( id, NULL );
		else
			fwrite ( ms->sData, sizeof(char), ms->lSize, id );
	}

	fclose ( id );

	return true;
}

bool CMultiSpkFile::ParseHeader ( String header )
{
	if ( header.GetToken ( 1, ';' ) != "MSPKCycrow" )
		return false;

	m_SHeader.fVersion = header.GetToken ( 2, ';' ).ToFloat();
	if ( m_SHeader.fVersion > FILEVERSION )
		return false;

	m_SHeader.iCompression = header.GetToken ( 3, ';' ).ToInt();
	m_SHeader.lUncomprLen = header.GetToken ( 4, ';' ).ToLong();
	m_SHeader.lComprLen = header.GetToken ( 5, ';' ).ToLong();
	m_SHeader.bSelection = (header.GetToken ( 4, ';' ).ToInt()) ? true : false;

	return true;
}

bool CMultiSpkFile::ParseValueLine ( String line )
{
	String first = line.GetToken ( 1, ' ' );
	String rest  = line.GetToken ( 2, -1, ' ' );

	if ( first == "Name:" )
		m_sName = rest;
	else if ( first == "SpkFile:" )
	{
		SMultiSpkFile *ms = new SMultiSpkFile;
		ms->bOn = (rest.GetToken ( 1, ':' ).ToInt()) ? true : false;
		ms->lSize = rest.GetToken ( 2, ':' ).ToLong();
		ms->sName = rest.GetToken ( 3, ':' );
		ms->sData = NULL;
		String r = rest.GetToken ( 4, -1, ':' );
		if ( !r.Empty() )
		{
			ms->sScriptName = r.GetToken ( 1, '|' );
			ms->sScriptAuthor = r.GetToken ( 2, '|' );
			ms->sScriptVersion = r.GetToken ( 3, -1, '|' );
		}
		m_lFiles.push_back ( ms );
	}
	else
		return false;

	return true;
}

void CMultiSpkFile::ReadValues ( String values )
{
	int num = 0;
	String *lines = values.SplitToken ( '\n', &num );

	for ( int i = 0; i < num; i++ )
		ParseValueLine ( lines[i] );
}

bool CMultiSpkFile::ReadFile ( String filename, bool readdata )
{
	FILE *id = fopen ( filename.c_str(), "rb" );
	if ( !id )
		return false;

	// first read the header
	if ( !ParseHeader ( CSpkFile::GetEndOfLine ( id, NULL, false ) ) )
		return false;

	ClearFiles ();

	int doneLen = 0;
	// next read the data values for the spk files
	if ( m_SHeader.lComprLen )
	{
		// read data to memory
		unsigned char *readData = new unsigned char[m_SHeader.lComprLen + 1];
		unsigned char size[4];
		fread ( size, 4, 1, id );
		fread ( readData, sizeof(unsigned char), m_SHeader.lComprLen, id );
		unsigned long uncomprLen = (size[0] << 24) + (size[1] << 16) + (size[2] << 8) + size[3];

		// check for zlib compression
		if ( m_SHeader.iCompression == SPKCOMPRESS_ZLIB )
		{
			// uncomress the data
			unsigned char *uncompr = new unsigned char[uncomprLen];
			
			int err = uncompress ( uncompr, &uncomprLen, readData, m_SHeader.lComprLen );
			if ( err == Z_OK )
				ReadValues ( String ((char *)uncompr) );
			doneLen = uncomprLen;
			delete uncompr;
		}
		else if ( m_SHeader.iCompression == SPKCOMPRESS_7ZIP )
		{
			long len = uncomprLen;
			#ifdef _WIN32
			unsigned char *compr = LZMADecodeData ( readData, m_SHeader.lComprLen, len );
			#else
			unsigned char *compr = LZMADecode_C ( readData, m_SHeader.lComprLen, (size_t *)&len, NULL );
			#endif
			if ( compr )
				ReadValues ( String ((char *)compr) );
		}
		// no compression
		else
			ReadValues ( String ((char *)readData) );

		delete readData;
	}

	if ( readdata )
	{
		for ( SMultiSpkFile *ms = m_lFiles.First(); ms; ms = m_lFiles.Next() )
		{
			ms->sData = new char[ms->lSize];
			fread ( ms->sData, sizeof(char), ms->lSize, id );
		}
	}

	m_sFilename = filename;

	fclose ( id );

	return true;
}

void CMultiSpkFile::ClearFiles ()
{
	for ( SMultiSpkFile *ms = m_lFiles.First(); ms; ms = m_lFiles.Next() )
		delete ms;
	m_lFiles.clear();
}

bool CMultiSpkFile::ExtractData ( SMultiSpkFile *ms )
{
	FILE *id = fopen ( m_sFilename.c_str(), "rb" );
	if ( !id )
		return false;

	// skip past the header
	CSpkFile::GetEndOfLine ( id, NULL, false );
	// seek past the values
	fseek ( id, 4, SEEK_CUR );
	fseek ( id, m_SHeader.lComprLen, SEEK_CUR );

	bool found = false;
	for ( SMultiSpkFile *it = m_lFiles.First(); it; it = m_lFiles.Next() )
	{
		if ( ms->sName.ToLower() == it->sName.ToLower() )
		{
			ms->lSize = it->lSize;
			ms->sScriptAuthor = it->sScriptAuthor;
			ms->sScriptName = it->sScriptName;
			ms->sScriptVersion = it->sScriptVersion;
			ms->bOn = it->bOn;
			ms->sData = new char[ms->lSize];
			if ( it->sData )
				memcpy ( ms->sData, it->sData, ms->lSize );
			else
				fread ( ms->sData, sizeof(char), ms->lSize, id );
			found = true;
			break;
		}
		else
			fseek ( id, it->lSize, SEEK_CUR );
	}

	fclose ( id );
	return found;
}

bool CMultiSpkFile::ReadFileToMemory ( SMultiSpkFile *ms )
{
	if ( ms->sData )
		return true;

	FILE *id = fopen ( m_sFilename.c_str(), "rb" );
	if ( !id )
		return false;

	// skip past the header
	CSpkFile::GetEndOfLine ( id, NULL, false );
	// seek past the values
	fseek ( id, 4, SEEK_CUR );
	fseek ( id, m_SHeader.lComprLen, SEEK_CUR );

	bool found = false;
	for ( SMultiSpkFile *it = m_lFiles.First(); it; it = m_lFiles.Next() )
	{
		if ( it == ms )
		{
			ms->sData = new char[ms->lSize];
			fread ( ms->sData, sizeof(char), ms->lSize, id );
			found = true;
			break;
		}
		else
			fseek ( id, it->lSize, SEEK_CUR );
	}

	fclose ( id );
	return found;
}

bool CMultiSpkFile::ExtractFile ( SMultiSpkFile *ms, String dir )
{
	if ( !ms->sData )
	{
		if ( !ReadFileToMemory ( ms ) )
			return false;
	}

	String filename = dir;
	if ( !dir.Empty() )
		filename += "/";
	filename += ms->sName;

	FILE *id = fopen ( filename.c_str(), "wb" );
	if ( !id )
		return false;

	fwrite ( ms->sData, sizeof(char), ms->lSize, id );
	fclose ( id );

	return true;
}

bool CMultiSpkFile::ExtractAll ( String dir )
{
	if ( m_sFilename.Empty() )
		return false;

	if ( !ReadAllFilesToMemory () )
		return false;

	for ( SMultiSpkFile *ms = m_lFiles.First(); ms; ms = m_lFiles.Next() )
	{
		if ( !ms->sData )
			continue;

		FILE *id = fopen ( String(dir + "/" + ms->sName).c_str(), "wb" );
		if ( !id )
			continue;

		fwrite ( ms->sData, sizeof(char), ms->lSize, id );
		fclose ( id );
	}
	return true;
}

bool CMultiSpkFile::SplitMulti ( String filename, String destdir )
{
	if ( !ReadFile ( filename ) )
		return false;

	bool doneone = false;
	for ( SMultiSpkFile *ms = m_lFiles.First(); ms; ms = m_lFiles.Next() )
	{
		if ( !ms->sData )
			continue;

		String destfile = destdir;
		if ( !destfile.Empty() )
			destfile += "/";
		destfile += ms->sName;
		FILE *id = fopen ( destfile.c_str(), "wb" );
		if ( !id )
			continue;

		fwrite ( ms->sData, sizeof(char), ms->lSize, id );
		fclose ( id );
		doneone = true;
	}

	return doneone;
}

bool CMultiSpkFile::ReadAllFilesToMemory ()
{
	// no file to read from
	if ( m_sFilename.Empty() )
		return false;

	// now open the file
	FILE *id = fopen ( m_sFilename.c_str(), "rb" );
	if ( !id )
		return false;

	// read the header
	CSpkFile::GetEndOfLine ( id, NULL, false );
	// skip past values
	fseek ( id, 4, SEEK_CUR );
	fseek ( id, m_SHeader.lComprLen, SEEK_CUR );

	for ( SMultiSpkFile *ms = m_lFiles.First(); ms; ms = m_lFiles.Next() )
	{
		if ( !ms->sData )
		{
			ms->sData = new char[ms->lSize];
			fread ( ms->sData, sizeof(char), ms->lSize, id );
		}
		else
			fseek ( id, ms->lSize, SEEK_CUR );
	}

	return true;
}

bool CMultiSpkFile::RemoveFile ( SMultiSpkFile *ms )
{
	int num = 0;
	for ( SMultiSpkFile *it = m_lFiles.First(); it; it = m_lFiles.Next(), num++ )
	{
		if ( it == ms )
			return RemoveFile ( num );
	}
	return false;
}

bool CMultiSpkFile::RemoveFile ( int id )
{
	if ( (id < 0) || (id >= m_lFiles.size()) )
		return false;

	SMultiSpkFile *file = m_lFiles.Get ( id );
	m_lFiles.erase ( id + 1 );

	if ( file )
		delete file;

	return true;
}

bool CMultiSpkFile::ReadSpk( SMultiSpkFile *ms, int type )
{
	// no file to read from
	if ( m_sFilename.Empty() )
		return false;

	// now open the file
	FILE *id = fopen ( m_sFilename.c_str(), "rb" );
	if ( !id )
		return false;

	// read the header
	CSpkFile::GetEndOfLine ( id, NULL, false );
	// skip past values
	fseek ( id, 4, SEEK_CUR );
	fseek ( id, m_SHeader.lComprLen, SEEK_CUR );

	for ( SMultiSpkFile *it = m_lFiles.First(); it; it = m_lFiles.Next() )
	{
		if ( it == ms )
		{
			if ( !ms->pFile )
				ms->pFile = new CSpkFile;
			ms->pFile->ReadFile ( id, type, NULL );
			break;
		}
		else
			fseek ( id, ms->lSize, SEEK_CUR );
	}

	fclose ( id );

	return true;
}

bool CMultiSpkFile::ReadAllSpk( int type )
{
	// no file to read from
	if ( m_sFilename.Empty() )
		return false;

	// now open the file
	FILE *id = fopen ( m_sFilename.c_str(), "rb" );
	if ( !id )
		return false;

	// read the header
	CSpkFile::GetEndOfLine ( id, NULL, false );
	// skip past values
	fseek ( id, 4, SEEK_CUR );
	fseek ( id, m_SHeader.lComprLen, SEEK_CUR );

	for ( SMultiSpkFile *ms = m_lFiles.First(); ms; ms = m_lFiles.Next() )
	{
		if ( ms->pFile )
		{
			fseek ( id, ms->lSize, SEEK_CUR );
			continue;
		}

		long tell = ftell ( id );

		ms->pFile = new CSpkFile;
		ms->pFile->ReadFile ( id, type, NULL );

		// move to correct position in file for next stream of data
		// should be fine, this is more of a failsafe
		rewind ( id );
		fseek ( id, tell, SEEK_CUR );
		fseek ( id, ms->lSize, SEEK_CUR );
	}

	fclose ( id );

	return true;
}

SMultiSpkFile *CMultiSpkFile::FindFile ( String name )
{
	for ( SMultiSpkFile *ms = m_lFiles.First(); ms; ms = m_lFiles.Next() )
	{
		if ( ms->sName.ToLower() == name.ToLower() )
			return ms;
	}
	return NULL;
}
