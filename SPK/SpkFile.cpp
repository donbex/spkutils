// SpkFile.cpp: implementation of the CSpkFile class.
//
//////////////////////////////////////////////////////////////////////

#include "SpkFile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSpkFile::CSpkFile() : CBaseFile ()
{
	SetDefaults ();

	m_iType = TYPE_SPK;
}

CSpkFile::~CSpkFile()
{
	Delete ();
}

/*
	Func:   SetDefaults
	Desc:   Sets the  default values when class is created
*/
void CSpkFile::SetDefaults ()
{
	m_bCustomStart = false;
	m_pLastWare = NULL;
	m_pParent = NULL;
	m_bPatch = false;

	m_bUpdate = m_bForceProfile = m_bPackageUpdate = false;

	CBaseFile::SetDefaults ();
}
void CBaseFile::SetDefaults ()
{
	m_pIconFile = NULL;

	m_SHeader.iValueCompression = SPKCOMPRESS_ZLIB;
	m_SHeader2.iFileCompression = SPKCOMPRESS_ZLIB;
#ifdef _WIN32
	m_SHeader2.iDataCompression = SPKCOMPRESS_7ZIP;
#else
	m_SHeader2.iDataCompression = SPKCOMPRESS_ZLIB;
#endif

	ClearError();

	m_iGameVersion = 0;

	m_bSigned = false;
	m_bEnable = m_bGlobal = m_bProfile = true;
}

CBaseFile::CBaseFile()
{
	SetDefaults ();

	m_iType = TYPE_BASE;
}
CBaseFile::~CBaseFile()
{
	Delete();
}

void CSpkFile::Delete ()
{

	for ( SWares *itw = m_lWares.First(); itw; itw = m_lWares.Next() )
		delete itw;
	m_lWares.clear();

	for ( SNames *itn = m_lNames.First(); itn; itn = m_lNames.Next() )
		delete itn;
	m_lNames.clear();

	for ( SSettingType *st = m_lSettings.First(); st; st = m_lSettings.Next() )
		delete st;
	m_lSettings.clear();

	CBaseFile::Delete ();
}
void CBaseFile::Delete ()
{
	for ( CFile *itf = m_lFiles.First(); itf; itf = m_lFiles.Next() )
		delete itf;
	m_lFiles.clear();

	if ( m_pIconFile )
	{
		delete m_pIconFile;
		m_pIconFile = NULL;
	}

	for ( SInstallText *itext = m_lInstallText.First(); itext; itext = m_lInstallText.Next() )
		delete itext;
	m_lInstallText.clear();

	for ( SInstallText *itext2 = m_lUninstallText.First(); itext2; itext2 = m_lUninstallText.Next() )
		delete itext2;
	m_lUninstallText.clear();
}

String CBaseFile::GetLanguageName ( String lang )
{
	for ( SNames *n = m_lNames.First(); n; n = m_lNames.Next() )
	{
		if ( n->sLanguage == lang )
			return n->sName;
	}
	return m_sName;
}


/*
##########################################################################################
##################                Base Class Functions                  ##################
##########################################################################################
*/


int CBaseFile::CheckFile ( String filename, float *version )
{
	FILE *id = fopen ( filename.c_str(), "rb" );
	if ( !id )
		return false;

	String line = GetEndOfLine ( id, NULL, false );
	String type = line.GetToken ( 1, ';' );
	fclose ( id );
	if ( version )
		*version = line.GetToken ( 2, ';' ).ToFloat();

	if ( type == "SPKCycrow" )
		return SPKFILE_SINGLE;
	if ( type == "MSPKCycrow" )
		return SPKFILE_MULTI;
	return SPKFILE_INVALID;
}

String CBaseFile::GetNameValidFile ()
{
	String name = m_sName;
	name.RemoveChar ( ':' );
	name.RemoveChar ( '/' );
	name.RemoveChar ( '\\' );
	name.RemoveChar ( '*' );
	name.RemoveChar ( '?' );
	name.RemoveChar ( '"' );
	name.RemoveChar ( '<' );
	name.RemoveChar ( '>' );
	name.RemoveChar ( '|' );
	return name;
}

bool CBaseFile::AnyFileType ( int type )
{
	for ( CFile *f = m_lFiles.First(); f; f = m_lFiles.Next() )
	{
		if ( f->GetFileType() == type )
			return true;
	}

	return false;
}

void CBaseFile::AddFile ( CFile *file )
{
	for ( CFile *f = m_lFiles.First(); f; f = m_lFiles.Next() )
	{
		if ( f->GetFileType() != file->GetFileType() )
			continue;
		if ( f->GetName() != file->GetName () )
			continue;
		if ( f->GetDir() != file->GetDir() )
			continue;

		// must already exist, delete this one
		CFile *oldfile = m_lFiles.CurrentData();
		m_lFiles.RemoveCurrent ();
		if ( oldfile )
			delete oldfile;
		break;
	}

	m_lFiles.push_back ( file );
}

CFile *CBaseFile::AddFile ( String file, String dir, int type )
{
	CFile *newfile = new CFile ( file );
	newfile->SetDir ( dir );
	newfile->SetFileType ( type );

	// first check if the file already exists
	for ( CFile *f = m_lFiles.First(); f; f = m_lFiles.Next() )
	{
		if ( f->GetFileType() != newfile->GetFileType() )
			continue;
		if ( f->GetName() != newfile->GetName () )
			continue;
		if ( f->GetDir() != newfile->GetDir() )
			continue;

		// must already exist, delete this one
		m_lFiles.RemoveCurrent ();
		break;
	}

	m_lFiles.push_back ( newfile );

	return newfile;
}

bool CBaseFile::AddFileNow ( String file, String dir, int type, CProgressInfo *progress )
{
	CFile *f = AddFile ( file, dir, type );
	f->ReadFromFile ();

	// compress the file
	return f->CompressData ( m_SHeader2.iDataCompression, progress );
}

CFile *CBaseFile::AppendFile ( String file, int type, String dir, CProgressInfo *progress )
{
	CFile *newfile = AddFile ( file, dir, type );
	if ( !newfile )
		return NULL;


	// read the file into memory
	if ( newfile->ReadFromFile () )
	{
		// now compress the file
		if ( newfile->CompressData ( m_SHeader2.iDataCompression, progress ) )
			return newfile;
	}
	else if ( newfile->GetLastError() == SPKERR_MALLOC )
	{
		if ( newfile->CompressFile ( progress ) )
			return newfile;
	}

	m_lFiles.pop_back ();
	delete newfile;

	return NULL;
}

CFile *CBaseFile::FindFileAt ( int filetype, int pos )
{
	int count = 0;
	for ( CFile *file = m_lFiles.First(); file; file = m_lFiles.Next() )
	{
		if ( file->GetFileType() != filetype )
			continue;

		if ( count == pos )
			return file;

		++count;
	}

	return NULL;
}

CFile *CBaseFile::FindFile ( String filename, int type, String dir )
{
	String lfile = filename.ToLower();
	lfile = lfile.FindReplace ( "\\", "/" );

	lfile = lfile.GetToken ( lfile.NumToken('/'), '/' );

	CListNode<CFile> *node = m_lFiles.Front();
	while ( node )
	{
		CFile *f = node->Data();
		node = node->next();

		if ( type != f->GetFileType() )
			continue;
		if ( dir != f->GetDir() )
			continue;
		if ( f->GetName().ToLower() == lfile )
			return f;
	}
	return NULL;
}

bool CBaseFile::RemoveFile ( String file, int type )
{
	CFile *f = FindFile (file, type);
	if ( !f ) 
		return false;
	return RemoveFile ( f );
}

bool CBaseFile::RemoveFile ( CFile *file )
{
	int count = 0;
	for ( CFile *f = m_lFiles.First(); f; f = m_lFiles.Next(), count++ )
	{
		if ( f == file )
			return RemoveFile ( count );
	}
	return false;
}

