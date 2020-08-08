/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypdg */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_PdgInputProducer_H
#define KyPdg_PdgInputProducer_H


#include <kypathdata/generic/memory.h>
#include <kypdg/pdginput/clientinputconsumer.h>
// a forward declare of ClientInputConsumer would be enough but #include is handy
// to avoid the derivation to #include manually <kypdg/pdginput/clientinputconsumer.h>

namespace Kaim
{


class PdgSystem;


/*!	The PdgInputProducer is an abstract base class for an object invoked by the PathData generation system to retrieve the geometry
	for which it should generate PathData.
	\pp This class cannot be used as-is; you must create your own class that derives from PdgInputProducer, and implement the Produce()
	method.
	\pp You pass an instance of this class to the Generator for each sector or multi-sector that you want to create. Therefore, each 
	object that derives from this class is typically responsible for handling a single sector chunk of the geometry in your terrain.
	\ingroup kypdg */
class PdgInputProducer : public VirtualMemObject
{
public:
	/*! \label_baseconstructor */
	PdgInputProducer() {}

	/*! Called by the PathData generation system to retrieve the geometry in the sector associated with this object.
		Your implementation of this method must push the triangles and terrain types contained in your geometry to
		the \c inputConsumer using the methods provided by the ClientInputConsumer class. 
		\pp Typically, this means iterating through the triangles in the geometry managed by this object, and making
		a call to one of the \c Consume... methods of the ClientInputConsumer for each triangle.
		\pp You can also add terrain type volumes by calling ClientInputConsumer::ConsumeTerrainTypeVolume, which
		automatically tags all triangles in the specified volume with a specified terrain type.*/
	virtual KyResult Produce(ClientInputConsumer& inputConsumer) = 0;
};


}


#endif

