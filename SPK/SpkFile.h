// SpkFile.h: interface for the CSpkFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPKFILE_H__E35FA619_B901_479F_BE42_2FF1519BA4D3__INCLUDED_)
#define AFX_SPKFILE_H__E35FA619_B901_479F_BE42_2FF1519BA4D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "lists.h"
#include "File.h"
#include "String.h"

#define FILEVERSION 4.00f
enum {SPKFILE_INVALID, SPKFILE_SINGLE, SPKFILE_MULTI, SPKFILE_SIGNLESHIP};
enum {SPKREAD_ALL, SPKREAD_NODATA, SPKREAD_VALUES, SPKREAD_HEADER};
enum {DELETESTATE_NONE, DELETESTATE_WAITING, DELETESTATE_DONE};
enum {SETTING_INTEGER, SETTING_STRING, SETTING_CHECK};

enum {TYPE_BASE, TYPE_SPK, TYPE_XSP};

// spk header struct
typedef struct SSPKHeader {
	SSPKHeader () { fVersion = 0; iValueCompression = lValueCompressSize = 0; }
	float fVersion;
	int iValueCompression;
	unsigned long lValueCompressSize;
} SSPKHeader;

typedef struct SSettingType {
	String	sKey;
	int		iType;
} SSettingType;

typedef struct SSettingString : public SSettingType {
	SSettingString () { iType = SETTING_STRING; }
	String sValue;
} SSettingString;

typedef struct SSettingInteger : public SSettingType {
	SSettingInteger () { iType = SETTING_INTEGER; }
	int iValue;
} SSettingInteger;

typedef struct SSettingCheck : public SSettingType {
	SSettingCheck () { iType = SETTING_CHECK; }
	bool bValue ;
} SSettingCheck;

typedef struct SSPKHeader2 {
	SSPKHeader2 () { iNumFiles = 0; lSize = 0; lFullSize = 0; }
	int iNumFiles;
	long lSize;
	long lFullSize;
	int iFileCompression;
	int iDataCompression;
} SSPKHeader2;

typedef struct SInstallText {
	String sLanguage;
	String sBefore;
	String sAfter;
} SInstallText;

typedef struct SWaresText {
	int		iLang;
	String  sName;
	String  sDesc;
} SWaresText;

typedef struct SWares {
	String  sID;
	char    cType;
	long	iPrice;
	int		iSize;
	int		iVolumn;
	int		iNotority;
	char	iDeleteState;
	bool	bEnabled;
	int		iPosID;
	int		iDescID;
	CLinkList<SWaresText> lText;
	int		iUsed;
} SWares;

typedef struct SNames {
	String sLanguage;
	String sName;
} SNames;


class CBaseFile 
{
public:
	CBaseFile();
	virtual ~CBaseFile();

	// Get basic Settings
	String GetName ()			{ return m_sName; }
	String GetVersion ()		{ return m_sVersion; }
	String GetAuthor ()			{ return m_sAuthor; }
	String GetWebSite ()		{ return m_sWebSite; }
	String GetWebAddress ()		{ return m_sWebAddress; }
	String GetEmail ()			{ return m_sEmail; }
	String GetCreationDate ()	{ return m_sCreationDate; }
	String GetWebMirror1 ()		{ return m_sWebMirror1; }
	String GetWebMirror2 ()		{ return m_sWebMirror2; }
	String GetDescription ()	{ return m_sDescription; }
	CLinkList<CFile> *GetFileList() { return &m_lFiles; }
	CFile *GetIcon () { return m_pIconFile; }
	String GetIconExt () { return m_sIconExt; }
	String GetNameValidFile ();
	int GetDataCompression () { return m_SHeader2.iDataCompression; }
	float GetFileVersion () { return m_SHeader.fVersion; }
	long GetFullFileSize ();
	int    GetGameVersion () { return m_iGameVersion; }
	String GetLanguageName ( String lang );

	virtual String CreateValuesLine ();

	// set basic settings
	void SetName			( String str ) { m_sName = str.Remove ( '|' ); }
	void SetVersion			( String str ) { m_sVersion = str; }
	void SetAuthor			( String str ) { m_sAuthor = str.Remove ( '|' ); }
	void SetWebAddress		( String str ) { m_sWebAddress = str; }
	void SetWebSite			( String str ) { m_sWebSite = str; }
	void SetEmail			( String str ) { m_sEmail = str; }
	void SetCreationDate	( String str ) { m_sCreationDate = str; }
	void SetWebMirror1		( String str ) { m_sWebMirror1 = str; }
	void SetWebMirror2		( String str ) { m_sWebMirror2 = str; }
	void SetDescription		( String str ) { m_sDescription = str; }
	void SetDataCompression ( int c )       { m_SHeader2.iDataCompression = c; }
	void SetFileCompression ( int c )       { m_SHeader2.iFileCompression = c; }
	void SetValueCompression( int c )       { m_SHeader.iValueCompression = c; }
	void SetIcon ( CFile *file, String ext ) { if ( m_pIconFile ) delete m_pIconFile; m_sIconExt = ext; m_pIconFile = file; }

