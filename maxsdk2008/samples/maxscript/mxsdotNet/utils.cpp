//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: utils.cpp  : MAXScript dotNet common utility code
// AUTHOR: Larry.Minton - created Jan.1.2006
//***************************************************************************/
//

#include "MaxIncludes.h"
#include "dotNetControl.h"
#include "utils.h"
#include "resource.h"

#include "defextfn.h"
#	include "namedefs.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

// 
#pragma managed

using namespace MXS_dotNet;

/* -------------------- MNETStr (MSTR converter) --------------- */
//
MSTR MXS_dotNet::MNETStr::ToMSTR(System::String^ pString)
{
	cli::pin_ptr<const System::Char> pChar = PtrToStringChars( pString );
	const wchar_t *psz = pChar;

	return MSTR(psz);
}

MSTR MXS_dotNet::MNETStr::ToMSTR(System::Exception^ e, bool InnerException)
{
	if( InnerException )
	{
		while( e->InnerException ) e = e->InnerException;
	}

	return ToMSTR(e->Message);
}

/* --------------------- MXS_dotNet functions ------------------ */
//

public ref class MethodInfoSorter: public System::Collections::IComparer
{
private:
	// Does case insensitive compare on MethodInfo names, then on # args
	virtual int Compare( System::Object^ x, System::Object^ y ) sealed = System::Collections::IComparer::Compare
	{
		if (x == y) return 0;
		if (x == nullptr) return -1;
		if (y == nullptr) return 1;
		using namespace System::Reflection;
		MethodInfo ^l_pMethod1 = dynamic_cast<MethodInfo^>(x);
		MethodInfo ^l_pMethod2 = dynamic_cast<MethodInfo^>(y);
		int res = System::String::Compare(l_pMethod1->Name, l_pMethod2->Name, false);
		if (res == 0)
			res = l_pMethod1->GetParameters()->Length - l_pMethod2->GetParameters()->Length;
		return res;
	}
};

bool MXS_dotNet::ShowMethodInfo(System::Type ^type, MCHAR* pattern, System::Text::StringBuilder ^sb, 
						   bool showStaticOnly, bool showSpecial, bool showAttributes, bool declaredOnTypeOnly)
{
	using namespace System;
	using namespace System::Reflection;

	BindingFlags flags = (BindingFlags)( BindingFlags::Static | BindingFlags::Public);
	if (!showStaticOnly)
		flags = (BindingFlags)(flags | BindingFlags::Instance);
	if (declaredOnTypeOnly)
		flags = (BindingFlags)(flags | BindingFlags::DeclaredOnly);
	else
		flags = (BindingFlags)(flags | BindingFlags::FlattenHierarchy);

	array<MethodInfo^> ^l_pMethods = type->GetMethods(flags);
	if (sb) // if not outputting, no need to sort methods
	{
		if (!showSpecial) // null out methods we aren't interested in
		{
			for(int m=0; m < l_pMethods->Length; m++)
			{
				MethodInfo ^l_pMethod = l_pMethods[m];
				if ((l_pMethod->Attributes & MethodAttributes::SpecialName) == MethodAttributes::SpecialName)
					l_pMethods[m] = nullptr;
			}
		}
		System::Collections::IComparer^ myComparer = gcnew MethodInfoSorter;
		System::Array::Sort(l_pMethods, myComparer);
	}

	bool res = false;
	for(int m=0; m < l_pMethods->Length; m++)
	{
		MethodInfo ^l_pMethod = l_pMethods[m];
		if (l_pMethod == nullptr) continue;
		if (!pattern || max_name_match(MNETStr::ToMSTR(l_pMethod->Name), pattern))
		{
			res = true;
			if (!sb) 
				return res; // signifies method was found
			sb->Append(_T("  ."));
			if (l_pMethod->IsStatic)
				sb->Append(_T("[static]"));
			if (l_pMethod->ReturnType == System::Void::typeid)
			{
				sb->AppendFormat(
					_T("{0}"),
					l_pMethod->Name);
			}
			else
			{
				sb->AppendFormat(
					_T("<{0}>{1}"),
					l_pMethod->ReturnType, l_pMethod->Name);
			}

			array<ParameterInfo^> ^l_pParams = l_pMethod->GetParameters();
			if (l_pParams->Length == 0)
				sb->Append(_T("()"));

			for(int p=0; p < l_pParams->Length; p++)
			{
				ParameterInfo ^l_pParam = l_pParams[p];
				sb->Append(_T(" "));
				if (l_pParam->IsIn)
					sb->Append(_T("[in]"));
				if (l_pParam->IsOut)
					sb->Append(_T("[out]"));
				System::String ^ l_pParamTypeName = l_pParam->ParameterType->FullName;
				if (!l_pParamTypeName)
					l_pParamTypeName = l_pParam->ParameterType->Name;
				sb->AppendFormat(
					_T("<{0}>{1}"),
					l_pParamTypeName, l_pParam->Name);
			}
			if (showAttributes)
			{
				sb->AppendFormat(
					_T(" [Attributes: {0}]"),
					l_pMethod->Attributes.ToString());
			}
			sb->Append(_T("\n"));
		}
	}
	return res;
}

public ref class PropertyInfoSorter: public System::Collections::IComparer
{
private:
	// Does case insensitive compare on PropertyInfo names
	virtual int Compare( System::Object^ x, System::Object^ y ) sealed = System::Collections::IComparer::Compare
	{
		if (x == y) return 0;
		if (x == nullptr) return -1;
		if (y == nullptr) return 1;
		using namespace System::Reflection;
		PropertyInfo ^l_pProp1 = dynamic_cast<PropertyInfo^>(x);
		PropertyInfo ^l_pProp2 = dynamic_cast<PropertyInfo^>(y);
		return System::String::Compare(l_pProp1->Name, l_pProp2->Name, false);
	}
};

