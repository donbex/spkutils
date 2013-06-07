/*
 SPKTool V1.00 Created by Cycrow (Matthew Gravestock)
*/

// Main Spk File Library Include
#include "../SPK/SpkFile.h"
// Multi Spk File format, required if using Multi Spk files
#include "../SPK/MultiSpkFile.h"
// Displays 7Zip compression progress to the command line
#include "../SPK/spkcmdprogress.h"
#include <time.h>

String g_dir;
bool g_read;

/*
	Func:	GetInput
	Desc:	Gets an input from the user, ie, any settings required to be typed in
*/
String GetInput ()
{
	g_read = true;

	String line;
	char c = getchar();

	while ( (c != '\n') && (c != '\0') )
	{
		line += c;
		c = getchar();
	}

	return line;
}


/*
	Func:	PrintSyntax
	Args:	String cmd - The command name to be displayed
	Desc:	Displays the syntax for the program
*/
void PrintSyntax ( String cmd )
{
	printf ( "Syntax: %s <command>\n", cmd.c_str() );
	printf ( "Commands:\n" );
	printf ( "\t-v <spkfile> - Views the contents of a spk file\n" );
	printf ( "\t-e <spkfile> [destination] - Extracts all the files to the destination directory\n" );
	printf ( "\t-x <spkfile> <type> <file> [destination] - Extracts a single file of <type>\n" );
	printf ( "\t-x <multispkfile> <file> [destination] - Extracts a single spk file from a Multi-Spk Package\n" );
	printf ( "\t-c <spkfile> - Creates a new spk file\n" );
	printf ( "\t-a <spkfile> <type> <filename> - Appends a file to the package of set <type>\n" );
	printf ( "\t-r <spkfile> <type> <filename> - Removes a file from the package of set <type>\n" );
	printf ( "\t-r <multispkfile> <filename> - Removes a spk file from the Multi-SPK package\n" );
	printf ( "\t-m <spkfile> <spkfile> - Merges spk files together, the second file will be merged into the first\n" );
	printf ( "\t-n <multispkfile> - Creates a multi spk file, and adds the spkfiles in\n" );
	printf ( "\t-s <multispkfile> [destination] - Splits a Multi-SPK file up, saves all the spk files to Destination\n" );
}

