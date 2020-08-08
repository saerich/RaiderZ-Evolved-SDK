//-------------------------------------------------------------
// Access to the Camera Map modifier
//
#include "iFnPub.h"

/***************************************************************
Function Publishing System stuff   
****************************************************************/

#define CLUSTNODE_MOD_INTERFACE Interface_ID(0x2f6d169b, 0x7c211e3f)
#define GetIClustNodeModInterface(cd) \
			(IClustNodeMod *)(cd)->GetInterface(CLUSTNODE_MOD_INTERFACE)

//****************************************************************


class IClustNodeMod : public OSModifier, public FPMixinInterface {
	public:
		
		//Function Publishing System
		enum {  get_control_node, set_control_node,  
				};
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP
			PROP_TFNS(get_control_node, getControlNode, set_control_node, setControlNode, TYPE_INODE);

		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 
		//**************************************************

		virtual INode*	getControlNode(TimeValue t)=0;
		virtual void	setControlNode(INode* cam, TimeValue t)=0;
	};


static INode*
find_scene_obj_node(ReferenceTarget* obj)
{
	// given a scene base object or modifier, look for a referencing node via successive 
	// reference enumerations up through any intervening mod stack entries
	return GetCOREInterface7()->FindNodeFromBaseObject(obj);
}