bool MXS_dotNet::ShowPropertyInfo(System::Type ^type, MCHAR* pattern, System::Text::StringBuilder ^sb, 
							 bool showStaticOnly, bool showMethods, bool showAttributes, bool declaredOnTypeOnly)
{
	using namespace System;
	using namespace System::Reflection;

	BindingFlags flags = (BindingFlags)( BindingFlags::Static | BindingFlags::Public);
	if (!showStaticOnly)
		flags = (BindingFlags)(flags | BindingFlags::Instance);
	if (declaredOnTypeOnly)
		flags = (BindingFlags)(flags | BindingFlags::DeclaredOnly);
	else
		flags = (BindingFlags)(flags | BindingFlags::FlattenHierarchy);

	array<PropertyInfo^>^ l_pProps = type->GetProperties(flags);
	if (sb) // if not outputting, no need to sort properties
	{
		System::Collections::IComparer^ myComparer = gcnew PropertyInfoSorter;
		System::Array::Sort(l_pProps, myComparer);
	}

	bool res = false;
	for(int m=0; m < l_pProps->Length; m++)
	{
		PropertyInfo ^l_pProp = l_pProps[m];
		if (!pattern || max_name_match(MNETStr::ToMSTR(l_pProp->Name), pattern))
		{
			res = true;
			if (!sb) 
				return res; // signifies property was found
			sb->AppendFormat(
				_T("  .{0}"),
				l_pProp->Name);
			MethodInfo ^l_pAccessor;
			if (l_pProp->CanRead)
			{
				l_pAccessor = l_pProp->GetGetMethod();
				if (l_pAccessor)
				{
					array<ParameterInfo^> ^l_pParams = l_pAccessor->GetParameters();
					if (l_pParams->Length != 0)
					{
						sb->Append(_T("["));
						bool first = true;
						for (int i = 0; i < l_pParams->Length; i++)
						{
							if (!first)
								sb->Append(_T(", "));
							first = false;
							ParameterInfo^ l_pParam = l_pParams[i];
							System::String ^ l_pParamTypeName = l_pParam->ParameterType->FullName;
							if (!l_pParamTypeName)
								l_pParamTypeName = l_pParam->ParameterType->Name;
							sb->AppendFormat(
								_T("<{0}>{1}"),
								l_pParamTypeName, l_pParam->Name);
						}
						sb->Append(_T("]"));
					}
				}
			}
			else if (l_pProp->CanWrite)
			{
				l_pAccessor = l_pProp->GetSetMethod();
				if (l_pAccessor)
				{
					array<ParameterInfo^> ^l_pParams = l_pAccessor->GetParameters();
					if (l_pParams->Length != 1)
					{
						sb->Append(_T("["));
						bool first = true;
						for (int i = 0; i < l_pParams->Length-1; i++)
						{
							if (!first)
								sb->Append(_T(", "));
							first = false;
							ParameterInfo^ l_pParam = l_pParams[i];
							System::String ^ l_pParamTypeName = l_pParam->ParameterType->FullName;
							if (!l_pParamTypeName)
								l_pParamTypeName = l_pParam->ParameterType->Name;
							sb->AppendFormat(
								_T("<{0}>{1}"),
								l_pParamTypeName, l_pParam->Name);
						}
						sb->Append(_T("]"));
					}
				}
			}

			sb->AppendFormat(
				_T(" : <{0}>"),
				l_pProp->PropertyType);
			if (!l_pProp->CanRead)
				sb->Append(_T(", write-only"));
			if (!l_pProp->CanWrite)
				sb->Append(_T(", read-only"));
			if (l_pAccessor && l_pAccessor->IsStatic)
				sb->Append(_T(", static"));
			if (showAttributes)
			{
				sb->AppendFormat(
					_T(", Attributes: {0}"),
					l_pProp->Attributes.ToString());
			}
			// debug testing here for multiple accessors, accessors that take additional args
			{
				array<MethodInfo^> ^l_pAccessors = l_pProp->GetAccessors();
				int nGetAccessors = 0;
				int nSetAccessors = 0;
				for (int i = 0; i < l_pAccessors->Length; i++)
				{
					MethodInfo ^l_pAccessor = l_pAccessors[i];
					if (l_pAccessor->ReturnType == System::Void::typeid)
						nSetAccessors++;
					else
						nGetAccessors++;
				}
				MethodInfo ^l_pAccessor = l_pProp->GetGetMethod();
				int i = (l_pAccessor != nullptr) ? 1 : 0;
				DbgAssert(i == nGetAccessors);
				if (l_pAccessor)
				{
					array<ParameterInfo^> ^l_pParams = l_pAccessor->GetParameters();
					DbgAssert(l_pParams->Length <= 1);
				}
				l_pAccessor = l_pProp->GetSetMethod();
				i = (l_pAccessor != nullptr) ? 1 : 0;
				DbgAssert(i == nSetAccessors);
				if (l_pAccessor)
				{
					array<ParameterInfo^> ^l_pParams = l_pAccessor->GetParameters();
					DbgAssert(l_pParams->Length <= 2);
				}
			}
			if (showMethods)
			{
				MethodInfo ^l_pAccessor = l_pProp->GetGetMethod();
				if (l_pAccessor)
				{
					sb->AppendFormat(
						_T(", get method: {0}, Attributes: {1}"), 
						l_pAccessor->Name, l_pAccessor->Attributes.ToString());
				}
				l_pAccessor = l_pProp->GetSetMethod();
				if (l_pAccessor)
				{
					sb->AppendFormat(
						_T(", set method: {0}, Attributes: {1}"), 
						l_pAccessor->Name, l_pAccessor->Attributes.ToString());
				}

				array<MethodInfo^> ^l_pAccessors = l_pProp->GetAccessors();
				sb->AppendFormat(_T("\n    Accessor methods:"));
				for(int p=0; p < l_pAccessors->Length; p++)
				{
					MethodInfo ^l_pAccessor = l_pAccessors[p];
					sb->AppendFormat(
						_T("\n      Name: {0}, Attributes: {1}"), 
						l_pAccessor->Name, l_pAccessor->Attributes.ToString());
				}
			}
			sb->Append(_T("\n"));
		}
	}
	return res;
}

public ref class EventInfoSorter: public System::Collections::IComparer
{
private:
	// Does case insensitive compare on EventInfo names
	virtual int Compare( System::Object^ x, System::Object^ y ) sealed = System::Collections::IComparer::Compare
	{
		if (x == y) return 0;
		if (x == nullptr) return -1;
		if (y == nullptr) return 1;
		using namespace System::Reflection;
		EventInfo ^l_pEvent1 = dynamic_cast<EventInfo^>(x);
		EventInfo ^l_pEvent2 = dynamic_cast<EventInfo^>(y);
		return System::String::Compare(l_pEvent1->Name, l_pEvent2->Name, false);
	}
};