void DisplayVersion ( String filename )
{
	// first chekc if file even exists
	FILE *id = fopen ( filename.c_str(), "rb+" );
	if ( !id )
	{
		printf ( "Error: File, %s, doesn't exist\n", filename.c_str() );
		return;
	}
	fclose ( id );

	int check = CSpkFile::CheckFile ( filename );
	if ( check == SPKFILE_SINGLE )
	{
		CSpkFile spkfile;
		printf ( "* Opening SPK File, %s...\n", filename.c_str() );
		if ( !spkfile.ReadFile ( filename, false ) )
		{
			printf ( "Failed to open the spk files, %s\n", filename.c_str() );
			return;
		}

		printf ( "File Format Version: %.2f\n", spkfile.GetFileVersion() );
		
		if ( !spkfile.GetEmail().Empty() ) 
			printf ( "Script Name: %s (%s)\n", spkfile.GetName ().c_str(), spkfile.GetEmail().c_str() );
		else
			printf ( "Script Name: %s\n", spkfile.GetName ().c_str() );
		printf ( "Script Author: %s\n", spkfile.GetAuthor().c_str() );
		if ( !spkfile.GetVersion().Empty() ) printf ( "Script Version: %s\n", spkfile.GetVersion().c_str() );
		if ( !spkfile.GetCreationDate().Empty() ) printf ( "Creation Date: %s\n", spkfile.GetCreationDate().c_str() );
		if ( !spkfile.GetDescription().Empty() ) printf ( "Description: %s\n", spkfile.GetDescription().c_str() );
		if ( spkfile.IsPatch() )
			printf ( "Script Type: Patch Mod\n" );
		else if ( spkfile.IsCustomStart() )
			printf ( "Script Type: Custom Start\n" );
		else if ( !spkfile.GetScriptType().Empty() ) 
		{
			String type = spkfile.GetScriptType().FindReplace ( "<br>", "|" );
			printf ( "Script Type: %s\n", type.GetToken ( 1, '|' ).c_str() );
		}
		if ( !spkfile.GetWebSite().Empty() ) printf ( "Web Site Address: %s\n", spkfile.GetWebSite().c_str() );
		if ( !spkfile.GetWebAddress().Empty() ) printf ( "Update Address: %s\n", spkfile.GetWebAddress().c_str() );
		if ( !spkfile.GetWebMirror1().Empty() ) printf ( "Update Mirror Address: %s\n", spkfile.GetWebMirror1().c_str() );
		if ( !spkfile.GetWebMirror2().Empty() ) printf ( "Update Mirror Address: %s\n", spkfile.GetWebMirror2().c_str() );
		if ( (!spkfile.GetOtherName().Empty()) && (!spkfile.GetOtherAuthor().Empty()) ) printf ( "Script is a child to the mod: %s by %s\n", spkfile.GetOtherName().c_str(), spkfile.GetOtherAuthor().c_str() );
		if ( spkfile.GetIcon() )
		{
			CFile *icon = spkfile.GetIcon();
			printf ( "Icon File Found, Type: %s, Size: %s\n", spkfile.GetIconExt().c_str(), icon->GetDataSizeString().c_str() );
		}

		if ( spkfile.GetFileList()->size() )
		{
			printf ( "\nListing files in package:\n" );
			CLinkList<CFile> *list = spkfile.GetFileList();
			for ( CFile *file = list->First(); file; file = list->Next() )
				printf ( "\t%s (%s) Size: %s\n", file->GetNameDirectory(&spkfile).c_str(), file->GetFileTypeString().c_str(), file->GetDataSizeString().c_str() );
		}
		else
			printf ( "\nThere are currently no files in the package\n" );
	}
	else if ( check == SPKFILE_MULTI )
	{
		CMultiSpkFile spkfile;
		printf ( "* Opening Multi-SPK file, %s...\n", filename.c_str() );
		if ( !spkfile.ReadFile ( filename, false ) )
		{
			printf ( "Error: Failed to open the Multi-SPK file, %s\n", filename.c_str() );
			return;
		}

		printf ( "Multi Package Name: %s\n", spkfile.GetName().c_str() );
		printf ( "Selection Mode: " );
		if ( spkfile.IsSelection () )
			printf ( "On\n" );
		else
			printf ( "Off\n" );

		CLinkList<SMultiSpkFile> *list = spkfile.GetFileList();
		for ( SMultiSpkFile *ms = list->First(); ms; ms = list->Next() )
		{
			printf ( "File, %s:\n", ms->sName.c_str() );
			printf ( "\tSize: %s\n", CFile::GetSizeString (ms->lSize).c_str() );
			if ( (!ms->sScriptName.Empty()) && (!ms->sScriptAuthor.Empty()) )
				printf ( "\tScript: %s %s by %s\n", ms->sScriptName.c_str(), ms->sScriptVersion.c_str(), ms->sScriptAuthor.c_str() );
			printf ( "\tDefault Install: " );
			if ( ms->bOn )
				printf ( "Yes\n" );
			else
				printf ( "No\n" );
		}
	}
	else
		printf ( "File, %s, is not a valid SPK file\n", filename.c_str() );
}

