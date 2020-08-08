#pragma once

#include <ref.h>
// forward declarations
class Control;
class INode;

//! \brief Class for enumerating reference makers for a given control.
/*! Given a controller, this class will enumerate all reference makers for that
	Control pointer. When it finds a reference maker that is an INode, it will examine
	it, to see if it uses the Control as a sub anim of it's transform controller.
	After the enumeration is done, use the GetNode method to get the INode that uses the
	specified Control.
	So for example, if called from inside of a Controller method, you could use it to
	find the INode that uses this controller.
	\code
	NodeSearchEnumProc dep(this);
	DoEnumDependents(&dep);

	INode* theNode = dep.GetNode();
	\endcode
	So in other words, this enumerator searches up the reference maker tree starting
	from a Control. And (internally) it utilizes an AnimEnum to search down the 
	sub anim tree looking for the specified Control.
*/
class NodeSearchEnumProc : public DependentEnumProc 
{
public:
	//! \brief Contructor
	/*! \param c - Pass in the controller ReferenceTarget to start the search */
	NodeSearchEnumProc(Control* c);
	//! \brief The callback method used by the system
	//! \param m - The dependent item that watches the Target (in this case a Control).
	virtual int proc(ReferenceMaker* m);
	//! Gets the node after Enumeration has finished.
	INode* GetNode();
protected:
	Control* mCtrl;
	INode* mNode;
};