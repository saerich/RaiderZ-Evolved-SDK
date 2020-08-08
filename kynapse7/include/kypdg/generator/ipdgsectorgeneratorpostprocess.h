/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

/*! \file
	\ingroup kypdg */

// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPdg_IPdgSectorGeneratorPostProcess_H
#define KyPdg_IPdgSectorGeneratorPostProcess_H

#include "kypathdata/graph/dynamicadditionalgraph.h"
#include "kypathdata/containers/array.h"
#include "kypdg/generator/additionaldatamaker.h"

namespace Kaim
{

class NavMesh;
class Graph;
class InputGraphVertices;

/*!	ISectorGeneratorPostProcess is an abstract base interface for a class of objects that can be invoked by the Generator
	to perform a post-processing phase after generating PathData for a sector.
	\pp This class cannot be used as-is; you must write a custom class that derives from ISectorGeneratorPostProcess and
	that implements at a minimum the OnPathDataDone() method.
	\pp The primary use case for conducting a post-processing phase on the PathData for a sector is to create a custom
	additional graph and add it to the PathData for the sector. For background information on additional graphs, why
	you might want to use them, and how to create them, see :USERGUIDE:"Using Custom Additional Graphs".
	\pp To instruct the Generator to call your ISectorGeneratorPostProcess for a sector, you must create an instance of
	your class and pass it in a call to PdgSector::AddPostProcess() before you add the sector to the Generator. The
	instance you provide is not copied, so you must ensure that it remains in memory until you have finished using the 
	Generator.
	\ingroup kypdg */
class ISectorGeneratorPostProcess
{
	KY_DEFINE_NEW_DELETE_OPERATORS

friend class PdgSectorGenerator;

public:
	virtual ~ISectorGeneratorPostProcess() {}

	/*! The Generator invokes this method automatically immediately after it finishes generating the Graph and
		NavMesh for each sector. You can implement this method to perform any post-processing steps that you
		need to carry out after generating the PathData for the sector.
		\pp For instance, to add an additional graph to the PathData for the sector, your implementation of this
		method should create an DynamicAdditionalGraph object on the heap using the #KY_NEW_CLASS macro, add vertices
		and edges to the additional graph as needed, and add the pointer to your new object to the 
		#m_additionalGraphsFromPostProcess list by calling m_additionalGraphsFromPostProcess.PushBack().
		\param navmesh			The NavMesh generated for this sector.
		\param graph			The Graph generated for this sector.
		\param inputVertices	A record of the vertex positions provided as input for this sector, if any. You can
								use this object to find the actual vertex in the Graph that has been created to
								represent each of your input positions. See Graph::GetGraphVertexFromClientInitialPos() 
								and Graph::GetInputVertexFromClientInitialPos(). */
	virtual void OnPathDataDone(const NavMesh* navmesh, const Graph* graph, const InputGraphVertices* inputVertices) = 0;
	
	/*! The Generator invokes this method automatically just before clearing the Graph for this sector from memory.
		The default implementation deletes all DynamicAdditionalGraph objects maintained in the 
		#m_additionalGraphsFromPostProcess list, if any. You can re-implement this method if you need to clear 
		any other resources that you allocate in your implementation of OnPathDataDone(). */
	virtual void OnGeneratedGraphsClear()
	{
		// default behavior is to clear all the DynamicAdditionalGraph that have been created
		for (KyUInt32 i = 0; i < m_additionalGraphsFromPostProcess.GetCount(); ++i)
		{
			KY_DELETE_CLASS(m_additionalGraphsFromPostProcess[i]);
		}
		m_additionalGraphsFromPostProcess.Clear();
	}
public:
	/*! Stores pointers to DynamicAdditionalGraph objects that will be bundled into the PathData for this sector.
		Do not remove any of these objects from memory. They will be deleted automatically by OnGeneratedGraphsClear(). */
	Array<DynamicAdditionalGraph*> m_additionalGraphsFromPostProcess;

};


}


#endif