bool MXS_dotNet::ShowEventInfo(System::Type ^type, MCHAR* pattern, System::Text::StringBuilder ^sb, 
							   bool showStaticOnly, bool declaredOnTypeOnly)
{
	using namespace System;
	using namespace System::Reflection;

	// EventInfo doesn't include an IsStatic member, so we need to collect the events in 2 passes - 
	// one for statics, one for instances

	BindingFlags flags = (BindingFlags)( BindingFlags::Static | BindingFlags::Public);
	if (declaredOnTypeOnly)
		flags = (BindingFlags)(flags | BindingFlags::DeclaredOnly);
	else
		flags = (BindingFlags)(flags | BindingFlags::FlattenHierarchy);

	array<EventInfo^> ^l_pEvents = type->GetEvents(flags);
	int numStaticEvents = l_pEvents->Length;

	if (!showStaticOnly)
	{
		flags = (BindingFlags)( BindingFlags::Instance | BindingFlags::Public);
		if (declaredOnTypeOnly)
			flags = (BindingFlags)(flags | BindingFlags::DeclaredOnly);
		else
			flags = (BindingFlags)(flags | BindingFlags::FlattenHierarchy);
		array<EventInfo^> ^l_pEvents_nonstatic = type->GetEvents(flags);
		if (l_pEvents_nonstatic->Length != 0)
		{
			System::Array::Resize(l_pEvents, l_pEvents_nonstatic->Length + numStaticEvents);
			l_pEvents_nonstatic->CopyTo(l_pEvents, numStaticEvents);
		}
	}

	if (sb) // if not outputting, no need to sort events
	{
		System::Collections::IComparer^ myComparer = gcnew EventInfoSorter;
		System::Array::Sort(l_pEvents, myComparer);
	}

	bool res = false;
	bool isControl = type->IsSubclassOf(System::Windows::Forms::Control::typeid);
	for(int e=0; e < l_pEvents->Length; e++)
	{
		EventInfo^ l_pEvent = l_pEvents[e];
		if (!pattern || max_name_match(MNETStr::ToMSTR(l_pEvent->Name), pattern))
		{
			res = true;
			if (!sb) 
				return res; // signifies event was found

			if (e < numStaticEvents)
				sb->Append(_T("   [static] "));
			else
				sb->Append(_T("   "));

			if (isControl)
				sb->AppendFormat(
					_T("on <control_name> {0}"),
					l_pEvent->Name);
			else
				sb->AppendFormat(
				_T("{0}"),
				l_pEvent->Name);


			Type ^l_pEventHandlerType = l_pEvent->EventHandlerType;
			Type ^l_pDelegateReturnType = l_pEventHandlerType->GetMethod(_T("Invoke"))->ReturnType;

			array<ParameterInfo^> ^l_pParams = l_pEventHandlerType->GetMethod(_T("Invoke"))->GetParameters();
			for (int i = isControl ? 1 : 0; i < l_pParams->Length; i++)
			{
				ParameterInfo ^l_pParam = l_pParams[i];
				sb->Append(_T(" "));
				if (l_pParam->IsIn)
					sb->Append(_T("[in]"));
				if (l_pParam->IsOut)
					sb->Append(_T("[out]"));
				System::String ^ l_pParamTypeName = l_pParam->ParameterType->FullName;
				if (!l_pParamTypeName)
					l_pParamTypeName = l_pParam->ParameterType->Name;
				sb->AppendFormat(
					_T("<{0}>{1}"),
					l_pParamTypeName, l_pParam->Name);
			}

			if (isControl)
				sb->Append(_T(" do ( ... )"));
			else
				sb->Append(_T(" = ( ... )"));
			if (l_pEvent->Attributes != EventAttributes::None)
			{
				sb->AppendFormat(
					_T(" [Attributes: {0}]"),
					l_pEvent->Attributes.ToString());
			}
			if (l_pDelegateReturnType != System::Void::typeid)
			{
				System::String ^ l_pReturnTypeName = l_pDelegateReturnType->FullName;
				if (!l_pReturnTypeName)
					l_pReturnTypeName = l_pDelegateReturnType->Name;
				sb->AppendFormat(
					_T(" [ return type: <{0}> ]"),
					l_pReturnTypeName);
			}
			sb->Append(_T("\n"));
		}
	}
	return res;
}

public ref class FieldInfoSorter: public System::Collections::IComparer
{
private:
	// Does case insensitive compare on FieldInfo names
	virtual int Compare( System::Object^ x, System::Object^ y ) sealed = System::Collections::IComparer::Compare
	{
		if (x == y) return 0;
		if (x == nullptr) return -1;
		if (y == nullptr) return 1;
		using namespace System::Reflection;
		FieldInfo ^l_pField1 = dynamic_cast<FieldInfo^>(x);
		FieldInfo ^l_pField2 = dynamic_cast<FieldInfo^>(y);
		return System::String::Compare(l_pField1->Name, l_pField2->Name, false);
	}
};

bool MXS_dotNet::ShowFieldInfo(System::Type ^type, MCHAR* pattern, System::Text::StringBuilder ^sb, 
						  bool showStaticOnly, bool showAttributes, bool declaredOnTypeOnly)
{
	using namespace System;
	using namespace System::Reflection;

	BindingFlags flags = (BindingFlags)( BindingFlags::Static | BindingFlags::Public);
	if (!showStaticOnly)
		flags = (BindingFlags)(flags | BindingFlags::Instance);
	if (declaredOnTypeOnly)
		flags = (BindingFlags)(flags | BindingFlags::DeclaredOnly);
	else
		flags = (BindingFlags)(flags | BindingFlags::FlattenHierarchy);

	array<FieldInfo^> ^l_pFields = type->GetFields(flags);
	if (sb) // if not outputting, no need to sort fields
	{
		System::Collections::IComparer^ myComparer = gcnew FieldInfoSorter;
		System::Array::Sort(l_pFields, myComparer);
	}

	bool res = false;
	for(int f=0; f < l_pFields->Length; f++)
	{
		FieldInfo^ l_pField = l_pFields[f];
		if (!pattern || max_name_match(MNETStr::ToMSTR(l_pField->Name), pattern))
		{
			res = true;
			if (!sb) 
				return res; // signifies property was found
			sb->AppendFormat(
				_T("  .{0} : <{1}>"),
				l_pField->Name, l_pField->FieldType);
			if (l_pField->IsInitOnly || l_pField->IsLiteral)
				sb->Append(_T(", read-only"));
			if (l_pField->IsStatic)
				sb->Append(_T(", static"));
			if (showAttributes)
			{
				sb->AppendFormat(
					_T(", Attributes: {0}"),
					l_pField->Attributes.ToString());
			}
			sb->Append(_T("\n"));
		}
	}
	return res;
}

