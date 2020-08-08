fileIn "MxsUnit/MxsUnit.ms"

struct LinkConstraintTransformTestFixture
(
	_xrefSceneName = "unittestlinkcsttransxrefscene.max",
	_BoxName = "MyBoxNode",
	_testNotifyDeleteNullSceneName = "unittestlinkcsttransTestNotifyDeleteNull.max",
	_TargetName = "MyTargetNode",
	
	fn CreateXRefScene =
	(
		-- create xref scene
		sphere()
		saveMaxFile _xrefSceneName quiet:true
	),
		
	fn CreateSceneTestNotifyDeleteNull =
	(
		local b1 = Box()
		b1.name = _BoxName
		local b2 = Box()
		b2.name = _TargetName
		
		-- Put link constraint on b1.
		b1.transform.controller = Link_Constraint()
		b1.transform.controller.addTarget b2 0
		b1.transform.controller.addworld frameNo:10
		b1.transform.controller.addTarget b2 20
		b1.transform.controller.addworld frameNo:30
		b1.transform.controller.addTarget b2 40

		saveMaxFile _testNotifyDeleteNullSceneName quiet:true
	),
		
	fn setUp =
	(
		CreateXRefScene()
		resetMaxFile #noprompt 
		
		CreateSceneTestNotifyDeleteNull()
	),
	
	fn tearDown =
	(
		resetMaxFile #noprompt 
	),

	fn testSetupScene = 
	(
		resetMaxFile #noprompt
		
		loadMaxFile _testNotifyDeleteNullSceneName quiet:true 
		
		local b1 = getNodeByName _BoxName exact:true
		assert_defined b1 message:"Loading test file: node missing"
		
		local numKeys = b1.transform.controller.getNumTargets()

		-- this is a pre condition
		assert_equal 5 numKeys message:"Loading test file: numKeys on Link_Constraint initial value" 

		local b2 = getNodeByName _TargetName exact:true
		assert_defined b2 message:"Loading test file: node missing"
	),
	
	fn testNotifyDeleteNull = 
	(
		resetMaxFile #noprompt
		
		loadMaxFile _testNotifyDeleteNullSceneName quiet:true 
		
		local b1 = getNodeByName _BoxName exact:true
		local numKeys = b1.transform.controller.getNumTargets()
		
		-- add XRef scene
		xrefScene = xrefs.addNewXRefFile _xrefSceneName
		numKeys = b1.transform.controller.getNumTargets()
		assert_equal 5 numKeys message:"Adding an XRefScene should not change numKeys on Link_Constraint initial value" 

		-- remove XRef Scene
		-- This test is meant to test Remove XRefScene. Note that when done through the UI, this does broadcast NOTIFY_SCENE_PRE_DELETED_NODE for node NULL
		-- but when done with Maxscript there is no notification
		delete xrefScene
		numKeys = b1.transform.controller.getNumTargets()
		assert_equal 5 numKeys message:"Removing an XRefScene should not change numKeys on Link_Constraint initial value" 
		
		-- Test responding to notification NOTIFY_SCENE_PRE_DELETED_NODE for a NULL node (maxscript can only broadcast with NULL)
		-- This is defect 1087341
		callbacks.broadcastCallback #nodePreDelete
		numKeys = b1.transform.controller.getNumTargets()
		assert_equal 5 numKeys message:"Broadcasting NOTIFY_SCENE_PRE_DELETED_NODE for a NULL node should not change numKeys on Link_Constraint initial value" 
	),
	
	fn testNotifyDeleteNode = 
	(
		resetMaxFile #noprompt
		
		loadMaxFile _testNotifyDeleteNullSceneName quiet:true 
		
		local b1 = getNodeByName _BoxName exact:true
		local b2 = getNodeByName _TargetName exact:true
		
		clearUndoBuffer()
		with undo on
		(
			delete b2
		)		
		
		assert_true (IsDeleted b2)
		
		local numKeys = b1.transform.controller.getNumTargets()
		assert_equal 2 numKeys message:"Delete target node should change numKeys on Link_Constraint initial value" 
		
		max undo

		numKeys = b1.transform.controller.getNumTargets()
		assert_equal 5 numKeys message:"Undoing delete target node should restore numKeys on Link_Constraint initial value" 
		
	),
	
	fn testMoveTargetNode = 
	(
		resetMaxFile #noprompt
		
		loadMaxFile _testNotifyDeleteNullSceneName quiet:true 
		
		local b1 = getNodeByName _BoxName exact:true
		local b2 = getNodeByName _TargetName exact:true
		
		sliderTime = 0f
		local xVal = b1.transform.pos.x
		
		b2.position.x += 10
		local expectedDelta = b1.transform.pos.x - (xVal + 10)
		
		assert_true ( abs(expectedDelta) < 0.001 ) message:"Moving target node should move linked node" 
		
	),
	
	tests = 
	#(
		testSetupScene,
		testNotifyDeleteNull,
		testNotifyDeleteNode,
		testMoveTargetNode
	)
		
)

run_test_fixture LinkConstraintTransformTestFixture script:(getThisScriptFilename())