bool CBaseFile::RemoveFile ( int pos )
{
	if ( (pos < 0) || (pos >= m_lFiles.size()) )
		return false;

	CFile *file = m_lFiles.Get ( pos );
	m_lFiles.erase ( pos + 1 );

	if ( file )
		delete file;

	return true;
}


void CBaseFile::RemoveAllFiles ( int type )
{
	CFile *file;
	for ( file = m_lFiles.First(); file; file = m_lFiles.Next() )
	{
		if ( (type == -1) || (file->GetFileType() == type) )
		{
			m_lFiles.RemoveCurrent ();
			delete file;
		}
	}

	file = m_lFiles.First();
	if ( (type == -1) || (file->GetFileType() == type) )
	{
		m_lFiles.remove ( file );
		delete file;
	}
}

void CBaseFile::CompressAllFiles ( int type )
{
	for ( CFile *file = m_lFiles.First(); file; file = m_lFiles.Next() )
		file->CompressData ( type );
}

bool CBaseFile::UncompressAllFiles ( CProgressInfo *progress )
{
	for ( CFile *fit = m_lFiles.First(); fit; fit = m_lFiles.Next() )
	{
		if ( progress )
			progress->DoingFile ( fit );

		bool uncomprToFile = false;
		if ( !fit->UncompressData ( progress ) )
		{
			if ( fit->GetCompressionType() == SPKCOMPRESS_7ZIP )
			{
				if ( !fit->UncompressToFile ( "temp", this, false, progress ) )
					return false;
				else
				{
					uncomprToFile = true;
					fit->SetFullDir ( "temp" );
				}
			}

			if ( !uncomprToFile )
				return false;
		}
	}
	return true;
}

long CBaseFile::GetFullFileSize()
{
	long fullsize = 1000;

	for ( CFile *checkfile = m_lFiles.First(); checkfile; checkfile = m_lFiles.Next() )
		fullsize += checkfile->GetUncompressedDataSize();

	if ( m_pIconFile )
		fullsize += m_pIconFile->GetUncompressedDataSize();

	return fullsize;
}


/*
	Func:   GetEndOfLine
	Input:  id - The file id for the current file to read from
	        line - Pointed to hold the line number thats read
			upper - true if it converts to uppercase
	Return: String - the string it has read
	Desc:   Reads a string from a file, simlar to readLine() classes, reads to the end of the line in a file
*/
String CBaseFile::GetEndOfLine ( FILE *id, int *line, bool upper )
{
	String word;

	char c = fgetc ( id );
	if ( c == -1 )
		return "";

	while ( (c != 13) && (!feof(id)) && (c != '\n') )
	{
		word += c;
		c = fgetc ( id );
	}

	if ( line )
		++(*line);

	if ( upper )
		return word.ToUpper();

	return word;
}

int CBaseFile::CountFiles ( int filetype )
{
	int i = 0;
	for ( CFile *file = m_lFiles.First(); file; file = m_lFiles.Next() )
	{
		if ( file->GetFileType() != filetype )
			continue;
		++i;
	}

	return i;
}

void CBaseFile::ClearNames ()
{
	SNames *n;
	for ( n = m_lNames.First(); n; n = m_lNames.Next() )
		delete n;
	m_lNames.clear();
}
void CBaseFile::AddLanguageName ( String lang, String name )
{
	// first check for an existing language
	SNames *n;
	for ( n = m_lNames.First(); n; n = m_lNames.Next() )
	{
		if ( n->sLanguage == lang )
		{
			n->sName = name;
			return;
		}
	}

	// not found, add a new entry
	n = new SNames;
	n->sLanguage = lang;
	n->sName = name;
	m_lNames.push_back ( n );
}

/*
	Func:   AddInstallText
	Input:	before - true for before text, false for after text
			install - true to add to install text list, false for unisntall text list
			lang - string containing the language to use
			data - The text to add for the language
	Return: install text - Returns the text struct that it used to add
	Desc:   Adds the text to list, adds either install or uninstall text
			If language already exists, then its overwritten
			Allows adding before and after seperatly to the same entry
*/
SInstallText *CBaseFile::AddInstallText ( bool before, bool install, String lang, String data )
{
	CLinkList<SInstallText> *list = NULL;
	if ( install )
		list = &m_lInstallText;
	else
		list = &m_lUninstallText;


	SInstallText *unist = NULL;
	// default language, use this when no other matching language available
	if ( lang == "0" )
	{
		unist = list->First();
		if ( !unist )
		{
			unist = new SInstallText;
			unist->sLanguage = lang;
			list->push_front ( unist );
		}
	}
	else
	{
		// check if language already exists and overright if needed
		for ( SInstallText *u = list->First(); u; u = list->Next() )
		{
			if ( u->sLanguage == lang )
			{
				unist = u;
				break;
			}
		}
		
		if ( !unist )
		{
			unist = new SInstallText;
			unist->sLanguage = lang;
			list->push_back ( unist );
		}
	}

	if ( !before )
		unist->sAfter = data;
	else
		unist->sBefore = data;

	return unist;
}

SInstallText *CBaseFile::FindInstallText ( bool install, String lang )
{
	CLinkList<SInstallText> *list = NULL;
	if ( install )
		list = &m_lInstallText;
	else
		list = &m_lUninstallText;

	for ( SInstallText *it = list->First(); it; it = list->Next() )
	{
		if ( it->sLanguage == lang )
			return it;
	}
	return NULL;
}

void CBaseFile::AddInstallText ( SInstallText *add )
{
	SInstallText *it = FindInstallText ( true, add->sLanguage );
	if ( it == add )
		return;

	if ( it )
	{
		m_lInstallText.remove ( it );
		delete it;
	}
	m_lInstallText.push_back ( add );
}

void CBaseFile::AddUninstallText ( SInstallText *add )
{
	SInstallText *it = FindInstallText ( false, add->sLanguage );
	if ( it == add )
		return;

	if ( it )
	{
		m_lUninstallText.remove ( it );
		delete it;
	}
	m_lUninstallText.push_back ( add );
}

void CBaseFile::RemoveInstallText ( bool install, String lang )
{
	SInstallText *it = FindInstallText ( install, lang );
	if ( it )
	{
		m_lUninstallText.remove ( it );
		delete it;
	}
}

bool CBaseFile::IsThereInstallText ( bool install )
{
	CLinkList<SInstallText> *list = NULL;
	if ( install )
		list = &m_lInstallText;
	else
		list = &m_lUninstallText;

	if ( list->size() > 1 )
		return true;
	if ( list->size() <= 0 )
		return false;

	SInstallText *it = list->First();
	if ( !it->sAfter.Empty() )
		return true;
	if ( !it->sBefore.Empty() )
		return true;
	return false;
}

String CBaseFile::GetBeforeText ( CLinkList<SInstallText> *list, String lang )
{
	String beforetext;
	for ( SInstallText *u = list->First(); u; u = list->Next() )
	{
		if ( (beforetext.Empty()) && ((u->sLanguage == "Default") || (u->sLanguage == "0")) )
			beforetext = u->sBefore;

		if ( (u->sLanguage == lang) && (!u->sBefore.Empty()) )
			beforetext = u->sBefore;
	}
	return beforetext;
}

String CBaseFile::GetAfterText ( CLinkList<SInstallText> *list, String lang )
{
	String text;
	for ( SInstallText *u = list->First(); u; u = list->Next() )
	{
		if ( (u->sLanguage == lang) && (!u->sAfter.Empty()) )
			text = u->sAfter;

		if ( (text.Empty()) && ((u->sLanguage == "Default") || (u->sLanguage == "0")) )
			text = u->sAfter;

	}
	return text;
}