/*
// too late to add to r9. This implementation of GetInterfaces returns the System::Type 
// associated with the interfaces. This isn't what is desired. What we want to do is return
// the object, but make it look like it has been cast to the interface type. For example:
//   myString = dotNetObject "System.String" "AAA"
//   showMethods myString -- shows all methods of String
//   newString = myString.Copy()
//   iclonable = getInterface myString "IClonable"
//   showMethods iclonable -- shows just that methods of IClonable interface
//   clonedString = iclonable.Clone()
// to do this means modifying DotNetObjectManaged to also store the Type it was created as - if nullptr
// use the type of the object
bool MXS_dotNet::ShowInterfaceInfo(System::Type ^type, MCHAR* pattern, System::Text::StringBuilder ^sb)
{
	using namespace System;
	using namespace System::Reflection;

	array<Type^> ^l_pInterfaces = type->GetInterfaces();
	bool res = false;
	for(int f=0; f < l_pInterfaces->Length; f++)
	{
		Type^ l_pInterface = l_pInterfaces[f];
		if (!pattern || max_name_match(MNETStr::ToMSTR(l_pInterface->Name), pattern))
		{
			res = true;
			if (!sb) 
				return res; // signifies interface was found
			sb->AppendFormat(
				_T("  .{0}"),
				l_pInterface->Name);
			sb->Append(_T("\n"));
		}
	}
	return res;
}

void MXS_dotNet::GetInterfaces(System::Type ^type, Array* result)
{
	using namespace System;
	using namespace System::Reflection;

	array<Type^> ^l_pInterfaces = type->GetInterfaces();
	for(int f=0; f < l_pInterfaces->Length; f++)
	{
		Type^ l_pInterface = l_pInterfaces[f];
		result->append(DotNetObjectWrapper::intern(l_pInterface));
	}
	return;
}

bool MXS_dotNet::ShowAttributeInfo(System::Type ^type, MCHAR* pattern, System::Text::StringBuilder ^sb,
								   bool declaredOnTypeOnly)
{
	using namespace System;
	using namespace System::Reflection;

	array<System::Object^> ^l_pAttributes = type->GetCustomAttributes(!declaredOnTypeOnly);
	bool res = false;
	for(int f=0; f < l_pAttributes->Length; f++)
	{
		System::Object^ l_pAttribute = l_pAttributes[f];
		if (!pattern || max_name_match(MNETStr::ToMSTR(l_pAttribute->GetType()->Name), pattern))
		{
			res = true;
			if (!sb) 
				return res; // signifies interface was found
			sb->AppendFormat(
				_T("  .{0}"),
				l_pAttribute->GetType()->Name);
			sb->Append(_T("\n"));
		}
	}
	return res;
}

void MXS_dotNet::GetAttributes(System::Type ^type, Array* result)
{
	using namespace System;
	using namespace System::Reflection;

	array<System::Object^> ^l_pAttributes = type->GetCustomAttributes(true);
	for(int f=0; f < l_pAttributes->Length; f++)
	{
		System::Object^ l_pAttribute = l_pAttributes[f];
		result->append(DotNetObjectWrapper::intern(l_pAttribute));
	}
	return;
}
*/

public ref class ConstructorInfoSorter: public System::Collections::IComparer
{
private:
	// Does case insensitive compare on MethodInfo names, then on # args
	virtual int Compare( System::Object^ x, System::Object^ y ) sealed = System::Collections::IComparer::Compare
	{
		if (x == y) return 0;
		if (x == nullptr) return -1;
		if (y == nullptr) return 1;
		using namespace System::Reflection;
		ConstructorInfo ^l_pConstructInfo1 = dynamic_cast<ConstructorInfo^>(x);
		ConstructorInfo ^l_pConstructInfo2 = dynamic_cast<ConstructorInfo^>(y);
		int res = System::String::Compare(l_pConstructInfo1->Name, l_pConstructInfo2->Name, false);
		if (res == 0)
			res = l_pConstructInfo1->GetParameters()->Length - l_pConstructInfo2->GetParameters()->Length;
		return res;
	}
};

bool MXS_dotNet::ShowConstructorInfo(System::Type ^type, System::Text::StringBuilder ^sb)
{
	using namespace System;
	using namespace System::Reflection;

	array<ConstructorInfo^> ^l_pConstructors = type->GetConstructors();
	if (sb) // if not outputting, no need to sort constructors
	{
		System::Collections::IComparer^ myComparer = gcnew ConstructorInfoSorter;
		System::Array::Sort(l_pConstructors, myComparer);
	}

	bool res = false;
	for(int c=0; c < l_pConstructors->Length; c++)
	{
		ConstructorInfo^ l_pConstructor = l_pConstructors[c];
		{
			res = true;
			if (!sb) 
				return res; // signifies property was found

			System::String ^ l_pTypeName = type->FullName;
			if (!l_pTypeName)
				l_pTypeName = type->Name;
			sb->AppendFormat(
				_T("  {0}"),
				l_pTypeName);

			array<ParameterInfo^> ^l_pParams = l_pConstructor->GetParameters();
			if (l_pParams->Length == 0)
				sb->Append(_T("()"));

			for(int p=0; p < l_pParams->Length; p++)
			{
				ParameterInfo ^l_pParam = l_pParams[p];
				sb->Append(_T(" "));
				if (l_pParam->IsIn)
					sb->Append(_T("[in]"));
				if (l_pParam->IsOut)
					sb->Append(_T("[out]"));
				System::String ^ l_pParamTypeName = l_pParam->ParameterType->FullName;
				if (!l_pParamTypeName)
					l_pParamTypeName = l_pParam->ParameterType->Name;
				sb->AppendFormat(
					_T("<{0}>{1}"),
					l_pParamTypeName, l_pParam->Name);
			}
			sb->Append(_T("\n"));
		}
	}
	return res;
}

void MXS_dotNet::GetPropertyAndFieldNames(System::Type ^type, Array* propNames, 
										  bool showStaticOnly, bool declaredOnTypeOnly)
{
	using namespace System;
	using namespace System::Reflection;

	BindingFlags flags = (BindingFlags)( BindingFlags::Static | BindingFlags::Public);
	if (!showStaticOnly)
		flags = (BindingFlags)(flags | BindingFlags::Instance);
	if (declaredOnTypeOnly)
		flags = (BindingFlags)(flags | BindingFlags::DeclaredOnly);
	else
		flags = (BindingFlags)(flags | BindingFlags::FlattenHierarchy);

	array<PropertyInfo^>^ l_pProps = type->GetProperties(flags);
	System::Collections::IComparer^ myComparer = gcnew PropertyInfoSorter;
	System::Array::Sort(l_pProps, myComparer);
	for(int m=0; m < l_pProps->Length; m++)
	{
		PropertyInfo ^l_pProp = l_pProps[m];
		propNames->append(Name::intern(MNETStr::ToMSTR(l_pProp->Name)));
	}
	array<FieldInfo^> ^l_pFields = type->GetFields(flags);
	myComparer = gcnew FieldInfoSorter;
	System::Array::Sort(l_pFields, myComparer);
	for(int f=0; f < l_pFields->Length; f++)
	{
		FieldInfo^ l_pField = l_pFields[f];
		propNames->append(Name::intern(MNETStr::ToMSTR(l_pField->Name)));
	}
}