void AppendFile ( String sfile, String type, String addfile )
{
	int t = GetFileTypeFromString(type);
	if ( t == -1 )
	{
		printf ( "The file type \"%s\" is invalid\n", type.c_str() );
		return;
	}

	CSpkFile spkfile;
	printf ( "Opening SPK File, %s... ", sfile.c_str() );
	if ( !spkfile.ReadFile ( sfile ) )
	{
		printf ( "(Error)\nUnable to open the SPK file, %s\n", sfile.c_str() );
		return;
	}
	printf ( "(Done)\n" );

	MyProgress progress(0);

	printf ( "Adding file %s to package\n\t>", addfile.c_str() );
		
	if ( spkfile.AppendFile ( addfile, t, NullString, &progress ) )
	{
		progress.PrintDone();
		printf ( "< (Done)\n" );

		spkfile.WriteFile ( sfile );
		printf ( "\nSPK file has been written sucessfully\n" );
	}
	else
		printf ( "< (Error)\n" );
}

void RemoveFile ( String sfile, String type, String addfile )
{
	FILE *id = fopen ( sfile.c_str(), "rb+" );
	if ( !id )
	{
		printf ( "Error: File, %s, doesn't exist\n", sfile.c_str() );
		return;
	}

	int check = CSpkFile::CheckFile ( sfile );
	if ( check == SPKFILE_SINGLE )
	{
		if ( addfile.Empty() )
		{
			printf ( "Error: Remove filename is invalid\n" );
			return;
		}
		int t = GetFileTypeFromString(type);
		if ( t == -1 )
		{
			printf ( "The file type \"%s\" is invalid\n", type.c_str() );
			return;
		}

		CSpkFile spkfile;
		printf ( "Opening SPK File, %s... ", sfile.c_str() );
		if ( !spkfile.ReadFile ( sfile ) )
		{
			printf ( "(Error)\nUnable to open the SPK file, %s\n", sfile.c_str() );
			return;
		}
		printf ( "(Done)\n" );

		printf ( "Removing file, %s, from Package\n", addfile.c_str() );

		if ( spkfile.RemoveFile ( addfile, t ) )
		{
			printf ( "File, %s, has been remove from package\n", addfile.c_str() );
			spkfile.WriteFile ( sfile );
			printf ( "SPK file has been written to disk successfully\n" );
		}
		else
			printf ( "Unable to remove the file, %s, from the package\n", addfile.c_str() );
	}
	else if ( check == SPKFILE_MULTI )
	{
		CMultiSpkFile spkfile;
		printf ( "Opening Multi-SPK file, %s...", sfile.c_str() );
		if ( !spkfile.ReadFile ( sfile ) )
		{
			printf ( "(Error)\nUnable to open the Multi-SPK file, %s\n", sfile.c_str() );
			return;
		}
		printf ( "(Done)\n" );

		SMultiSpkFile *ms = spkfile.FindFile ( type );
		if ( !ms )
		{
			printf ( "Unable to find the file \"%s\" in the package\n", type.c_str() );
			return;
		}

		printf ( "Removing file, %s, from Package\n", addfile.c_str() );
		if ( !spkfile.RemoveFile ( ms ) )
		{
			printf ( "Error: Unable to remove file, %s, from package\n", type.c_str() );
			return;
		}

		printf ( "Writing SPK File, %s... ", sfile.c_str() );
		if ( spkfile.WriteFile ( sfile ) )
			printf ( "(Done)\n" );
		else
			printf ( "(Error)\n" );
	}
	else
		printf ( "Error: Invalid file format, unable to open\n" );
}


