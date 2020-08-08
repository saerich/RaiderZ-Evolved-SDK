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
// DESCRIPTION: Reference Notifier
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/

#ifndef __NOTIFIER__H__
#define __NOTIFIER__H__

#include <max.h>
#include <Maxapi.h>

class NotifyCallback
{
public:
	virtual RefResult NotifyFunc(Interval changeInt,
         RefTargetHandle hTarget, PartID& partID,
            RefMessage message)=0;

};

class NotifyMgr : public ReferenceTarget {
   private:      
      NotifyCallback *mpNotifyCB;
      Tab<RefTargetHandle> mpReferences;
   public:
      // --- Inherited Virtual Methods From Animatable ---
      void GetClassName(MSTR &s) { s = _T("Notifier"); }

      // --- Inherited Virtual Methods From ReferenceMaker ---
      RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
         PartID& partID,RefMessage message);
      int NumRefs();
      RefTargetHandle GetReference(int i);
      void SetReference(int i, RefTargetHandle rtarg);
	  void DeleteThis();
      // --- Methods From NotifyMgr ---
      NotifyMgr::NotifyMgr();
      NotifyMgr::~NotifyMgr();
	  void SetNumberOfReferences(int num);
      BOOL CreateReference(int which, RefTargetHandle hTarget);
      BOOL RemoveReference(int which);
	  void RemoveAllReferences(); 
      void SetNotifyCB(NotifyCallback *cb);
      void ResetNotifyFunc();
};

#endif