/*
	Func:   CreateFilesLine
	Return: String - returns the full string for files list
	Desc:   Creates a signle line list of all the files
*/
String CBaseFile::CreateFilesLine ( bool updateheader, CProgressInfo *progress )
{
	String line;

	if ( updateheader )
	{
		m_SHeader2.iNumFiles = 0;
		m_SHeader2.lFullSize = 0;
	}

	if ( m_pIconFile )
	{
		// no data, read it from file
		if ( !m_pIconFile->GetData() )
			m_pIconFile->ReadFromFile ();

		// compress the file
		if ( m_pIconFile->CompressData ( m_SHeader2.iDataCompression, progress ) )
		{
			line += String("Icon:") + (m_pIconFile->GetDataSize() + (long)4) + ":" + m_pIconFile->GetUncompressedDataSize() + ":" + (long)m_SHeader2.iDataCompression + ":" + m_sIconExt + "\n";

			if ( updateheader )
			{
				++m_SHeader2.iNumFiles;
				m_SHeader2.lFullSize += (m_pIconFile->GetDataSize() + 4);
			}
		}
	}

	for ( CFile *file = m_lFiles.First(); file; file = m_lFiles.Next() )
	{
		if ( progress )
			progress->DoingFile ( file );

		// no data, read it from file
		if ( !file->GetData() )
		{
			if ( !file->ReadFromFile () )
			{
				if ( file->GetLastError() == SPKERR_MALLOC )
				{
					if ( !file->CompressFile ( progress ) )
						continue;
				}
			}
		}

		if ( !file->GetData() )
			continue;

		// compress the file
		if ( !file->CompressData ( m_SHeader2.iDataCompression, progress ) )
			continue;

		String command = GetFileTypeString ( file->GetFileType() );
		if ( command.Empty() )
			continue;

		if ( file->IsShared() )
			command = String("$") + command;

		if ( file->GetDir().Empty() )
			line += (command + ":" + (file->GetDataSize() + (long)4) + ":" + file->GetUncompressedDataSize() + ":" + (long)m_SHeader2.iDataCompression + ":" + (long)file->GetCreationTime() + ":" + ((file->IsCompressedToFile()) ? "1" : "0") + ":" + file->GetFilename() + "\n");
		else
			line += (command + ":" + (file->GetDataSize() + (long)4) + ":" + file->GetUncompressedDataSize() + ":" + (long)m_SHeader2.iDataCompression + ":" + (long)file->GetCreationTime() + ":" + ((file->IsCompressedToFile()) ? "1" : "0") + ":" + file->GetFilename() + ":" + file->GetDir() + "\n");

		if ( updateheader )
		{
			++m_SHeader2.iNumFiles;
			m_SHeader2.lFullSize += (file->GetDataSize() + 4);
		}
	}

	return line;
}


/*
######################################################################################
##########							Reading Functions			    		##########
######################################################################################
*/

void CBaseFile::ReadAllFilesToMemory ()
{
	// no file to read from
	if ( m_sFilename.Empty() )
		return;

	// now open the file
	FILE *id = fopen ( m_sFilename.c_str(), "rb" );
	if ( !id )
		return;

	// read the header
	GetEndOfLine ( id, NULL, false );
	// skip past values
	fseek ( id, 4, SEEK_CUR );
	fseek ( id, m_SHeader.lValueCompressSize, SEEK_CUR );

	// read the next header
	GetEndOfLine ( id, NULL, false );
	// skip past files
	fseek ( id, 4, SEEK_CUR );
	fseek ( id, m_SHeader2.lSize, SEEK_CUR );

	if ( m_pIconFile )
	{
		if ( (!m_pIconFile->GetData()) && (!m_pIconFile->Skip()) )
			m_pIconFile->ReadFromFile ( id, m_pIconFile->GetDataSize() );
		else
		{
			fseek ( id, 4, SEEK_CUR );
			fseek ( id, m_pIconFile->GetDataSize(), SEEK_CUR );
		}
	}

	for ( CFile *fit = m_lFiles.First(); fit; fit = m_lFiles.Next() )
	{
		if ( (!fit->GetData()) && (!fit->Skip()) )
			fit->ReadFromFile ( id, fit->GetDataSize() );
		else
		{
			fseek ( id, 4, SEEK_CUR );
			fseek ( id, fit->GetDataSize(), SEEK_CUR );
		}
	}

	fclose ( id );
}

bool CBaseFile::InstallFiles ( String destdir, CProgressInfo *progress, CLinkList<CFile> *filelist, bool enabled )
{
	// first rename any fake patches
	CFile *fit;
	if ( enabled )
	{
		int startfake = 1;
		while ( startfake < 99 )
		{
			String filename = destdir;
			if ( !filename.Empty() )
				filename += "/";
			if ( startfake < 10 )
				filename += "0";
			filename += (long)startfake;
			filename += ".cat";

			FILE *id = fopen ( filename.c_str(), "rb+" );
			if ( !id )
				break;
			fclose ( id );
			startfake++;
		}

		for ( fit = m_lFiles.First(); fit; fit = m_lFiles.Next() )
		{
			// only do fake patchs
			if ( !fit->IsFakePatch() )
				continue;

			if ( !fit->CheckFileExt ("cat") )
				continue;

			String newname;
			if ( startfake < 10 )
				newname += "0";
			newname += (long)startfake;

			// now finding matching file
			CListNode<CFile> *node = m_lFiles.Front();
			while ( node )
			{
				CFile *fit2 = node->Data();
				node = node->next();

				if ( fit2 == fit )
					continue;
				if ( !fit2->IsFakePatch() )
					continue;
				if ( !fit2->CheckFileExt ("dat") )
					continue;
				fit2->SetName ( newname + ".dat" );
				break;
			}
			fit->SetName ( newname + ".cat" );
			++startfake;
		}
	}

	for ( fit = m_lFiles.First(); fit; fit = m_lFiles.Next() )
	{
		bool fileEnabled = enabled;
		if ( !fileEnabled )
		{
			if ( (fit->GetFileType() == FILETYPE_UNINSTALL) || (fit->GetFileType() == FILETYPE_README) )
				fileEnabled = true;
			else if ( (fit->GetFileType() == FILETYPE_EXTRA) && (fit->GetDir().Left(7).ToLower() == "Extras/") )
				fileEnabled = true;
			else if ( (IsPatch()) && (fit->GetFileType() == FILETYPE_MOD) && (!fit->IsFakePatch()) )
				fileEnabled = true;
		}

		if ( progress )
			progress->DoingFile ( fit );

		// first uncompress the file
		bool uncomprToFile = false;
		m_sLastError = fit->GetNameDirectory(this);
		m_iLastError = SPKERR_UNCOMPRESS;
		if ( !fit->UncompressData ( progress ) )
		{
			if ( fit->GetCompressionType() == SPKCOMPRESS_7ZIP )
			{
				if ( !fit->UncompressToFile ( NullString, this, false, progress ) )
					return false;
				else
					uncomprToFile = true;
			}

			if ( !uncomprToFile )
				return false;
		}
		ClearError ();
		bool dofile = true;

		// new check if we should install the file
		// first get the version
		if ( fit->ReadScriptVersion () )
		{
			CFile checkfile;
			String checkfilename = destdir;
			if ( !checkfilename.Empty() )
				checkfilename += "/";
			checkfilename += fit->GetNameDirectory(this);
			checkfile.SetFilename ( checkfilename );
			checkfile.SetFileType ( fit->GetFileType() );
			if ( checkfile.CheckValidFilePointer() )
			{
				if ( checkfile.ReadScriptVersion() > fit->GetVersion() )
					dofile = false;
			}
		}

		// change file pointer
		String filename = destdir;
		if ( !filename.Empty() )
			filename += "/";
		if ( (IsPatch()) && (fit->GetFileType() == FILETYPE_MOD) )
			fit->SetDir ( String("Patch") );
		fit->SetFilename ( filename + fit->GetNameDirectory(this) );
		if ( !fileEnabled )
			fit->SetFullDir ( fit->GetFullDir() + "/Disabled" );

		CFile *adjustPointer = NULL;

		bool checkFile = true;
		if ( fit->GetFileType() == FILETYPE_README )
			checkFile = false;

		if ( filelist )
		{
			CFile *cfile = NULL;
			if ( checkFile )
			{
				if ( !fit->IsFakePatch() )
				{
					for ( cfile = filelist->First(); cfile; cfile = filelist->Next() )
					{
						if ( !cfile->MatchFile ( fit ) )
							continue;

						if ( !cfile->CompareNew ( fit ) )
							dofile = false;
						break;
					}
				}
			}

			if ( !cfile )
				filelist->push_back ( fit );
			else
			{
				// found a file, check if its in the disabled directory
				String dir = cfile->GetFilePointer();
				dir = dir.GetToken ( 1, dir.NumToken ('/') - 1, '/' );
				String lastDir = dir.GetToken ( dir.NumToken('/'), '/' ).ToLower();

				if ( ((lastDir == "disabled") || (dir.ToLower().IsIn ("/disabled/"))) && (enabled) )
				{
					rename ( cfile->GetFilePointer().c_str(), String(dir.GetToken ( 1, dir.NumToken ('/') - 1, '/' ) + "/" + cfile->GetName()).c_str() );
					cfile->SetFilename ( String(dir.GetToken ( 1, dir.NumToken ('/') - 1, '/' ) + "/" + cfile->GetName()) );
				}

				adjustPointer = cfile;
				if ( dofile )
					adjustPointer->SetCreationTime ( fit->GetCreationTime() );
			}
		}

		if ( dofile )
		{
			// uncompressed to file, rename and move
			if ( uncomprToFile )
			{
				m_iLastError = SPKERR_WRITEFILE;
				String to = destdir;
				if ( !to.Empty() )
					to += "/";
				to += fit->GetNameDirectory(this);
				CreateDirectory ( to.GetToken ( 1, to.NumToken ('/') - 1, '/' ) );

				int err = 1;
				m_sLastError = to;
				if ( !fit->GetTempFile ().Empty() ) 
					err = rename ( fit->GetTempFile().c_str(), to.c_str() );
				if ( err )
					return false;
			}
			//otherwise, just extract the file
			else
			{
				// old file is found in list, switch to using new one
				if ( (filelist) && (adjustPointer) )
				{
					filelist->Switch ( adjustPointer, fit );
					adjustPointer = NULL;
				}

				String fpointer = fit->GetFilePointer();
				m_iLastError = SPKERR_CREATEDIRECTORY;
				String dir = fit->GetFilePointer().GetToken ( 0, fit->GetFilePointer().NumToken('/') - 1, '/' );
				m_sLastError = dir;
				if ( !dir.IsIn ( "::" ) )
				{
					if ( !CreateDirectory ( dir ) )
						return false;
				}

				m_iLastError = SPKERR_WRITEFILE;
				m_sLastError = fit->GetFilePointer();
				if ( !fit->WriteFilePointer() )
					return false;
			}
			ClearError ();
		}

		if ( adjustPointer )
		{
			m_lFiles.ChangeCurrent ( adjustPointer );
			delete fit;
		}
	}

	// now clear or data memory
	for ( fit = m_lFiles.First(); fit; fit = m_lFiles.Next() )
		fit->DeleteData();

	return true;
}



