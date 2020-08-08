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
// AUTHOR: Larry.Minton / David Cunningham - created Oct 2, 2006
//***************************************************************************/
//

#pragma once

#include <string>

namespace MXS_dotNet
{

	typedef std::basic_string<MCHAR> MString;
	class dotNetBase;

#pragma managed
	bool ShowMethodInfo(System::Type ^type, MCHAR* pattern, System::Text::StringBuilder ^sb, 
		bool showStaticOnly, bool showSpecial, bool showAttributes, bool declaredOnTypeOnly);
	bool ShowPropertyInfo(System::Type ^type, MCHAR* pattern, System::Text::StringBuilder ^sb, 
		bool showStaticOnly, bool showMethods, bool showAttributes, bool declaredOnTypeOnly);
	bool ShowEventInfo(System::Type ^type, MCHAR* pattern, System::Text::StringBuilder ^sb, 
		bool showStaticOnly, bool declaredOnTypeOnly);
	bool ShowFieldInfo(System::Type ^type, MCHAR* pattern, System::Text::StringBuilder ^sb, 
		bool showStaticOnly, bool showAttributes, bool declaredOnTypeOnly);
	/*
	// see comments in utils.cpp
	bool ShowInterfaceInfo(System::Type ^type, MCHAR* pattern, System::Text::StringBuilder ^sb);
	void GetInterfaces(System::Type ^type, Array* result);
	bool ShowAttributeInfo(System::Type ^type, MCHAR* pattern, System::Text::StringBuilder ^sb, bool declaredOnTypeOnly);
	void GetAttributes(System::Type ^type, Array* result);
	*/
	bool ShowConstructorInfo(System::Type ^type, System::Text::StringBuilder ^sb);
	void GetPropertyAndFieldNames(System::Type ^type, Array* propNames, 
		bool showStaticOnly, bool declaredOnTypeOnly);
	System::Object^ MXS_Value_To_Object(Value* val, System::Type ^ type);
	Value* Object_To_MXS_Value(System::Object^ val);

	void PrepArgList(array<System::Reflection::ParameterInfo^>^ candidateParamList, 
					Value**                 arg_list, 
					array<System::Object^>^ argArray, 
					int                     count);
	int FindMethodAndPrepArgList(System::Collections::Generic::List<array<System::Reflection::ParameterInfo^>^>^ candidateParamLists, 
							Value**                 arg_list, 
							array<System::Object^>^ argArray,
							int                     count, 
							Array*                  userSpecifiedParamTypes);
	bool isCompatible(array<System::Reflection::ParameterInfo^>^ candidateParamList,
					Value**   arg_list, 
					int       count);
	int CalcParamScore(Value *val, System::Type^ type);
	Value* CombineEnums(Value** arg_list, int count);
	bool CompareEnums(Value** arg_list, int count);
	Value* LoadAssembly(MCHAR* assyDll);
	// converts maxscript value to a dotNetObject value of type specified by argument 'type'
	Value* MXSValueToDotNetObjectValue(Value *val, dotNetBase *type);

	System::Type^ ResolveTypeFromName(const MCHAR* pTypeString);
};
