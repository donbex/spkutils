// File.h: interface for the CFile class.
//
//////////////////////////////////////////////////////////////////////

/*
File class, Created by Matthew Gravestock (Cycrow)

	This class handles the store of data for each of the file in the package.
	It also includes all the compression functions to compress and uncompress the files from the package
  
	Enums:
		Compression Type - The type of compression for each section and file, theres currently 3 types
			7Zip - Compress via 7Zip compression, only works on windows version, but linux version can decompress them. Default for file data on Windows version
			ZLIB - Compress via the ZLIB libray, ie Zip copression. Default compression for headers on all versions, and file data on Linux Version
			None - No compression, just write in plain text (Not really used, prodcudes much larger files)
		
		FileType - This is what type of file they are, deterimes how and where the file is installed
			Script - A script file, goes into the X3/Scripts directory
			Text - A Text file, ie 447532.xml, contains all text data used by script/mod.  Goes in the X3/t directory
			Readme - A Readme .txt file, can be displayed in installer in Rich Text format
			Map - A Map script, these are xml files that create a new universe
			Mod - .cat/.dat file pair, can also be a fake patch if named as a number, ie 01.cat
			Uninstall - An uninstall script, only goes to X3/Scripts directory when plugin is uninstalled, allows plugins to clean themselves when removed
			Sound - Soundtrack files, goes in the X3/Soundtrack directory, used for the sector music
			Screen - Screen shot file, goes in the X3/loadscr directory, will be displayed as a loading screen
			Extra - Any other file that doesn't fit in the section, can be placed in any directory thats required

		Error - Stores the last error found (currently only used for a few errors)
			None - No error, function was successful
			Malloc - Error trying to malloc memory space, possible cause (Not enough memory)
			Fileopen - Unable to open the file, possible cause (File doesn't exist)
			Fileread - Error trying to read a file, happens after the file is open

	Class includes all files needed for both 7Zip and ZLIB Libraries, and acts as the path between them

	Functions (Windows Only):
		LZMAEncodeData - Encodes a data stream using 7Zip, returns the encoded data as char array
		LZMADecodeData - Decodes a compressed stream using 7Zip, returns the uncompressed data
		LZMAEncodeFile - Encodes a file stream using 7Zip, writes to another file
		LZMADecodeFile - Decodes a file stream using 7Zip, writes uncomressed data to file
		
	Classes
		CProgressInfo - Used to store progress info, basse class that needs to be dervived from to handle the progress update.
						When decoding multi files, calls DoingFile() for each functions, allows class to display the current progress of the file
						ProgressUpdated() is called for each progress, as the processed size and max size
		CProgressInfo7Zip (Windows Only) - 7Zip specific progress, dervive from this for using progress on 7Zip Compression. (Currently no progress for ZLIB)

	CFile:
		The files here are stored in 1 of 2 ways, either file data, or a file pointer
		
		File Data:
			Uses the m_sData and m_lDataSize varibles.  As well as the m_iDataCompression.
			This method is used when reading the file to memory, either from a file or direct from the SPK Package.
			This can be stored in compressed form so when writing the SPK File, it doesn't need to be compressed again.
			CompressData() will compress the current data stream to m_sData, changes m_lDataSize and m_iDataCompression to match
			UncomressData() will uncompress the data, it can either uncomress and return the uncomressed data stream, without effecting m_sData,
			or it can change m_sData to the uncompressed data.

		File Pointer:
			When adding a new file, it will be first added as a pointer, this means that there is no data loaded, it just points to a filename on disk.
			ReadFileSize() will read the size of the file to m_lSize
			ReadLastModifed() will read the modified data tag of the file and store it in the class
			ReadFromFile() will open and read the file to the data stream, m_sData, the compression will be set to None.
			ReadFromFile (FILE *id) will read from a currently open file stream into the data, can be used to read a file from an existing SPK Package


*/

#if !defined(AFX_FILE_H__A0C15B81_4FD1_40D7_8EE8_2ECF5824BB8B__INCLUDED_)
#define AFX_FILE_H__A0C15B81_4FD1_40D7_8EE8_2ECF5824BB8B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "String.h"

// compression type
enum { SPKCOMPRESS_NONE, SPKCOMPRESS_ZLIB, SPKCOMPRESS_7ZIP };
// file type
enum {FILETYPE_SCRIPT, FILETYPE_TEXT, FILETYPE_README, FILETYPE_MAP, FILETYPE_MOD, FILETYPE_UNINSTALL, FILETYPE_SOUND, FILETYPE_EXTRA, FILETYPE_SCREEN, FILETYPE_BACKUP, FILETYPE_MISSION, FILETYPE_MAX, FILETYPE_SHIPOTHER, FILETYPE_SHIPMODEL, FILETYPE_SHIPSCENE, FILETYPE_COCKPITSCENE};
// error
enum {SPKERR_NONE, SPKERR_MALLOC, SPKERR_FILEOPEN, SPKERR_FILEREAD, SPKERR_UNCOMPRESS, SPKERR_WRITEFILE, SPKERR_CREATEDIRECTORY, SPKERR_FILEMISMATCH};