/*######################################################################################################*/


/*
	Func:   ParseHeader
	Input:  Header String - string formated directly from the file
	Return: Boolean - If string is a valid header
	Desc:   Splits up the main header string to get all required settings
*/
bool CSpkFile::ParseHeader ( String header )
{
	if ( header.GetToken ( 1, ';' ) != "SPKCycrow" )
		return false;

	m_SHeader.fVersion = header.GetToken ( 2, ';' ).ToFloat();
	if ( m_SHeader.fVersion > FILEVERSION )
		return false;

	m_SHeader.iValueCompression = header.GetToken ( 3, ';' ).ToInt();
	m_SHeader.lValueCompressSize = header.GetToken ( 4, ';' ).ToLong();

	return true;
}

/*
	Func:   ParseFileHeader
	Input:  Header String - string formated directly from the file
	Return: Boolean - If string is a valid header
	Desc:   Splits up the file header string to get all required settings
*/
bool CSpkFile::ParseFileHeader ( String header )
{
	if ( header.GetToken ( 1, ';' ) != "FileHeader" )
		return false;

	m_SHeader2.iNumFiles = header.GetToken ( 2, ';' ).ToInt();
	m_SHeader2.lSize = header.GetToken ( 3, ';' ).ToInt();
	m_SHeader2.lFullSize = header.GetToken ( 4, ';' ).ToInt();
	m_SHeader2.iFileCompression = header.GetToken ( 5, ';' ).ToInt();
	m_SHeader2.iDataCompression = header.GetToken ( 6, ';' ).ToInt();

	return true;
}


/*
	Func:   ParseValueLine
	Input:  String - single line from a file to set
	Return: Boolean - returns true if value exists
	Desc:   Reads the line and assigns the parameters for the file
*/
bool CBaseFile::ParseValueLine ( String line )
{
	String first = line.GetToken ( 1, ' ' );
	String rest  = line.GetToken ( 2, -1, ' ' );

	if ( first == "Name:" )
		m_sName = rest;
	else if ( first == "Author:" )
		m_sAuthor = rest;
	else if ( first == "Version:" )
		m_sVersion = rest;
	else if ( first == "GameVersion:" )
		m_iGameVersion = rest.ToInt();
	else if ( first == "Date:" )
		m_sCreationDate = rest;
	else if ( first == "WebAddress:" )
		m_sWebAddress = rest;
	else if ( first == "WebSite:" )
		m_sWebSite = rest;
	else if ( first == "Email:" )
		m_sEmail = rest;
	else if ( first == "WebMirror1:" )
		m_sWebMirror1 = rest;
	else if ( first == "WebMirror2:" )
		m_sWebMirror2 = rest;
	else if ( first == "Desc:" )
		m_sDescription = rest;
	else if ( first == "UninstallAfter:" )
		AddUninstallAfterText ( rest.GetToken ( 1, '|' ), rest.GetToken ( 2, -1, '|' ) );
	else if ( first == "UninstallBefore:" )
		AddUninstallBeforeText ( rest.GetToken ( 1, '|' ), rest.GetToken ( 2, -1, '|' ) );
	else if ( first == "InstallAfter:" )
		AddInstallAfterText ( rest.GetToken ( 1, '|' ), rest.GetToken ( 2, -1, '|' ) );
	else if ( first == "InstallBefore:" )
		AddInstallBeforeText ( rest.GetToken ( 1, '|' ), rest.GetToken ( 2, -1, '|' ) );
	else if ( first == "ScriptName:" )
		AddLanguageName ( rest.GetToken ( 1, ':' ),  rest.GetToken ( 2, -1, ':' ) );
	else
		return false;

	return true;
}

/*
	Func:   ParseValueLine
	Input:  String - single line from a file to set
	Return: Boolean - returns true if value exists
	Desc:   Reads the line and assigns the parameters for the file
*/
bool CSpkFile::ParseValueLine ( String line )
{
	String first = line.GetToken ( 1, ' ' );
	String rest  = line.GetToken ( 2, -1, ' ' );

	if ( first == "AnotherMod:" )
	{
		m_sOtherAuthor = rest.GetToken ( 1, '|' );
		m_sOtherName = rest.GetToken ( 2, -1, '|' );
	}
	else if ( line == "CustomStart" )
		m_bCustomStart = true;
	else if ( line == "PackageUpdate" )
		m_bPackageUpdate = true;
	else if ( line == "Patch" )
		m_bPatch = true;
	else if ( line == "ForceProfile" )
		m_bForceProfile = true;
	else if ( first == "ScriptType:" )
		m_sScriptType = rest;
	else if ( first == "Ware:" )
		AddWare ( rest );
	else if ( (first == "WareText:") && (m_pLastWare) )
		AddWareText ( rest );
	else if ( first == "Setting:" )
	{
		SSettingType *t = AddSetting ( rest.GetToken ( 2, '|' ), rest.GetToken ( 1, '|' ).ToInt() );
		ConvertSetting ( t, rest.GetToken ( 3, -1, '|' ) );
	}
	else
		return CBaseFile::ParseValueLine ( line );

	return true;
}

