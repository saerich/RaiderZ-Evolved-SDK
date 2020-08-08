-- UnitTest_UnwrapAlignSpace.ms
-- need to test empty case
--need to test 
-- This is optional, but if you want your test to be executable by hitting ctrl-e in the
-- editor window, it helps.
fileIn "MxsUnit/MxsUnit.ms"

-- MxsUnit unit test fixtures are written as structs.  
-- The runner uses the return value from calling fileIn on the script file as the
-- test fixture.  For your test to work correctly from the command-line, it MUST
-- return its fixture.  The easiest ways to ensure that the fixture is returned
-- are to have your script file contain only the fixture struct or to end it with a
-- run_test_fixture call as in this example.
struct UnwrapAlignSpaceUnitTest
(
   
   -- An optional set up function to set up the scene or local data for testing.
   -- setUp is called before every test case function.
   function setUp =
   (

   ),
   
   function CreatePlane convertToPoly=
   (
	   resetMaxFile #noprompt
	   	Plane length:206.222 width:235.556 transform:(matrix3 [1,0,0] [0,0,1] [0,-1,0] [5.33334,1.47647e-006,-33.7778]) isSelected:on
		if (convertToPoly) then convertTo $ PolyMeshObject	 
		else convertTo $ TriMeshGeometry
		modPanel.addModToSelection (Unwrap_UVW ()) ui:on
		$.modifiers[#unwrap_uvw].unwrap2.setFreeFormMode on
		$.modifiers[#Unwrap_UVW].texMapList.count = 0
		$.modifiers[#Unwrap_UVW].texMapIDList.count = 0
		$.modifiers[#Unwrap_UVW].baseMaterial_list.count = 1
		$.modifiers[#Unwrap_UVW].baseMaterial_list[1] = undefined
		$.modifiers[#Unwrap_UVW].texMapList.count = 0
		$.modifiers[#Unwrap_UVW].texMapIDList.count = 0
		$.modifiers[#Unwrap_UVW].texMapList.count = 1
		$.modifiers[#Unwrap_UVW].texMapIDList.count = 1
		$.modifiers[#Unwrap_UVW].texMapList[1] = Checker ()
		$.modifiers[#Unwrap_UVW].texMapIDList[1] = -1
		subobjectLevel = 1
		$.modifiers[#unwrap_uvw].unwrap.edit ()
		$Plane01.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{31} $
		$.modifiers[#unwrap_uvw].unwrap2.MoveSelected [-0.101375,0.085155,0]
		$Plane01.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{26} $
		$.modifiers[#unwrap_uvw].unwrap2.MoveSelected [0.0811,0.004055,0]
		$Plane01.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{16} $
		$.modifiers[#unwrap_uvw].unwrap2.MoveSelected [-0.12165,0.004055,0]
		$Plane01.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{31} $
		actionMan.executeAction 0 "40015"  -- Edit: Undo Scene Operation
		$.modifiers[#unwrap_uvw].unwrap2.MoveSelected [-0.239245,0,0]
		$Plane01.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{11} $
		$.modifiers[#unwrap_uvw].unwrap2.MoveSelected [0.12165,0,0]
		$Plane01.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{16} $
		$.modifiers[#unwrap_uvw].unwrap2.MoveSelected [-0.17031,0,0]
		$Plane01.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{21} $
		$.modifiers[#unwrap_uvw].unwrap2.MoveSelected [0.101375,0,0]
		$Plane01.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{31} $
		$.modifiers[#unwrap_uvw].unwrap2.MoveSelected [0.07299,0.13787,0]
		$Plane01.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{11} $
		$.modifiers[#unwrap_uvw].unwrap2.MoveSelected [-0.12976,-0.133815,0]
		$Plane01.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{33} $
		$.modifiers[#unwrap_uvw].unwrap2.MoveSelected [-0.004055,0.214915,0]
		$Plane01.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{23} $
		$.modifiers[#unwrap_uvw].unwrap2.MoveSelected [-0.08921,0.141925,0]
		$Plane01.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{18} $
		$.modifiers[#unwrap_uvw].unwrap2.MoveSelected [0.109485,0.03244,0]
		$Plane01.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{13} $
		$.modifiers[#unwrap_uvw].unwrap2.MoveSelected [0.10543,-0.239245,0]
		$Plane01.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{32} $
		$.modifiers[#unwrap_uvw].unwrap2.MoveSelected [0,-0.04866,0]
		$Plane01.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{12} $
		$.modifiers[#unwrap_uvw].unwrap2.MoveSelected [0.150035,0.012165,0]
		$Plane01.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{14} $
		$.modifiers[#unwrap_uvw].unwrap2.MoveSelected [0.125705,-0.158145,0]
		$Plane01.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{15} $
		$.modifiers[#unwrap_uvw].unwrap2.MoveSelected [0.133815,0,0]
		$Plane01.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{34} $
		$.modifiers[#unwrap_uvw].unwrap2.MoveSelected [0.077045,0.182475,0]
   ),
   
   -- An optional tearDown function to clear any resources.
   -- tearDown is called after every test case function.
   function tearDown =
   (
--		resetMaxFile #noprompt
	   
   ),
   
   function TestPoint i p2 = 
   (
		p = $.modifiers[#unwrap_uvw].unwrap6.GetVertexPositionByNode 0 i $
        p = p - p2
	    l = Length p
		assert_float  l 0.0 	   
   ),
   -- A test case function
   function alignPolySelection =
   (
		resetMaxFile #noprompt
		CreatePlane true	
		$.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{1, 6, 11, 16, 21} $
	    
	   
		$.modifiers[#unwrap_uvw].unwrap6.align false

	   
	   TestPoint 1 [-0.039739,0,0]
	   TestPoint 6 [-0.039739,0.25,0]
	   TestPoint 11 [-0.039739,0.366185,0]
	   TestPoint 21 [-0.039739,1,0]
	   
	   
		$.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{21..25} $
	   
		$.modifiers[#unwrap_uvw].unwrap6.align true

		TestPoint 21 [-0.039739,1.02838,0]
		TestPoint 22 [0.25,1.02838,0]
		TestPoint 23 [0.41079,1.02838,0]
		TestPoint 24 [0.75,1.02838,0]
		TestPoint 25 [1,1.02838,0]

		
      
   ),
   function alignMeshSelection =
   (
		resetMaxFile #noprompt
	    CreatePlane false

	   $.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{11, 16, 21, 26, 31} $

	    
	   
		$.modifiers[#unwrap_uvw].unwrap6.align false

		TestPoint 11 [-0.077045,-0.133815,0]
		TestPoint 16 [-0.077045,0.254055,0]
		TestPoint 21 [-0.077045,0.5,0]
		TestPoint 26 [-0.077045,0.754055,0]
		TestPoint 31 [-0.077045,1.22302,0]
	   
	   
	   
		$.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{31..35} $   
		$.modifiers[#unwrap_uvw].unwrap6.align true
	   
		TestPoint 31 [-0.077045,1.11435,0]
		TestPoint 32 [0.25,1.11435,0]
		TestPoint 33 [0.495945,1.11435,0]
		TestPoint 34 [0.827045,1.11435,0]
		TestPoint 35 [1,1.11435,0]	   
	
   ),	   
   
   function spacePolySelection =
   (
		resetMaxFile #noprompt
		CreatePlane true	
		$.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{1, 6, 11, 16, 21} $
	    
	   
		$.modifiers[#unwrap_uvw].unwrap6.space false

		TestPoint 1 [0,0,0]
		TestPoint 6 [0,0.25,0]
		TestPoint 11 [-0.00810999,0.5,0]
		TestPoint 16 [-0.29196,0.75,0]
		TestPoint 21 [0.101375,1,0]
	   
	   
		$.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{21..25} $
	   
		$.modifiers[#unwrap_uvw].unwrap6.space true
	   
		TestPoint 21 [0.101375,1,0]
		TestPoint 22 [0.326031,1,0]
		TestPoint 23 [0.550688,1.14192,0]
		TestPoint 24 [0.775344,1,0]
		TestPoint 25 [1,1,0]	   
	
   ),	     
   
   function spaceMeshSelection =
   (
		resetMaxFile #noprompt
	    CreatePlane false

	   $.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{11, 16, 21, 26, 31} $

	    
	   
		$.modifiers[#unwrap_uvw].unwrap6.space false

		TestPoint 11 [-0.00810999,-0.133815,0]
		TestPoint 16 [-0.29196,0.205395,0]
		TestPoint 21 [0.101375,0.544605,0]
		TestPoint 26 [0.0811,0.883815,0]
		TestPoint 31 [-0.26763,1.22303,0]
	   
	   
	   
		$.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{31..35} $   
		$.modifiers[#unwrap_uvw].unwrap6.space true
	   
		TestPoint 31 [-0.26763,1.22303,0]
		TestPoint 32 [0.0492775,0.95134,0]
		TestPoint 33 [0.366185,1.21492,0]
		TestPoint 34 [0.683092,1.18247,0]
		TestPoint 35 [1,1,0]	   
	
   ),	   
   
   -- The tests array.  This publishes the test case functions to the
   -- MxsUnitRunner.  It must be named "tests"

	tests = #(alignPolySelection, alignMeshSelection , spacePolySelection , spaceMeshSelection)
)

-- Optional, but makes the test immediately executable from the editor window.
-- run_test_fixture is implemented to do nothing if executing this test from the
-- command line runner or from the run_test_script utility method, thus preventing
-- the test from being executed twice.
-- run_test_fixture returns the fixture, so the test will work correctly from
-- the command line runner.
run_test_fixture UnwrapAlignSpaceUnitTest


