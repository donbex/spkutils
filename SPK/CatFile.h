#include "File.h"
#include "File_IO.h"
#include "lists.h"

enum {CATERR_NONE, CATERR_NODATFILE, CATERR_NOCATFILE, CATERR_FILEEMPTY, CATERR_READCAT, CATERR_DECRYPT, CATERR_MISMATCH, CATERR_NOFILE, CATERR_CANTREAD, CATERR_CANTCREATEDIR, CATERR_INVALIDDEST,
		CATERR_CREATED};
enum {CATFILE_NONE, CATFILE_READ, CATFILE_DECYPTED};
enum {CATREAD_CAT, CATREAD_CATDECRYPT, CATREAD_DAT};

typedef struct SInCatFile {
//	struct SInCatFile () { lSize = 0; sData = 0; lOffset = 0; }
	String sFile;
	size_t lSize;
	unsigned char  *sData;
	size_t lOffset;
} SInCatFile;

unsigned char *PCKData ( unsigned char *data, size_t oldsize, size_t *newsize );

class CCatFile
{
public:
	CCatFile ();
	~CCatFile ();

	int  Open ( String catfile, int readtype = CATREAD_CAT, bool = true );
	bool DecryptData ();
	bool DecryptData ( unsigned char *data, size_t size );
	bool ReadFiles ();
	void LoadDatFile ();

	bool CheckExtensionPck ( String filename );
	bool CheckPackedExtension ( String filename );
	String PckChangeExtension ( String f );

	unsigned char *ReadData ( String filename, size_t *size );
	unsigned char *ReadData ( SInCatFile *c, size_t *size );
	SInCatFile *FindData ( String filename );
	bool ReadFileToData ( String filename );
	bool ReadFileToData ( SInCatFile *c );

	int GetNumFiles () { return m_lFiles.size(); }
	SInCatFile *GetFile ( int num ) { return m_lFiles.Get(num); }

	unsigned char *UnpackFile ( SInCatFile *c, size_t *size );

	bool RemoveFile ( SInCatFile *f );
	bool RemoveFile ( String filename );
	bool WriteCatFile ();
	bool AppendFile ( String filename, String to, bool pck = true );
	bool AppendData ( unsigned char *data, size_t size, String to, bool pck = true );
	bool AddData ( String catfile, unsigned char *data, size_t size, String to, bool pck = true, bool create = true );

	String ChangeExtension ( String f );

	bool ExtractFile ( String filename, String to );

	void ClearError () { m_iError = CATERR_NONE; }
	int Error () { return m_iError; }
	String GetErrorString ();
	
	bool WriteFromCat ( String catfile, String file );

private:
	void RemoveData ();

	CFileIO m_fCatFile;
	CFileIO m_fDatFile;

	unsigned char *m_sData;
	size_t m_lSize;
	char m_iDataType;

	CLinkList<SInCatFile> m_lFiles;

	int m_iError;

	bool m_bCreate;
};