/*
	Func:   ReadValues
	Input:  String - values in one long line
	Desc:   splits the values data into each line to read the data
*/
void CSpkFile::ReadValues ( String values )
{
	int num = 0;
	String *lines = values.SplitToken ( '\n', &num );

	for ( int i = 0; i < num; i++ )
		ParseValueLine ( lines[i] );
}

/*
	Func:   ParseFilesLine
	Input:  String - single line from a file to set
	Return: Boolean - returns true if value exists
	Desc:   Reads the line and assigns the parameters for the file
*/
bool CSpkFile::ParseFilesLine ( String line )
{
	if ( !line.IsIn(":") )
		return false;

	String command = line.GetToken ( 1, ':' );
	
	long size = line.GetToken ( 2, ':').ToInt ();
	long usize = line.GetToken ( 3, ':').ToInt ();
	long compression = line.GetToken ( 4, ':').ToInt ();

	if ( command == "Icon" )
	{
		m_sIconExt = line.GetToken ( 5, ':' );
		m_pIconFile = new CFile ();
		m_pIconFile->SetDataSize ( size - 4 );
		m_pIconFile->SetDataCompression ( compression );
		m_pIconFile->SetUncompressedDataSize ( usize );

		return true;
	}

	time_t time = line.GetToken ( 5,':' ).ToLong();
	bool compressToFile = (line.GetToken ( 6, ':').ToInt() == 1) ? true : false;
	String name  = line.GetToken ( 7, ':' );
	String dir = line.GetToken ( 8, ':' );

	if ( name.Empty() )
		return true;

	bool shared = false;
	if ( command.Left(1) == "$" )
	{
		shared = true;
		command.Erase ( 0, 1 );
	}

	int type = -1;
	if ( command == "Script" )
		type = FILETYPE_SCRIPT;
	else if ( command == "Text" )
		type = FILETYPE_TEXT;
	else if ( command == "Readme" )
		type = FILETYPE_README;
	else if ( command == "Map" )
		type = FILETYPE_MAP;
	else if ( command == "Mod" )
		type = FILETYPE_MOD;
	else if ( command == "Uninstall" )
		type = FILETYPE_UNINSTALL;
	else if ( command == "Sound" )
		type = FILETYPE_SOUND;
	else if ( command == "Mission" )
		type = FILETYPE_MISSION;
	else if ( command == "Extra" )
		type = FILETYPE_EXTRA;
	else if ( command == "Screen" )
		type = FILETYPE_SCREEN;
	else if ( command == "Backup" )
		type = FILETYPE_BACKUP;

	if ( type == -1 )
		return false;

	CFile *file = new CFile ();
	file->SetFileType ( type );
	file->SetCreationTime ( time );
	file->SetName ( name );
	file->SetDir ( dir );
	file->SetDataSize ( size - 4 );
	file->SetDataCompression ( compression );
	file->SetUncompressedDataSize ( usize );
	file->SetShared ( shared );
	file->SetCompressedToFile ( compressToFile );

	m_lFiles.push_back ( file );

	return true;
}


/*
	Func:   ParseFiles
	Input:  String - values in one long line
	Desc:   splits the files data into each line to read the data
*/
void CSpkFile::ReadFiles ( String values )
{
	int num = 0;
	String *lines = values.SplitToken ( '\n', &num );

	for ( int i = 0; i < num; i++ )
		ParseFilesLine ( lines[i] );
}



/*
	Func:   ReadFile
	Input:  filename - the name of the file to open and read
			readdata - If falses, dont read the files to memory, just read the headers and values
	Return: boolean - return ture if acceptable format
	Desc:   Opens and reads the spk file and loads all data into class
*/
bool CSpkFile::ReadFile ( String filename, int readtype, CProgressInfo *progress )
{
	FILE *id = fopen ( filename.c_str(), "rb" );
	if ( !id )
		return false;

	bool ret = ReadFile ( id, readtype, progress );
	if ( ret )
		m_sFilename = filename;

	fclose ( id );

	return ret;
}
bool CSpkFile::ReadFile ( FILE *id, int readtype, CProgressInfo *progress )
{
	ClearError ();

	// first read the header
	if ( !ParseHeader ( GetEndOfLine ( id, NULL, false ) ) )
		return false;

	if ( readtype == SPKREAD_HEADER )
		return true;

	long doneLen = 0;
	// next read the data values for the spk files
	if ( m_SHeader.lValueCompressSize )
	{
		// read data to memory
		unsigned char *readData = new unsigned char[m_SHeader.lValueCompressSize];
		unsigned char size[4];
		fread ( size, 4, 1, id );
		fread ( readData, sizeof(unsigned char), m_SHeader.lValueCompressSize, id );
		unsigned long uncomprLen = (size[0] << 24) + (size[1] << 16) + (size[2] << 8) + size[3];

		// check for zlib compression
		if ( m_SHeader.iValueCompression == SPKCOMPRESS_ZLIB )
		{
			// uncomress the data
			unsigned char *uncompr = new unsigned char[uncomprLen];
			
			int err = uncompress ( uncompr, &uncomprLen, readData, m_SHeader.lValueCompressSize );
			if ( err == Z_OK )
				ReadValues ( String ((char *)uncompr) );
			doneLen = uncomprLen;
			delete uncompr;
		}
		else if ( m_SHeader.iValueCompression == SPKCOMPRESS_7ZIP )
		{
			long len = uncomprLen;
			
			#ifdef _WIN32
			unsigned char *compr = LZMADecodeData ( readData, m_SHeader.lValueCompressSize, len );
			#else
			unsigned char *compr = LZMADecode_C ( readData, m_SHeader.lValueCompressSize, (size_t*)&len, NULL );
			#endif

			if ( compr )
				ReadValues ( String ((char *)compr) );
		}
		// no compression
		else
			ReadValues ( String ((char *)readData) );

		delete readData;
	}

	if ( readtype == SPKREAD_VALUES )
		return true;

	// next should be the next header
	if ( !ParseFileHeader ( GetEndOfLine (id, NULL, false) ) )
		return false;

	// clear the current file list
	for ( CFile *f = m_lFiles.First(); f; f = m_lFiles.Next() )
		delete f;
	m_lFiles.clear();

	if ( m_SHeader2.lSize )
	{
		unsigned char *readData = new unsigned char[m_SHeader2.lSize];
		unsigned char size[4];
		fread ( size, 4, 1, id );
		fread ( readData, sizeof(char), m_SHeader2.lSize, id );

		unsigned long uncomprLen = (size[0] << 24) + (size[1] << 16) + (size[2] << 8) + size[3];
		// check for zlib compression
		if ( m_SHeader.iValueCompression == SPKCOMPRESS_ZLIB )
		{

			if ( uncomprLen < doneLen )
				uncomprLen = doneLen;

			unsigned char *uncompr = new unsigned char[uncomprLen];
			int err = uncompress ( uncompr, &uncomprLen, readData, m_SHeader2.lSize );
			if ( err == Z_OK )
				ReadFiles ( String ((char *)uncompr) );
			delete uncompr;
		}
		else if ( m_SHeader.iValueCompression == SPKCOMPRESS_7ZIP )
		{
			long len = uncomprLen;
			#ifdef _WIN32
			unsigned char *compr = LZMADecodeData ( readData, m_SHeader2.lSize, len );
			#else
			unsigned char *compr = LZMADecode_C ( readData, m_SHeader2.lSize, (size_t*)&len, NULL );
			#endif
			if ( compr )
				ReadFiles ( String ((char *)compr) );
		}
		else
			ReadFiles ( String ((char *)readData) );

		delete readData;
	}

	// file mismatch
	long numfiles = m_lFiles.size();
	if ( m_pIconFile )
		++numfiles;
	if ( m_SHeader2.iNumFiles != numfiles )
	{
		m_iLastError = SPKERR_FILEMISMATCH;
		return false;
	}

	if ( readtype == SPKREAD_ALL )
	{
		if ( m_pIconFile )
			m_pIconFile->ReadFromFile ( id, m_pIconFile->GetDataSize() );

		// ok finally we need to read all the files
		for ( CFile *df = m_lFiles.First(); df; df = m_lFiles.Next() )
			df->ReadFromFile ( id, df->GetDataSize() );
	}

	return true;
}