	// error handling
	void ClearError () { m_sLastError = ""; m_iLastError = SPKERR_NONE; }
	int GetLastError () { return m_iLastError; }
	String GetLastErrorString() { return m_sLastError; }

	// file handling
	void AddFile ( CFile *file );
	CFile *AddFile ( String, String, int type );
	CFile *AppendFile ( String file, int type, String dir = NullString, CProgressInfo *progress = NULL );
	CFile *FindFile ( String, int, String = NullString );
	bool AddFileNow ( String, String, int type, CProgressInfo *progress = NULL );
	int CountFiles ( int filetype );
	CFile *FindFileAt ( int filetype, int pos );
	virtual bool RemoveFile ( int pos );
	virtual bool RemoveFile ( CFile *files );
	virtual bool RemoveFile ( String file, int type );
	void RemoveAllFiles ( int type );
	String CreateFilesLine ( bool updateheader, CProgressInfo * = NULL );

	CLinkList<SNames> *GetNamesList() { return &m_lNames; }

	// reading files
	void ReadAllFilesToMemory ();

	// compression
	void CompressAllFiles ( int compresstype );
	bool UncompressAllFiles ( CProgressInfo * = NULL );

	// static functions
	static String GetEndOfLine ( FILE *id, int *line = NULL, bool upper = true );
	static int CheckFile ( String filename, float *version = NULL );

	void SetGameVersion ( int i ) { m_iGameVersion = i; }

	// installing
	bool InstallFiles ( String destdir, CProgressInfo *progress, CLinkList<CFile> *spklist, bool enabled = true );
	virtual bool IsPatch () { return false; }

	// installer functions
	bool IsProfileEnabled () { return m_bProfile; }
	bool IsEnabled () { return m_bEnable; }
	bool IsGlobalEnabled () { return m_bGlobal; }

	void SetProfileEnabled ( bool en ) { m_bProfile = en; }
	void SetEnabled ( bool en ) { m_bEnable = en; }
	void SetGlobalEnabled ( bool en ) { m_bGlobal = en; }

	// language functions
	void AddLanguageName ( String lang, String name );
	void ClearNames ();

	virtual bool ParseValueLine ( String line );

	CLinkList<SInstallText> *GetInstallTextList() { return &m_lInstallText; }
	CLinkList<SInstallText> *GetUninstallTextList() { return &m_lUninstallText; }
	String GetInstallAfterText ( String lang ) { return GetAfterText ( &m_lInstallText, lang ); }
	String GetInstallBeforeText ( String lang ) { return GetBeforeText ( &m_lInstallText, lang ); }
	String GetUninstallAfterText ( String lang ) { return GetAfterText ( &m_lUninstallText, lang ); }
	String GetUninstallBeforeText ( String lang ) { return GetBeforeText ( &m_lUninstallText, lang ); }

	SInstallText *AddInstallBeforeText ( String lang, String data ) { return AddInstallText ( true, true, lang, data ); }
	SInstallText *AddInstallAfterText ( String lang, String data ) { return AddInstallText ( false, true, lang, data ); }
	SInstallText *AddUninstallBeforeText ( String lang, String data ) { return AddInstallText ( true, false, lang, data ); }
	SInstallText *AddUninstallAfterText ( String lang, String data ) { return AddInstallText ( false, false, lang, data ); }
	SInstallText *FindInstallText ( String lang ) { return FindInstallText ( true, lang ); }
	SInstallText *FindUninstallText ( String lang ) { return FindInstallText ( false, lang ); }
	void AddInstallText ( SInstallText * );
	void AddUninstallText ( SInstallText * );
	void RemoveInstallText ( String lang ) { RemoveInstallText ( true, lang ); }
	void RemoveUninstallText ( String lang ) { RemoveInstallText ( false, lang ); }
	bool IsThereInstallText () { return IsThereInstallText ( true ); }
	bool IsThereUninstallText () { return IsThereInstallText ( false ); }

	virtual int GetType () { return TYPE_BASE; }
	bool AnyFileType ( int type );


	bool   IsSigned ()		{ return m_bSigned;}

protected:
	virtual void Delete ();
	virtual void SetDefaults ();

	SInstallText *AddInstallText ( bool before, bool install, String lang, String data );
	SInstallText *FindInstallText ( bool install, String lang );
	void RemoveInstallText ( bool install, String lang );
	bool IsThereInstallText ( bool );

	String GetAfterText ( CLinkList<SInstallText> *list, String lang );
	String GetBeforeText ( CLinkList<SInstallText> *list, String lang );

	int m_iType;
	SSPKHeader m_SHeader;
	SSPKHeader2 m_SHeader2;

	String m_sName;
	String m_sVersion;
	String m_sAuthor;
	String m_sWebSite;
	String m_sWebAddress;
	String m_sEmail;
	String m_sWebMirror1;
	String m_sWebMirror2;
	String m_sDescription;
	String m_sCreationDate;

	CFile *m_pIconFile;
	String m_sIconExt;
	String m_sLastError;
	int m_iLastError;