System::Object^ MXS_dotNet::MXS_Value_To_Object(Value* val, System::Type ^ type)
{
	if (type->IsArray && is_array(val))
	{
		System::Type ^ l_pEleType = type->GetElementType();
		Array *l_theArray = (Array*)val;
		System::Array ^l_pResArray = System::Array::CreateInstance(l_pEleType, l_theArray->size);
		for (int i = 0; i < l_theArray->size; i++)
			l_pResArray->SetValue(MXS_dotNet::MXS_Value_To_Object(l_theArray->data[i], l_pEleType), i);
		return l_pResArray;
	}

	// for 'byref' and 'byptr' types, get the underlying type. But don't lose whether we are
	// dealing with an Array type.
	System::Type ^ l_pOrigType = type;
	if (type->HasElementType)
	{
		type = type->GetElementType();
		if (l_pOrigType->IsArray)
			type = type->MakeArrayType();
	}

	if (is_dotNetMXSValue(val) && type == System::Object::typeid)
	{
		DotNetMXSValue *l_wrapper = ((dotNetMXSValue*)val)->m_pDotNetMXSValue;
		return l_wrapper->GetWeakProxy();
	}
	else if (is_dotNetObject(val) || is_dotNetControl(val))
	{
		DotNetObjectWrapper *l_wrapper = NULL;
		if (is_dotNetObject(val)) l_wrapper = ((dotNetObject*)val)->GetDotNetObjectWrapper();
		else l_wrapper = ((dotNetControl*)val)->GetDotNetObjectWrapper();
		if (l_wrapper)
		{
			System::Object ^ l_pObj = l_wrapper->GetObject();
			if (l_pObj && type->IsAssignableFrom(l_pObj->GetType()))
				return l_pObj;
		}
	}
	else if (is_dotNetClass(val))
	{
		DotNetObjectWrapper *l_wrapper = ((dotNetClass*)val)->GetDotNetObjectWrapper();
		System::Type ^l_pTheType = l_wrapper->GetType();
		if (l_pTheType && type->IsAssignableFrom(l_pTheType->GetType()))
			return l_pTheType;
	}
	else if (type == System::Byte::typeid || type == System::SByte::typeid || 
		type == System::Decimal::typeid || 
		type == System::Int16::typeid || type == System::UInt16::typeid || 
		type == System::Int32::typeid || type == System::UInt32::typeid)
		return val->to_int();
	else if (type == System::IntPtr::typeid || type == System::UIntPtr::typeid)
		return val->to_intptr();
	else if (type == System::Int64::typeid || type == System::UInt64::typeid) 
		return val->to_int64();
	else if (type == System::Single::typeid)
		return val->to_float();
	else if (type == System::Double::typeid)
		return  val->to_double();
	else if (type == System::Boolean::typeid)
		return (val->to_bool() != FALSE);
	else if (type == System::String::typeid)
	{
		CString cString(val->to_string());
		return System::String::Intern(gcnew System::String(cString));
	}
	else if (type == System::Char::typeid->MakeArrayType())
	{
		CString cString(val->to_string());
		int len = cString.GetLength();
		array<System::Char> ^l_pCharArray = gcnew array<System::Char>(len);
		for (int i = 0; i < len; i++)
			l_pCharArray[i] = cString[i];
		return l_pCharArray;
	}
	else if (type == System::Char::typeid)
	{
		CString cString(val->to_string());
		if (cString.GetLength())
			return cString.GetAt(0);
		else
			throw RuntimeError(_M("Can't convert 0 length string to character"));
	}
	else if (val == &undefined)
		return nullptr;
	else if (type == System::Object::typeid)
	{
		if (is_array(val))
		{
			Array *theArray = (Array*)val;
			array<System::Object^> ^resArray = gcnew array<System::Object^>(theArray->size);
			for (int i = 0; i < theArray->size; i++)
				resArray[i] = MXS_Value_To_Object(theArray->data[i], type);
			return resArray;
		}
		else if (is_integer(val))
			return val->to_int();
		else if (is_integerptr(val))
			return val->to_intptr();
		else if (is_integer64(val)) 
			return val->to_int64();
		else if (is_float(val))
			return val->to_float();
		else if (is_double(val))
			return  val->to_double();
		else if (is_bool(val))
			return (val == &true_value);
		else if (is_string(val))
		{
			CString cString(val->to_string());
			return gcnew System::String(cString);
		}
	}

	System::String ^ l_pTypeName = type->FullName;
	if (!l_pTypeName)
		l_pTypeName = type->Name;
	throw ConversionError(val, MNETStr::ToMSTR(l_pTypeName));
}

Value* MXS_dotNet::Object_To_MXS_Value(System::Object^ val)
{
	if (!val)
		return &undefined;
	System::Array ^ l_pValArray = dynamic_cast<System::Array^>(val);
	if (l_pValArray)
	{
		one_typed_value_local(Array* result);
		vl.result = new Array (l_pValArray->Length);
		for (int i = 0; i < l_pValArray->Length; i++)
		{
			System::Object ^ l_pEle = l_pValArray->GetValue(i);
			vl.result->append(MXS_dotNet::Object_To_MXS_Value(l_pEle));
		}
		return_value(vl.result);
	}
	one_value_local(result);
	System::Type ^ type = val->GetType();
	if (type == System::Boolean::typeid)
	{
		vl.result = bool_result(System::Convert::ToBoolean(val));
	}
	else if (type == System::Byte::typeid || type == System::SByte::typeid || 
		type == System::Decimal::typeid || 
		type == System::Int16::typeid || type == System::UInt16::typeid || 
		type == System::Int32::typeid || type == System::UInt32::typeid )
	{
		vl.result = Integer::intern(System::Convert::ToInt32(val));
	}
	else if (type == System::Int64::typeid || type == System::UInt64::typeid ) 
	{
		vl.result = Integer64::intern(System::Convert::ToInt64(val));
	}
	else if (type == System::IntPtr::typeid || type == System::UIntPtr::typeid ) 
	{
		void *ptr = safe_cast<System::IntPtr^>(val)->ToPointer();
		vl.result = IntegerPtr::intern((INT_PTR)ptr);
	}
	else if (type == System::Single::typeid ) 
	{
		vl.result = Float::intern(System::Convert::ToSingle(val));
	}
	else if (type == System::Double::typeid ) 
	{
		vl.result = Double::intern(System::Convert::ToDouble(val));
	}
	else if (type == System::String::typeid ) 
	{
		MSTR str(MNETStr::ToMSTR(System::Convert::ToString(val)));
		vl.result = new String(str);
	}
	else if (type == System::Char::typeid ) 
	{
		MSTR c(CString(System::Convert::ToChar(val)));
		vl.result = new String(c);
	}
	else if (type == DotNetMXSValue::DotNetMXSValue_proxy::typeid )
	{
		DotNetMXSValue::DotNetMXSValue_proxy ^ l_pProxy = safe_cast<DotNetMXSValue::DotNetMXSValue_proxy^>(val);
		return l_pProxy->get_value();
	}
	else
	{
		vl.result = DotNetObjectWrapper::intern(val);
	}
	return_value(vl.result);
}

