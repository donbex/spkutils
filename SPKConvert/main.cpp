#include "spkprogress.h"

#ifndef _WIN32
#include "../SPK/ansi7zip/7Decoder.h"
#else
#include "../HiP/HiP.h"
#endif

#include "../SPK/MultiSpkFile.h"


#ifdef _DEBUG
#define CLEANUP fclose ( id ); if ( data ) delete data; if ( uncomprData ) delete uncomprData; if ( !removeFile.Empty() ) remove ( removeFile.c_str() ); char pause; printf ( "Press Enter to Close\n" ); scanf ( "%c", &pause );
#else
#define CLEANUP fclose ( id ); if ( data ) delete data; if ( uncomprData ) delete uncomprData; if ( !removeFile.Empty() ) remove ( removeFile.c_str() ); 
#endif

char *ReadNextLine ( char *data, long *len, String *str )
{
	int pos = 0;
	bool end = false;
	while ( pos < *len )
	{
		if ( data[pos] == '\n' )
			break;
		if ( data[pos] == '\0' )
		{
			end = true;
			break;
		}
		++pos;
	}

	if ( end )
	{
		*len = 0;
		*str = data;
		return NULL;
	}

	data[pos] = '\0';
	*str = data;
	data[pos] = '\n';
	*len -= (pos + 1);

	return data + (pos + 1);
}

char *LineByLineRead ( char *data, long *len, String end, String *readData )
{
	String line;
	while ( true )
	{
		data = ReadNextLine ( data, len, &line );

		if ( line == end )
			break;
		*readData += (line + "\n");
	}

	return data;
}

