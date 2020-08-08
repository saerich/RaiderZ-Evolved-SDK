/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KYRUNTIME_LADDER_PATHOBJECT_BEHAVIOR_H
#define KYRUNTIME_LADDER_PATHOBJECT_BEHAVIOR_H

#include "kyruntime/pathobjects/offtheshelf/basepathobjectbehavior.h"
#include "kypathdata/containers/chunkbasedpool.h"
#include "kyruntime/pathobjects/offtheshelf/ladder/laddertraversefsm.h"

namespace Kaim
{

class LadderTraverseBehavior;

class LadderPathObjectBehavior : public BasePathObjectBehavior
{
public:
	/*!	Creates a ladder PathObject behavior with default moves and behavior.*/
	LadderPathObjectBehavior();

	/*!	Creates a ladder PathObject behavior with default moves and behavior.
	\param maxBotsUsingThisBehaviorAtheSameTime		Sets the typical number of bots using this behavior at the same time.
													Each time a bot uses this behavior, it retrieves a context from a pool which
													is initialized with this parameter. See ChunkBasedPool class for more details.
	*/
	LadderPathObjectBehavior(KyUInt16 maxBotsUsingThisBehaviorAtheSameTime);

	/*!	Creates a ladder PathObject behavior with custom moves in arguments.
		\param goToLadderMove							The custom move to go from entrance connexion space to the ladder (KY_NULL to use a default move).
		\param grabLadderMove							The custom move to grab the ladder (KY_NULL to use a default move).
		\param climbLadderMove							The custom move to climb the ladder (KY_NULL to use a default move).
		\param releaseLadderMove						The custom move to release the ladder (KY_NULL to use a default move).
		\param goToExitLadderMove						The custom move to go from ladder to exit connexion space (KY_NULL to use a default move).
		\param customTraverseBehavior					Custom traverse behavior (KY_NULL to use default traverseBehavior).
		\param maxBotsUsingThisBehaviorAtheSameTime		Sets the typical number of bots using this behavior at the same time.
														Each time a bot uses this behavior, it retrieves a context from a pool which
														is initialized with this parameter. See ChunkBasedPool class for more details.
	*/
	LadderPathObjectBehavior(IMove* goToLadderMove, IMove* grabLadderMove, IMove* climbLadderMove,
		IMove* releaseLadderMove, IMove* goToExitLadderMove, LadderTraverseBehavior* customTraverseBehavior,
		KyUInt16 maxBotsUsingThisBehaviorAtheSameTime);

	virtual ~LadderPathObjectBehavior();

	virtual void OnEnterPathObject(PathObjectBehaviorContext& /*context*/);
	virtual void OnLeavePathObject(PathObjectBehaviorContext& /*context*/);

	virtual void OnStartApproaching(PathObjectBehaviorContext& /*context*/);
	virtual void OnEndApproaching(PathObjectBehaviorContext& /*context*/);
	virtual void Approach(PathObjectBehaviorContext& /*context*/);
	virtual bool ApproachFailed(PathObjectBehaviorContext& /*context*/);

	virtual void OnStartTraversing(PathObjectBehaviorContext& /*context*/);
	virtual void Traverse(PathObjectBehaviorContext& /*context*/);
	virtual void OnEndTraversing(PathObjectBehaviorContext& /*context*/);
	virtual bool ExitReached(PathObjectBehaviorContext& context);

	virtual void OnStartEscaping(PathObjectBehaviorContext& /*context*/);
	virtual void Escape(PathObjectBehaviorContext& /*context*/);
	virtual void OnEndEscaping(PathObjectBehaviorContext& /*context*/);

protected:
	virtual void Init(IMove* goToLadderMove, IMove* grabLadderMove, IMove* climbLadderMove,
		IMove* releaseLadderMove, IMove* goToExitLadderMove, LadderTraverseBehavior* customTraverseBehavior,
		KyUInt16 defaultContextCount);

public:
	LadderTraverseBehavior* m_traverseBehavior;	//!< The behavior to traverse the ladder.

	IMove* m_goToLadderMove;					//!< The move used to reach the ladder before grabbing it.
	IMove* m_grabLadderMove;					//!< The move used to grab the ladder.
	IMove* m_climbLadderMove;					//!< The move used to go up or down the ladder after the ladder was grabbed.
	IMove* m_releaseLadderMove;					//!< The move used to release the ladder.
	IMove* m_goToExitLadderMove;				//!< The move used to go out of the ladder after it was released.

protected:
	bool m_useDefaultTraverseBehavior;			//!< True if no custom traverse behavior was specified. Then a default traverse behavior is created in constructor, and deleted in destructor.

	bool m_useDefaultGoToLadderMove;			//!< True if no GoToLadderMove was specified. Then a default move is created in constructor, and deleted in destructor.
	bool m_useDefaultGrabLadderMove;			//!< True if no GoToLadderMove was specified. Then a default move is created in constructor, and deleted in destructor.
	bool m_useDefaultClimbLadderMove;			//!< True if no ClimbLadderMove was specified. Then a default move is created in constructor, and deleted in destructor.
	bool m_useDefaultReleaseLadderMove;			//!< True if no ReleaseLadderMove was specified. Then a default move is created in constructor, and deleted in destructor.
	bool m_useDefaultGoToExitLadderMove;		//!< True if no GoToExitLadderMove was specified. Then a default move is created in constructor, and deleted in destructor.
};

}

#endif // #ifndef KYRUNTIME_LADDER_PATHOBJECT_BEHAVIOR_H
