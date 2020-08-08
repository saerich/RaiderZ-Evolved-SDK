///*
//* Copyright 2010 Autodesk, Inc.  All rights reserved.
//* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
//* or which otherwise accompanies this software in either electronic or hard copy form.
//*/
//
//	
//
//
//// primary contact: GUAL - secondary contact: MAMU
//#ifndef KyPathData_IVisualRenderer_H
//#define KyPathData_IVisualRenderer_H
//
//
//#include <kypathdata/generic/memory.h>
//#include <kypathdata/visualsystem/visualgeometry.h>
//
//
//namespace Kaim
//{
//
//class VisualGeometry;
//
///*!
//Interface to be implemented by game engines.
//IVisualRenderer functions are called by the VisualSystem when VisualRepresentation are added or removed.
//*/
//class IVisualRenderer // don't use Kynapse Allocator
//{
//public:
//	virtual ~IVisualRenderer() {}
//
//	virtual void Add(VisualGeometry* geometry) = 0;
//
//	virtual void Remove(VisualGeometry* geometry) = 0;
//
//	/*! Called by VisualSystem::ChangeRenderMode on each geometry of the VisualRepresentation
//	    to request a change of render mode (eg: wireframe, solid, alpha,...).
//		Each visual representation contains a default render mode.
//		Consequently, if you dont want to allow to change the render mode you can provide an empty implementation.
//	*/
//	virtual void ChangeRenderMode(VisualGeometry* /*geometry*/, VisualRenderMode /*renderMode*/, void* /*userRenderMode*/) = 0;
//};
//
//
//}
//
//
//#endif
