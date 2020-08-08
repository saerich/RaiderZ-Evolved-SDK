// Copyright 2009 Autodesk, Inc. All rights reserved.
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law. They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.

#pragma once
#include "NativeInclude.h"

#pragma managed(push, on)

class CompassRoseObject;

class CompassRoseObjCreateCallBack : public CreateMouseCallBack 
{
public:
	int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
	void SetObj(CompassRoseObject *obj);
	
private:
	Point3 p0;
	Point3 p1;
	CompassRoseObject *ob;
};

#pragma managed(pop)