	String m_sFilename;

	CLinkList<CFile>  m_lFiles;
	CLinkList<SNames> m_lNames;
	CLinkList<SInstallText> m_lInstallText;
	CLinkList<SInstallText> m_lUninstallText;

	int m_iGameVersion;

	bool m_bSigned;

	//installer varibles
	bool m_bEnable;
	bool m_bGlobal;
	bool m_bProfile;
};

class CSpkFile : public CBaseFile
{
public:
	// get functions
	String GetScriptType ()		{ return m_sScriptType; }
	String GetOtherName ()		{ return m_sOtherName; }
	String GetOtherAuthor ()	{ return m_sOtherAuthor; }
	CLinkList<SWares> *GetWaresList() { return &m_lWares; }
	CLinkList<SSettingType> *GetSettingsList() { return &m_lSettings; }
	CSpkFile *GetParent () { return m_pParent; }

	bool CheckValidReadmes ();

	bool   IsCustomStart () { return m_bCustomStart; }
	bool   IsPackageUpdate () { return m_bPackageUpdate; }
	bool   IsPatch ()		{ return m_bPatch; }
	bool   IsAnotherMod()   { if ( (m_sOtherName.Empty()) || (m_sOtherAuthor.Empty()) ) return false; return true; }
	bool   IsForceProfile() { return m_bForceProfile; }

	// set functions
	void SetScriptType		( String str ) { m_sScriptType = str; }
	void SetAnotherMod ( String name, String author ) { m_sOtherName = name; m_sOtherAuthor = author; }
	void SetPatch ( bool p ) { m_bPatch = p; }
	void SetForceProfile ( bool p ) { m_bForceProfile = p; }
	void SetParent ( CSpkFile *file ) { m_pParent = file; }

	void SetCustomStart ( bool yes ) { m_bCustomStart = yes; if ( yes ) m_bPackageUpdate = false; }
	void SetPackageUpdate ( bool yes ) { m_bPackageUpdate = yes; if ( yes ) m_bCustomStart = false; }

	CSpkFile();
	virtual ~CSpkFile();

	bool IsMatchingMod ( String mod );

	/// reading of files
	virtual bool ReadFile ( String filename, int readType = SPKREAD_ALL, CProgressInfo *progress = NULL );
	virtual void ReadValues ( String values );
	virtual void ReadFiles ( String values );
	virtual bool ReadFileToMemory ( CFile *file );
	virtual bool ReadFile ( FILE *id, int readtype, CProgressInfo *progress );
	virtual bool ParseValueLine ( String line );

	// writing of files
	String CreateValuesLine ();
	bool WriteData ( FILE *id, CProgressInfo * = NULL );
	bool WriteFile ( String filename, CProgressInfo * = NULL );
	virtual bool ExtractFile ( CFile *file, String dir, bool includedir = true, CProgressInfo *progress = NULL );
	virtual bool ExtractFile ( int file, String dir, bool includedir = true, CProgressInfo *progress = NULL );
	virtual bool ExtractAll ( String dir, bool includedir = true, CProgressInfo *progress = NULL );

	SWares *FindWare ( String id );
	void AddWare ( String  );
	void AddWareText ( String );
	void AddWare ( SWares * );
	void AddWareText ( SWares *, int, String, String );
	void RemoveWare ( String );
	void RemoveWareText ( String, int );
	void ClearWareText ( SWares * );
	void ClearWareText ( String );
	void ClearWares ();

	int CheckValidCustomStart ();

	bool UpdateSigned ();

	SSettingType *AddSetting ( String key, int type );
	void ClearSettings ();
	void ConvertSetting ( SSettingType *t, String set );
	String GetSetting ( SSettingType *t );

	// installer usage
	bool IsUpdateChecked () { return m_bUpdate; }
	String GetLastReadme () { return m_sLastReadme; }
	String GetCustomMap () { return m_sCustomMap; }

	void SetCustomMap ( String map ) { m_sCustomMap = map; }
	void SetLastReadme ( String r ) { m_sLastReadme = r; }
	void SetUpdateChecked ( bool en ) { m_bUpdate = en; }

	virtual int GetType () { return TYPE_SPK; }

protected:
	virtual void Delete ();
	virtual void SetDefaults ();

	// reading of files
	virtual bool ParseHeader ( String header );
	virtual bool ParseFileHeader ( String header );
	virtual bool ParseFilesLine ( String line );


	// varibles

	String m_sOtherAuthor;
	String m_sOtherName;
	String m_sScriptType;

	bool m_bPackageUpdate;
	bool m_bCustomStart;
	bool m_bPatch;
	bool m_bForceProfile;

	CSpkFile *m_pParent;

	CLinkList<SWares> m_lWares;
	CLinkList<SSettingType> m_lSettings;


	SWares *m_pLastWare;

	// installer varibles
	bool m_bUpdate;
	String m_sLastReadme;
	String m_sCustomMap;
};

#endif // !defined(AFX_SPKFILE_H__E35FA619_B901_479F_BE42_2FF1519BA4D3__INCLUDED_)

