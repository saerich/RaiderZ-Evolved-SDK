// Copyright 2009 Autodesk, Inc. All rights reserved.
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law. They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.

#include "CompassRoseObjCreateCallBack.h"
#include "compass.h"

#pragma managed

void CompassRoseObjCreateCallBack::SetObj(CompassRoseObject *obj) 
{ 
	ob = obj; 
}

int CompassRoseObjCreateCallBack::proc( ViewExp *vpt,
	int msg, 
	int point, 
	int flags, 
	IPoint2 m, 
	Matrix3& mat ) 
{ 
	if (msg == MOUSE_POINT)
	{
		if (point == 0)
		{
			ob->suspendSnap = TRUE;
			p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
			mat.SetTrans(p0);
		}
		else
		{
			ob->suspendSnap = FALSE;
			return 0;
		}
	}
	else if (msg==MOUSE_MOVE)
	{
		p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
		ob->axisLength = max(AXIS_LENGTH, Length(p1-p0));
		PostMessage(ob->hParams, RU_UPDATE, 0, 0);
	}
	else if (msg == MOUSE_ABORT)
	{     
		return CREATE_ABORT;
	}
	else if (msg == MOUSE_FREEMOVE)
	{
		vpt->SnapPreview(m,m,NULL,SNAP_IN_PLANE);
	}
	return 1;
}
