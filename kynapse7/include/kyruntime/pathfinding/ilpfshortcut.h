/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ILPFSHORTCUT_H
#define KY_ILPFSHORTCUT_H

#include <kyruntime/pathfinding/imodifier.h>

#include <kyruntime/core/inextmovemanager.h>
#include <kyruntime/core/pointwrapper.h>


namespace Kaim
{

class IPathFinder;
class ILpfShortcut;


///////////////////////////////////////////////////////////////////////////////////////////
// ILpfShortcut
///////////////////////////////////////////////////////////////////////////////////////////
/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from ILpfShortcut. Each class that derives from 
	ILpfShortcut must contain a constructor function that matches this type definition. The argument list
	of this type definition is empty, which indicates that the default constructor of your derived class
	will be invoked.
	\ingroup kyruntime_pathfinding */
typedef ILpfShortcut* (*LpfShortcutConstructor)();

/*!	LpfShortcutClass is the MetaClass for ILpfShortcut, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
class LpfShortcutClass: public MetaClass<ILpfShortcut, LpfShortcutConstructor>
{
public:
	/*!	Constructor for an ILpfShortcut MetaClass. For internal use. */
	LpfShortcutClass(const char* className, LpfShortcutConstructor ctr, LpfShortcutClass* baseClass);
};

/*!	This macro must be included in the declaration of any IModifier class that derives
	from ILpfShortcut, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_DECLARE_LPFSHORTCUT(ky_class) \
	static Kaim::LpfShortcutClass Class; \
	static Kaim::ILpfShortcut* KyCreate(); \
	Kaim::LpfShortcutClass& GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any IModifier class that derives
	from ILpfShortcut, to set up the MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_pathfinding */
