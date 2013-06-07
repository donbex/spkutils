// File.cpp: implementation of the CFile class.
//
//////////////////////////////////////////////////////////////////////

#include "File.h"

#include <time.h>

#ifndef _WIN32
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#endif

#include <iostream>
#include <fstream>

#include "SpkFile.h"
#include "CatFile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef _WIN32
bool g_IsNT = false;
static inline bool IsItWindowsNT()
{
  OSVERSIONINFO versionInfo;
  versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
  if (!::GetVersionEx(&versionInfo)) 
    return false;
  return (versionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
}
#endif

int CFile::m_iTempNum = 0;

CFile::CFile()
{
	m_bUsedMalloc = false;
	m_sData = NULL;
	Reset();
}

CFile::CFile ( String filename )
{
	m_bUsedMalloc = false;
	m_sData = NULL;
	Reset ();
	SetFilename ( filename );
}

CFile::CFile ( const char *filename )
{
	m_bUsedMalloc = false;
	m_sData = NULL;
	Reset ();
	SetFilename ( String(filename) );
}

CFile::~CFile()
{
	DeleteData ();

	if ( !m_sTmpFile.Empty() )
		remove ( m_sTmpFile.c_str() );
}


/*
	Func:	GetDirectory()
	Return: Directory String
	Desc:	Returns the directory the file goes into, based on m_sDir and Filetype
*/
String CFile::GetDirectory ( CBaseFile *file )
{
	if ( IsFakePatch() )
		return "";

	if ( (m_iFileType == FILETYPE_MOD) && (m_sDir == "Patch") )
		return "Mods/Patch";

	if ( (!m_sDir.Empty()) && (m_iFileType != FILETYPE_README) )
	{
		String dir = m_sDir.FindReplace ( "\\", "/" );
		if ( file )
		{
			dir = dir.FindReplace ( "$scriptname", file->GetNameValidFile() );
			dir = dir.FindReplace ( "$scriptauthor", file->GetAuthor() );
		}
		return dir;
	}

	switch ( m_iFileType )
	{
		case FILETYPE_SCRIPT:
			return "Scripts";
		case FILETYPE_TEXT:
			return "T";
		case FILETYPE_README:
		{
			if ( file )
				return String("Readme/") + file->GetNameValidFile();
			return "Readme";
		}
		case FILETYPE_MAP:
			return "Maps";
		case FILETYPE_MOD:
//			if ( (file) && (file->IsPatch()) )
//				return "Mods/Patch";
			return "Mods";
		case FILETYPE_UNINSTALL:
			return "Uninstall";
		case FILETYPE_SOUND:
			return "Soundtrack";
		case FILETYPE_EXTRA:
			return "Extras";
		case FILETYPE_SCREEN:
			return "loadscr";
		case FILETYPE_MISSION:
			return "Director";
	}
	return NullString;
}

String CFile::GetNameDirectory ( CBaseFile *file ) 
{ 
	String dir = GetDirectory( file );  
	if ( !dir.Empty() ) 
		dir += "/"; 
	return String(dir + m_sName).FindReplace ( "\\", "/" ); 
}

/*
	Func:	Reset()
	Desc:	Resets the file data, clears all data inside
			Clears the Data stream
*/
void CFile::Reset ()
{
	m_iLastError = SPKERR_NONE;
	m_bLoaded = false;
	m_iFileType = -1;
	m_lSize = 0;
	m_iVersion = 0; 
	m_bSigned = false;
	m_iUsed = 0; 
	m_tTime = 0; 
	m_lDataSize = m_lUncomprDataSize = 0; 
	DeleteData ();
	m_iDataCompression = 0;
	m_bSkip = false;
	m_bShared = false;
	m_bCompressedToFile = false;
}


/*
	Func:	DeleteData()
	Desc:	Clears the data stream, uses free() if created by malloc, otherwise use delete.
*/
void CFile::DeleteData ()
{
	if ( m_sData )
	{
		if ( m_bUsedMalloc )
			free ( m_sData );
		else
			delete m_sData;
		m_sData = NULL;
		m_bLoaded = false;
		m_lDataSize = 0;
	}
	m_bUsedMalloc = false;
	m_iDataCompression = SPKCOMPRESS_NONE;
}
 
bool CFile::IsFakePatch ()
{
	if ( m_iFileType != FILETYPE_MOD )
		return false;

	if ( m_sName.GetToken ( 1, '.' ).ToInt() )
		return true;

	if ( m_sName.Left (10) == "FakePatch_" )
		return true;

	return false;
}

/*
########################################################################################################################
####################################             File Pointer Functions             ####################################
########################################################################################################################
*/

/*
	Func:	SetFilename
	Accept:	filename - String for the filename of disk
	Desc:	Sets the file pointer
			Reads the file size and last modifed time to store in the class
			Splits up the filename and dir path
*/
void CFile::SetFilename ( String filename )
{
	String file = filename.FindReplace ( "\\", "/" ).FindReplace ( "//", "/" );
	int tok = file.NumToken ( '/' );

	m_sFullDir = file.GetToken ( 1, tok - 1, '/' );
	m_sName = file.GetToken ( tok, '/' );

	ReadFileSize ();

	ReadLastModified ();
}


/*
	Func:	ReadFromFile
	Return:	Boolean - Returns true if read was successfull
	Desc:	Reads data from file pointer into data stream
			As its read from a file, there will be no compression, so its set to None
*/
bool CFile::ReadFromFile ()
{
	FILE *id = fopen ( GetFilePointer().c_str(), "rb" );
	if ( !id )
	{
		m_iLastError = SPKERR_FILEOPEN;
		return false;
	}

	if ( !m_lSize )
	{
		fseek ( id, 0, SEEK_END );
		m_lSize = ftell ( id );
		rewind ( id );
	}

	m_iDataCompression = SPKCOMPRESS_NONE;
	m_lDataSize = m_lUncomprDataSize = m_lSize;

	DeleteData ();
	
	m_sData = new unsigned char[m_lSize];
	if ( !m_sData ) { fclose ( id ); m_iLastError = SPKERR_MALLOC; return false; }

	fread ( m_sData, sizeof(unsigned char), m_lSize, id );
	if ( ferror(id) )
	{
		m_iLastError = SPKERR_FILEREAD;
		DeleteData ();
		m_lDataSize = 0;
		fclose ( id );
		return false;
	}

	m_iLastError = SPKERR_NONE;

	m_bLoaded = true;

	fclose ( id );

	return true;
}

/*
	Func:	ReadFromData
	Accept:	data - The data stream to read
			size - The length of the data stream
	Return: Boolean - Return true if successfull
	Desc:	Copys data to the data stream in the file
			Used when data is already loaded into memory
*/
bool CFile::ReadFromData ( char *data, long size )
{
	DeleteData ();

	m_lDataSize = size ;
	m_sData = new unsigned char[m_lDataSize];

	memcpy ( m_sData, data, size );

	return true;
}


/*
	Func:	ReadFromFile
	Accept:	id		- File Pointer Stream of open file
			size	- amount of data to read from file
			dosize	- Read the 4 character size
	Func:	Reads a data stream from a currently open file
			Can be used to read directly from a SPK Package
			dosize will read the initial 4 character uncompressed size if needed
*/
bool CFile::ReadFromFile ( FILE *id, long size, bool dosize )
{
	// remove data

	m_lDataSize = size ;
	m_sData = new unsigned char[m_lDataSize];
	if ( dosize )
	{
		unsigned char s[4];
		fread ( s, sizeof(unsigned char), 4, id );
	}

	fread ( m_sData, sizeof(unsigned char), m_lDataSize, id );

	if ( ferror (id) )
	{
		DeleteData ();
		m_lDataSize = 0;
		return false;
	}

	return true;
}



/*
	Func:	GetFilePointer
	Desc:	Returns the file pointer name
			Joins dir and name together
			Works for relative paths as well
*/
String CFile::GetFilePointer ()
{
	String fullfile = m_sFullDir;
	if ( !fullfile.Empty() )
		fullfile += "/";

	if ( !m_sName.Empty() )
		fullfile += m_sName;

	return fullfile;
}


/*
	Func:	ReadFileSize()
	Return:	Returns the file size read
	Desc:	Opens the file and seeks to the end
*/
long CFile::ReadFileSize ()
{

	FILE *id = fopen ( GetFilePointer().c_str(), "rb" );
	if ( id )
	{
		fseek ( id, 0, SEEK_END );
		m_lSize = ftell ( id );
		fclose ( id );
	}

	m_lUncomprDataSize = m_lSize;

	return m_lSize;
}

/*
	Func:	ReadLastModifed()
	Desc:	Reads the last modified time of the file and returns
			Uses seperate rountines for Windows and Linux
*/
time_t CFile::ReadLastModified ()
{
	String file = GetFilePointer();
	if ( file.Empty() )
		return m_tTime;

	#ifndef _WIN32
	struct stat attrib;			// create a file attribute structure
    stat ( file.c_str(), &attrib);	
	m_tTime = attrib.st_mtime;
	#else
	#endif

	return m_tTime;
}

bool CFile::CheckValidFilePointer ()
{
	String filename = GetFilePointer();
	if ( filename.Empty() )
		return false;

	FILE *id = fopen ( filename.c_str(), "rb+" );
	if ( !id )
		return false;

	fclose ( id );
	return true;
}


int CFile::ReadScriptVersion ()
{
	if ( (m_iFileType != FILETYPE_SCRIPT) && (m_iFileType != FILETYPE_UNINSTALL) )
		return 0;

	// check file pointer
	String file = GetFilePointer();
	// check file extenstion
	if ( (file.GetToken ( file.NumToken('.'), '.' ).ToLower() == "pck" ) && (CheckPCK()) )
	{
		size_t size = 0;
		unsigned char *data = UnPCKFile ( &size );
		if ( (data) && (size) )
			m_iVersion = ::ReadScriptVersionFromData ( data, size );
	}
	else if ( (m_sData) && (m_iDataCompression == SPKCOMPRESS_NONE) )
		m_iVersion = ::ReadScriptVersionFromData ( m_sData, m_lDataSize );
	else 
	{
		FILE *id = fopen ( file.c_str(), "rb+" );
		if ( id )
		{
			fclose ( id );
			std::string line;
			std::ifstream myfile ( file.c_str() );
			if ( myfile.is_open() )
			{
				bool inscript = false;
				while (! myfile.eof() )
				{
					std::getline ( myfile, line );
					while ( line[0] == ' ' )
						line.erase ( 0, 1 );
					String sLine = line;
					if ( !inscript )
					{
						if ( sLine.GetToken ( 1, '>' ) == "<script" )
							inscript = true;
					}
					else
					{
						if ( sLine.GetToken ( 1, '>' ) == "<version" )
						{
							m_iVersion = sLine.GetToken ( 2, '>' ).GetToken ( 1, '<' ).ToInt();
							break;
						}
					}
				}
				myfile.close();
			}
		}
	}

	return m_iVersion;
}

bool CFile::MatchFile ( CFile *file )
{
	if ( file->GetFileType() != m_iFileType )
		return false;

	if ( file->GetDir() != m_sDir )
		return false;

	if ( file->GetName() != m_sName )
		return false;

	return true;
}

/*
########################################################################################################################
####################################              Compression Functions             ####################################
########################################################################################################################
*/

bool CFile::CompressFile ( CProgressInfo *progress )
{
#ifdef _WIN32
	// check for file
	if ( m_sName.Empty() )
		return false;
	// check u can read file
	String file = m_sFullDir + "/" + m_sName;
	FILE *id = fopen ( file.c_str(), "rb" );
	if ( !id )
		return false;

	fclose ( id );

	bool done = false;
	if ( LZMAEncodeFile ( file.c_str(), "encodedfile.tmp", (CProgressInfo7Zip*)progress ) )
	{
		id = fopen ( "encodedfile.tmp", "rb" );
		if ( id )
		{
			fseek ( id, 0, SEEK_END );
			m_lDataSize = ftell ( id );
			rewind ( id );

			DeleteData ();
			m_sData = new unsigned char[m_lDataSize];
			if ( !m_sData ) m_iLastError = SPKERR_MALLOC;
			else
			{
				fread ( m_sData, sizeof(unsigned char), m_lDataSize, id );
				if ( ferror(id) )
				{
					m_iLastError = SPKERR_FILEREAD;
					DeleteData ();
				}
				else
				{
					done = true;
					m_iDataCompression = SPKCOMPRESS_7ZIP;
					m_bCompressedToFile = true;
				}
			}
			fclose ( id );
		}
	}

	remove ( "encodedfile.tmp" );

	return done;
#else
	return false;
#endif
}

bool CFile::CompressData ( int compressionType, CProgressInfo *progress )
{
	// no data to try to compress
	if ( (!m_sData) || (!m_lDataSize) )
		return false;

	// if comopression is set to noe, dont bother
	if ( compressionType == SPKCOMPRESS_NONE )
		return true;

	// if its already compressed, no need to compress again
	if ( compressionType == m_iDataCompression )
		return true;

	m_lUncomprDataSize = m_lDataSize;
	
#ifndef _WIN32
	if ( compressionType == SPKCOMPRESS_7ZIP )
		compressionType = SPKCOMPRESS_ZLIB;
#endif

	if ( compressionType == SPKCOMPRESS_ZLIB )
	{
		unsigned long comprLen = m_lDataSize;;
		if ( comprLen < 100 )
			comprLen = 200;
		else if ( comprLen < 1000 )
			comprLen *= 2;

		unsigned char *compr = new unsigned char[comprLen];

		int err = compress ( compr, &comprLen, (const unsigned char *)m_sData, m_lDataSize );

		// if its compressed ok, remove old data and use new one
		if ( err == Z_OK )
		{
			DeleteData ();
			m_sData = compr;
			m_bUsedMalloc = true;
			m_lDataSize = comprLen;
			m_iDataCompression = compressionType;
			return true;
		}
	}
#ifdef _WIN32
	else if ( compressionType == SPKCOMPRESS_7ZIP )
	{
		long comprLen = m_lDataSize;;
		unsigned char *compr = (unsigned char *)LZMAEncodeData ( m_sData, m_lDataSize, comprLen, (CProgressInfo7Zip *)progress );
		if ( compr )
		{
			DeleteData ();
			m_sData = compr;
			m_lDataSize = comprLen;
			m_iDataCompression = compressionType;

			return true;
		}
	}
#endif

	return false;
}

bool CFile::UncompressData ( CProgressInfo *progress )
{
	// no data to try to uncompress
	if ( (!m_sData) || (!m_lDataSize) )
		return false;

	if ( m_bCompressedToFile )
		return false;

	// if comopression is set to none, dont bother
	if ( m_iDataCompression == SPKCOMPRESS_NONE )
		return true;

	if ( m_iDataCompression == SPKCOMPRESS_ZLIB )
	{
		unsigned long uncomprLen = m_lUncomprDataSize;
		unsigned char *uncompr = new unsigned char[m_lUncomprDataSize];
		int err = uncompress ( uncompr, &uncomprLen, (const unsigned char *)m_sData, m_lDataSize );
		if ( err == Z_OK )
		{
			DeleteData ();
			m_iDataCompression = SPKCOMPRESS_NONE;
			m_lDataSize = uncomprLen;
			m_sData = uncompr;
			return true;
		}
	}
	if ( m_iDataCompression == SPKCOMPRESS_7ZIP )
	{
		long len = m_lUncomprDataSize;
		
		#ifdef _WIN32
		unsigned char *compr = LZMADecodeData ( m_sData, m_lDataSize, len, (CProgressInfo7Zip *)progress );
		#else
		unsigned char *compr = LZMADecode_C ( (unsigned char *)m_sData, m_lDataSize, (size_t*)&len, NULL );
		#endif

		if ( compr )
		{
			DeleteData ();
			m_sData = compr;
			m_lDataSize = len;
			m_iDataCompression = SPKCOMPRESS_NONE;
			return true;
		}
	}

	return false;
}

unsigned char *CFile::UncompressData ( long *size, CProgressInfo *progress )
{
	// no data to try to uncompress
	if ( (!m_sData) || (!m_lDataSize) )
		return NULL;

	if ( m_bCompressedToFile )
		return NULL;

	// if comopression is set to none, dont bother
	if ( m_iDataCompression == SPKCOMPRESS_NONE )
	{
		*size = m_lDataSize;
		return m_sData;
	}

	if ( m_iDataCompression == SPKCOMPRESS_ZLIB )
	{
		unsigned long uncomprLen = m_lUncomprDataSize;
		unsigned char *uncompr = new unsigned char[m_lUncomprDataSize];
		int err = uncompress ( uncompr, &uncomprLen, (const unsigned char *)m_sData, m_lDataSize );
		if ( err == Z_OK )
		{
			*size = uncomprLen;
			return uncompr;
		}
	}
	if ( m_iDataCompression == SPKCOMPRESS_7ZIP )
	{
		long len = m_lUncomprDataSize;

		#ifdef _WIN32
		unsigned char *compr = LZMADecodeData ( m_sData, m_lDataSize, len, (CProgressInfo7Zip *)progress );
		#else
		unsigned char *compr = LZMADecode_C ( m_sData, m_lDataSize, (size_t *)&len, NULL );
		#endif

		if ( compr )
		{
			*size = len;
			return compr;
		}
	}

	return NULL;
}

bool CFile::UncompressToFile ( String toFile, CBaseFile *spkfile, bool includedir, CProgressInfo *progress )
{
#ifdef _WIN32
	if ( (!m_sData) || (!m_lDataSize) )
		return false;
	// if theres a tmp file, open it and check it still exists
	if ( !m_sTmpFile.Empty() )
	{
		FILE *id = fopen ( m_sTmpFile.c_str(), "rb" );
		if ( id )
		{
			fclose ( id );
			return true;
		}
		m_sTmpFile = "";
	}

	// now uncompress to the file
	String file = toFile;
	if ( file.Empty() )
	{
		m_iTempNum++;
		file = String("uncompr") + (long)m_iTempNum + ".tmp";
	}
	else
		file = GetFullFileToDir ( file, includedir, spkfile );

	FILE *id = fopen ( "compr.tmp", "wb" );
	if ( !id )
		return false;

	fwrite ( m_sData, sizeof(unsigned char), m_lDataSize, id );
	bool ret = false;
	int err = ferror(id);
	fclose ( id );
	if ( !err )
	{
		if ( LZMADecodeFile ( "compr.tmp", file.c_str(), (CProgressInfo7Zip *)progress ) )
		{
			ret = true;
			if ( toFile.Empty() )
				m_sTmpFile = file;
		}
	}

	remove ( "compr.tmp" );

	return ret;
#else
	return false;
#endif
}


bool CFile::WriteFilePointer ( unsigned char *cData, long len )
{
	return WriteToFile ( GetFilePointer(), cData, len );
}

bool CFile::WriteToFile ( String filename, unsigned char *cData, long len )
{
	unsigned char *data = cData;
	if ( (!len) || (!data) )
	{
		len = m_lDataSize;
		data = m_sData;
	}

	if ( (!len) || (!data) )
		return false;

	// check for cat file
	if ( filename.IsIn ( "::" ) )
	{
		String catfile = filename.GetToken ( "::", 1, 1 );
		String file = filename.GetToken ( "::", 2, 2 );

		CCatFile newcat;
		return newcat.AddData ( catfile, data, len, file, true, true );
	}
	else
	{
		String filen = filename.FindReplace ( "\\", "/" );

		FILE *id = fopen ( filename.c_str(), "wb" );
		if ( !id )
			return false;

		fwrite ( data, sizeof(unsigned char), len, id );

		bool ret = true;
		if ( ferror(id) )
			ret = false;

		fclose ( id );

		return ret;
	}

	return false;
}

String CFile::GetFullFileToDir ( String dir, bool includedir, CBaseFile *file )
{
	String fullfile = dir;
	if ( includedir )
	{
		String d = GetDirectory ( file );
		if ( !d.Empty() )
		{
			if ( !fullfile.Empty() )
				fullfile += "/";
			fullfile += d;
		}
	}
	if ( !fullfile.Empty() )
		fullfile += "/";
	fullfile += m_sName;

	fullfile = fullfile.FindReplace ( "\\", "/" );
	return fullfile;
}

bool CFile::WriteToDir ( String &dir, CBaseFile *spkfile, bool includedir, String appendDir, unsigned char *data, long len )
{
	String fullfile = GetFullFileToDir ( dir, includedir, spkfile );

	if ( !appendDir.Empty() )
	{
		if ( !fullfile.Empty() )
			fullfile += "/";
		fullfile += appendDir;
	}

	String fulldir = fullfile.GetToken ( 1, fullfile.NumToken('/') - 2, '/' );
	if ( !fulldir.Empty() )
		CreateDirectory ( fulldir );

	return WriteToFile ( fullfile, data, len );
}

String CFile::GetDataSizeString ()
{
	return GetSizeString ( m_lDataSize );
}
String CFile::GetUncompressedSizeString ()
{
	return GetSizeString ( GetUncompressedDataSize() );
}

String CFile::GetCreationTimeString ()
{
	if ( !m_tTime )
		return NullString;

	struct tm   *currDate;
	char    dateString[100];

	time_t n = m_tTime;

	currDate = localtime(&n);

	strftime(dateString, sizeof dateString, "(%d/%m/%Y) %H:%M", currDate);


	return String(dateString);
}

bool CFile::CompareNew ( CFile *file )
{
	if ( !m_iVersion )
		ReadScriptVersion ();

	// if version, check if its later version
	if ( (m_iVersion) && (file->GetVersion()) )
	{
		if ( m_iVersion > file->GetVersion() )
			return false;
	}

	// now check for last modified time
	if ( (m_tTime) && (file->GetLastModified()) )
	{
		if ( m_tTime > file->GetLastModified() )
			return false;
	}

	// assume same or newer
	return true;
}

String CFile::GetSizeString ( long lSize)
{
	float size = (float)lSize;

	int level = 0;
	while ( size > 1000 )
	{
		size /= 1024;
		++level;
		if ( level >= 3 )
			break;
	}

	String s;
	char str[20];
	switch ( level )
	{
		case 0:
			sprintf ( str, "%.0f", size );
			s = str;
			s += "B";
			break;
		case 1:
			sprintf ( str, "%.2f", size );
			s = str;
			s += "KB";
			break;
		case 2:
			sprintf ( str, "%.2f", size );
			s = str;
			s += "MB";
			break;
		case 3:
			sprintf ( str, "%.2f", size );
			s = str;
			s += "GB";
			break;
	}
	return s;
}

#ifdef _WIN32
unsigned char *LZMAEncodeData ( unsigned char *fromData, long fromSize, long &toSize, CProgressInfo7Zip *progress )
{
	bool eos = false;

	CMyComPtr<ISequentialInStream> inStream;
	CInDataStream *inStreamSpec = new CInDataStream;
	inStream = inStreamSpec;
	inStreamSpec->LoadData ( fromData, fromSize );

	CMyComPtr<ISequentialOutStream> outStream;
	COutDataStream *outStreamSpec = new COutDataStream;
	outStream = outStreamSpec;
	outStreamSpec->Create ( toSize );

    NCompress::NLZMA::CEncoder *encoderSpec = new NCompress::NLZMA::CEncoder;
    CMyComPtr<ICompressCoder> encoder = encoderSpec;

	UInt32 dictionary = 1 << 21;
    UInt32 posStateBits = 2;
    UInt32 litContextBits = 3; // for normal files
    // UInt32 litContextBits = 0; // for 32-bit data
    UInt32 litPosBits = 0;
    // UInt32 litPosBits = 2; // for 32-bit data
    UInt32 algorithm = 2;
    UInt32 numFastBytes = 128;
    UInt32 matchFinderCycles = 16 + numFastBytes / 2;
    bool matchFinderCyclesDefined = false;

    //bool eos = parser[NKey::kEOS].ThereIs || stdInMode;
 
    PROPID propIDs[] = 
    {
      NCoderPropID::kDictionarySize,
      NCoderPropID::kPosStateBits,
      NCoderPropID::kLitContextBits,
      NCoderPropID::kLitPosBits,
      NCoderPropID::kAlgorithm,
      NCoderPropID::kNumFastBytes,
      NCoderPropID::kMatchFinder,
      NCoderPropID::kEndMarker,
      NCoderPropID::kMatchFinderCycles
    };
    const int kNumPropsMax = sizeof(propIDs) / sizeof(propIDs[0]);

	UString mf = L"BT4";

    PROPVARIANT properties[kNumPropsMax];
    for (int p = 0; p < 6; p++)
      properties[p].vt = VT_UI4;

    properties[0].ulVal = UInt32(dictionary);
    properties[1].ulVal = UInt32(posStateBits);
    properties[2].ulVal = UInt32(litContextBits);
    properties[3].ulVal = UInt32(litPosBits);
    properties[4].ulVal = UInt32(algorithm);
    properties[5].ulVal = UInt32(numFastBytes);

    properties[8].vt = VT_UI4;
    properties[8].ulVal = UInt32(matchFinderCycles);
    
    properties[6].vt = VT_BSTR;
    properties[6].bstrVal = (BSTR)(const wchar_t *)mf;

    properties[7].vt = VT_BOOL;
    properties[7].boolVal = eos ? VARIANT_TRUE : VARIANT_FALSE;

    int numProps = kNumPropsMax;
    if (!matchFinderCyclesDefined)
      numProps--;

    if ( encoderSpec->SetCoderProperties(propIDs, properties, numProps) != S_OK )
		return NULL;
    encoderSpec->WriteCoderProperties(outStream);

	UInt64 fileSize = 0;
	inStreamSpec->GetSize ( &fileSize );

    for (int i = 0; i < 8; i++)
    {
      Byte b = Byte(fileSize >> (8 * i));
      if (outStream->Write(&b, 1, 0) != S_OK)
		  return NULL;
    }

//	CProgressInfo *progressInfoSpec = new CProgressInfo;
	//CMyComPtr<ICompressProgressInfo> progressInfo = progress;

	if ( progress )
	{
		progress->ApprovedStart = 1 << 21;
		progress->SetMax ( fileSize );
		progress->SetIn ( true );
	}

    HRESULT result = encoder->Code(inStream, outStream, 0, 0, progress );

//	delete progressInfo;
    if (result == E_OUTOFMEMORY)
		return NULL;

    else if (result != S_OK)
		return NULL;

	toSize = outStreamSpec->GetCurrentSize();
	return outStreamSpec->GetData ();
}

unsigned char *LZMADecodeData ( unsigned char *fromData, long fromSize, long &toSize, CProgressInfo7Zip *progress )
{
	CMyComPtr<ISequentialInStream> inStream;
	CInDataStream *inStreamSpec = new CInDataStream;
	inStream = inStreamSpec;

	inStreamSpec->LoadData ( fromData, fromSize );

	NCompress::NLZMA::CDecoder *decoderSpec = new NCompress::NLZMA::CDecoder;

	CMyComPtr<ICompressCoder> decoder = decoderSpec;

	const UInt32 kPropertiesSize = 5;
	Byte properties[kPropertiesSize];
	UInt32 processedSize;
	
	if ( ReadStream (inStream, properties, kPropertiesSize, &processedSize) != S_OK )
		return NULL;

	if ( processedSize != kPropertiesSize )
		return NULL;

	if ( decoderSpec->SetDecoderProperties2(properties, kPropertiesSize) != S_OK )
		return NULL;

	UInt64 fileSize = 0;
	for (int i = 0; i < 8; i++)
	{
	  Byte b;
	  if ( inStream->Read(&b, 1, &processedSize) != S_OK )
		  return NULL;

	  if ( processedSize != 1 )
		  return NULL;
	  fileSize |= ((UInt64)b) << (8 * i);
	}

	CMyComPtr<ISequentialOutStream> outStream;
	COutDataStream *outStreamSpec = new COutDataStream;
	outStream = outStreamSpec;
	outStreamSpec->Create ( fileSize );

//	CProgressInfo *progressInfoSpec = new CProgressInfo;
//	CMyComPtr<ICompressProgressInfo> progressInfo = progress;

	if ( progress )
	{
		progress->Init();
		progress->ApprovedStart = 1 << 21;
		progress->SetMax ( fileSize );
	}

	if ( decoder->Code(inStream, outStream, 0, &fileSize, progress) != S_OK )
		// decoder error
		return NULL;

	toSize = outStreamSpec->GetCurrentSize ();
	return outStreamSpec->GetData ();
}

bool LZMAEncodeFile ( const char *fromFile, const char *toFile, CProgressInfo7Zip *progress )
{
	bool eos = false;

	CMyComPtr<ISequentialInStream> inStream;
	CInFileStream *inStreamSpec = new CInFileStream;
	inStream = inStreamSpec;

	if ( !inStreamSpec->Open ( GetSystemString(fromFile) ) )
		return false;

	CMyComPtr<ISequentialOutStream> outStream;
	COutFileStream *outStreamSpec = new COutFileStream;
	outStream = outStreamSpec;
	if ( !outStreamSpec->Create ( GetSystemString(toFile), true ) )
		return false;

    NCompress::NLZMA::CEncoder *encoderSpec = new NCompress::NLZMA::CEncoder;
    CMyComPtr<ICompressCoder> encoder = encoderSpec;

	UInt32 dictionary = 1 << 21;
    UInt32 posStateBits = 2;
    UInt32 litContextBits = 3; // for normal files
    // UInt32 litContextBits = 0; // for 32-bit data
    UInt32 litPosBits = 0;
    // UInt32 litPosBits = 2; // for 32-bit data
    UInt32 algorithm = 2;
    UInt32 numFastBytes = 128;
    UInt32 matchFinderCycles = 16 + numFastBytes / 2;
    bool matchFinderCyclesDefined = false;

    //bool eos = parser[NKey::kEOS].ThereIs || stdInMode;
 
    PROPID propIDs[] = 
    {
      NCoderPropID::kDictionarySize,
      NCoderPropID::kPosStateBits,
      NCoderPropID::kLitContextBits,
      NCoderPropID::kLitPosBits,
      NCoderPropID::kAlgorithm,
      NCoderPropID::kNumFastBytes,
      NCoderPropID::kMatchFinder,
      NCoderPropID::kEndMarker,
      NCoderPropID::kMatchFinderCycles
    };
    const int kNumPropsMax = sizeof(propIDs) / sizeof(propIDs[0]);

	UString mf = L"BT4";

    PROPVARIANT properties[kNumPropsMax];
    for (int p = 0; p < 6; p++)
      properties[p].vt = VT_UI4;

    properties[0].ulVal = UInt32(dictionary);
    properties[1].ulVal = UInt32(posStateBits);
    properties[2].ulVal = UInt32(litContextBits);
    properties[3].ulVal = UInt32(litPosBits);
    properties[4].ulVal = UInt32(algorithm);
    properties[5].ulVal = UInt32(numFastBytes);

    properties[8].vt = VT_UI4;
    properties[8].ulVal = UInt32(matchFinderCycles);
    
    properties[6].vt = VT_BSTR;
    properties[6].bstrVal = (BSTR)(const wchar_t *)mf;

    properties[7].vt = VT_BOOL;
    properties[7].boolVal = eos ? VARIANT_TRUE : VARIANT_FALSE;

    int numProps = kNumPropsMax;
    if (!matchFinderCyclesDefined)
      numProps--;

    if ( encoderSpec->SetCoderProperties(propIDs, properties, numProps) != S_OK )
		return false;
    encoderSpec->WriteCoderProperties(outStream);

	UInt64 fileSize = 0;
    if ( eos )
      fileSize = (UInt64)(Int64)-1;
    else
      inStreamSpec->File.GetLength(fileSize);

    for (int i = 0; i < 8; i++)
    {
      Byte b = Byte(fileSize >> (8 * i));
      if (outStream->Write(&b, 1, 0) != S_OK)
		  return false;
    }

	if ( progress )
	{
		progress->Init();
		progress->SetMax ( fileSize );
		progress->ApprovedStart = 1 << 21;
		progress->SetIn ( true );
	}

    HRESULT result = encoder->Code(inStream, outStream, 0, 0, progress );
    if (result == E_OUTOFMEMORY)
		return false;

    else if (result != S_OK)
		return false;

	return true;
}

bool LZMADecodeFile ( const char *fromFile, const char *toFile, CProgressInfo7Zip *progress )
{
	CMyComPtr<ISequentialInStream> inStream;
	CInFileStream *inStreamSpec = new CInFileStream;
	inStream = inStreamSpec;

	if ( !inStreamSpec->Open ( GetSystemString(fromFile) ) )
		return false;

	CMyComPtr<ISequentialOutStream> outStream;
	COutFileStream *outStreamSpec = new COutFileStream;
	outStream = outStreamSpec;
	if ( !outStreamSpec->Create ( GetSystemString(toFile), true ) )
		return false;


	NCompress::NLZMA::CDecoder *decoderSpec = new NCompress::NLZMA::CDecoder;

	CMyComPtr<ICompressCoder> decoder = decoderSpec;

	const UInt32 kPropertiesSize = 5;
	Byte properties[kPropertiesSize];
	UInt32 processedSize;
	
	if ( ReadStream (inStream, properties, kPropertiesSize, &processedSize) != S_OK )
		return false;

	if ( processedSize != kPropertiesSize )
		return false;

	if ( decoderSpec->SetDecoderProperties2(properties, kPropertiesSize) != S_OK )
		return false;

	UInt64 fileSize = 0;
	for (int i = 0; i < 8; i++)
	{
	  Byte b;
	  if ( inStream->Read(&b, 1, &processedSize) != S_OK )
		  return false;

	  if ( processedSize != 1 )
		  return false;
	  fileSize |= ((UInt64)b) << (8 * i);
	}

	if ( progress )
	{
		progress->Init();
		progress->ApprovedStart = 1 << 21;
		progress->SetMax ( fileSize );
	}

	if ( decoder->Code(inStream, outStream, 0, &fileSize, progress) != S_OK )
		// decoder error
		return false;

	return true;
}
#endif

String GetFileTypeString ( int type )
{
	switch ( type )
	{
		case FILETYPE_SCRIPT:
			return "Script";
		case FILETYPE_TEXT:
			return "Text";
		case FILETYPE_README:
			return "Readme";
		case FILETYPE_MAP:
			return "Map";
		case FILETYPE_MOD:
			return "Mod";
		case FILETYPE_UNINSTALL:
			return "Uninstall";
		case FILETYPE_SOUND:
			return "Sound";
		case FILETYPE_EXTRA:
			return "Extra";
		case FILETYPE_SCREEN:
			return "Screen";
		case FILETYPE_MISSION:
			return "Mission";
		case FILETYPE_BACKUP:
			return "Backup";
		case FILETYPE_SHIPOTHER:
			return "ShipOther";
		case FILETYPE_SHIPMODEL:
			return "ShipModel";
		case FILETYPE_SHIPSCENE:
			return "ShipScene";
		case FILETYPE_COCKPITSCENE:
			return "CockpitScene";
	}
	
	return NullString;
}

int GetFileTypeFromString ( String type )
{
	String ltype = type.ToLower();
	if ( ltype == "script" )
		return FILETYPE_SCRIPT;
	else if ( ltype == "text" )
		return FILETYPE_TEXT;
	else if ( ltype == "readme" )
		return FILETYPE_README;
	else if ( ltype == "map" )
		return FILETYPE_MAP;
	else if ( ltype == "mod" )
		return FILETYPE_MOD;
	else if ( ltype == "uninstall" )
		return FILETYPE_UNINSTALL;
	else if ( ltype == "sound" )
		return FILETYPE_SOUND;
	else if ( ltype == "extra" )
		return FILETYPE_EXTRA;
	else if ( ltype == "screen" )
		return FILETYPE_SCREEN;
	else if ( ltype == "mission" )
		return FILETYPE_MISSION;
	else if ( ltype == "backup" )
		return FILETYPE_BACKUP;
	else if ( ltype == "shipother" )
		return FILETYPE_SHIPOTHER;
	else if ( ltype == "shipmodel" )
		return FILETYPE_SHIPMODEL;
	else if ( ltype == "shipscene" )
		return FILETYPE_SHIPSCENE;
	else if ( ltype == "cockpitscene" )
		return FILETYPE_COCKPITSCENE;

	return -1;
}

#ifdef _WIN32
STDMETHODIMP CProgressInfo7Zip::SetRatioInfo(const UInt64 *inSize, const UInt64 *outSize)
{
	ProgressUpdated ( (m_bDoIn) ? *inSize : *outSize, m_lMaxSize );
	
	/*
	if (*inSize >= ApprovedStart && InSize == 0)
	{
		InSize = *inSize;
	}*/
	return S_OK;
}
#endif

bool CreateDirectory ( String dir )
{
	if ( dir.Empty() )
		return true;

	int max = 0;
	String *dirs = dir.FindReplace ( "/", "\\" ).FindReplace ( "\\\\", "\\" ).SplitToken ( '\\', &max );
	
	int start = 1;
	String curDir = dirs[0];
//	if ( !curDir.IsIn(":\\") ) 
//	{
		curDir = "";
		start = 0;
//	}

	for ( int i = start; i < max; i++ )
	{
		if ( !curDir.Empty() )
			curDir += "\\";
		curDir += dirs[i];

		#ifdef _WIN32
		CreateDirectory ( curDir.c_str(), 0 );
		#else
		String newDir = curDir;
		newDir = newDir.FindReplace ( "\\", "/" );
		mkdir(newDir.c_str(), 0755);
		#endif
	}

	return true;
}

//bool CreateDirectory ( String dir )
//{
//	return false;
//}
//#endif


bool CFile::CheckPCK ()
{
	if ( (m_sData) && (m_lDataSize) && (m_iDataCompression == SPKCOMPRESS_NONE) )
		return IsDataPCK ( m_sData, m_lDataSize );

	String filename = GetFilePointer();
	if ( !filename.Empty() )
	{
		FILE *id = fopen ( filename.c_str(), "rb+" );
		if ( id )
		{
			unsigned char data[3];
			fread ( data, sizeof(unsigned char), 3, id );
			fclose ( id );

			return IsDataPCK ( data, 3 );
		}
	}

	return false;
}

unsigned char *CFile::UnPCKFile ( size_t *len )
{
	unsigned char *data = NULL;
	size_t datasize = 0;

	if ( CheckValidFilePointer() )
	{
		FILE *id = fopen ( GetFilePointer().c_str(), "rb+" );
		if ( id )
		{
			fseek ( id, 0, SEEK_END );
			datasize = ftell ( id );
			rewind ( id );

			data = new unsigned char[datasize];
			fread ( data, sizeof(unsigned char), datasize, id );
			fclose ( id );
		}
	}

	if ( !data )
	{
		datasize = m_lDataSize;
		data = new unsigned char[datasize];
		memcpy ( data, m_sData, datasize );
	}

	if ( data )
	{
		unsigned char *newdata = UnPCKData ( data, datasize, len );
		delete data;
		return newdata;
	}

	return NULL;
}

void CFile::UnPCKFile ()
{
	unsigned char *data = NULL;
	size_t datasize = 0;

	if ( CheckValidFilePointer() )
	{
		FILE *id = fopen ( GetFilePointer().c_str(), "rb+" );
		if ( id )
		{
			fseek ( id, 0, SEEK_END );
			datasize = ftell ( id );
			rewind ( id );

			data = new unsigned char[datasize];
			fread ( data, sizeof(unsigned char), datasize, id );
			fclose ( id );
		}
	}

	if ( !data )
	{
		datasize = m_lDataSize;
		data = new unsigned char[datasize];
		memcpy ( data, m_sData, datasize );
	}

	if ( data )
	{
		size_t len = 0;
		m_sData = UnPCKData ( data, datasize, &len );
		delete data;
		m_lDataSize = len;
	}
}

unsigned char *UnPCKFile ( const char *file, size_t *len )
{
	FILE *id = fopen ( file, "rb" );
	if ( !id )
		return NULL;

	fseek ( id, 0, SEEK_END );
	size_t size = ftell ( id );
	fseek ( id, 0, SEEK_SET );

	unsigned char *data = new unsigned char[size];
	fread ( data, sizeof(unsigned char), size, id );

	if ( ferror(id) )
	{
		delete data;
		data = NULL;
	}
	
	fclose ( id );

	if ( data )
	{
		unsigned char *unData = UnPCKData ( data, size, len );
		delete data;
		return unData;
	}

	return NULL;
}

unsigned char *UnPCKData ( unsigned char *data, size_t datasize, size_t *len )
{
	unsigned char magic = data[0] ^ 0xC8;
		
	for ( size_t i = 0; i < datasize; i++ )
		data[i] ^= magic;
	++data;
	--datasize;

	// create data buffer
	size_t *uncomprLenSize = (size_t*)(data + (datasize - 4));
	unsigned long uncomprLen = *uncomprLenSize;
	unsigned char *uncompr = new unsigned char[uncomprLen];
	memset ( uncompr, 0, sizeof(uncompr) );


	// find header size
	unsigned char *buf = data + PCKHEADERSIZE;

	char flag = data[3];

	if ( flag & GZ_FLAG_EXTRA )
	{
		size_t xlen = *((short int*)(buf));
		buf += xlen;
	}
	
	if ( flag & GZ_FLAG_FILENAME )
	{
		char *origname = (char*)(buf);
		buf += strlen (origname) + 1;
	}
	if ( flag & GZ_FLAG_COMMENT )
	{
		char *comment = (char*)(buf);
		buf += strlen(comment) + 1;
	}
	if ( flag & GZ_FLAG_HCRC )
		buf += 2;
	long bufSize = datasize - (buf-data) - 8;

	int err = uncompress2 ( uncompr, &uncomprLen, buf, bufSize );
	if ( err != Z_OK )
	{
		delete uncompr;
		uncompr = NULL;
		uncomprLen = 0;
	}

	*len = uncomprLen;
	return uncompr;
}

bool IsDataPCK ( const unsigned char *data, size_t size )
{
	if ( size >=3 )
	{
		unsigned char magic=data[0] ^ 0xC8;
		return ((data[1] ^ magic)==0x1F && (data[2] ^ magic)==0x8B);
	}
	else
		return false;

}


int ReadScriptVersionFromData ( unsigned char *data, long size )
{
	if ( IsDataPCK ( data, size ) )
	{
		size_t unpckSize = 0;
		unsigned char *unpckData = UnPCKData ( data, size, &unpckSize );
		if ( (unpckData) && (unpckSize) )
			return ReadScriptVersionFromData ( unpckData, unpckSize );
		return 0;
	}

	int pos = 0;
	bool found = false;
	int iVersion = 0;

	// skip past initial space
	while ( !found )
	{
		while ( (pos < size) && (data[pos] == ' ') )
			++pos;

		char check = data[pos];
		if ( data[pos] == '<' )
		{
			String checktag;
			while ( (pos < size) && (data[pos] != '>') )
			{
				checktag += (char)data[pos];
				++pos;
			} 
			++pos;

			if ( checktag == "<version" )
			{
				String version;
				while ( (pos < size) && (data[pos] != '<') )
				{
					version += (char)data[pos];
					++pos;
				}
				iVersion = version.ToInt();
				found = true;
				break;
			}
		}

		if ( found )
			break;

		// skip to next line
		while ( (pos < size) && (data[pos] != '\n') )
			++pos;
		++pos;

		if ( pos >= size )
			break;
	}

	return iVersion;
}

String CFile::GetBaseName ()
{
	// remove any directory
	String file = m_sName.GetToken ( "/", m_sName.NumToken ( '/' ) );

	// remove file extension
	file = file.GetToken ( ".", 1, file.NumToken ( '.' ) - 1 );

	return file;
}