#include "zlib/zlib.h"

#define PCKHEADERSIZE 10

bool IsDataPCK ( const unsigned char *data, size_t size );
unsigned char *UnPCKData ( unsigned char *data, size_t datasize, size_t *len );
unsigned char *UnPCKFile ( const char *file, size_t *len );
int ReadScriptVersionFromData ( unsigned char *data, long size );


#if defined(_WIN32) || defined(OS2) || defined(MSDOS)
#include <fcntl.h>
#include <io.h>
#define MY_SET_BINARY_MODE(file) setmode(fileno(file),O_BINARY)
#else
#define MY_SET_BINARY_MODE(file)
#endif

class CFile;
class CProgressInfo 
{
public:
	CProgressInfo () { m_bDoIn = false; m_lMaxSize = 0; }

	virtual void ProgressUpdated ( const long cur, const long max ) = 0;
	virtual void DoingFile ( CFile *file ) = 0;
	void SetIn ( bool in ) { m_bDoIn = in; }
	void SetMax ( long max ) { m_lMaxSize = max; }

protected:
	bool m_bDoIn;
	long m_lMaxSize;
};


// for win 32
#ifdef _WIN32
#include <windows.h>
#include "7zip/Common/StringConvert.h"
#include "7zip/7zipDataStream.h"
#include "7zip/7zip/Common/StreamUtils.h"
#include "7zip/7zip/Common/FileStreams.h"

#include "7zip/7zip/Compress/LZMA/LZMADecoder.h"
#include "7zip/7zip/Compress/LZMA/LZMAEncoder.h"
class CProgressInfo7Zip : public ICompressProgressInfo,  public CMyUnknownImp, public CProgressInfo
{
public:
  UInt64 ApprovedStart;
  bool doIn;
  void Init()
  {
	doIn = false;
  }
  MY_UNKNOWN_IMP
  STDMETHOD(SetRatioInfo)(const UInt64 *inSize, const UInt64 *outSize);
  virtual void ProgressUpdated ( const long cur, const long max ) = 0;
  virtual void DoingFile ( CFile *file ) = 0;
};


unsigned char *LZMAEncodeData ( unsigned char *fromData, long fromSize, long &toSize, CProgressInfo7Zip *progress = NULL );
unsigned char *LZMADecodeData ( unsigned char *fromData, long fromSize, long &toSize, CProgressInfo7Zip *progress = NULL );
bool LZMADecodeFile ( const char *fromFile, const char *toFile, CProgressInfo7Zip *progress );
bool LZMAEncodeFile ( const char *fromFile, const char *toFile, CProgressInfo7Zip *progress );

// for any other machine
#else
#include "ansi7zip/7Decoder.h"
#endif

#define GZ_FLAG_TEXT     1     // 0
#define GZ_FLAG_HCRC     2     // 1
#define GZ_FLAG_EXTRA    4     // 2
#define GZ_FLAG_FILENAME 8     // 3
#define GZ_FLAG_COMMENT  16    // 4
#define GZ_FLAG_RES1     32    // 5
#define GZ_FLAG_RES2     64    // 6
#define GZ_FLAG_RES3     128   // 7

class CProgressInfo2
{
public:
	virtual void ProgressPercent ( float percent ) = 0;
};


String GetFileTypeString ( int type );
int    GetFileTypeFromString ( String type );
bool CreateDirectory ( String dir );

class CBaseFile;
class CFile  
{
public:
	void ClearUsed () { m_iUsed = 0; }
	void IncUsed () { ++m_iUsed; }
	void DeleteData ();
	// varible setting functions
	void SetFileType ( int t ) { m_iFileType = t; }
	void SetFilename ( String filename );
	void SetName ( String name ) { m_sName = name; }
	void SetDir ( String dir ) { m_sDir = dir; }
	void SetCreationTime ( time_t time ) { m_tTime = time; }
	void SetFileSize ( long size ) { m_lSize = size; }
	void SetDataSize ( long size ) { m_lDataSize = size; }
	void SetUncompressedDataSize ( long size ) { m_lUncomprDataSize = size; }
	void SetDataCompression ( int c ) { m_iDataCompression = c; }
	void SetShared ( bool b ) { m_bShared = b; }
	void SetSigned ( bool b ) { m_bSigned = b; }
	void SetFullDir ( String dir ) { m_sFullDir = dir; }
	void SetCompressedToFile ( bool b ) { m_bCompressedToFile = b; }