#define KY_IMPLEMENT_LPFSHORTCUT(ky_class,ky_base_class) \
	Kaim::LpfShortcutClass ky_class::Class(#ky_class,ky_class::KyCreate,ky_base_class); \
	Kaim::ILpfShortcut* ky_class::KyCreate() \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::ILpfShortcut* ptr = (Kaim::ILpfShortcut*) KY_NEW_CLASS(ky_class)(); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	ILpfShortcut is an abstract base class for a type of IModifier that the PathFinder uses to
	determine what an Bot should do when it encounters an \LpfArea. 
	\pp Whenever an Bot encounters an \LpfArea, it can either follow the local path around the \LpfArea as
	calculated by the LpfManager, or it can follow a shortcut trajectory provided by an IModifier class
	that implements ILpfShortcut. The Bot makes this decision by comparing the cost of following the local 
	path around the obstacles with the cost of following the shortcut, as provided by the ILpfShortcut::GetCost() method. If the 
	cost of following the shortcut is less than the cost of following the local path, the PathFinder 
	will then call ILpfShortcut::Goto() at each frame to move the Bot along the shortcut.
	\pp This class is only effective when using Local Pathfinding. For details, see :USERGUIDE:"Using Obstacles".
	\section writingcustomilpfshortcut Writing a custom ILpfShortcut class
	Writing a custom class that derives from ILpfShortcut involves the following considerations.
	\subsection ilpfshortcutconst Constructor
	All classes that derive from ILpfShortcut must have a constructor that follows the #LpfShortcutConstructor type definition. 
	\subsection ilpfshortcutvm Virtual methods
	All classes that derive from ILpfShortcut must implement the GoTo() method.
	\pp You may also re-implement any of the other virtual methods, or use the default implementations if you prefer.
	\subsection ilpfshortcutmetaclass MetaClass macros
	In order for \SDKName to recognize your ILpfShortcut class, you must include the following macro definitions:
	-	#KY_DECLARE_LPFSHORTCUT: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_LPFSHORTCUT: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from ILpfShortcut.
			For example, &Kaim::LpfShortcut_SimpleJump::Class. If your new class derives directly from ILpfShortcut,
			the value of this argument should be #KY_NULL.
*/
class ILpfShortcut: public IModifier
{
public:
	/*!	\label_constructor */
	ILpfShortcut():
		IModifier() {};
		
	/*!	\label_getclass */
	virtual LpfShortcutClass& GetClass() const = 0;

	/*!	The LpfManager calls this method in order to decide whether or not the specified Bot 
		should follow the specified shortcut through an \LpfArea. 
		\pp The shortcut is defined by the point at which the global path of the Bot through the Graph
		first intersects the boundaries of the \LpfArea, and the point at which the global path exits the
		boundaries of the \LpfArea. For example, in the image below, the \c begin parameter would contain 
		the coordinates of point \b a, and the \c end parameter would contain the coordinates of point \b b. 
		\pp \image html "graphics/lpf_shortcut_getcost.png"
		\pp If the cost determined by this method is less than the cost of following the local path calculated by 
		the LpfManager, the Bot will take the shortcut, and begin calling the ILpfShortcut::Goto() 
		method at each frame to direct the Bot along the shortcut.
		\param bot				The Bot using the PathFinder.
		\param from				The point at which the global path of the Bot first intersects the \LpfArea.
		\param to				The point at which the global path of the Bot exits the \LpfArea.
		\param constraint		The current OConstraint in use by the PathFinder.
		\param[out] cost		Stores the cost computed by this method.
		\return #KY_TRUE if the Bot can follow the specified shortcut and the \c cost parameter was successfully 
				updated, or #KY_FALSE if the Bot should not shortcut through the \LpfArea under any circumstances. */
	virtual KyBool GetCost(Bot* bot, const PointWrapper& from, const PointWrapper& to, 
		IConstraint& constraint, KyFloat32& cost)
	{
		return constraint.GetCost(bot, from, to, cost);
	}

	/*!	The PathFinder calls this method when a Bot currently using the ILpfShortcut needs to 
		calculate a new path, in order to determine whether or not the Bot can leave the shortcut immediately. 
		\pp If this method returns #KY_FALSE, the Bot will not re-calculate its path. Instead, it will continue to 
		use the ILpfShortcut modifier until the GoTo() method indicates that the Bot has finished using the shortcut.
		If this method returns #KY_TRUE, the Bot will be allowed to calculate a new path. 
		\pp The default implementation of this method returns #KY_FALSE in all cases.
		\param Bot&				The Bot using the shortcut.
		\return #KY_TRUE if the Bot may recompute its Path and leave the shortcut immediately, or #KY_FALSE if it 
				must finish traversing the shortcut before it can recompute its path. */
	virtual KyBool CanLeave(const Bot& /*entity*/) {
		return KY_FALSE;
	}

	/*!	The PathFinder calls this method transparently when its Bot is about to follow a new shortcut.
		You can override this method in order to perform any steps needed to initialize your class.
		\param Vec3fList*		The local Path calculated for the Bot. */
	virtual void InitShortcut(Vec3fList* /*localPath*/) {};

	/*!	The PathFinder calls this method transparently when its Bot has completed following its shortcut.
		You can override this method in order to release any resources used by your class that are no longer
		needed. */
	virtual void ReleaseShortcut() {};

	/*!	This method determines how a Bot traverses the shortcut. It can use either of the following approaches: 
		-	It can determine a point to which the Bot must move, and pass this point to the PathFinder by updating the value 
			of the \c targetPoint parameter. The PathFinder will take responsibility for moving the Bot to the specified point. 
			In this case, the GoTo() method should return #KY_FALSE.
		-	It can take total control over the way the Bot crosses the obstacles by creating a new Action that represents the movements 
			required, and saving that Action in the \c action parameter. In this case, the GoTo() method should return #KY_TRUE. To create a 
			new Action, you can compute it yourself from scratch, or you can invoke the ISteering::ComputeAction() method of the ISteering 
			modifier currently in use by the PathFinder.
		\pp \param[out] targetPoint		A new target point for the PathFinder. Read only when this method returns #KY_FALSE.
		\param[out] action				An Action that will make the Bot use the shortcut. Read only when this method returns #KY_TRUE.
		\return #KY_TRUE if \c action has been updated to provide a specific set of movements for the Bot, or #KY_FALSE
				otherwise. */
	virtual KyBool Goto(const PointWrapper& targetPoint, Action& action) = 0;

	/*!	The PathFinder calls this method to determines whether or not a Bot is using the shortcut as expected. 
		For example, if the configuration of the \LpfArea changes while the Bot is following the shortcut such that the
		conditions for the Bot to be able to shortcut past it are no longer met, you may want your modifier to trigger
		an accident.
		\pp Implementing this method involves a combination of the return value and the \c KyBool& parameter. 
		\pp The return value indicates whether the CheckAccident() method of the ILpfShortcut modifier performs accident detection, 
		or whether it defers accident detection to the IDetectAccident modifier currently in use by the PathFinder. 
		\pp To defer accident detection to the IDetectAccident modifier currently in use by the PathFinder, the CheckAccident() method 
		should return #KY_FALSE. In this case, the PathFinder does not take the value of the \c KyBool& parameter into account.
		\pp To make the ILpfShortcut modifier responsible for detecting accidents, the CheckAccident() method should return #KY_TRUE. 
		In this case, the value of the \c KyBool& parameter determines whether or not an accident has occurred. 
		-	If an accident has occurred, the CheckAccident() method should set the value of the \c KyBool& argument to #KY_TRUE. In this case, 
			the PathFinder leaves the shortcut and recomputes a new path for the Bot to get to its destination. 
		-	If an accident has not occurred, and the Bot is following the shortcut as expected, the CheckAccident() method should set 
			the value of the \c KyBool& parameter to #KY_FALSE. 
		\pp The default implementation of this method returns #KY_FALSE in all cases.
		\param Bot&						The Bot using the shortcut.
		\param[out] KyBool&				Indicates whether or not the Bot has encountered an accident while traversing the shortcut.
										Read only when the method returns #KY_TRUE.
		\return #KY_TRUE if the method checked for accidents and updated the \c accidentDetected parameter accordingly, or #KY_FALSE to defer
				accident detection to the IDetectAccident modifier in use by the PathFinder. */
	virtual KyBool CheckAccident(const Bot& /*entity*/, KyBool& /*accidentDetected*/) { 
		return KY_FALSE;
	}
};

} // namespace Kaim

#endif // KY_ILPFSHORTCUT_H
