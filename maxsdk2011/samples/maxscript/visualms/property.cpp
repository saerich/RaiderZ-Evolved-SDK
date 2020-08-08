// ============================================================================
// Property.cpp
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "Property.h"
#include "VisualMSDoc.h"

#pragma warning(push)
#pragma warning(disable : 4800)   // forcing value to bool 'true' or 'false'


// ============================================================================
CProperty::CProperty()
{
	m_name = m_string = _T("");
	m_type = 0;
	m_flags = 0;
}

// ============================================================================
CProperty::CProperty(const CProperty &src)
{
	m_name = src.m_name;
	m_type = src.m_type;
	m_flags = src.m_flags;
	switch(m_type)
	{
	case PROP_STRING:
	case PROP_NAME: 
	case PROP_LITERAL: m_string = src.m_string; break;
	case PROP_INTEGER: m_integer = src.m_integer; break;
	case PROP_REAL: m_real = src.m_real; break;
	case PROP_BOOLEAN: m_boolean = src.m_boolean; break;
	case PROP_POINT2: m_point2 = src.m_point2; break;
	case PROP_POINT3: m_point3 = src.m_point3; break;

	// NOTE: this must fall through since m_array is a list of the options it can be.
	case PROP_OPTION:
		m_integer = src.m_integer;
	case PROP_ARRAY:
		m_array.RemoveAll();
		m_array.Copy(src.m_array);
		break;
	}
}

// ============================================================================
CProperty::CProperty(const CString &name, const CString &string, UINT flags)
{
	m_name = name;
	m_type = PROP_STRING;
	m_string = string;
	m_flags = flags;
}

// ============================================================================
CProperty::CProperty(const CString &name, const CStringArray &array, UINT flags)
{
	m_name = name;
	m_type = PROP_ARRAY;
	m_array.Copy(array);
	m_flags = flags;
}

// ============================================================================
CProperty::CProperty(const CString &name, const CStringArray &array, int idx, UINT flags)
{
	m_name = name;
	m_type = PROP_OPTION;
	m_integer = idx;
	m_array.Copy(array);
	m_flags = flags;
}

// ============================================================================
CProperty::CProperty(const CString &name, int integer, UINT flags)
{
	m_name = name;
	m_type = PROP_INTEGER;
	m_integer = integer;
	m_flags = flags;
}

// ============================================================================
CProperty::CProperty(const CString &name, float real, UINT flags)
{
	m_name = name;
	m_type = PROP_REAL;
	m_real = real;
	m_flags = flags;
}

// ============================================================================
CProperty::CProperty(const CString &name, bool boolean, UINT flags)
{
	m_name = name;
	m_type = PROP_BOOLEAN;
	m_boolean = boolean;
	m_flags = flags;
}

// ============================================================================
CProperty::CProperty(const CString &name, Point3 p3, UINT flags)
{
	m_name = name;
	m_type = PROP_POINT3;
	m_point3 = p3;
	m_flags = flags;
}

// ============================================================================
CProperty::CProperty(const CString &name, Point2 p2, UINT flags)
{
	m_name = name;
	m_type = PROP_POINT2;
	m_point2 = p2;
	m_flags = flags;
}

// ============================================================================
CProperty::CProperty(const CString &name, int type, const CString &string, UINT flags)
{
	m_name = name;
	m_type = type;
	m_string = string;
	m_flags = flags;
}

// ============================================================================
void CProperty::operator =(const CProperty &src)
{
	m_name = src.m_name;
	m_type = src.m_type;
	m_flags = src.m_flags;
	switch(m_type)
	{
	case PROP_STRING:
	case PROP_NAME: 
	case PROP_LITERAL: m_string = src.m_string; break;
	case PROP_INTEGER: m_integer = src.m_integer; break;
	case PROP_REAL: m_real = src.m_real; break;
	case PROP_BOOLEAN: m_boolean = src.m_boolean; break;
	case PROP_POINT2: m_point2 = src.m_point2; break;
	case PROP_POINT3: m_point3 = src.m_point3; break;

	// NOTE: this must fall through since m_array is a list of the options it can be.
	case PROP_OPTION:
		m_integer = src.m_integer;
	case PROP_ARRAY:
		m_array.RemoveAll();
		m_array.Copy(src.m_array);
		break;
	}
	CVisualMSDoc* pDoc = CVisualMSDoc::GetDoc();
	if(pDoc) pDoc->SetModifiedFlag(TRUE);
}

