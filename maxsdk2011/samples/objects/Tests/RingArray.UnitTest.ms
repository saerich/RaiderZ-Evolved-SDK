--------------------------------------------------------------------------------
-- Ring Array System unit test
-- Author:	Attila Szabo
-- Created:	Marc.20.2006
--------------------------------------------------------------------------------
fileIn "MxsUnit/MxsUnit.ms"

struct RingArraySystem_UnitTest
(
private
	mUndoLevels = 0,
	mQuietMode = true,
	mRingArrayFile = "",
	mXrefRingArrayFile = "",
public
	function setup =
	(
		resetmaxFile #noPrompt
		mUndoLevels = theHold.getMaxUndoLevels()
		mQuietMode = SetQuietMode true
		mRingArrayFile = (getdir #Scene) + "\\Temp.RingArraySystem.max"
		mXrefRingArrayFile = (getdir #Scene) + "\\Temp.RingArraySystem.Xref.max"

		deleteFile mRingArrayFile
		deleteFile mXrefRingArrayFile
	),

	function teardown =
	(
		theHold.SetMaxUndoLevels mUndoLevels
		SetQuietMode mQuietMode
	),

	-----------------------------------------------------------------------
	-- Unit tests --
	-----------------------------------------------------------------------
	function testCreate =
	(
		local objCountPreCreate = objects.count
		undo "RA_Create" on
		(
			ra = RingArray.create()
			assert_true (0 < ra.numNodes) message:"Create"
		)
		local objCountPostCreate = objects.count

		max undo
		assert_equal objCountPreCreate objects.count message: "Create undone"
		max redo
		assert_equal objCountPostCreate objects.count message: "Create redone"
		max undo
		assert_equal objCountPreCreate objects.count message: "Create undone"
	),

	function testPersistence =
	(
		local objCountPreCreate = objects.count
		local ra = RingArray.create()
		local objCountPostCreate = objects.count
		
		max hold
		resetmaxFile #noPrompt
		assert_equal objCountPreCreate objects.count message: "Scene reset after hold"
		
		max fetch
		assert_equal objCountPostCreate objects.count message: "Scene fetched"
	),

	function testAddNodes =
	(
		undo off
		(
			local ra = RingArray.create()
		)
		
		local objCountPreAdd = objects.count
		undo "RA_AddNodes" on
		(
			ra.numNodes = 10
			assert_equal 10 ra.numNodes message:"numNode == 10"
		)
		local objCountPostAdd = objects.count
		
		max undo
		assert_equal objCountPreAdd  objects.count  message:"numNode increase undone"
		max redo
		assert_equal objCountPostAdd objects.count  message:"numNode increase redone"
		max undo
		assert_equal objCountPreAdd  objects.count  message:"numNode increase undone"
	),

	function testRemoveNodes =
	(
		undo off
		(
			local ra = RingArray.create numNodes:8
		)

		local objCountPreRemove = objects.count
		undo "RA_RemoveNodes" on
		(
			ra.numNodes = 1
			assert_equal 1 ra.numNodes message:"numNode == 1"
		)
		local objCountPostRemove = objects.count

		max undo
		assert_equal objCountPreRemove  objects.count message:"numNode decrease undone"
		max redo                        
		assert_equal objCountPostRemove objects.count message:"numNode decrease redone"
		max undo                        
		assert_equal objCountPreRemove  objects.count message:"numNode decrease undone"
	),

	function testDeleteSys =
	(
		-- note that deleting ring array system nodes does not
		-- notify the ring array master of the deletion of the nodes
		-- and thus it still points to the deleted nodes

		-- Indirect references will fix this and ensure that the
		-- ring array master does not try to dereference deleted nodes

		undo off
		(
			local ra = RingArray.create()
		)

		local objCountPreSysDelete = objects.count
		undo "RA_DeleteSys" on
		(
			delete (ra.GetNode 1)
			assert_equal 1 objects.count message:"System was deleted"
		)
		local objCountPostSysDelete = objects.count

		max undo
		assert_equal objCountPreSysDelete  objects.count   message:"system deletion undone"
		max redo                          
		assert_equal objCountPostSysDelete objects.count   message:"system deletion redone"
		max undo                          
		assert_equal objCountPreSysDelete  objects.count   message:"System deletion undone"
	),

	function testCloneSys =
	(
		undo off
		(
			local ra = RingArray.create()
		)

		local objCountPreSysClone = objects.count
		undo "RA_CloneSys" on
		(
			local sysNodes = #()
			ra.GetNodes &sysNodes #Ctx_Clone

			local oneNode = ra.GetNode 1
			local actualNodes = #()
			local newNodes = #()
			res = maxOps.cloneNodes &oneNode cloneType:#copy actualNodeList:&actualNodes newNodes:&newNodes
			assert_true res message:  "Clone"
			assert_equal newNodes.count sysNodes.count message:"Cloned node count"
		)
		local objCountPostSysClone = objects.count

		max undo
		assert_equal objCountPreSysClone  objects.count message:"system clone undone"
		max redo
		assert_equal objCountPostSysClone objects.count message:"system clone redone"
		max undo
		assert_equal objCountPreSysClone  objects.count message:"System clone undone"
		max redo
		assert_equal objCountPostSysClone objects.count message:"system clone redone"

		max hold
		resetmaxFile #noPrompt
		assert_equal 0 objects.count message: "Hold and Reset Scene"

		max fetch
		assert_equal objCountPostSysClone objects.count message:"Scene fetched"
	),

	function saveRingArray fileName =
	(
		deleteFile fileName
		assert_equal 0 ((getFiles fileName).count) message:"Temp file does not exist"

		local ra = RingArray.create()

		local res = saveMaxFile fileName  quiet:true
		assert_true res message:"Save Temp File"

		local objCount = objects.count

		resetmaxFile #noPrompt

		return objCount
	),

	function testMergeSys =
	(
		local objCountPreSysMerge = saveRingArray(mRingArrayFile)

		local res = mergeMaxFile mRingArrayFile quiet:true
		assert_true res message: "Merge Temp File"

		local objCountPostSysMerge = objects.count
		assert_equal objCountPostSysMerge objCountPreSysMerge message:"System merged"

		-- Test that the merged objects still form a system
		select $Box*
		assert_true (selection.count >= 1) 
		local mc = getMasterController selection[1]
		local ra = getInterface mc "IRingArrayMaster"
		assert_equal ra.numNodes (objCountPreSysMerge-1) message:"System merged"

		testCloneSys()
	),

	function testXrefSys =
	(
		local objCountPreSysXref = saveRingArray(mRingArrayFile)

		local xoRec = objXrefMgr.AddXrefItemsFromFile mRingArrayFile -- xref objects\materials\ctrls
		assert_equal 1 objXrefMgr.recordCount message:"Xref objects  - check record count"
		assert_equal objCountPreSysXref (xoRec.itemCount #XRefObjectType) message:"Xref objects - check xref item count in record"

		local objCountPostSysXref = objects.count
		assert_equal objCountPostSysXref objCountPreSysXref message:"System xrefed (object Count Test)"

		select $Box*
		assert_true (selection.count >= 2) 
		local mc1 = getMasterController selection[1]
		local mc2 = getMasterController selection[2]
		local ra = getInterface mc1 "IRingArrayMaster"
		assert_equal ra.numNodes (objCountPreSysXref - 1) message:"System Xrefed (numNodes test)"

		assert_true (mc1 == mc2) message:"System Xrefed (master controller identity test)"
	),

	function testNestedXrefSys =
	(
		-- create and save ring array, this resets the scene
		local objCountPreSysXref = saveRingArray(mRingArrayFile)

		-- xref saved ring array and save the file
		local xoRec = objXrefMgr.AddXrefItemsFromFile mRingArrayFile -- xref objects\materials\ctrls
		assert_equal 1 objXrefMgr.recordCount message:"Xref objects - check record count"
		assert_equal objCountPreSysXref (xoRec.itemCount #XRefObjectType) message:"Xref objects - check xref item count in record"
		local res = saveMaxFile mXrefRingArrayFile quiet:true
		assert_true res message: "Save Xrefed objects"
		resetmaxFile #noPrompt

		-- xref the scene containing the xrefed RingArray
		local xoRec = objXrefMgr.AddXrefItemsFromFile mXrefRingArrayFile -- xref objects\materials\ctrls
		assert_equal 2 objXrefMgr.recordCount message:"Xref Xrefed Objects - check record count"
		assert_equal objCountPreSysXref (xoRec.itemCount #XRefObjectType) message:"Xref Xrefed Objects - check xref item count in record"

		select $Box*
		assert_true (selection.count >= 2) 
		local mc1 = getMasterController selection[1]
		local mc2 = getMasterController selection[2]
		local ra = getInterface mc1 "IRingArrayMaster"
		assert_equal ra.numNodes (objCountPreSysXref - 1) message:"System Xrefed (numNodes test)"

		assert_true (mc1 == mc2) message: "System Xrefed (master controller identity test)"
	),

	Tests =
	#(
		testCreate,
		testPersistence,
		testAddNodes,
		testRemoveNodes,
		testDeleteSys,
		testCloneSys,
		testMergeSys,
		testXrefSys,
		testNestedXrefSys
	)
)

run_test_fixture RingArraySystem_UnitTest script:(getThisScriptFilename())