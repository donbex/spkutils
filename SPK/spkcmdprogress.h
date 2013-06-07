#include "SpkFile.h"

class MyProgress : public CProgressInfo
{
public:
	MyProgress ( int t = 0 ) { current = 0; type = t; }

	int GetCurrent() { return current; }
	void ProgressUpdated ( const long cur, const long max )
	{
		int percent = (int)(((float)cur / (float)max) * 100);
		while ( (percent - current) >= 5 )
		{
			current += 5;
			PrintType();
		}
	}

	void DoingFile ( CFile *file ) {}

	void PrintDone ()
	{
		while ( current < 100 )
		{
			current += 5;
			PrintType();
		}
		Reset ();
	}

	void Reset ()
	{
		current = 0;
	}

private:
	void PrintType ()
	{
		if ( type == 0 )
			printf ( "#" );
		else
		{
			if ( (current % 10) )
				printf ( "#" );
			else if ( current >= 100 )
				printf ( "0" );
			else
				printf ( "%1d", current / 10 );
		}
	}

	int type;
	int current;
};