// ============================================================================
void CProperty::operator =(const CString &src)
{
	m_flags &= ~FLAG_BADVALUE;
	switch(m_type)
	{
	case PROP_OPTION:
		{
         int i;
			for(i = 0; i < m_array.GetSize(); i++)
			{
				if(src.CompareNoCase(m_array[i]) == 0)
				{
					m_integer = i;
					break;  // breaks out of for loop
				}
			}

			if(i < m_array.GetSize())
				break; // found in m_array so break out of switch statement
			else
				m_type = PROP_LITERAL; // not found, morph property into a literal and fall through!
		}
	case PROP_STRING:
	case PROP_NAME:
	case PROP_LITERAL:
		m_string = src;
		break;
	case PROP_INTEGER:
		if (sscanf(src, _T(" %d "), &m_integer) != 1)
			m_flags |= FLAG_BADVALUE;
		break;
	case PROP_REAL:
		if (sscanf(src, _T(" %g "), &m_real) != 1)
			m_flags |= FLAG_BADVALUE;
		break;
	case PROP_BOOLEAN:
		if (CString("true") == src)
			m_boolean = true;
		else if (CString("false") == src)
			m_boolean = false;
		else
			m_flags |= FLAG_BADVALUE;
		break;
	case PROP_POINT2:
		if (sscanf(src, _T(" [ %g , %g ] "), &m_point2.x, &m_point2.y) != 2)
			m_flags |= FLAG_BADVALUE;
		break;
	case PROP_POINT3:
		if (sscanf(src, _T(" [ %g , %g , %g ] "), &m_point3.x, &m_point3.y, &m_point3.z) != 3)
			m_flags |= FLAG_BADVALUE;
		break;
	}
	m_flags |= FLAG_CHANGED;

	CVisualMSDoc* pDoc = CVisualMSDoc::GetDoc();
	if(pDoc) pDoc->SetModifiedFlag(TRUE);
}

// ============================================================================
void CProperty::operator =(LPCSTR lpcstr)
{
	CString src = lpcstr;
	*this = src;

/* DUPLICATE CODE FROM operator =(CString) !
	m_flags &= ~FLAG_BADVALUE;
	switch(m_type)
	{
	case PROP_STRING:
	case PROP_NAME:
	case PROP_LITERAL:
		m_string = src;
		break;
	case PROP_INTEGER:
		if (sscanf(src, _T(" %d "), &m_integer) != 1)
			m_flags |= FLAG_BADVALUE;
		break;
	case PROP_REAL:
		if (sscanf(src, _T(" %g "), &m_real) != 1)
			m_flags |= FLAG_BADVALUE;
		break;
	case PROP_BOOLEAN:
		if (CString("true") == src)
			m_boolean = true;
		else if (CString("false") == src)
			m_boolean = false;
		else
			m_flags |= FLAG_BADVALUE;
		break;
	case PROP_POINT2:
		if (sscanf(src, _T(" [ %g , %g ] "), &m_point2.x, &m_point2.y) != 2)
			m_flags |= FLAG_BADVALUE;
		break;
	case PROP_POINT3:
		if (sscanf(src, _T(" [ %g , %g , %g ] "), &m_point3.x, &m_point3.y, &m_point3.z) != 3)
			m_flags |= FLAG_BADVALUE;
		break;
	}
	CVisualMSDoc* pDoc = CVisualMSDoc::GetDoc();
	if(pDoc) pDoc->SetModifiedFlag(TRUE);
	m_flags |= FLAG_CHANGED;
*/
}

// ============================================================================
void CProperty::operator =(const CStringArray &src)
{
	if(m_type == PROP_ARRAY)
	{
		m_array.RemoveAll();
		m_array.Copy(src);
	}
	m_flags |= FLAG_CHANGED;

	CVisualMSDoc* pDoc = CVisualMSDoc::GetDoc();
	if(pDoc) pDoc->SetModifiedFlag(TRUE);
}

// ============================================================================
void CProperty::operator =(int integer)
{
	switch(m_type)
	{
	case PROP_STRING:
		m_string.Format("%d", integer);
		break;
	case PROP_INTEGER:
	case PROP_OPTION:
		m_integer = integer;
		break;
	case PROP_REAL:
		m_real = (float)integer;
		break;
	case PROP_BOOLEAN:
		m_boolean = (bool)integer;
		break;
	}
	CVisualMSDoc* pDoc = CVisualMSDoc::GetDoc();
	if(pDoc) pDoc->SetModifiedFlag(TRUE);
	m_flags |= FLAG_CHANGED;
}

// ============================================================================
void CProperty::operator =(float real)
{
	switch(m_type)
	{
	case PROP_STRING:
		m_string.Format("%g", real);
		break;
	case PROP_INTEGER:
	case PROP_OPTION:
		m_integer = (int)real;
		break;
	case PROP_REAL:
		m_real = real;
		break;
	case PROP_BOOLEAN:
		m_boolean = (bool)real;
		break;
	}
	CVisualMSDoc* pDoc = CVisualMSDoc::GetDoc();
	if(pDoc) pDoc->SetModifiedFlag(TRUE);
	m_flags |= FLAG_CHANGED;
}

