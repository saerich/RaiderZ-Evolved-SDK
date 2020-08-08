// ============================================================================
// Property.h
// Copyright ©2000, Asylum Software
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __PROPERTY_H__
#define __PROPERTY_H__

#include "max.h"

#define PROP_STRING  1
#define PROP_INTEGER 2
#define PROP_BOOLEAN 3
#define PROP_REAL    4
#define PROP_ARRAY   5
#define PROP_FILENAME 6
#define PROP_OPTION  7
#define PROP_NAME	 8
#define PROP_LITERAL 9
#define PROP_POINT2  10
#define PROP_POINT3  11

#define FLAG_READONLY   0x00000001L
#define FLAG_DONTSAVE   0x00000002L
#define FLAG_DONTEMIT   0x00000004L
#define FLAG_CHANGED    0x00000008L
#define FLAG_BADVALUE   0x00000010L
#define FLAG_HIDDEN     0x00000020L

// ============================================================================
class CProperty : public CObject
{
public:
	CString m_name;
	int     m_type;
	UINT    m_flags;

	// possible prop values
	CString m_string;
	CStringArray m_array;
	Point3	m_point3;
	Point2  m_point2;
	union
	{
		int     m_integer;
		float   m_real;
		bool    m_boolean;
	};

	// construct & set type & default value
	CProperty();
	CProperty(const CProperty &src);
	CProperty(const CString &name, const CString &string, UINT flags=0);  // PROP_STRING
	CProperty(const CString &name, const CStringArray &array, UINT flags=0); // PROP_ARRAY
	CProperty(const CString &name, const CStringArray &array, int idx, UINT flags=0); // PROP_OPTION
	CProperty(const CString &name, int integer, UINT flags=0); // PROP_INTEGER
	CProperty(const CString &name, float real, UINT flags=0);  // PROP_REAL
	CProperty(const CString &name, bool boolean, UINT flags=0); // PROP_BOOLEAN
	CProperty(const CString &name, Point3 p3, UINT flags=0); // PROP_POINT3
	CProperty(const CString &name, Point2 p2, UINT flags=0); // PROP_POINT2
	CProperty(const CString &name, int type, const CString &string, UINT flags=0);  // PROP_STRING/PROP_NAME/PROP_LITERAL

	void operator =(const CProperty &src);
	void operator =(const CString &src);
	void operator =(const CStringArray &src);
	void operator =(LPCSTR lpsz);
	void operator =(int integer);
	void operator =(float real);
	void operator =(bool boolean);
	void operator =(Point3 p3);
	void operator =(Point2 p2);
	operator LPCTSTR();
	operator int() const;
	operator bool() const;
	operator float() const;
	operator Point3() const;
	operator Point2() const;

	BOOL IsReadOnly() { return (BOOL)(m_flags & FLAG_READONLY); }
	virtual BOOL HasDefaultValue() { return !(m_flags & FLAG_CHANGED); }   // HEY make this smarter
};


// ============================================================================
class CHandler : public CObject
{
public:
	bool    m_use;
	CString m_name;
	CString m_args;
	CString m_text;

	CHandler();
	CHandler(const CHandler &src);
	CHandler(bool use, CString name, CString args, CString text);
	void operator =(const CHandler &src);
};


#endif //__PROPERTY_H__