int main ( int argc, char **argv )
{
	String filename ( argv[0] );
	filename = filename.GetToken ( filename.NumToken ( '\\' ), '\\' );

	printf ( "SPKConvert V1.10 (SPK VERSION %.2f) 28/07/2007 by Cycrow\n", FILEVERSION );

	if ( argc < 3 )
	{
		printf ( "Syntax, %s <oldspkfile> <newspkfile>\n", filename.c_str() );
		exit ( 1 );
	}

	String oldfile ( argv[1] );
	String newfile ( argv[2] );

	if ( CBaseFile::CheckFile ( oldfile ) != SPKFILE_INVALID )
	{
		printf ( "Spk file is already in the new format, unable to convert\n" );
		exit ( 1 );
	}

	// firstcheck if the file exists
	FILE *id = fopen ( argv[1], "rb" );
	if ( !id )
	{
		printf ( "Unable to open file: %s\n", argv[1] );
		exit ( 0 );
	}

	// read the first 3 charaters
	String check = (char)fgetc ( id );
	check += (char)fgetc ( id );
	check += (char)fgetc ( id );

	String removeFile;

	unsigned char *uncomprData = NULL;
	unsigned char *data = NULL;
	long len = 0, newlen = 0;

	MyProgress *progress = new MyProgress ( 0 );
#ifdef _WIN32
	if ( check == "HiP" )
	{
		fclose ( id );
		bool opened = false;
		if ( DecompressFile ( argv[1], "uncompr.tmp" ) )
		{
			removeFile = "uncompr.tmp";
			id = fopen ( "uncompr.tmp", "r" );
			if ( id )
				opened = true;
		}

		if ( !opened )
		{
			printf ( "Unable to uncompress file, exiting...\n" );
			exit ( 0 );
		}

		printf ( "* Reading file into memory... " );
		// get file length
		fseek ( id, 0, SEEK_END );
		len = ftell ( id );
		
		// move back to beginning
		fseek ( id, 0, SEEK_SET );

		// read the data from file into memory
		uncomprData = new unsigned char[len + 1];
		fread ( uncomprData, sizeof(unsigned char), len, id );

		newlen = len;
	}
	else
#endif
	{
		printf ( "* Reading file into memory... " );
		// get file length
		fseek ( id, 0, SEEK_END );
		len = ftell ( id );
		
		// move back to beginning
		fseek ( id, 0, SEEK_SET );

		// read the data from file into memory
		data = new unsigned char[len + 1];
		fread ( data, sizeof(unsigned char), len, id );

		// uncompress the file (currently only 7zip compression)
		printf ( "\t(Done)\n* Uncompressing file...\n\t>" );
		newlen = len;
	#ifdef _WIN32
		uncomprData = LZMADecodeData ( data, len, newlen, progress );
	#else
		uncomprData = LZMADecode_C ( (unsigned char *)data, len, (size_t*)&newlen, NULL );
	#endif
	}

	// uncomressed failed
	if ( !uncomprData )
	{
		printf ( "\t(Error)\n" );
		CLEANUP
		exit ( 0 );
	}

	len = newlen;

	printf ( "< (Done)\n" );

	// now we can read the data
	char *d = (char *)uncomprData;
	String str;
	CSpkFile *spkfile = new CSpkFile;
	CMultiSpkFile *mspk = NULL;
	SMultiSpkFile *cur_mspk = NULL;

	int numscripts = 0, curscript = 0;
	bool verbose = true;
	float fVersion = 1;

	printf ( "* Reading spk data..." );
	if ( verbose )
		printf ( "\n" );


	while ( (d) && (len > 0) )
	{
		d = ReadNextLine ( d, &len, &str );

		String first = str.GetToken ( 1, ' ' );
		String rest = str.GetToken ( 2, -1, ' ' );
		if ( first == "MultiPackage:" )
			mspk = new CMultiSpkFile;
		else if ( (first == "SelectScript:") && (mspk) )
		{
			mspk->AddFileEntry ( rest.GetToken ( 2, -1, ' ' ) + ".spk" );
			++numscripts;
		}
		else if ( (str == "AllowSelection") && (mspk) )
			mspk->SetSelection ( true );
		else if ( str == "-- Start New Script --" )
		{
			if ( !mspk )
			{
				printf ( "Invalid file format, seems to be multi package file but isn't\n" );
				CLEANUP
				exit ( 0 );
			}
			cur_mspk = mspk->GetFileList()->Get ( curscript );
			++curscript;
			cur_mspk->pFile = new CSpkFile;
			spkfile = cur_mspk->pFile;
		}
		else if ( first == "Packager:" )
		{
			fVersion = rest.ToFloat ();
			if ( verbose ) printf ( "\tPackager Version: %.2f\n", fVersion );
		}
		else if ( first == "Name:" )
		{
			spkfile->SetName ( rest );
			if ( verbose ) printf ( "\tScript Name: %s\n", rest.c_str() );
		}
		else if ( first == "Author:" )
		{
			spkfile->SetAuthor ( rest );
			if ( verbose ) printf ( "\tScript Author: %s\n", rest.c_str() );
		}
		else if ( str == "CustomStart" )
		{
			spkfile->SetCustomStart ( true );
			if ( verbose ) printf ( "\tPackage is a custom start!!\n" );
		}
		else if ( first == "AnotherMod:" )
		{
			spkfile->SetAnotherMod ( rest.GetToken ( 1, '|' ), rest.GetToken ( 2, -1, '|' ) );
			if ( verbose ) printf ( "\tFor another Mod, Name: %s, Author: %s\n", spkfile->GetOtherName().c_str(), spkfile->GetOtherAuthor().c_str() );
		}
		else if ( str == "PATCH" )
		{
			spkfile->SetPatch ( true );
			if ( verbose ) printf ( "\tPackage is a Patch Mod!!\n" );
		}
		else if ( first == "Version:" )
		{
			spkfile->SetVersion ( rest );
			if ( verbose ) printf ( "\tScript Version: %s\n", rest.c_str() );
		}
		else if ( first == "Date:" )
		{
			spkfile->SetCreationDate ( rest );
			if ( verbose ) printf ( "\tScript Creation Date: %s\n", rest.c_str() );
		}
		else if ( first == "Desc:" )
		{
			spkfile->SetDescription ( rest.FindReplace ( "<br>", "\n" ) );
			if ( verbose ) printf ( "\tScript Description: %s\n", spkfile->GetDescription().c_str() );
		}
		else if ( first == "WebAddress:" )
		{
			spkfile->SetWebAddress ( rest );
			if ( verbose ) printf ( "\tWeb Address: %s\n", rest.c_str() );
		}
		else if ( first == "WebMirror1:" )
		{
			spkfile->SetWebMirror1 ( rest );
			if ( verbose ) printf ( "\tWeb Mirror Address: %s\n", rest.c_str() );
		}
		else if ( first == "WebMirror2:" )
		{
			spkfile->SetWebMirror2 ( rest );
			if ( verbose ) printf ( "\tWeb Mirror Address: %s\n", rest.c_str() );
		}
		
		else if ( first == "ScriptType:" )
			spkfile->SetScriptType ( rest );
		else if ( first == "WebSite:" )
		{
			spkfile->SetWebSite ( rest );
			if ( verbose ) printf ( "\tWeb Site: %s\n", rest.c_str() );
		}
		else if ( first == "Email:" )
		{
			spkfile->SetEmail ( rest );
			if ( verbose ) printf ( "\tAuthor Email Address: %s\n", rest.c_str() );
		}
		else if ( first == "GameVersion:" )
		{
			int version = rest.ToInt();
			if ( version == 0 )
				spkfile->SetGameVersion ( 1 );
			else if (version == 1 )
				spkfile->SetGameVersion ( 0 );
			else
				spkfile->SetGameVersion ( version );
			if ( verbose ) printf ( "\tGame Version: %d\n", spkfile->GetGameVersion () );
		}
		
		else if ( first == "Ware:" )
		{
			spkfile->AddWare ( rest );
			if ( verbose ) printf ( "\tAdding Custom Ware\n" );
		}
		else if ( first == "WareText:" )
			spkfile->AddWareText ( rest );
		else if ( first == "UninstallAfter:" )
			spkfile->AddUninstallAfterText ( rest.GetToken ( 1, ' ' ), rest.GetToken ( 2, -1, ' ' ) );
		else if ( first == "UninstallBefore:" )
			spkfile->AddUninstallBeforeText ( rest.GetToken ( 1, ' ' ), rest.GetToken ( 2, -1, ' ' ) );
		else if ( first == "InstallAfter:" )
			spkfile->AddInstallAfterText ( rest.GetToken ( 1, ' ' ), rest.GetToken ( 2, -1, ' ' ) );
		else if ( first == "InstallBefore:" )
			spkfile->AddInstallBeforeText ( rest.GetToken ( 1, ' ' ), rest.GetToken ( 2, -1, ' ' ) );
		else if ( first == "ScriptName:" )
		{
			String lang = rest.GetToken ( 1, ':' );
			String name = rest.GetToken ( 2, -1, ':' );
			spkfile->AddLanguageName ( name, lang );
			if ( verbose ) printf ( "\tScript Name Language (%s) %s\n", lang.c_str(), name.c_str() );
		}
		else if ( first == "Icon:" )
		{
			String ext = rest.GetToken ( 2, ' ' );
			long size = rest.GetToken ( 1, ' ' ).ToLong ();
			
			CFile *file = new CFile ();
			file->ReadFromData ( d, size );

			d += size;

			spkfile->SetIcon ( file, ext );

			if ( verbose ) printf ( "\tIcon (%s) Size: %s\n", ext.c_str(), file->GetDataSizeString ().c_str() );
		}
		else if ( (first == "$$$SharedScript:") || (first == "$$$Script:") || (first == "$$$SharedText:") || (first == "$$$Text:") || (first == "$$$Uninstall:") || (first == "$$$SharedMap:") || (first == "$$$Map:") || (first == "$$$Readme:") )
		{
			int type;
			String end, print;
			if ( (first == "$$$SharedText:") || (first == "$$$Text:") )
			{
				type = FILETYPE_TEXT;
				end = "-- End of Script --";
				print = "Text";
			}
			else if ( first == "$$$Uninstall:" )
			{
				type = FILETYPE_UNINSTALL;
				end = "-- End of Uninstall --";
				print = "Uninstall Script";
			}
			else if ( first == "$$$Readme:" )
			{
				type = FILETYPE_README;
				end = "-- End of Readme --";
				print = "Readme";
			}
			else if ( (first == "$$$SharedMap:") || (first == "$$$Map:") )
			{
				type = FILETYPE_MAP;
				end = "-- End of Map --";
				print = "Universe Map";
			}
			else
			{
				type = FILETYPE_SCRIPT;
				end = "-- End of Script --";
				print = "Script";
			}

			String filename, dir;
			long time = 0, size = 0;
			bool shared = false;

			if ( fVersion >= 3.00f )
				filename = rest.GetToken ( 3, -1, ' ' );
			else if ( fVersion >= 2.00f )
				filename = rest.GetToken ( 2, -1, ' ' );
			else
				filename = rest;

			if ( filename.IsIn ( "<br>" ) )
			{
				filename = filename.FindReplace ( "<br>", "|" );
				if ( filename[0] == '|' )
				{
					filename = filename.GetToken ( 1, '|' );
				}
				else
				{
					dir = filename.GetToken ( 1, '|' );
					filename = filename.GetToken ( 2, -1, '|' );
				}
			}

			if ( fVersion >= 2.00f )
				time = rest.GetToken ( 1, ' ' ).ToLong();

			if ( fVersion >= 3.00f )
				size = rest.GetToken ( 2, ' ' ).ToLong();
			
			if ( first.Left (9) == "$$$Shared" )
				shared = true;

			CFile *file = new CFile ();

			bool binaryRead = false;
			String ext = filename.GetToken ( filename.NumToken ( '.' ), '.' );
			if ( ext.ToUpper() == "PCK" )
				binaryRead = true;

			if ( verbose )
			{
				if ( shared )
					printf ( "\tFound %s File (Shared): %s, Reading...", print.c_str(), filename.c_str() );
				else
					printf ( "\tFound %s File: %s, Reading...", print.c_str(), filename.c_str() );
			}

			if ( binaryRead )
			{
				file->ReadFromData ( d, size );
				d += size;
			}
			else
			{
				String readData;
				d = LineByLineRead ( d, &len, end, &readData );
				file->ReadFromData ( (char *)readData.c_str(), readData.Length() );
			}
			file->SetName ( filename );
			file->SetFileType ( type );
			file->SetShared ( shared );
			file->SetCreationTime ( time );
			if ( !dir.Empty() )
				file->SetDir ( dir );
			
			spkfile->AddFile ( file );

			printf ( "(Done) Size: %s\n", file->GetDataSizeString().c_str() );

		}
		else if ( (first == "$$$Mod:") || (first == "$$$Sound:") || (first == "$$$Extra:") || (first == "$$$Screen:") )
		{
			int type = -1;
			String print;
			if ( first == "$$$Mod:" )
			{
				type = FILETYPE_MOD;
				print = "Mod";
			}
			else if ( first == "$$$Extra:" )
			{
				type = FILETYPE_EXTRA;
				print = "Extra";
			}
			else if ( first == "$$$Screen:" )
			{
				type = FILETYPE_SCREEN;
				print = "Screenshot";
			}
			else if ( first == "$$$Sound:" )
			{
				type = FILETYPE_SOUND;
				print = "Sound Track";
			}

			String filename = rest.GetToken ( 3, -1, ' ' ), dir;
			long time = rest.GetToken ( 1, ' ' ).ToLong(), size = rest.GetToken ( 2, ' ' ).ToLong();
			bool shared = false;

			if ( filename.IsIn ( "<br>" ) )
			{
				filename = filename.FindReplace ( "<br>", "|" );
				if ( filename[0] == '|' )
					filename = filename.GetToken ( 1, '|' );
				else
				{
					dir = filename.GetToken ( 1, '|' );
					filename = filename.GetToken ( 2, -1, '|' );
				}
			}

			CFile *file = new CFile ();
			file->ReadFromData ( d, size );
			file->SetName ( filename );
			file->SetFileType ( type );
			file->SetShared ( shared );
			file->SetCreationTime ( time );
			if ( !dir.Empty() )
				file->SetDir ( dir );
			
			d += size;

			spkfile->AddFile ( file );

			if ( verbose )
			{
				if ( shared )
					printf ( "\tFound %s File (Shared): %s, Size(%s)\n", print.c_str(), filename.c_str(), file->GetDataSizeString().c_str() );
				else
					printf ( "\tFound %s File: %s, Size(%s)\n", print.c_str(), filename.c_str(), file->GetDataSizeString().c_str() );
			}
		}
	}

	if ( verbose ) printf ( "* Reading spk data..." );
	printf ( " (Done)\n" );

	// now save the spk file
	printf ( "\nStarting to write new spk file\n" );
	if ( mspk )
	{
		for ( SMultiSpkFile *it = mspk->GetFileList()->First(); it; it = mspk->GetFileList()->Next() )
		{
			for ( CFile *f = it->pFile->GetFileList()->First(); f; f = it->pFile->GetFileList()->Next() )
			{
				printf ( "* Compressing %s...\n\t>", f->GetNameDirectory(it->pFile).c_str() );
				if ( f->CompressData ( spkfile->GetDataCompression(), progress ) )
				{
					progress->PrintDone();
					printf ( "< (Done)\n" );
				}
				else
				{
					progress->Reset();
					printf ( "< (Error)\n" );
				}
			}

			FILE *id = tmpfile();
			if ( id )
			{
				it->pFile->WriteData ( id, NULL );
				it->lSize = ftell ( id );
				fclose ( id );
			}
		}
	}
	else
	{
		for ( CFile *f = spkfile->GetFileList()->First(); f; f = spkfile->GetFileList()->Next() )
		{
			printf ( "* Compressing %s...\n\t>", f->GetNameDirectory(spkfile).c_str() );
			if ( f->CompressData ( spkfile->GetDataCompression(), progress ) )
			{
				progress->PrintDone();
				printf ( "< (Done)\n" );
			}
			else
			{
				progress->Reset();
				printf ( "< (Error)\n" );
			}
		}
	}

	printf ( "* Writing to %s... ", argv[2] );
	if ( mspk )
	{
		if ( mspk->WriteFile ( String(argv[2]) ) )
			printf ( "(Done)\n" );
		else
			printf ( "(Error)\n" );
	}
	else
	{
		spkfile->WriteFile ( String(argv[2]) );
		printf ( "(Done)\n" );
	}

	printf ( "SPK file has been converted successfully\n" );
	CLEANUP

	return 1;
}