System::Type^ MXS_dotNet::ResolveTypeFromName(const MCHAR* pTypeString)
{
	using namespace System;
	using namespace System::Reflection;
	using namespace System::Diagnostics;

	// look for type using fully qualified class name (includes assembly)
	TSTR	l_TypeString(pTypeString);  // MCHAR -> TCHAR
	CString	cTypeString(l_TypeString);  // TCHAR -> MFC
	System::String ^l_ObjectTypeString = gcnew System::String(cTypeString);
	Type ^l_ObjectTypeRes;

	// TODO: need to add method for specifying additional name spaces to automatically search.
	// for right now, just hacking in hard coded list
	array<System::String^>^autoNameSpaces = gcnew array<System::String^>{_T(""), _T("System."), _T("System.Windows.Forms.")};

	for (int i = 0; (!l_ObjectTypeRes) && i < autoNameSpaces->Length; i++)
	{
		System::String ^ l_pTypeString = autoNameSpaces[i] + l_ObjectTypeString;
		l_ObjectTypeRes = Type::GetType(l_pTypeString, false, true); // no throw, case insensitive
		if (l_ObjectTypeRes && (l_ObjectTypeRes->IsPublic || l_ObjectTypeRes->IsNestedPublic))
			break;
	}

	if (!l_ObjectTypeRes)
	{
		// look for in each assembly
		AppDomain^ currentDomain = AppDomain::CurrentDomain;
		//Make an array for the list of assemblies.
		array<Assembly^>^l_pAssemblies = currentDomain->GetAssemblies();

		//List the assemblies in the current application domain.
//		Debug::WriteLine( "List of assemblies loaded in current appdomain:" );
		for (int i = 0; (!l_ObjectTypeRes) && i < autoNameSpaces->Length; i++)
		{
			System::String ^ l_pTypeString = autoNameSpaces[i] + l_ObjectTypeString;
			for (int j = 0; j < l_pAssemblies->Length; j++)
			{
				Assembly^ l_pAssembly = l_pAssemblies[j];

//				array<Type^>^types = assem->GetTypes();
//				Debug::WriteLine( assem );
//				Debug::WriteLine( types->Length );

				l_ObjectTypeRes = l_pAssembly->GetType(l_pTypeString, false, true);
				if (l_ObjectTypeRes && (l_ObjectTypeRes->IsPublic || l_ObjectTypeRes->IsNestedPublic))
					break;
			}
		}
	}

	return l_ObjectTypeRes;
}

void MXS_dotNet::PrepArgList(array<System::Reflection::ParameterInfo^> ^candidateParamList, 
							 Value** arg_list, array<System::Object^> ^argArray, int count)
{
	using namespace System::Reflection;
	for (int i = 0; i < count; i++)
	{
		// if the param is a [out] param, can just pass a null pointer.
		ParameterInfo ^ l_pParameterInfo = candidateParamList[i];
		if (l_pParameterInfo->IsOut)
			argArray[i] = nullptr;
		else
		{
			System::Type ^ l_pType = l_pParameterInfo->ParameterType;
			Value *val = arg_list[i];
			argArray[i] = MXS_dotNet::MXS_Value_To_Object(val, l_pType);
		}
	}
}

int MXS_dotNet::FindMethodAndPrepArgList(System::Collections::Generic::List<array<System::Reflection::ParameterInfo^>^> ^candidateParamLists, 
					   Value** arg_list, array<System::Object^> ^argArray, int count, Array* userSpecifiedParamTypesVals)
{
	using namespace System::Reflection;

	if (userSpecifiedParamTypesVals)
	{
		// convert mxs values to Type.
		array<System::Type^> ^l_pUserSpecifiedParamTypes = gcnew array<System::Type^>(count);
		for (int i = 0; i < count; i++)
		{
			Value *val = userSpecifiedParamTypesVals->data[i];
			if (is_dotNetObject(val) || is_dotNetControl(val))
			{
				DotNetObjectWrapper *wrapper = NULL;
				if (is_dotNetObject(val)) wrapper = ((dotNetObject*)val)->GetDotNetObjectWrapper();
				else wrapper = ((dotNetControl*)val)->GetDotNetObjectWrapper();
				if (wrapper)
				{
					System::Type ^l_pType = dynamic_cast<System::Type^>(wrapper->GetObject());
					if (l_pType)
						l_pUserSpecifiedParamTypes[i] = l_pType;
				else
					throw ConversionError(val, _M("System::Type"));
				}
			}
		}
		// if we are here, userSpecifiedParamTypes now contains the Types. Find candidate ParamList that matches, if any 
		for (int j = 0; j < candidateParamLists->Count; j++)
		{
			bool match = true;
			array<ParameterInfo^> ^l_pCandidateParamList = candidateParamLists[j];
			for (int i = 0; i < count; i++)
			{
				if (l_pUserSpecifiedParamTypes[i] != l_pCandidateParamList[i]->ParameterType)
				{
					match = false;
					break;
				}
			}
			if (match)
			{
				// we have a match! try convert mxs values to .net values
				PrepArgList(l_pCandidateParamList, arg_list, argArray, count);
				// if we got here, the values converted ok. Return the index of the matching candidate
				return j;
			}
		}
		// alas, no match. Return -1, caller can throw exception with good message
		return -1;
	}
	// user didn't specify ParamTypes, so we need to figure out the best match. For each
	// candidate, calculate a score based on matching each parameter type with the MXS value
	// type. If for any parameter the mxs value and type aren't compatible, rule out that candidate.
	int bestMatchIndex = -1;
	int bestMatchScore = -1;
	for (int j = 0; j < candidateParamLists->Count; j++)
	{
		int myScore = 0;
		array<ParameterInfo^> ^l_pCandidateParamList = candidateParamLists[j];
		for (int i = 0; i < count; i++)
		{
			int paramScore;
			if (l_pCandidateParamList[i]->IsOut)
				paramScore = 9;
			else
				paramScore = CalcParamScore(arg_list[i], l_pCandidateParamList[i]->ParameterType);
			if (paramScore == 0)
			{
				myScore = -1;
				break;
			}
			myScore += paramScore;
		}
		if (myScore > bestMatchScore)
			bestMatchIndex = j;
	}
	if (bestMatchIndex >= 0)
	{
		array<ParameterInfo^> ^l_pCandidateParamList = candidateParamLists[bestMatchIndex];
		// we have a match! try convert mxs values to .net values
		PrepArgList(l_pCandidateParamList, arg_list, argArray, count);
	}

	return bestMatchIndex;
}

