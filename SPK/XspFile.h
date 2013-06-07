#ifndef _XSPFILE_H__
#define _XSPFILE_H__

#include "SpkFile.h"
#include "StringList.h"

#define SHIPYARD_NONE		  0
#define SHIPYARD_ARGON		  1
#define SHIPYARD_BORON		  2
#define SHIPYARD_PARANID	  4
#define SHIPYARD_SPLIT		  8
#define SHIPYARD_TELADI		 16
#define SHIPYARD_PIRATES	 32
#define SHIPYARD_FRIEND		 64
#define SHIPYARD_XENON		128
#define SHIPYARD_TERRAN		256

typedef struct SText
{
	int		iId;
	String sName;
	String sDesc;
} SText;

typedef struct SDummy
{
	String	sSection;
	String sData;
} SDummy;

typedef struct SComponent
{
	String sSection;
	String sSection2;
	String sData;
} SComponent;

class CXspFile : public CBaseFile
{
public:
	CXspFile ();

	bool IsLanguageText () { return m_bLanguageText; }
	bool IsExistingShip () { return m_bExistingShip; }

	int  GetOriginalDescription () { return m_iOrgDesc; }
	int  GetShipyards () { return m_iShipyard; }

	String GetShipID () { return m_sID; }
	String GetShipData () { return m_sData; }
	String GetShipFilename () 
	{ 
		String s = m_sName + "-" + m_sAuthor; 
		s = s.Remove ( ' ' );
		s = s.Remove ( '\\' );
		s = s.Remove ( '/' );
		return s;
	}


	void SetLanguageText		( bool b )   { m_bLanguageText = b; }
	void SetExistingShip		( bool b )   { m_bExistingShip = b; }
	void SetOriginalDescription	( int i )	 { m_iOrgDesc = i; }
	void SetShipID				( String s ) { m_sID = s; }
	void SetShipData			( String s ) { m_sData = s; }

	void SetSceneFile ( CFile *f ) { m_pSceneFile = f; }
	void SetCockpitFile ( CFile *f ) { m_pCockpitFile = f; }

	void AddText ( int id, String name, String desc );
	void AddCockpit ( String cockpit ) { m_lCockpit.PushBack ( cockpit ); }
	void AddDummy ( String section, String data );
	void AddComponent ( String section, String section2, String data );

	void AddShipyard ( int s ) { m_iShipyard |= s; }
	void RemoveShipyard ( int s ) { m_iShipyard &= ~(s); }

	bool IsValid ();

	String CreateValuesLine ();
	bool ParseValueLine ( String line );

	virtual int GetType () { return TYPE_XSP; }

	String GetShipName(int lang, String sLang);

protected:
	virtual void Delete ();
	virtual void SetDefaults ();

	CFile *m_pSceneFile, *m_pCockpitFile;

	bool m_bLanguageText, m_bExistingShip;

	int m_iOrgDesc;

	String m_sID;
	String m_sData;

	CStringList m_lCockpit;
	CLinkList<SText> m_lText;
	CLinkList<SComponent> m_lComponent;
	CLinkList<SDummy> m_lDummy;

	int m_iShipyard;
};


#endif //_XSPFILE_H__