void CreateMultiFile ( String filename )
{
	printf ( "* Creating new Multi-SPK File, %s\n\n", filename.c_str() );

	FILE *id = fopen ( filename.c_str(), "rb+" );
	if ( id )
	{	
		fclose ( id );
		printf ( "* File already exists, unable to create\n" );
		return;
	}

	id = fopen ( filename.c_str(), "wb" );
	if ( !id )
	{
		printf ( "* Unable to open file for writing\n" );
		return;
	}
	fclose ( id );
	remove ( filename.c_str() );

	CMultiSpkFile spkfile;

	String sInput;
	printf ( "Enter Multi-Spk Package Name: " );
	spkfile.SetName ( GetInput() );

	while ( true )
	{
		printf ( "\nDo you want users to select scripts to install? (Y/N): " );
		String i = GetInput();
		i = i.ToUpper();
		if ( i == "Y" )
			spkfile.SetSelection ( true );
		if ( (i == "Y") || (i == "N") )
			break;
	}

	while ( true )
	{
		printf ( "\nEnter Spk File to add (Enter \"0\" to finish): " );
		sInput = GetInput();
		if ( sInput == "0" )
			break;

		// check if file can be opened
		FILE *id2 = fopen ( sInput.c_str(), "rb+" );
		if ( !id2 )
			printf ( "Error: Unable to open SPK file %s\n", sInput.c_str() );
		else
		{
			fclose ( id2 );
			if ( !spkfile.AddFile ( sInput ) )
				printf ( "Error: Unable to add SPK file to package\n" );
			else
				printf ( "File Added to package (%s)\n", sInput.c_str() );
		}
	}

	if ( spkfile.GetNumFiles() < 1 )
		printf ( "\nError: You have added no files, you must add at least one file to create a package\n" );
	else
	{
		printf ( "Writing MultiSpk file... " );
		if ( spkfile.WriteFile ( filename ) )
			printf ( "(Done)\n" );
		else
			printf ( "(Error)\n" );
	}

}

void CreateFile ( String filename )
{
	printf ( "* Creating new SPK File, %s\n\n", filename.c_str() );

	FILE *id = fopen ( filename.c_str(), "rb+" );
	if ( id )
	{	
		fclose ( id );
		printf ( "* File already exists, unable to create\n" );
		return;
	}

	id = fopen ( filename.c_str(), "wb" );
	if ( !id )
	{
		printf ( "* Unable to open file for writing\n" );
		return;
	}
	fclose ( id );
	remove ( filename.c_str() );

	CSpkFile spkfile;

	printf ( "Enter Script Name: " );
	spkfile.SetName ( GetInput() );

	printf ( "Enter Script Author: " );
	spkfile.SetAuthor ( GetInput() );

	printf ( "Enter Script Version: " );
	spkfile.SetVersion ( GetInput() );

	printf ( "Enter Script Description: " );
	spkfile.SetDescription ( GetInput() );

	struct tm   *currDate;
	char    dateString[100];
	time_t now = time(NULL);

	currDate = localtime( &now );
	strftime(dateString, sizeof dateString, "%d %m %Y", currDate);
	spkfile.SetCreationDate ( String(dateString) );

	spkfile.WriteFile ( filename, NULL );
	printf ( "SPK file has been written to disk\n", filename.c_str() );
}

void ExtractFiles ( String sfile, String dir )
{
	// open the SPK file
	CSpkFile spkfile;
	printf ( "Opening SPK File, %s... ", sfile.c_str() );
	if ( !spkfile.ReadFile ( sfile ) )
	{
		printf ( "(Error)\nUnable to open the SPK file, %s\n", sfile.c_str() );
		return;
	}
	printf ( "(Done)\n" );

	printf ( "Extracting all files from archive..." );
	if ( spkfile.ExtractAll ( dir ) )
		printf ( "(Done)\nFiles have been extracted successfully\n" );
	else
		printf ( "(Error)\nThere was a problem extracting the files\n" );
}