int MXS_dotNet::CalcParamScore(Value *val, System::Type^ type)
{
	// calculate a score based on how well val's type matches the desired type.
	// using score values of 9, 6, 3, 0. This will allow easy tweaking in the future
	// a score of 0 means no match at all.

	if (type->IsArray && is_array(val))
	{
		System::Type ^ l_pEleType = type->GetElementType();
		Array *theArray = (Array*)val;
		if (theArray->size == 0)
			return 9;
		int arrayScore = 0;
		for (int i = 0; i < theArray->size; i++)
			arrayScore += MXS_dotNet::CalcParamScore(theArray->data[i], l_pEleType);
		arrayScore /= theArray->size;
		return arrayScore;
	}

	// for 'byref' and 'byptr' types, get the underlying type. But don't lose whether we are
	// dealing with an Array type.
	System::Type ^ l_pOrigType = type;
	if (type->HasElementType)
	{
		type = type->GetElementType();
		if (l_pOrigType->IsArray)
			type = type->MakeArrayType();
	}

	if (is_dotNetObject(val) || is_dotNetControl(val) )
	{
		DotNetObjectWrapper *l_wrapper = NULL;
		if (is_dotNetObject(val)) l_wrapper = ((dotNetObject*)val)->GetDotNetObjectWrapper();
		else l_wrapper = ((dotNetControl*)val)->GetDotNetObjectWrapper();
		if (!l_wrapper)
			throw RuntimeError (_M("Attempt to use dotNetObject or dotNetControl with no wrapped object"));

		// if target type is System::Object, we have a match
		if (type == System::Object::typeid)
			return 9;

		System::Object ^l_pTheObj = l_wrapper->GetObject();
		// see if object's type is assignable to the target type
		if (l_pTheObj)
		{
			System::Type ^l_pTheObjType = l_pTheObj->GetType();
			if (type->IsAssignableFrom(l_pTheObjType))
				return 9;
			else
				return 0; // no match
		}
		else if (type->IsPointer || type->IsArray || type->IsInterface)
			return 9; // assuming a nullptr is ok for these. Valid? Any more?
		else
			return 0;
	}
	if (is_dotNetClass(val))
	{
		// if target type is System::Object, we have a match
		if (type == System::Object::typeid)
			return 9;
		DotNetObjectWrapper *l_wrapper = ((dotNetClass*)val)->GetDotNetObjectWrapper();
		System::Type ^l_pTheType = l_wrapper->GetType();
		// see if the type is assignable to the target type
		if (l_pTheType && type->IsAssignableFrom(l_pTheType->GetType()))
			return 9;
		else
			return 0;
	}
	if (is_bool(val))
	{
		if (type == System::Boolean::typeid)
			return 9;
		else
			return 0;
	}
	if (is_string(val) || is_stringstream(val))
	{
		if (type == System::String::typeid || type == System::Char::typeid) 
			return 9;
		else if (type == System::Char::typeid->MakeArrayType() )
			return 6;
		else
			return 0;
	}
	if (val == &undefined)
	{
		if (type->IsPointer || type->IsArray || type->IsInterface)
			return 9; // assuming a nullptr is ok for these. Valid? Any more?
		else
			return 0;
	}
	if (is_int(val))
	{
		if (type == System::Byte::typeid || type == System::SByte::typeid || 
			type == System::Decimal::typeid || 
			type == System::Int16::typeid || type == System::UInt16::typeid || 
			type == System::Int32::typeid || type == System::UInt32::typeid)
			return 9;
		else if (type == System::IntPtr::typeid || type == System::UIntPtr::typeid)
			return 6;
		else if (type == System::Int64::typeid || type == System::UInt64::typeid) 
			return 6;
		else if (type == System::Single::typeid || type == System::Double::typeid)
			return 3;
		else
			return 0;
	}
	if (is_int64(val))
	{
		if (type == System::Byte::typeid || type == System::SByte::typeid || 
			type == System::Int16::typeid || type == System::UInt16::typeid || 
			type == System::Int32::typeid || type == System::UInt32::typeid)
			return 6;
		else if (type == System::IntPtr::typeid || type == System::UIntPtr::typeid)
			return 6;
		else if (type == System::Int64::typeid || type == System::UInt64::typeid ||
			type == System::Decimal::typeid)
			return 9;
		else if (type == System::Single::typeid || type == System::Double::typeid)
			return 3;
		else
			return 0;
	}
	if (is_intptr(val))
	{
		if (type == System::Byte::typeid || type == System::SByte::typeid || 
			type == System::Decimal::typeid || 
			type == System::Int16::typeid || type == System::UInt16::typeid || 
			type == System::Int32::typeid || type == System::UInt32::typeid)
			return 6;
		else if (type == System::IntPtr::typeid || type == System::UIntPtr::typeid)
			return 9;
		else if (type == System::Int64::typeid || type == System::UInt64::typeid) 
			return 6;
		else if (type == System::Single::typeid || type == System::Double::typeid)
			return 3;
		else
			return 0;
	}
	if (is_float(val))
	{
		if (type == System::Byte::typeid || type == System::SByte::typeid || 
			type == System::Decimal::typeid || 
			type == System::Int16::typeid || type == System::UInt16::typeid || 
			type == System::Int32::typeid || type == System::UInt32::typeid)
			return 3;
		else if (type == System::IntPtr::typeid || type == System::UIntPtr::typeid)
			return 3;
		else if (type == System::Int64::typeid || type == System::UInt64::typeid) 
			return 3;
		else if (type == System::Single::typeid)
			return 9;
		else if (type == System::Double::typeid)
			return 6;
		else
			return 0;
	}
	if (is_double(val))
	{
		if (type == System::Byte::typeid || type == System::SByte::typeid || 
			type == System::Decimal::typeid || 
			type == System::Int16::typeid || type == System::UInt16::typeid || 
			type == System::Int32::typeid || type == System::UInt32::typeid)
			return 3;
		else if (type == System::IntPtr::typeid || type == System::UIntPtr::typeid)
			return 3;
		else if (type == System::Int64::typeid || type == System::UInt64::typeid) 
			return 3;
		else if (type == System::Single::typeid)
			return 6;
		else if (type == System::Double::typeid)
			return 9;
		else
			return 0;
	}
	return 0;
}