String CBaseFile::CreateValuesLine ()
{
	String values ( "Name: " );
	values += (m_sName + "\n");
	values += (String("Author: ") + m_sAuthor + "\n");
	values += (String("Version: ") + m_sVersion + "\n");
	if ( !m_sCreationDate.Empty() )
		values += (String("Date: ") + m_sCreationDate + "\n");
	if ( !m_sWebAddress.Empty() )
		values += (String("WebAddress: ") + m_sWebAddress + "\n");
	if ( !m_sWebSite.Empty() )
		values += (String("WebSite: ") + m_sWebSite + "\n");
	if ( !m_sEmail.Empty() )
		values += (String("Email: ") + m_sEmail + "\n");
	if ( !m_sWebMirror1.Empty() )
		values += (String("WebMirror1: ") + m_sWebMirror1 + "\n");
	if ( !m_sWebMirror2.Empty() )
		values += (String("WebMirror2: ") + m_sWebMirror2 + "\n");
	if ( !m_sDescription.Empty() )
		values += (String("Desc: ") + m_sDescription + "\n");
	values += (String("GameVersion: ") + String::Number(m_iGameVersion) + "\n");

	SInstallText *it;
	for ( it = m_lUninstallText.First(); it; it = m_lUninstallText.Next() )
	{
		if ( !it->sAfter.Empty() )
			values += (String("UninstallAfter: ") + it->sLanguage + "|" + it->sAfter + "\n");
		if ( !it->sBefore.Empty() )
			values += (String("UninstallBefore: ") + it->sLanguage + "|" + it->sBefore + "\n");
	}
	for ( it = m_lInstallText.First(); it; it = m_lInstallText.Next() )
	{
		if ( !it->sAfter.Empty() )
			values += (String("InstallAfter: ") + it->sLanguage + "|" + it->sAfter + "\n");
		if ( !it->sBefore.Empty() )
			values += (String("InstallBefore: ") + it->sLanguage + "|" + it->sBefore + "\n");
	}

	for ( SNames *sn = m_lNames.First(); sn; sn = m_lNames.Next() )
		values += String("ScriptName: ") + sn->sLanguage + ":" + sn->sName + "\n";

	return values;
}

/*
	Func:   CreateValuesLine
	Return: String - returns the full string for values
	Desc:   Creates a single string for all values, this is used when compressing to write to the spk file
*/
String CSpkFile::CreateValuesLine ()
{
	String values = CBaseFile::CreateValuesLine ();
	// combine all values together
	if ( (!m_sOtherAuthor.Empty()) && (!m_sOtherName.Empty()) )
		values += (String("AnotherMod: ") + m_sOtherAuthor + "|" + m_sOtherName + "\n");
	if ( m_bCustomStart )
		values += "CustomStart\n";
	if ( m_bPackageUpdate )
		values += "PackageUpdate\n";
	if ( m_bPatch )
		values += "Patch\n";
	if ( m_bForceProfile )
		values += "ForceProfile\n";
	if ( !m_sScriptType.Empty() )
		values += (String("ScriptType: ") + m_sScriptType + "\n");

	for ( SSettingType *st = m_lSettings.First(); st; st = m_lSettings.Next() )
		values += (String("Setting: ") + String::Number(st->iType) + "|" + st->sKey + "|" + GetSetting(st) + "\n");

	for ( SWares *ware = m_lWares.First(); ware; ware = m_lWares.Next() )
	{
		values += String("Ware: ") + ware->cType + ":" + ware->iPrice + ":" + (long)ware->iSize + ":" + (long)ware->iVolumn + ":" + ware->sID + ":" + (long)ware->iNotority + "\n";
		for ( SWaresText *wt = ware->lText.First(); wt; wt = ware->lText.Next() )
			values += String("WareText: ") + (long)wt->iLang + " " + wt->sName + "|" + wt->sDesc + "\n";
	}

	return values;
}


/*
	Func:   WriteFile
	Input:  filename - The filename of the spk file to write to
	Desc:   Writes the data to an spk file
*/
bool CSpkFile::WriteFile ( String filename, CProgressInfo *progress )
{
	FILE *id = fopen ( filename.c_str(), "wb" );
	if ( !id )
		return false;

	bool ret = WriteData ( id, progress );
	fclose ( id );

	return ret;
}

