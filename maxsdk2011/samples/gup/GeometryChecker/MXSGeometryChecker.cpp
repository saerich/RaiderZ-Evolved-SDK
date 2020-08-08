/*==============================================================================
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Geometry Checker
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/
#include <Max.h>
#include <bmmlib.h>
#include <maxapi.h>
#include <IGeometryChecker.h>
#include "checkerimplementations.h"

MXSGeometryChecker::MXSGeometryChecker(Value *checkerFunc,Value *isSupportedFunc,  IGeometryChecker::ReturnVal returnVal,MSTR &name,Value *popupFunc,Value *textOverrideFunc, Value *displayFunc): 
	mCheckerFunc(checkerFunc), mIsSupportedFunc(isSupportedFunc),mName(name), mReturnVal(returnVal), mPopupFunc(popupFunc), mTextOverrideFunc(textOverrideFunc),
		mDisplayOverrideFunc(displayFunc)
{
	IGeometryCheckerManager *man = GetIGeometryCheckerManager();
	if(man)
		mID = man->GetNextValidGeometryCheckerID();
	else
		mID = -1;
};

//inherited funcs
bool MXSGeometryChecker::IsSupported(INode * node)
{
	if( mIsSupportedFunc == NULL )
		return false;

	bool bresult;

	init_thread_locals();
	push_alloc_frame();
	one_value_local(node);	
	save_current_frames();
	trace_back_active = FALSE;

	try 
	{
		vl.node = MAXClass::make_wrapper_for((INode*)node);

		Value* args[1] = {vl.node };
		Value* result = mIsSupportedFunc->apply(args, 1);

		bresult = result->to_bool()?true:false;
		pop_value_locals();
		pop_alloc_frame();
		return bresult;
	}
	catch (MAXScriptException& e)
	{
		restore_current_frames();
		MAXScript_signals = 0;
		if (progress_bar_up)
			MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
		show_source_pos();
		error_message_box(e, GetString(IDS_REGISTER_EXCEPTION_SUPPORT));
	}
	catch (...)
	{
		restore_current_frames();
		if (progress_bar_up)
			MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
		show_source_pos();
		error_message_box(UnknownSystemException(), GetString(IDS_REGISTER_EXCEPTION_SUPPORT));
	}

	// will only get here if error occurred.. we unregister that function
	IGeometryCheckerManager *man = GetIGeometryCheckerManager();
	man->UnRegisterGeometryChecker(this);
	pop_value_locals();
	pop_alloc_frame();
	return false;
}

IGeometryChecker::ReturnVal MXSGeometryChecker::TypeReturned()
{
	return mReturnVal;
}

//! do the check
IGeometryChecker::ReturnVal MXSGeometryChecker::GeometryCheck(TimeValue t,INode *node, IGeometryChecker::OutputVal &val)
{

	if( mCheckerFunc == NULL||IsSupported(node)==FALSE )
		return IGeometryChecker::eFail;

	Tab<int> index;

	init_thread_locals();
	push_alloc_frame();
	three_value_locals(t,node,index);	
	save_current_frames();
	trace_back_active = FALSE;

	try 
	{
		vl.t = MSTime::intern(t);
		vl.node = MAXClass::make_wrapper_for((INode*)node);
		vl.index = new Array(val.mIndex.Count());
		int i;
		for(i=0;i<val.mIndex.Count();++i)
			((Array*)vl.index)->append(Integer::intern(val.mIndex[i]+1));
		Value* args[3] = {vl.t,vl.node,vl.index };
		Value* result = mCheckerFunc->apply(args, 3);

		int resultVal = result->to_int();
		IGeometryChecker::ReturnVal returnVal;
		switch(resultVal)
		{
		case 1:
			returnVal = IGeometryChecker::eVertices;
		break;
		case 2:
			returnVal = IGeometryChecker::eEdges;
		break;
		case 3:
			returnVal = IGeometryChecker::eFaces;
		break;
		default:
			returnVal = IGeometryChecker::eFail;
			break;
		}
		//okay now get the returned indices... .. need to subtract 1
		val.mIndex.ZeroCount();
		for (i = 0; i < ((Array*)args[2])->size; i++)
		{
	        int index = (*((Array*)args[2]))[i]->to_int()-1;
			val.mIndex.Append(1,&index);
		}
		pop_value_locals();
		pop_alloc_frame();
		return returnVal;
	}
	catch (MAXScriptException& e)
	{
		restore_current_frames();
		MAXScript_signals = 0;
		if (progress_bar_up)
			MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
		show_source_pos();
		error_message_box(e, GetString(IDS_REGISTER_EXCEPTION_CHECKER));
	}
	catch (...)
	{
		restore_current_frames();
		if (progress_bar_up)
			MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
		show_source_pos();
		error_message_box(UnknownSystemException(), GetString(IDS_REGISTER_EXCEPTION_CHECKER));
	}

	// will only get here if error occurred.. we unregister that function
	IGeometryCheckerManager *man = GetIGeometryCheckerManager();
	man->UnRegisterGeometryChecker(this);
	pop_value_locals();
	pop_alloc_frame();
	return IGeometryChecker::eFail;
}

//! the name returned
MSTR MXSGeometryChecker::GetName()
{
	return mName;
}

//property dialog stuff.  whether or not it has a property dialog, and if so, show it.
bool MXSGeometryChecker::HasPropertyDlg()
{
	if(mPopupFunc)
		return true;
	else
		return false;
}
void MXSGeometryChecker::ShowPropertyDlg()
{
	if( mPopupFunc == NULL )
		return;

	init_thread_locals();
	push_alloc_frame();
	save_current_frames();
	trace_back_active = FALSE;

	try 
	{
		Value* result = mPopupFunc->apply(NULL, 0);
		pop_alloc_frame();
		return;
	}
	catch (MAXScriptException& e)
	{
		restore_current_frames();
		MAXScript_signals = 0;
		if (progress_bar_up)
			MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
		show_source_pos();
		error_message_box(e, GetString(IDS_REGISTER_EXCEPTION_POPUP));
	}
	catch (...)
	{
		restore_current_frames();
		if (progress_bar_up)
			MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
		show_source_pos();
		error_message_box(UnknownSystemException(), GetString(IDS_REGISTER_EXCEPTION_POPUP));
	}

	// will only get here if error occurred.. we unregister that function
	IGeometryCheckerManager *man = GetIGeometryCheckerManager();
	man->UnRegisterGeometryChecker(this);
	pop_alloc_frame();
	return;
}

bool MXSGeometryChecker::HasTextOverride()
{
	if(mTextOverrideFunc)
		return true;
	else
		return false;
}
MSTR MXSGeometryChecker::TextOverride()
{

	if( mTextOverrideFunc == NULL )
		return MSTR("");;

	init_thread_locals();
	push_alloc_frame();
	save_current_frames();
	trace_back_active = FALSE;

	try 
	{
		Value* result = mTextOverrideFunc->apply(NULL, 0);
		MSTR str = result->to_string();
		pop_alloc_frame();
		return str;
	}
	catch (MAXScriptException& e)
	{
		restore_current_frames();
		MAXScript_signals = 0;
		if (progress_bar_up)
			MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
		show_source_pos();
		error_message_box(e, GetString(IDS_REGISTER_EXCEPTION_TEXT));
	}
	catch (...)
	{
		restore_current_frames();
		if (progress_bar_up)
			MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
		show_source_pos();
		error_message_box(UnknownSystemException(), GetString(IDS_REGISTER_EXCEPTION_TEXT));
	}

	// will only get here if error occurred.. we unregister that function
	IGeometryCheckerManager *man = GetIGeometryCheckerManager();
	man->UnRegisterGeometryChecker(this);
	pop_alloc_frame();
	return MSTR("");
}

bool MXSGeometryChecker::HasDisplayOverride()
{
	if(mDisplayOverrideFunc)
		return true;
	else 
		return false;
}
void MXSGeometryChecker::DisplayOverride(TimeValue t, INode* node, HWND hwnd,Tab<int> &results)
{
	if( mDisplayOverrideFunc == NULL )
		return;

	init_thread_locals();
	push_alloc_frame();
	four_value_locals(t,node,hwnd,index);	
	save_current_frames();
	trace_back_active = FALSE;

	try 
	{
		vl.t = MSTime::intern(t);
		vl.node = MAXClass::make_wrapper_for((INode*)node);
		vl.hwnd = Integer::intern((INT_PTR)hwnd);
		vl.index = new Array(results.Count());
		for(int i=0;i<results.Count();++i)
		{
            int val = results[i] + 1;
            ((Array*)vl.index)->append(Integer::intern(val)); //need to add 1 to make it in mxs space
		}

		Value* args[4] = {vl.t,vl.node,vl.hwnd,vl.index };
		Value* result = mDisplayOverrideFunc->apply(args, 4);

		pop_value_locals();
		pop_alloc_frame();
		return;
	}
	catch (MAXScriptException& e)
	{
		restore_current_frames();
		MAXScript_signals = 0;
		if (progress_bar_up)
			MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
		show_source_pos();
		error_message_box(e, GetString(IDS_REGISTER_EXCEPTION_DISPLAY));
	}
	catch (...)
	{
		restore_current_frames();
		if (progress_bar_up)
			MAXScript_interface->ProgressEnd(), progress_bar_up = FALSE;
		show_source_pos();
		error_message_box(UnknownSystemException(), GetString(IDS_REGISTER_EXCEPTION_DISPLAY));
	}

	// will only get here if error occurred.. we unregister that function
	IGeometryCheckerManager *man = GetIGeometryCheckerManager();
	man->UnRegisterGeometryChecker(this);
	pop_value_locals();
	pop_alloc_frame();
	return;	
}

//remove all of the new mxs funcs
void MXSGeometryChecker::CleanOutFuncs()
{
	for(int x = mxsFuncs->size-1; x>=0 && (mCheckerFunc||mIsSupportedFunc||mPopupFunc||mTextOverrideFunc||mDisplayOverrideFunc); x--)
	{
		bool foundOne = false;
		Value *thisCallback = mxsFuncs->get(x+1); // mxs 1-based
		if (thisCallback == mCheckerFunc)
		{
			foundOne = true;
			mCheckerFunc = NULL;
		}
		else if (thisCallback == mIsSupportedFunc)
		{
			foundOne = true;
			mIsSupportedFunc = NULL;
		}
		else if (thisCallback == mIsSupportedFunc)
		{
			foundOne = true;
			mIsSupportedFunc = NULL;
		}
		else if (thisCallback == mPopupFunc)
		{
			foundOne = true;
			mPopupFunc = NULL;
		}
		else if (thisCallback == mTextOverrideFunc)
		{
			foundOne = true;
			mTextOverrideFunc = NULL;
		}
		else if (thisCallback == mDisplayOverrideFunc)
		{
			foundOne = true;
			mDisplayOverrideFunc = NULL;
		}
		if (foundOne)
		{
			Value* which = Integer::intern(x+1); // mxs 1-based
			mxsFuncs->deleteItem_vf(&which, 1);
		}
	}

}


