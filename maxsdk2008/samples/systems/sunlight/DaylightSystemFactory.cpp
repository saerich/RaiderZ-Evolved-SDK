#include <max.h>
#include <iparamb2.h>
#include <INodeMonitor.h>
#include "suntypes.h"
#include "sunclass.h"
#include "natLight.h"
#include "compass.h"
#include "citylist.h"
#include "DaylightSystemFactory.h"

// Interface instance and descriptor
DaylightSystemFactory DaylightSystemFactory::mTheDaylightSystemFactory
(
	DAYLIGHTSYSTEM_FACTORY_INTERFACE,
	_T("DaylightSystemFactory"), // interface name used by maxscript - don't localize it!
	0, 
	NULL, 
	FP_CORE,

	end
	
);

float DaylightSystemFactory::kDefaultCompassDiameter = 100.0f;
float DaylightSystemFactory::kDefaultOrbitalScale = 70.0f;

INode* DaylightSystemFactory::Create(
								   IDaylightSystem*& pDaylight
								   )
{
	
	Interface* ip = GetCOREInterface();
	IObjCreate* pICreate = ip->GetIObjCreate();
	DbgAssert(pICreate != NULL);

	//Create a new SunMaster - which controls the time and geographic location of the system
	SunMaster* pSunMaster = new SunMaster(true);
	DbgAssert(pSunMaster != NULL);

	// Create the NaturalLightAssembly - it is a helper object controls the sun and sky components.
	NatLightAssembly* pNatLightHelper = NULL;
	INode* newNode = NatLightAssembly::CreateAssembly(pICreate, pNatLightHelper);
	DbgAssert(newNode != NULL);
	DbgAssert(pNatLightHelper != NULL);

	//Create and set several controllers.
	pNatLightHelper->SetMultController(new SlaveControl(pSunMaster,LIGHT_MULT));
	pNatLightHelper->SetIntenseController(new SlaveControl(pSunMaster,LIGHT_INTENSE));
	pNatLightHelper->SetSkyCondController(new SlaveControl(pSunMaster,LIGHT_SKY));	
	newNode->SetTMController(new SlaveControl(pSunMaster,LIGHT_TM));

	// Create the compass object 
	CompassRoseObject* compassObj = (CompassRoseObject*)pICreate->CreateInstance(HELPER_CLASS_ID, COMPASS_CLASS_ID); 			
	DbgAssert(compassObj != NULL);
	//Set the compass axis length to a default length just for visual purpose.
	//This parameter doesnt affect the precision of the sun.
	compassObj->axisLength = kDefaultCompassDiameter;

	//Create a dummy node to hold everything together.
	//This results on the compass node being the parent of the daylight node.
	INode* compassNode = pICreate->CreateObjectNode(compassObj);
	// Attach the new node as a child of the central node.
	compassNode->AttachChild(newNode);

	//Create a node monitor.
	if(pSunMaster->theObjectMon)
	{
		INodeMonitor* nodeMon = static_cast<INodeMonitor*>(pSunMaster->theObjectMon->GetInterface(IID_NODEMONITOR));
		DbgAssert(nodeMon != NULL);
		nodeMon->SetNode(newNode);
	}

	//the master references the point so it can get
	//notified when the	user rotates it
	pSunMaster->ReplaceReference(REF_POINT, compassNode );

	//We have to create controllers for look at and multiplier, these controllers are not
	//created in the constructor.

	//create a new PRS controller for the sun's lookat
	Control* pSunLookAt = static_cast<Control*>(CreateInstance(CTRL_MATRIX3_CLASS_ID, Class_ID(PRS_CONTROL_CLASS_ID,0)));
	pSunMaster->ReplaceReference(REF_LOOKAT, pSunLookAt);

	//Initialize the rotation for the look at controller 
	Control* pLookatRot = static_cast<Control*>(CreateInstance(CTRL_ROTATION_CLASS_ID, Class_ID(LOOKAT_CONSTRAINT_CLASS_ID, 0)));
	Animatable* a = pLookatRot;
	ILookAtConstRotation* ipLookatRot = GetILookAtConstInterface(a);
	DbgAssert(ipLookatRot != NULL);
	ipLookatRot->AppendTarget(pSunMaster->thePoint);
	ipLookatRot->SetTargetAxis(2);
	ipLookatRot->SetTargetAxisFlip(true);
	ipLookatRot->SetVLisAbs(false);
	pSunMaster->theLookat->SetRotationController(pLookatRot);

	//Create and a new controller for the sun's multiplier.
	Control* pMultiplier = static_cast<Control*>(
		CreateInstance(CTRL_FLOAT_CLASS_ID, Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0)));
	pSunMaster->ReplaceReference(REF_MULTIPLIER, pMultiplier );

	//Set the orbital scale to a default value, just for visual purpose.
	//This parameter doesnt affect the precision of the sun.
	pSunMaster->SetRad(ip->GetTime(),kDefaultOrbitalScale);

	//We are creating a daylight system.
	pSunMaster->daylightSystem = true;

	//fill in the IDaylightSystem pointer for access to sun and sky objects.
	pDaylight = static_cast<IDaylightSystem*>(pNatLightHelper->GetInterface(DAYLIGHT_SYSTEM_INTERFACE));

	return newNode;
}


