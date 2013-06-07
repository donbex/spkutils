#ifndef __MUTLISPKFILE_H__
#define __MUTLISPKFILE_H__

#include "String.h"
#include "SpkFile.h"

typedef struct SMultiSpkFile
{
#ifdef _WIN32
	struct SMultiSpkFile () { sData = NULL; lSize = 0; bOn = true; pFile = NULL; }
#endif
	String	sName;
	char   *sData;
	long    lSize;
	bool	bOn;
	String  sScriptName;
	String  sScriptVersion;
	String  sScriptAuthor;
	CSpkFile *pFile;
} SMultiSpkFile;

typedef struct SMultiHeader
{
#ifdef _WIN32
	struct SMultiHeader () { iCompression = SPKCOMPRESS_ZLIB; bSelection = false; lUncomprLen = lComprLen = 0; }
#endif
	float	fVersion;
	int		iCompression;
	long	lUncomprLen;
	long	lComprLen;
	bool	bSelection;
} SMultiHeader;

class CMultiSpkFile
{
public:
	CMultiSpkFile () { }

	SMultiSpkFile *AddFileEntry ( String filename );
	bool AddFile ( String, bool = true );
	bool RemoveFile ( int id );
	bool RemoveFile ( SMultiSpkFile *ms );

	void ClearFiles ();

	bool ReadFile ( String, bool = true );
	bool ParseHeader ( String header );
	bool ParseValueLine ( String line );
	void ReadValues ( String values );

	bool ReadAllFilesToMemory ();
	bool ReadFileToMemory ( SMultiSpkFile *ms );
	bool ReadAllSpk ( String filename, int type = SPKREAD_NODATA ) { m_sFilename = filename; return ReadAllSpk ( type ); }
	bool ReadAllSpk ( int type = SPKREAD_NODATA );
	bool ReadSpk ( SMultiSpkFile *ms, int type = SPKREAD_NODATA );

	SMultiSpkFile *FindFile ( String name );

	bool ExtractAll ( String dir );
	bool ExtractData ( SMultiSpkFile *ms );
	bool ExtractFile ( SMultiSpkFile *ms, String dir );
	bool SplitMulti ( String filename, String destdir );
	
	bool WriteFile ( String );
	String CreateData ();

	void SetName ( String n ) { m_sName = n; }
	void SetSelection ( bool on ) { m_SHeader.bSelection = on; }
	void SetCompression ( int c ) { m_SHeader.iCompression = c; }

	String GetName () { return m_sName; }
	int GetNumFiles () { return m_lFiles.size(); }
	CLinkList<SMultiSpkFile> *GetFileList () { return &m_lFiles; }
	bool IsSelection () { return m_SHeader.bSelection; }

private:
	CLinkList<SMultiSpkFile> m_lFiles;
	String m_sName, m_sFilename;

	SMultiHeader m_SHeader;
};

#endif //__MUTLISPKFILE_H__