/*
	Func:	AppendMultiFile
	Args:	String toFile	- The spk file to append onto
			String addfile	- The spk file to append
	Desc:	Appends a spk file into a Multi-Spk Archive
			If toFile is a single spk file, it will be converted to a Multi-Spk File
			if addfile is a Multi-Spk file, all files from it will be appended
*/
void AppendMultiFile ( String toFile, String addfile )
{
	// create destination object
	CMultiSpkFile spkfile;

	// checks the destination
	int checkto = CSpkFile::CheckFile ( toFile );

	// if the destination is a single file, then convert it to a Multi-Spk package
	if ( checkto == SPKFILE_SINGLE )
	{
		// adds the single file to the Multi-Spk object
		// Add file also reads it into memory
		if ( !spkfile.AddFile ( toFile ) )
		{
			printf ( "Error: Unable to create Multi-Spk file\n" );
			return;
		}
	}
	else
	{
		// if its already a multispk file, then simply open it and read it to memory
		if ( !spkfile.ReadFile ( toFile ) )
		{
			printf ( "Error: Unable to open Multi-Spk file, %s\n", toFile.c_str() );
			return;
		}
	}

	// now add the file into the Multi-Spk Object
	// the AddFile function will handle both single and Multi-Spk files
	// So you dont need to test what the appending file is
	if ( spkfile.AddFile ( addfile ) )
	{

		// if it added correctly, then simply write the new Multi-Spk Object to disk
		printf ( "File, %s, has been added to Multi-Spk Package\n", addfile.c_str() );
		printf ( "Saving Multi-Spk File: %s... ", toFile.c_str() );
		if ( spkfile.WriteFile ( toFile ) )
			printf ( "(Done)\n" );
		else
			printf ( "(Error)\n" );
	}
	else
		printf ( "Error: Unable to add files, %s, to Multi-Spk Package\n", addfile.c_str() );
}

/*
	Func:	ExtractFile
	Args:	String sfile	- the spk file to read from
			String type		- the type of the file to find
			String addfile	- The filename to extract
			String dir		- The directory to extract to
	Desc:	Finds and extracts a file from a Spk Package
*/
void ExtractFile ( String sfile, String type, String addfile, String dir )
{
	// First checks if the file exists by opening it
	FILE *id = fopen ( sfile.c_str(), "rb+" );
	if ( !id )
	{
		printf ( "Error: File, %s, doesn't exist\n", sfile.c_str() );
		return;
	}
	fclose ( id );

	// now check the type of file it is, using the static function CheckFile(filename).
	// This will return the type of file
	//		SPK_INVALID - Invalid file format, wont be able to open
	//		SPK_SINGLE	- A Single spk package file
	//		SPK_MULTI	- A Multi-Spk Archive
	int check = CSpkFile::CheckFile ( sfile );

	// extracts a file from single packages file
	if ( check == SPKFILE_SINGLE )
	{
		// first get the file type is valid
		// converts the string type into its filetype flag
		int t = GetFileTypeFromString(type);
		// incorrect text type, display the error
		if ( t == -1 )
		{
			printf ( "The file type \"%s\" is invalid\n", type.c_str() );
			return;
		}

		// creates the spkfile object
		CSpkFile spkfile;
		printf ( "Opening SPK File, %s... ", sfile.c_str() );
		// reads the file into memory
		// the SPKREAD_NODATA flag causes it to just read the settings, and skips the file data
		if ( !spkfile.ReadFile ( sfile, SPKREAD_NODATA ) )
		{
			printf ( "(Error)\nUnable to open the SPK file, %s\n", sfile.c_str() );
			return;
		}

		// No read all the file data into memory
		// This can be done all together in the ReadFile function
		// Doing it seperatly allows you to save time if an error occurs, so you dont have to wait for it to read the whole thing
		spkfile.ReadAllFilesToMemory ();

		printf ( "(Done)\n" );

		// uses the FindFile function to find the selected file in the archive
		// requires the filename, type and custom directory (only for Extra File Type)
		CFile *f = spkfile.FindFile ( addfile, t );
		if ( !f )
		{
			printf ( "Unable to find the file \"%s\" in the package\n", addfile.c_str() );
			return;
		}

		// creates the directory so it can be extracted
		if ( !CreateDirectory (dir + "//" + f->GetDirectory(&spkfile)) )
		{
			printf ( "Unable to create the directory \"%s\" to extract into\n", dir.c_str() );
			return;
		}

		// sets up the progress pointer
		// if it uses 7zip, the progress will be displayed in the command prompt
		MyProgress progress(0);
		printf ( "Extracting the file from package\n\t>" );

		// Extracts the file to the specified directory
		if ( !spkfile.ExtractFile ( f, dir, true, &progress ) )
			printf ( "< (Error)\nUnable to extract the file\n" );
		else
		{
			progress.PrintDone ();
			printf ( "< (Done)\nFile has been extracted successfully\n" );
		}
	}

	// the file is a Multi-Spk File, extracts a single spk file from the archive
	else if ( check == SPKFILE_MULTI )
	{
		// creates MultiSpkFile object
		CMultiSpkFile spkfile;
		printf ( "Opening Multi-SPK file, %s...", sfile.c_str() );

		// reads the MultiSpkFile into memory
		if ( !spkfile.ReadFile ( sfile ) )
		{
			printf ( "(Error)\nUnable to open the Multi-SPK file, %s\n", sfile.c_str() );
			return;
		}
		printf ( "(Done)\n" );

		// searchs the archive for a matching file
		SMultiSpkFile *ms = spkfile.FindFile ( type );
		if ( !ms )
		{
			printf ( "Unable to find the file \"%s\" in the package\n", type.c_str() );
			return;
		}

		// extracts the file from the archive, to the given directory
		printf ( "Extracting SPK file, %s, from package... ", ms->sName.c_str() );
		if ( spkfile.ExtractFile ( ms, addfile ) )
			printf ( "(Done)\n" );
		else
			printf ( "(Error)\n" );
	}
	else
		printf ( "Error: Invalid file format, unable to open\n" );
}

