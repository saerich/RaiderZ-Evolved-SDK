#include "NodeSearchEnumProc.h"
#include "SubAnimEnumProc.h"
// SDK includes
#include <control.h>
#include <inode.h>
//#include <dbgprint.h>

NodeSearchEnumProc::NodeSearchEnumProc(Control* c)
: mCtrl(c), mNode(NULL)
{ }

INode* NodeSearchEnumProc::GetNode()
{
	return mNode;
}

int NodeSearchEnumProc::proc(ReferenceMaker* rmaker)
{
	//MSTR className;
	//rmaker->GetClassName(className);
	//DebugPrint("NodeSearchEnumProc::proc %s\n", className.data());
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)
	{
		// More than one INode can reference a Control pointer. Therefore
		// for every INode that references the specified controller (mCtrl), we 
		// have to enumerate the sub anims of that node's transform controller
		// to see if it uses the specified controller. 
		INode* tempNode = (INode*)rmaker;
		Control* nodeTMController = tempNode->GetTMController();
		if (nodeTMController)
		{
			// Instantiate our AnimEnum that looks for our specified controller (mCtrl).
			SubAnimEnumProc controllerEnum(mCtrl, SCOPE_ALL);
			// Here is where we enumerate all the sub anims of the transform controller.
			nodeTMController->EnumAnimTree(&controllerEnum, NULL, 0);
			// Check if the transform controller found a sub anim that matched mCtrl.
			if (controllerEnum.FoundControl())
			{
				mNode = tempNode;
				return DEP_ENUM_HALT;
			}			
		}
	}
	// We have to enumerate all dependents, since any one of them
	// could reference the Jiggle Controller
	return DEP_ENUM_CONTINUE;
}