	// get functions
	int GetUsed () { return m_iUsed; }
	int GetLastError () { return m_iLastError; }
	int GetFileType () { return m_iFileType; }
	String GetFilename () { return m_sName; }
	String GetFullFilename () { return m_sFullDir + "/" + m_sName; }
	long GetSize () { return m_lSize; }
	time_t GetCreationTime () { return m_tTime; }
	String GetName () { return m_sName; }
	String GetDir() { return m_sDir; }
	String GetDataSizeString ();
	String GetUncompressedSizeString ();
	String GetDirectory ( CBaseFile *spkfile );
	int GetVersion () { return m_iVersion; }
	size_t GetLastModified() { return m_tTime; }
	String GetFileTypeString () { return ::GetFileTypeString ( m_iFileType ); }
	String GetCreationTimeString ();
	String GetTempFile () { return m_sTmpFile; }
	String GetFullDir () { return m_sFullDir; }
	String GetBaseName ();

	bool IsFakePatch ();
	bool IsCompressedToFile () { return m_bCompressedToFile; }
	bool CompareNew ( CFile *file );
	String GetFileExt () { return m_sName.GetToken ( m_sName.NumToken('.'), '.' ).ToUpper(); }
	bool CheckFileExt ( String ext ) { ext = ext.ToUpper(); if ( ext == GetFileExt() ) return true; return false; }

	int GetCompressionType () { return m_iDataCompression; }
	long GetDataSize () { return m_lDataSize; }
	// returns the Uncompressed Data size, based on how the file is loaded, ie view data, view pointer, etc
	long GetUncompressedDataSize () 
	{ 
		if ( m_lUncomprDataSize ) 
			return m_lUncomprDataSize; 
		if ( m_lSize )
			return m_lSize; 
		return m_lDataSize; 
	}
	String GetNameDirectory ( CBaseFile *spkfile );

	unsigned char *GetData () { return m_sData; }
	bool IsShared () { return m_bShared; }
	bool IsSigned () { return m_bSigned; }

	// file reading functions
	long ReadFileSize ();
	time_t ReadLastModified ();
	bool ReadFromFile ();
	bool ReadFromFile ( FILE *id, long size, bool = true );
	bool ReadFromData ( char *data, long size );
	int ReadScriptVersion ();
	String GetFilePointer ();

	// file writing functions
	bool WriteToFile ( String filename, unsigned char * = NULL, long = 0 );
	bool WriteToDir ( String &dir, CBaseFile *, bool = true, String appendDir = NullString, unsigned char * = NULL, long = 0 );
	bool WriteFilePointer ( unsigned char *cData = NULL, long len = 0 );

	// file compression functions
	bool UncompressToFile ( String, CBaseFile *, bool = true, CProgressInfo * = NULL );
	bool CompressData ( int compressionType, CProgressInfo * = NULL );
	bool CompressFile ( CProgressInfo * = NULL );
	bool UncompressData ( CProgressInfo * = NULL );
	unsigned char *UncompressData ( long *size, CProgressInfo * = NULL );


	void MarkSkip () { m_bSkip = true; }
	bool Skip () { return m_bSkip; }
	bool CheckPCK ();
	bool CheckValidFilePointer ();
	unsigned char *UnPCKFile ( size_t *len );
	bool MatchFile ( CFile *file );
	void UnPCKFile ();

	// contructor/decontructor
	CFile();
	CFile ( String filename );
	CFile ( const char *filename );
	virtual ~CFile();

	// static functions
	static String GetSizeString ( long size );

protected:
	static int m_iTempNum; // Used when creating temp files, allows new tmp files without overrighing used ones

	// private functions
	void Reset();
	String GetFullFileToDir ( String dir, bool includedir, CBaseFile *file );

	String  m_sName; // just the filename
	String  m_sDir;  // the extra dir (only for extras)

	// file pointer varibles
	String  m_sFullDir; // the full dir path of the file (This is used for file pointers when not loaded data in file)
	long    m_lSize;  // size of current file pointer

	// Main file varibles
	int		m_iVersion;  // used when reading script versions
	bool	m_bSigned;   // signed status of file, installer only, read from file
	bool    m_bShared;   // is file a marked shared file (Not used much anymore)
	int		m_iUsed;     // used by installer, number of plugins that uses the file
	time_t	m_tTime;     // Creation time of the file

	// File data varibles
	long	m_lDataSize;  // size of the data stream in what ever compression is set
	long	m_lUncomprDataSize; // size of stream if it was uncompressed
	unsigned char   *m_sData;  // stores the file data if loaded in memory
	int		m_iDataCompression; // the current compression of m_sData

	int     m_iFileType; // type if file, ie Script, Text, etc

	bool    m_bUsedMalloc; // malloc type of m_sData, so it can use with free() or delete
	String	m_sTmpFile;

	bool	m_bCompressedToFile;

	bool    m_bSkip;
	bool	m_bLoaded;

	int m_iLastError;
};


#endif // !defined(AFX_FILE_H__A0C15B81_4FD1_40D7_8EE8_2ECF5824BB8B__INCLUDED_)