/*
	Func:	SplitMulti
	Args:	String filename - the filename of the multispk file to open
			String dest		- The destination directory to extract the files to
	Desc:	Splits a multi-spk file into its seperate spk files
*/
void SplitMulti ( String filename, String dest )
{
	// Check the file type, Must return SPKFILE_MULTI, otherwise its not a Multi-Spk Packages
	if ( CSpkFile::CheckFile ( filename ) != SPKFILE_MULTI )
	{
		printf ( "Error: The file is not a Multi-Spk packages\n" );
		return;
	}

	printf ( "Spliting Multi-SPK File to, %s... ", dest.c_str() );

	// create the MultiSpkFile object
	CMultiSpkFile spkfile;

	// Splits the files to the destination
	if ( !spkfile.SplitMulti  ( filename, dest ) )
	{
		printf ( "(Error)\nUnable to split Multi-SPK package, %s\n", filename.c_str() );
		return;
	}

	printf ( "(Done)\n" );
}


/*
	Main entry point to program
*/
int main ( int argc, char **argv )
{
	// display program header to command prompt
	printf ( "\nSPKTool V1.00 (SPK File Version %.2f) 12/06/2007 Created by Cycrow\n\n", (float)FILEVERSION );

	// parse the cmd name
	String cmd (argv[0]);
	cmd = cmd.FindReplace ( "\\", "/" );
	g_dir = cmd.GetToken ( 0, cmd.NumToken('/') - 1, '/' );
	cmd = cmd.GetToken ( cmd.NumToken('/'), '/' );

	g_read = false;

	// not enough arguments, display the syntax and exit
	if ( argc < 2 )
	{
		PrintSyntax ( cmd );
		exit ( 1 );
	}

	// get the command flag
	String command(argv[1]);

	// display the contents of the spk file
	if ( command == "-v" )
	{
		if ( argc < 3 )
			printf ( "Syntax: %s -v <spkfile>\n\tWill open and display the contents of the spkfile\n", cmd.c_str() );
		else
			DisplayVersion ( argv[2] );		
	}

	// creates a new spk file
	else if ( command == "-c" )
	{
		if ( argc < 3 )
			printf ( "Syntax: %s -c <spkfile>\n\tThis will create a new SPK file and allow you to set some basic settings for the file\n", cmd.c_str() );
		else
			CreateFile ( argv[2] );		
	}

	// appends a file onto the spk archive
	else if ( command == "-a" )
	{
		if ( argc < 4 )
			printf ( "Syntax: %s -a <spkfile> <type> <filename>\n\tThis will append the file into the archive and compress it according to the files default compression\n\t<type> = Script, Text, Mod, Map, Readme, Uninstall, Sound, Extra, Screen\n", cmd.c_str() );
		else
		{
			String arg4;
			if ( argc > 4 )
				arg4 = argv[4];
			AppendFile ( argv[2], argv[3], arg4 );
		}
	}

	// removes a file from the spk archive
	else if ( command == "-r" )
	{
		if ( argc < 4 )
		{
			printf ( "Syntax:\n\t%s -r <spkfile> <type> <filename>\n\tThis will remove a file from the archive\n\t<type> = Script, Text, Mod, Map, Readme, Uninstall, Sound, Extra, Screen\n", cmd.c_str() );
			printf ( "\t%s -r <multispkfile> <filename>\n\tThis will remove a spk file from the Multi-SPK package\n", cmd.c_str() );
		}
		else
		{
			String arg4;
			if ( argc > 4 )
				arg4 = argv[4];
			RemoveFile ( argv[2], argv[3], arg4 );
		}
	}

	// extracts a file from a spk file
	else if ( command == "-x" )
	{
		if ( argc < 4 )
		{
			printf ( "Syntax:\n\t%s -x <spkfile> <type> <filename> [destination]\n\tThis will extract a file from the package and save it to the corect path in <directory>\n\t<type> = Script, Text, Mod, Map, Readme, Uninstall, Sound, Extra, Screen\n", cmd.c_str() );
			printf ( "\t%s -x <multispkfile> <filename> [destination]\n\tThis will extract a spk file from the Multi-Spk package and save it to the destination path\n", cmd.c_str() );
		}
		else
		{
			String arg5, arg4;
			if ( argc > 5 )
				arg5 = argv[5];
			if ( argc > 4 )
				arg4 = argv[4];
			ExtractFile ( argv[2], argv[3], arg4, arg5 );
		}
	}

	// extracts all the files from an archive
	else if ( command == "-e" )
	{
		if ( argc < 3 )
			printf ( "Syntax: %s -e <spkfile> [destination]\n\tThis will extract all files into the destination directory\n", cmd.c_str() );
		else
		{
			String arg3;
			if ( argc > 3 )
				arg3 = argv[3];
			ExtractFiles ( argv[2], arg3 );
		}
	}

	// creates a multispk archive
	else if ( command == "-n" )
	{
		if ( argc < 3 )
			printf ( "Syntax: %s -n <multispkfile>\n\tThis will create a multispk file and allow you to add spk files to it\n", cmd.c_str() );
		else
			CreateMultiFile ( argv[2] );
	}

	// merges 2 multi-spk archives together, or appends a single spk file into a multi-spk file
	else if ( command == "-m" )
	{
		if ( argc < 4 )
			printf ( "Syntax: %s -m <spkfile1> <spkfile2>\n\tThis will add spkfile2 into spkfile1, if spkfile1 is a normal SPK file, it will be saved into a Multi-Spk file\nspkfile2 can also be a Multi-Spk file, all files within it will be added\n", cmd.c_str() );
		else
			AppendMultiFile ( argv[2], argv[3] );
	}

	// splits the multi-spk file, exracts all spk files
	else if ( command == "-s" )
	{
		if ( argc < 3 )
			printf ( "Syntax: %s -s <multispkfile> [destination]\n\tSplits the Multi-SPK file and saves each spk file to the destiantion directory\n", cmd.c_str() );
		else
		{
			String arg3;
			if ( argc > 3 )
				arg3 = argv[3];
			SplitMulti ( argv[2], arg3 );
		}
	}

	// not a valid switch, display syntax
	else
		PrintSyntax ( cmd );

#ifdef _DEBUG
	char pause;
	scanf ( "%s", &pause );
#endif

	return 0;
}
