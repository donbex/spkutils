#include "XFile.h"

CXFile::CXFile () : CFile ()
{
}


CModelFile::CModelFile () : CXFile ()
{
	m_iFileType = FILETYPE_SHIPMODEL;
}