bool CSpkFile::WriteData ( FILE *id, CProgressInfo *progress )
{
	int valueheader = m_SHeader.iValueCompression, fileheader = m_SHeader.iValueCompression;
#ifndef _WIN32
	if ( valueheader == SPKCOMPRESS_7ZIP )
		valueheader = SPKCOMPRESS_ZLIB;
	if ( fileheader == SPKCOMPRESS_7ZIP )
		fileheader = SPKCOMPRESS_ZLIB;
#endif

	// get the script values
	String values = this->CreateValuesLine();

	// compress the values
	int valueUncomprLen = values.Length();
	unsigned long valueComprLen = 0;
	unsigned char *valueCompr = NULL;
	bool compressed = false;
	if ( valueheader == SPKCOMPRESS_ZLIB )
	{
		valueComprLen = valueUncomprLen;	
		if ( valueComprLen < 100 )
			valueComprLen = 200;
		else if ( valueComprLen < 1000 )
			valueComprLen *= 2;

		valueCompr = (unsigned char *)calloc((unsigned int)valueComprLen, 1);
		int err = compress ( (unsigned char *)valueCompr, &valueComprLen, (const unsigned char *)values.c_str(), values.Length() );
		if ( err == Z_OK )
			compressed = true;
	}
#ifdef _WIN32
	else if ( valueheader == SPKCOMPRESS_7ZIP )
	{
		long len = values.Length();
		valueCompr = LZMAEncodeData ( (unsigned char *)values.c_str(), len, len );
		if ( valueCompr )
		{
			valueComprLen = len;
			compressed = true;
		}
	}
#endif

	if ( !compressed )
	{
		valueComprLen = valueUncomprLen;
		valueCompr = (unsigned char *)calloc((unsigned int)valueComprLen, 1);
		memcpy ( valueCompr, values.c_str(), valueComprLen );
		valueheader = SPKCOMPRESS_NONE;
	}

	// write the main header to the file
	fprintf ( id, "SPKCycrow;%.2f;%d;%d\n", FILEVERSION, valueheader, valueComprLen );
	if ( ferror(id) )
		return false;

	// write the compressed data to file
	fputc ( (unsigned char)(valueUncomprLen >> 24), id ); 
	fputc ( (unsigned char)(valueUncomprLen >> 16), id ); 
	fputc ( (unsigned char)(valueUncomprLen >> 8), id ); 
	fputc ( (unsigned char)valueUncomprLen, id ); 
	fwrite ( valueCompr, sizeof(char), valueComprLen, id );

	free ( valueCompr );

	// now compress the files header
	// create the files values
	String files = CreateFilesLine ( true, progress );

	// compress the files values
	long fileUncomprLen = files.Length(), fileComprLen = fileUncomprLen;
	unsigned char *fileCompr = NULL;

	compressed = false;
	if ( fileUncomprLen )
	{
		if ( fileheader == SPKCOMPRESS_ZLIB )
		{
			if ( fileComprLen < 100 )
				fileComprLen = 200;
			else if ( fileComprLen < 1000 )
				fileComprLen *= 2;
			fileCompr = (unsigned char *)calloc((unsigned int)fileComprLen, 1);
			int err = compress ( (unsigned char *)fileCompr, (unsigned long *)&fileComprLen, (const unsigned char *)files.c_str(), files.Length() );
			if ( err == Z_OK )
				compressed = true;
		}
#ifdef _WIN32
		else if ( fileheader == SPKCOMPRESS_7ZIP )
		{
			long len = files.Length();
			fileCompr = LZMAEncodeData ( (unsigned char *)files.c_str(), len, len );
			if ( fileCompr )
			{
				fileComprLen = len;
				compressed = true;
			}
		}
#endif
	}

	// if unable to compress, store it as plain text
	if ( !compressed )
	{
		fileComprLen = fileUncomprLen;
		fileCompr = (unsigned char *)calloc((unsigned int)fileComprLen, 1);
		memcpy ( fileCompr, files.c_str(), fileComprLen );
		fileheader = SPKCOMPRESS_NONE;
	}

	// now write the file header
	m_SHeader2.lSize = fileComprLen;
	fprintf ( id, "FileHeader;%d;%d;%d;%d;%d\n", m_SHeader2.iNumFiles, m_SHeader2.lSize, m_SHeader2.lFullSize, fileheader, m_SHeader2.iDataCompression );

	fputc ( (unsigned char)(fileUncomprLen >> 24), id ); 
	fputc ( (unsigned char)(fileUncomprLen >> 16), id ); 
	fputc ( (unsigned char)(fileUncomprLen >> 8), id ); 
	fputc ( (unsigned char)fileUncomprLen, id ); 
	fwrite ( fileCompr, sizeof(char), fileComprLen, id );

	free ( fileCompr );

	// now finally, write all the file data
	if ( m_pIconFile )
	{
		fputc ( (unsigned char)(m_pIconFile->GetUncompressedDataSize() >> 24), id ); 
		fputc ( (unsigned char)(m_pIconFile->GetUncompressedDataSize() >> 16), id ); 
		fputc ( (unsigned char)(m_pIconFile->GetUncompressedDataSize() >> 8), id ); 
		fputc ( (unsigned char)m_pIconFile->GetUncompressedDataSize(), id ); 
		fwrite ( m_pIconFile->GetData(), sizeof(char), m_pIconFile->GetDataSize(), id );
	}
	
	for ( CFile *file = m_lFiles.First(); file; file = m_lFiles.Next() )
	{
		fputc ( (unsigned char)(file->GetUncompressedDataSize() >> 24), id ); 
		fputc ( (unsigned char)(file->GetUncompressedDataSize() >> 16), id ); 
		fputc ( (unsigned char)(file->GetUncompressedDataSize() >> 8), id ); 
		fputc ( (unsigned char)file->GetUncompressedDataSize(), id ); 
		fwrite ( file->GetData(), sizeof(char), file->GetDataSize(), id );
	}

	return true;
}




void CSpkFile::AddWareText ( String rest )
{
	if ( !m_pLastWare )
		return;

	SWaresText *wt = new SWaresText;
	wt->iLang = rest.GetToken ( 1, ' ' ).ToInt();
	wt->sName = rest.GetToken ( 2, -1, ' ' ).GetToken ( 1, '|' );
	wt->sDesc = rest.GetToken ( 2, -1, ' ' ).GetToken ( 2, -1, '|' );
	m_pLastWare->lText.push_back ( wt );
}

void CSpkFile::AddWare ( String rest )
{
	SWares *ware = new SWares;
	ware->iDeleteState = DELETESTATE_NONE;
	ware->cType = rest.GetToken ( 1, ':' )[0];
	ware->iPrice = rest.GetToken ( 2, ':' ).ToLong();
	ware->iSize = rest.GetToken ( 3, ':' ).ToInt();
	ware->iVolumn = rest.GetToken ( 4, ':' ).ToInt();
	ware->sID = rest.GetToken ( 5, ':' );
	ware->iNotority = rest.GetToken ( 6, ':' ).ToInt();
	m_lWares.push_back ( ware );
	m_pLastWare = ware;
}



bool CSpkFile::ReadFileToMemory ( CFile *file )
{
	if ( !file )
		return false;

	// check if data is already extracted
	if ( (file->GetDataSize ()) && (file->GetData()) )
		return true;

	// no file to read from
	if ( m_sFilename.Empty() )
		return false;

	// now open the file
	FILE *id = fopen ( m_sFilename.c_str(), "rb" );
	if ( !id )
		return false;

	// read the header
	GetEndOfLine ( id, NULL, false );
	// skip past values
	fseek ( id, m_SHeader.lValueCompressSize, SEEK_CUR );

	// read the next header
	GetEndOfLine ( id, NULL, false );
	// skip past files
	fseek ( id, 4, SEEK_CUR );
	fseek ( id, m_SHeader2.lSize, SEEK_CUR );

	// skip the icon file
	if ( m_pIconFile )
	{
		fseek ( id, 4, SEEK_CUR );
		fseek ( id, m_pIconFile->GetDataSize (), SEEK_CUR );
	}

	// now were in the file section
	// skip past each one
	for ( CFile *fit = m_lFiles.First(); fit; fit = m_lFiles.Next() )
	{
		if ( fit == file )
			break;

		fseek ( id, 4, SEEK_CUR );
		fseek ( id, fit->GetDataSize(), SEEK_CUR );
	}

	// now we should be at the start of the file
	// read the data into memory
	if ( !file->ReadFromFile ( id, file->GetDataSize() ) )
	{
		fclose ( id );
		return false;
	}
	fclose ( id );

	return true;
}

bool CSpkFile::ExtractFile ( CFile *file, String dir, bool includedir, CProgressInfo *progress )
{
	if ( ReadFileToMemory ( file ) )
	{
		// now finally, uncompress the file
		long len = 0;
		unsigned char *data = file->UncompressData ( &len, progress );
		if ( !data )
		{
			// attempt a file decompress
			if ( file->GetCompressionType() == SPKCOMPRESS_7ZIP )
			{
				if ( file->UncompressToFile ( dir, this, includedir, progress ) )
					return true;
			}
			return false;
		}

		if ( !file->WriteToDir ( dir, this, includedir, NullString, data, len ) )
			return false;

		return true;

	}
	else
		return false;
}

bool CSpkFile::ExtractFile ( int filenum, String dir, bool includedir, CProgressInfo *progress )
{
	// invalid valus
	if ( filenum < 0 )
		return false;
	// out of range
	if ( filenum > m_lFiles.size() )
		return false;
	
	// get the file pointer
	CFile *file = m_lFiles.Get ( filenum );
	return ExtractFile ( file, dir, includedir, progress );
}



