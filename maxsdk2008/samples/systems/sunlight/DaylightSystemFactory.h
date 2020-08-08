#ifndef SUNCREATIONIMP_H
#define SUNCREATIONIMP_H

#include "IDaylightSystem.h"
#include "maxtypes.h"
#include "paramtype.h"
#include "Noncopyable.h"

class DaylightSystemFactory : public IDaylightSystemFactory, public MaxSDK::Util::Noncopyable 
{
private:
	// The single instance of this class
	static DaylightSystemFactory	mTheDaylightSystemFactory;
	static float kDefaultCompassDiameter;
	static float kDefaultOrbitalScale;
public:

	DECLARE_DESCRIPTOR_NDC(DaylightSystemFactory); 

	INode* Create(
		IDaylightSystem*& pDaylight
		);
};


#endif