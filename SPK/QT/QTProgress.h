#ifndef __QTPROGRESS_H__
#define __QTPROGRESS_H__

#include "../spk.h"
#include <QProgressBar.h>

class QtMyProgress : public CProgressInfo7Zip
{
public:
	QtMyProgress ( QProgressBar *bar ) { m_pBar = bar; }

	virtual void ProgressUpdated ( const long cur, const long max )
	{
		if ( m_pBar )
			m_pBar->setProgress ( cur, max );
	}
	virtual void DoingFile ( CFile *file ) {}

protected:
	QProgressBar *m_pBar;
};

class QtFullProgress : public QtMyProgress
{
public:
	QtFullProgress ( QProgressBar *bar, long fullsize ) : QtMyProgress ( bar )
	{
		m_lFullSize = fullsize;
		m_lFileSize = 0;
		m_lCurrent = 0;
	}
	virtual void ProgressUpdated ( const long cur, const long max )
	{
		float diff = (float)m_lFileSize / (float)(int)max;
		UInt64 newCur = (UInt64)((int)cur * diff);

		QtMyProgress::ProgressUpdated ( (UInt64)(m_lCurrent + newCur), (UInt64)m_lFullSize );
	}
	void SetNextFile ( long size )
	{
		m_lCurrent += m_lFileSize;
		m_lFileSize = size;
	}
	virtual void DoingFile ( CFile *file ) { SetNextFile ( file->GetUncompressedDataSize() ); }

private:
	long m_lFullSize;
	long m_lFileSize;
	long m_lCurrent;
};

#endif //__QTPROGRESS_H__