// ============================================================================
void CProperty::operator =(bool boolean)
{
	switch(m_type)
	{
	case PROP_STRING:
		if(boolean)
			m_string = _T("true");
		else
			m_string = _T("false");
		break;
	case PROP_INTEGER:
		m_integer = (int)boolean;
		break;
	case PROP_REAL:
		m_real = (float)boolean;
		break;
	case PROP_BOOLEAN:
		m_boolean = boolean;
		break;
	}
	CVisualMSDoc* pDoc = CVisualMSDoc::GetDoc();
	if(pDoc) pDoc->SetModifiedFlag(TRUE);
	m_flags |= FLAG_CHANGED;
}

// ============================================================================
void CProperty::operator =(Point3 p3)
{
	switch(m_type)
	{
	case PROP_LITERAL:
	case PROP_STRING:
		m_string.Format("[%g,%g,%g]", p3.x, p3.y, p3.z);
		break;
	case PROP_POINT3:
		m_point3 = p3;
		break;
	}
	CVisualMSDoc* pDoc = CVisualMSDoc::GetDoc();
	if(pDoc) pDoc->SetModifiedFlag(TRUE);
	m_flags |= FLAG_CHANGED;
}

// ============================================================================
void CProperty::operator =(Point2 p2)
{
	switch(m_type)
	{
	case PROP_LITERAL:
	case PROP_STRING:
		m_string.Format("[%g,%g]", p2.x, p2.y);
		break;
	case PROP_POINT2:
		m_point2 = p2;
		break;
	}
	m_flags |= FLAG_CHANGED;
	CVisualMSDoc* pDoc = CVisualMSDoc::GetDoc();
	if(pDoc) pDoc->SetModifiedFlag(TRUE);
}

// ============================================================================
CProperty::operator LPCTSTR()
{
	if (m_flags & FLAG_BADVALUE) 
		return _T("????");

	switch(m_type)
	{
	case PROP_INTEGER:
		m_string.Format("%d", m_integer);
		break;
	case PROP_REAL:
		m_string.Format("%g", m_real);
		break;
	case PROP_BOOLEAN:
		if(m_boolean)
			m_string = _T("true");
		else
			m_string = _T("false");
		break;
	case PROP_ARRAY:
		{
			m_string = _T("#(");
			for(int i = 0; i < m_array.GetSize(); i++)
			{
				m_string += m_array[i];
				if(i != m_array.GetSize()-1)
					m_string += _T(", ");
			}
			m_string += _T(")");
			break;
		}
	case PROP_OPTION:
		if(m_integer >= 0 && m_integer < m_array.GetSize())
			m_string = m_array[m_integer];
		break;
	case PROP_POINT2:
		m_string.Format("[%g,%g]", m_point2.x, m_point2.y);
		break;
	case PROP_POINT3:
		m_string.Format("[%g,%g,%g]", m_point3.x, m_point3.y, m_point3.z);
		break;
	case PROP_STRING:
//		m_string.Format("\"%s\"", m_string);  // TODO: WHY WAS THIS OUTPUTTING QUOTES ???
		break;
	case PROP_NAME:
		m_string.Format("#%s", m_string);
		break;
	}
	return m_string;
}

// ============================================================================
CProperty::operator int() const
{
	switch(m_type)
	{
	case PROP_INTEGER:
	case PROP_OPTION:
		return m_integer;
	case PROP_REAL:
		return (int)m_real;
	case PROP_BOOLEAN:
		return (int)m_boolean;
	}
	return 0;
}

// ============================================================================
CProperty::operator bool() const
{
	switch(m_type)
	{
	case PROP_INTEGER:
		return (bool)m_integer;
	case PROP_REAL:
		return (bool)m_real;
	case PROP_BOOLEAN:
		return m_boolean;
	}
	return false;
}

// ============================================================================
CProperty::operator float() const
{
	switch(m_type)
	{
	case PROP_INTEGER:
	case PROP_OPTION:
		return (float)m_integer;
	case PROP_REAL:
		return m_real;
	case PROP_BOOLEAN:
		return (float)m_boolean;
	}
	return 0.f;
}

// ============================================================================
CProperty::operator Point3() const
{
	switch(m_type)
	{
	case PROP_POINT3:
		return m_point3;
	}
	return Point3(0.f,0.f,0.f);
}

// ============================================================================
CProperty::operator Point2() const
{
	switch(m_type)
	{
	case PROP_POINT2:
		return m_point2;
	}
	return Point2 (0.f,0.f);
}

// ============================================================================
CHandler::CHandler()
{
	m_use = FALSE;
	m_name = m_text = _T("");
	m_args = _T(" do");
}

// ============================================================================
CHandler::CHandler(const CHandler &src)
{
	m_use  = src.m_use;
	m_name = src.m_name;
	m_args = src.m_args;
	m_text = src.m_text;
}

// ============================================================================
CHandler::CHandler(bool use, CString name, CString args, CString text)
{
	m_use = use;
	m_name = name;
	m_args = args;
	m_text = text;
}

// ============================================================================
void CHandler::operator =(const CHandler& src)
{
	m_use  = src.m_use;
	m_name = src.m_name;
	m_args = src.m_args;
	m_text = src.m_text;
}

#pragma warning(pop) 
