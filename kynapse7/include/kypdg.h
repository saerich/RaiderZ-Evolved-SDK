/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

/*! \file
	\brief Includes all necessary files for integrating the \SDKName PathData generation framework. */

#ifndef KyPdg_GlobalInclude_H
#define KyPdg_GlobalInclude_H

#include <kypdg/pdginput/clientinput.h>
#include <kypdg/pdginput/clientinputconsumer.h>
#include <kypdg/pdginput/clientinputtriangle.h>
#include <kypdg/pdginput/clientinputtrianglesoup.h>
#include <kypdg/pdginput/inputtriangle.h>
#include <kypdg/pdginput/objfilereader.h>
#include <kypdg/pdginput/pdginput.h>
#include <kypdg/pdginput/pdginputcell.h>
#include <kypdg/pdginput/pdginputcellinfo.h>
#include <kypdg/pdginput/pdginputcellvisualrepresentation.h>
#include <kypdg/pdginput/pdginputproducer.h>
#include <kypdg/pdginput/pdginputproducerfromclientinput.h>
#include <kypdg/pdginput/pdginputproducerfromclientinputgeometry.h>
#include <kypdg/pdginput/pdginputproducerobj.h>
#include <kypdg/pdginput/pdginputterraintypevolume.h>
#include <kypdg/pdginput/pdginputterraintypevolumesvisualrepresentation.h>
#include <kypdg/pdginput/pdginputterraintypevolumevisualrepresentation.h>
#include <kypdg/pdginput/pdginputvisualrepresentation.h>
#include <kypdg/pdginput/pdgsectorconfig.h>
#include <kypdg/pdginput/seedpoint.h>
#include <kypdg/pdginput/seedpointvisualrepresentation.h>
#include <kypdg/pdginput/terraintypevolume.h>
 
#include <kypdg/generator/cellgeneratorprofiling.h>
#include <kypdg/generator/additionaldatamaker.h>
#include <kypdg/generator/fixedsizesectorpartitioner.h>
#include <kypdg/generator/generator.h>
#include <kypdg/generator/generatorprofiling.h>
#include <kypdg/generator/globalcellgrid.h>
#include <kypdg/generator/iparallelforinterface.h>
#include <kypdg/generator/ipdgsectorgeneratorpostprocess.h>
#include <kypdg/generator/partitioningcelldesc.h>
#include <kypdg/generator/pdgadvancedparameters.h>
#include <kypdg/generator/pdgglobalconfig.h>
#include <kypdg/generator/pdgguidcompound.h>
#include <kypdg/generator/pdgpathdataelement.h>
#include <kypdg/generator/pdgrunoptions.h>
#include <kypdg/generator/pdgsector.h>
#include <kypdg/generator/pdgsectorgenerator.h>
#include <kypdg/generator/pdgsectorpartitioner.h>
#include <kypdg/generator/pdgsystem.h>
#include <kypdg/generator/squarepartition.h>
#include <kypdg/generator/squarepartitiontypes.h>
#include <kypdg/generator/squarepartitionutils.h>
#include <kypdg/generator/pathdataindexblobbuilder_fromgenerator.h>


#endif //KyPdg_GlobalInclude_H