bool MXS_dotNet::CompareEnums(Value** arg_list, int count)
{
	try
	{
		System::UInt64 l_bitfield[] = {0, 0};

		for (int i = 0; i < 2; i++)
		{
			Value* val = arg_list[i];
			if (is_dotNetObject(val))
			{
				System::Object ^ l_pObj = ((dotNetObject*)val)->m_pDotNetObject->GetObject();
				System::Type ^ l_pType_tmp = l_pObj->GetType();
				if (!l_pType_tmp->IsEnum)
					throw RuntimeError(_M("dotNetObject is not an enum type: "), val);
				l_bitfield[i] = System::Convert::ToUInt64(l_pObj);
			}
			else if (is_number(val))
				l_bitfield[i] = val->to_int64();

			else
				throw ConversionError(val, _M("dotNetObject"));
		}

		return (l_bitfield[0] & l_bitfield[1]) != 0L;
	}
	catch (MAXScriptException&)
	{
		throw;
	}
	catch (System::Exception ^ e)
	{
		throw RuntimeError(_M("dotNet runtime exception: "), MNETStr::ToMSTR(e));
	}
}

Value* MXS_dotNet::CombineEnums(Value** arg_list, int count)
{
	using namespace System::Reflection;

	try
	{
		if (count == 0)
			check_arg_count(dotNet.combineEnums, 1, count);

		System::UInt64 l_bitfield = 0;

		System::Type ^ l_pType;
		for (int i = 0; i < count; i++)
		{
			Value* val = arg_list[i];
			if (is_dotNetObject(val))
			{
				System::Object ^ l_pObj = ((dotNetObject*)val)->m_pDotNetObject->GetObject();
				System::Type ^ l_pType_tmp = l_pObj->GetType();
				if (!l_pType_tmp->IsEnum)
					throw RuntimeError(_M("dotNetObject is not an enum type: "), val);
				if (!l_pType)
				{
					l_pType = l_pType_tmp;
					bool hasFlagsAttribute = false;
					// Iterate through all the Attributes for each method.
					System::Collections::IEnumerator^ myEnum1 = System::Attribute::GetCustomAttributes( l_pType )->GetEnumerator();
					while ( myEnum1->MoveNext() )
					{
						System::Attribute^ attr = safe_cast<System::Attribute^>(myEnum1->Current);

						// Check for the FlagsTypeAttribute attribute.
						if ( attr->GetType() == System::FlagsAttribute::typeid )
						{
							hasFlagsAttribute = true;
							break;
						}
					}

					if (!hasFlagsAttribute)
						throw RuntimeError(_M("enum type can not be treated as a bit field (as a set of flags)"), val);
				}
				if (l_pType != l_pType_tmp)
					throw RuntimeError(_M("not all enums are of the same type: "), val);
				l_bitfield += System::Convert::ToUInt64(l_pObj);

			}
			else if (is_number(val))
				l_bitfield += val->to_int64();

			else
				throw ConversionError(val, _M("dotNetObject"));
		}
		if (!l_pType)
			throw RuntimeError(_M("at least one argument needs to be a dotNetObject wrapping a System::Enum derived class"));
		System::Object ^ l_pRes = System::Enum::ToObject(l_pType, l_bitfield);
		return DotNetObjectWrapper::intern(l_pRes);
	}
	catch (MAXScriptException&)
	{
		throw;
	}
	catch (System::Exception ^ e)
	{
		throw RuntimeError(_M("dotNet runtime exception: "), MNETStr::ToMSTR(e));
	}
}

Value* MXS_dotNet::LoadAssembly(MCHAR* assyDll)
{
	using namespace System::Reflection;

	try
	{
		CString cString(assyDll);
		System::String ^ l_pAssyDll = gcnew System::String(cString);
#pragma warning(disable:4947)
		Assembly ^ l_pAssy;
		try {l_pAssy = Assembly::LoadWithPartialName(l_pAssyDll);}
		catch (System::IO::FileLoadException ^) {}
		if (!l_pAssy)
		{
			int i = l_pAssyDll->LastIndexOf(_T(".dll"));
			if (i == (l_pAssyDll->Length - 4))
			{
				try {l_pAssy = Assembly::LoadWithPartialName(l_pAssyDll->Substring(0,i));}
				catch (System::IO::FileLoadException ^) {}
			}
		}
#pragma warning(default:4947)
		if (!l_pAssy)
		{
			int i = l_pAssyDll->LastIndexOf(_T(".dll"));
			if (i != (l_pAssyDll->Length - 4))
				l_pAssyDll = l_pAssyDll + _T(".dll");
			try {l_pAssy = Assembly::LoadFrom(l_pAssyDll);}
			catch (System::IO::FileNotFoundException ^) {}
			catch (System::BadImageFormatException ^) {}
		}
		if (!l_pAssy)
		{
			// look in the directory holding the dll for the WindowsForm Assembly
			Assembly ^mscorlibAssembly = System::String::typeid->Assembly;
			System::String ^ l_pAssyLoc = mscorlibAssembly->Location;
			int i = l_pAssyLoc->LastIndexOf(_T('\\'))+1;
			l_pAssyLoc = l_pAssyLoc->Substring(0,i);
			try {l_pAssy = Assembly::LoadFrom(l_pAssyLoc + l_pAssyDll);}
			catch (System::IO::FileNotFoundException ^) {}
			catch (System::BadImageFormatException ^) {}
		}
		return DotNetObjectWrapper::intern(l_pAssy);

	}
	catch (MAXScriptException&)
	{
		throw;
	}
	catch (System::Exception ^ e)
	{
		throw RuntimeError(_M("dotNet runtime exception: "), MNETStr::ToMSTR(e));
	}
}
