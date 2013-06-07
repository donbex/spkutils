#include "XspFile.h"

CXspFile::CXspFile () : CBaseFile()
{
	SetDefaults ();

	m_iType = TYPE_XSP;
}

void CXspFile::Delete ()
{
	CBaseFile::Delete();

	if ( m_pSceneFile )
		delete m_pSceneFile;
	if ( m_pCockpitFile )
		delete m_pCockpitFile;

	m_pSceneFile = m_pCockpitFile = NULL;
}

void CXspFile::SetDefaults ()
{
	CBaseFile::SetDefaults ();

	m_pSceneFile = m_pCockpitFile = NULL;
	m_bLanguageText = m_bExistingShip = false;
	m_iOrgDesc = 0;

	m_iShipyard = SHIPYARD_NONE;
}

void CXspFile::AddText ( int id, String name, String desc )
{
	// first check if theres an existing id
	SText *newtext = NULL;
	for ( SText *t = m_lText.First(); t; t = m_lText.Next() )
	{
		if ( t->iId == id )
		{
			newtext = t;
			break;
		}
	}

	if ( !newtext )
	{
		newtext = new SText;
		newtext->iId = id;
		m_lText.push_back ( newtext );
	}

	newtext->sName = name;
	newtext->sDesc = desc;
}

void CXspFile::AddDummy ( String section, String data )
{
	SDummy *d = new SDummy;
	d->sData = data;
	d->sSection = section;

	if ( d->sData.Right (1) != ";" )
		d->sData += ";";

	m_lDummy.push_back ( d );
}

void CXspFile::AddComponent ( String section, String section2, String data )
{
	SComponent *c = new SComponent;
	c->sData = data;
	c->sSection = section;
	c->sSection2 = section2;

	if ( c->sData.Right (1) != ";" )
		c->sData += ";";

	m_lComponent.push_back ( c );
}

bool CXspFile::IsValid ()
{
	if ( m_sName.Empty() )
		return false;
	if ( m_sAuthor.Empty() )
		return false;

	return true;
}

String CXspFile::CreateValuesLine()
{
	String values = CBaseFile::CreateValuesLine ();

	values += String("Data: ") + m_sData + "\n";
	values += String("ID: ") + m_sID + "\n";

	if ( m_bLanguageText )
		values += "LanguageText\n";
	if ( m_bExistingShip )
		values += "ExistingShip\n";
	values += String("OrgDesc: ") + (long)m_iOrgDesc + "\n";
	values += String("Shipyard: ") + (long)m_iShipyard + "\n";

	for ( SStringList *node = m_lCockpit.Head(); node; node = node->next )
		values += String("Cockpit: ") + node->data + "\n";

	for ( SText *text = m_lText.First(); text; text = m_lText.Next() )
		values += String("Text: ") + (long)text->iId + "|" + text->sName.FindReplace("|", "<::PiPe::>") + "|" + text->sDesc + "\n";

	for ( SComponent *comp = m_lComponent.First(); comp; comp = m_lComponent.Next() )
		values += String("Component: ") + comp->sData.FindReplace("|", "<::PiPe::>") + "|" + comp->sSection.FindReplace("|", "<::PiPe::>") + "|" + comp->sSection2 + "\n";

	for ( SDummy *dummy = m_lDummy.First(); dummy; dummy = m_lDummy.Next() )
		values += String("Dummy: ") + dummy->sData.FindReplace("|", "<::PiPe::>") + "|" + dummy->sSection + "\n";

	return values;
}

bool CXspFile::ParseValueLine ( String line )
{
	String first = line.GetToken ( 1, ' ' );
	String rest  = line.GetToken ( 2, -1, ' ' );

	if ( first == "Data:" )
		m_sData = rest;
	else if ( first == "ID:" )
		m_sID = rest;
	else if ( line == "LanguageText" )
		m_bLanguageText = true;
	else if ( line == "ExistingShip" )
		m_bExistingShip = true;
	else if ( first == "OrgDesc:" )
		m_iOrgDesc = rest.ToInt();
	else if ( first == "Shipyard:" )
		m_iShipyard = rest.ToInt();
	else if ( first == "Cockpit:" )
		m_lCockpit.PushBack ( rest );
	else if ( first == "Text:" )
	{
		SText *text = new SText;
		text->iId = rest.GetToken ( "|", 1, 1 ).ToInt();
		text->sName = rest.GetToken ( "|", 2, 2 ).FindReplace ("<::PiPe::>", "|");
		text->sDesc = rest.GetToken ( "|", 3 );
		m_lText.push_back ( text );
	}
	else if ( first == "Component:" )
	{
		SComponent *c = new SComponent;
		c->sData = rest.GetToken ( "|", 1, 1 ).FindReplace ("<::PiPe::>", "|");
		c->sSection = rest.GetToken ( "|", 2, 2 ).FindReplace ("<::PiPe::>", "|");
		c->sSection2 = rest.GetToken ( "|", 3 );
		m_lComponent.push_back ( c );
	}
	else if ( first == "Dummy:" )
	{
		SDummy *d = new SDummy;
		d->sData = rest.GetToken ( "|", 1, 1 ).FindReplace ("<::PiPe::>", "|");
		d->sSection = rest.GetToken ( "|", 2 );
		m_lDummy.push_back ( d );
	}
	else
		return CBaseFile::ParseValueLine ( line );

	return true;
}

String CXspFile::GetShipName(int lang, String sLang)
{
	String name;
	if ( (m_bLanguageText) && (lang) )
	{
		for ( SText *text = m_lText.First(); text; text = m_lText.Next() )
		{
			if ( text->iId == lang )
			{
				name = text->sName;
				break;
			}
		}
	}

	if ( name.Empty() )
		name = GetLanguageName(sLang);

	return name;
}