bool CSpkFile::ExtractAll ( String dir, bool includedir, CProgressInfo *progress )
{
	// no file to read from
	if ( m_sFilename.Empty() )
		return false;

	// now open the file
	FILE *id = fopen ( m_sFilename.c_str(), "rb" );
	if ( !id )
		return false;

	fseek ( id, 0, SEEK_SET );
	// read the header
	GetEndOfLine ( id, NULL, false );
	// skip past values
	fseek ( id, m_SHeader.lValueCompressSize, SEEK_CUR );

	// read the next header
	GetEndOfLine ( id, NULL, false );
	// skip past files
	fseek ( id, 4, SEEK_CUR );
	fseek ( id, m_SHeader2.lSize, SEEK_CUR );

	// now were in the file section
	// skip past each one
	if ( m_pIconFile )
	{
		fseek ( id, 4, SEEK_CUR );
		fseek ( id, m_pIconFile->GetDataSize (), SEEK_CUR );
	}

	for ( CFile *fit = m_lFiles.First(); fit; fit = m_lFiles.Next() )
	{
		if ( progress )
			progress->DoingFile ( fit );

		if ( (!fit->GetDataSize ()) || (!fit->GetData()) )
		{
			if ( !fit->ReadFromFile ( id, fit->GetDataSize() ) )
			{
				fclose ( id );
				return false;
			}
		}
		else
			fseek ( id, fit->GetDataSize(), SEEK_CUR );

		// create directory first
		CreateDirectory ( dir + "/" + fit->GetDirectory(this) );

		long size = 0;
		unsigned char *data = fit->UncompressData (&size, progress);
		if ( (!data) && (fit->GetCompressionType() == SPKCOMPRESS_7ZIP) )
		{
			if ( !fit->UncompressToFile ( dir, this, includedir, progress ) )
			{
				fclose ( id );
				return false;
			}
		}
		else if ( (!data) || (!fit->WriteToDir ( dir, this, includedir, NullString, data, size )) )
		{
			fclose ( id );
			return false;
		}
	}

	fclose ( id );

	return true;
}



bool CSpkFile::CheckValidReadmes ()
{
	for ( CFile *file = m_lFiles.First(); file; file = m_lFiles.Next() )
	{
		if ( file->GetFileType() != FILETYPE_README )
			continue;
		if ( !file->CheckValidFilePointer() )
			continue;
		return true;
	}

	return false;
}



void CSpkFile::ClearWares()
{
	for ( SWares *w = m_lWares.First(); w; w = m_lWares.Next() )
	{
		for ( SWaresText *wt = w->lText.First(); wt; wt = w->lText.Next() )
			delete wt;
		w->lText.clear();
		delete w;
	}
	m_lWares.clear();
}

SWares *CSpkFile::FindWare ( String id )
{
	for ( SWares *w = m_lWares.First(); w; w = m_lWares.Next() )
	{
		if ( w->sID.ToUpper() == id.ToUpper() )
			return w;
	}
	return NULL;
}

void CSpkFile::RemoveWare ( String id )
{
	for ( SWares *w = m_lWares.First(); w; w = m_lWares.Next() )
	{
		if ( w->sID.ToUpper() == id.ToUpper() )
		{
			m_lWares.RemoveCurrent ();
			delete w;
			return;
		}
	}
}

void CSpkFile::AddWare ( SWares *ware )
{
	ware->sID.RemoveChar ( ' ' );
	ware->sID = ware->sID.ToUpper();

	SWares *newware = FindWare ( ware->sID );
	if ( newware )
		m_lWares.remove ( newware );

	m_lWares.push_back ( ware );
}

void CSpkFile::AddWareText ( SWares *w, int lang, String name, String desc )
{
	SWaresText *wt;
	for ( wt = w->lText.First(); wt; wt = w->lText.Next() )
	{
		if ( wt->iLang == lang )
		{
			wt->sDesc = desc;
			wt->sName = name;
			return;
		}
	}

	wt = new SWaresText;
	wt->iLang = lang;
	wt->sName = name;
	wt->sDesc = desc;

	w->lText.push_back ( wt );
}


void CSpkFile::ClearWareText ( String id )
{
	SWares *w = FindWare ( id );
	ClearWareText ( w );
}

void CSpkFile::ClearWareText ( SWares *w )
{
	if ( !w ) return;

	for ( SWaresText *wt = w->lText.Next(); wt; wt = w->lText.First() )
		delete wt;
	w->lText.clear();
}

void CSpkFile::RemoveWareText ( String wid, int lang )
{
	SWares *w = FindWare ( wid );
	if ( w )
	{
		for ( SWaresText *wt = w->lText.First(); wt; wt = w->lText.Next() )
		{
			if ( wt->iLang == lang )
			{
				w->lText.RemoveCurrent();
				delete wt;
				break;
			}
		}
	}
}

int CSpkFile::CheckValidCustomStart ()
{
	if ( !IsAnotherMod() )
		return 1;

	for ( CFile *file = m_lFiles.First(); file; file = m_lFiles.Next() )
	{
		if ( file->GetFileType() != FILETYPE_SCRIPT )
			continue;

		String basename = file->GetName().GetToken ( 1, file->GetName().NumToken('.') - 1, '.' );
		if ( basename.ToLower().Right(15) == ".initplayership" )
			return 0;
	}
	return 2;
}

bool CSpkFile::UpdateSigned ()
{
	// check for any custom wares
	// patch mods and custom starts are also not signed
	if ( (!m_lWares.empty()) || (m_bPatch) || (m_bCustomStart) )
	{
		m_bSigned = false;
		return false;
	}

	m_bSigned = true;
	for ( CFile *file = m_lFiles.First(); file; file = m_lFiles.Next() )
	{
		// extra, text, soundtrack, readmes and screen files do not require a modified game directly
		if ( (file->GetFileType() == FILETYPE_EXTRA) || (file->GetFileType() == FILETYPE_TEXT) || (file->GetFileType() == FILETYPE_SOUND) || (file->GetFileType() == FILETYPE_SCREEN) || (file->GetFileType() == FILETYPE_README) )
			continue;
		// mods and maps always need modified game
		else if ( (file->GetFileType() == FILETYPE_MOD) || (file->GetFileType() == FILETYPE_MAP) )
		{
			m_bSigned = false;
			break;
		}

		// else should be a script file, script or uninstall type
		// all scripts must be signed, if any are not, then no signed status
		if ( !file->IsSigned () )
		{
			m_bSigned = false;
			break;
		}
	}

	return m_bSigned;
}

SSettingType *CSpkFile::AddSetting ( String key, int type )
{
	key.RemoveChar ( '|' );
	SSettingType *t;
	for ( t = m_lSettings.First(); t; t = m_lSettings.Next() )
	{
		if ( t->sKey.upper() == key.lower() )
			return NULL;
	}

	switch ( type )
	{
		case SETTING_STRING:
			t = new SSettingString;
			break;
		case SETTING_INTEGER:
			t = new SSettingInteger;
			break;
		case SETTING_CHECK:
			t = new SSettingCheck;
			break;
	}

	if ( !t )
		return NULL;

	t->sKey = key;
	t->iType = type;

	m_lSettings.push_back ( t );

	return t;
}

void CSpkFile::ConvertSetting ( SSettingType *t, String set )
{
	if ( !t )
		return;

	switch ( t->iType )
	{
		case SETTING_STRING:
			((SSettingString *)t)->sValue = set;
			break;
		case SETTING_INTEGER:
			((SSettingInteger *)t)->iValue = set.ToInt();
			break;
		case SETTING_CHECK:
			((SSettingCheck *)t)->bValue = set.ToBool();
			break;
	}
}
String CSpkFile::GetSetting ( SSettingType *t )
{
	if ( !t )
		return "";

	switch ( t->iType )
	{
		case SETTING_STRING:
			return ((SSettingString *)t)->sValue;
		case SETTING_INTEGER:
			return String::Number(((SSettingInteger *)t)->iValue);
		case SETTING_CHECK:
			return (((SSettingInteger *)t)->iValue) ? "1" : "0";
	}

	return "";
}

void CSpkFile::ClearSettings ()
{
	SSettingType *n;
	for ( n = m_lSettings.First(); n; n = m_lSettings.Next() )
		delete n;
	m_lSettings.clear();
}


bool CSpkFile::IsMatchingMod ( String mod )
{
	for ( CListNode<CFile> *node = m_lFiles.Front(); node; node = node->next() )
	{
		CFile *file = node->Data();
		if ( file->GetFileType() != FILETYPE_MOD )
			continue;

		if ( file->IsFakePatch() )
			continue;

		String filename = file->GetBaseName();
		if ( filename.lower() == mod.lower() )
			return true;
	}
	return false;
}

