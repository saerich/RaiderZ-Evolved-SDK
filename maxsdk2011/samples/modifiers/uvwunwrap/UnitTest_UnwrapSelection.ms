-- UnitTest_UnwrapSelection.ms

-- This is optional, but if you want your test to be executable by hitting ctrl-e in the
-- editor window, it helps.
fileIn "MxsUnit/MxsUnit.ms"

-- MxsUnit unit test fixtures are written as structs.  
-- The runner uses the return value from calling fileIn on the script file as the
-- test fixture.  For your test to work correctly from the command-line, it MUST
-- return its fixture.  The easiest ways to ensure that the fixture is returned
-- are to have your script file contain only the fixture struct or to end it with a
-- run_test_fixture call as in this example.
struct UnwrapSelectionUnitTest
(
   
   -- An optional set up function to set up the scene or local data for testing.
   -- setUp is called before every test case function.
   function setUp =
   (

   ),
   
   -- An optional tearDown function to clear any resources.
   -- tearDown is called after every test case function.
   function tearDown =
   (
		resetMaxFile #noprompt
	   
   ),
   
   -- A test case function
   function testPolySelection =
   (
		resetMaxFile #noprompt
		Sphere radius:42.7912 smooth:on segs:32 chop:0 slice:off sliceFrom:0 sliceTo:0 mapcoords:on recenter:off pos:[-5.75972,-1.12526,0] isSelected:on
			
		convertTo $ PolyMeshObject	   
		modPanel.addModToSelection (Unwrap_UVW ()) ui:on
		
		emptyValue = #{}
		emptyArray = emptyValue as array
		val = #{}
--empty tests			
		$.modifiers[#unwrap_uvw].unwrap.edit ()
		subobjectLevel = 1
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedVerticesByNode $
		t1 = val as array
		assert_array_equal  t1 emptyArray
			
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedVerticesByNode $
		t1 = val as array
		assert_array_equal  t1 emptyArray			

		$.modifiers[#unwrap_uvw].unwrap6.UVRing 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedVerticesByNode $
		t1 = val as array
		assert_array_equal  t1 emptyArray			

		subobjectLevel = 2
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedEdgesByNode $
		t1 = val as array
		assert_array_equal  t1 emptyArray
			
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedEdgesByNode $
		t1 = val as array
		assert_array_equal  t1 emptyArray			

		$.modifiers[#unwrap_uvw].unwrap6.UVRing 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedEdgesByNode $
		t1 = val as array
		assert_array_equal  t1 emptyArray
		
		subobjectLevel = 3
			$.modifiers[#unwrap_uvw].unwrap6.UVRing 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedFacesByNode $
		t1 = val as array
		assert_array_equal  t1 emptyArray
			
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedFacesByNode $
		t1 = val as array
		assert_array_equal  t1 emptyArray			

		$.modifiers[#unwrap_uvw].unwrap6.UVRing 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedFacesByNode $
		t1 = val as array
		assert_array_equal  t1 emptyArray
		
--full ring		
		subobjectLevel = 1
		$.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{309..310} $
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedVerticesByNode $
		t1 = val as array
		correctVal = #{45..46, 78..79, 111..112, 144..145, 177..178, 210..211, 243..244, 276..277, 309..310, 342..343, 375..376, 408..409, 441..442, 474..475, 507..508}
		assert_array_equal  t1 correctVal
			
		subobjectLevel = 2
		$.modifiers[#unwrap_uvw].unwrap6.selectEdgesByNode #{609} $
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedEdgesByNode $
		t1 = val as array
		correctVal = #{89, 154, 219, 284, 349, 414, 479, 544, 609, 674, 739, 804, 869, 934, 1010}
		assert_array_equal  t1 correctVal
		
		subobjectLevel = 3
		$.modifiers[#unwrap_uvw].unwrap6.selectFacesByNode #{271, 303} $
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedFacesByNode $
		t1 = val as array
		correctVal = #{257..320}
		assert_array_equal  t1 correctVal		
		
		
	--full loop		
		subobjectLevel = 1
		$.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{309..310} $
		$.modifiers[#unwrap_uvw].unwrap6.UVLoop 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedVerticesByNode $
		t1 = val as array
		correctVal = #{297..329}
		assert_array_equal  t1 correctVal
		
		subobjectLevel = 2
		$.modifiers[#unwrap_uvw].unwrap6.selectEdgesByNode #{609} $
		$.modifiers[#unwrap_uvw].unwrap6.UVLoop 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedEdgesByNode $
		t1 = val as array
		correctVal = #{585, 587, 589, 591, 593, 595, 597, 599, 601, 603, 605, 607, 609, 611, 613, 615, 617, 619, 621, 623, 625, 627, 629, 631, 633, 635, 637, 639, 641, 643, 645, 647}
		assert_array_equal  t1 correctVal
		
		subobjectLevel = 3
		$.modifiers[#unwrap_uvw].unwrap6.selectFacesByNode #{271, 303} $
		$.modifiers[#unwrap_uvw].unwrap6.UVLoop 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedFacesByNode $
		t1 = val as array
		correctVal = #{47, 79, 111, 143, 175, 207, 239, 271, 303, 335, 367, 399, 431, 463}
		assert_array_equal  t1 correctVal		
		
		
		--inc ring		
		subobjectLevel = 1
		$.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{309..310} $
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedVerticesByNode $
		t1 = val as array
		correctVal = #{276..277, 309..310, 342..343}
		assert_array_equal  t1 correctVal
			
		subobjectLevel = 2
		$.modifiers[#unwrap_uvw].unwrap6.selectEdgesByNode #{609} $
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedEdgesByNode $
		t1 = val as array
		correctVal = #{544, 609, 674}
		assert_array_equal  t1 correctVal
		
		subobjectLevel = 3
		$.modifiers[#unwrap_uvw].unwrap6.selectFacesByNode #{271, 303} $
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedFacesByNode $
		t1 = val as array
		correctVal = #{270..272, 302..304}
		assert_array_equal  t1 correctVal		
		
	--inc loop		
		subobjectLevel = 1
		$.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{309..310} $
		$.modifiers[#unwrap_uvw].unwrap6.UVLoop 1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedVerticesByNode $
		t1 = val as array
		correctVal = #{308..311}
		assert_array_equal  t1 correctVal
			
		subobjectLevel = 2
		$.modifiers[#unwrap_uvw].unwrap6.selectEdgesByNode #{609} $
		$.modifiers[#unwrap_uvw].unwrap6.UVLoop 1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedEdgesByNode $
		t1 = val as array
		correctVal = #{607, 609, 611}
		assert_array_equal  t1 correctVal
		
		subobjectLevel = 3
		$.modifiers[#unwrap_uvw].unwrap6.selectFacesByNode #{271, 303} $
		$.modifiers[#unwrap_uvw].unwrap6.UVLoop 1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedFacesByNode $
		t1 = val as array
		correctVal = #{239, 271, 303, 335}
		assert_array_equal  t1 correctVal	

	--dec loop		
		subobjectLevel = 1
		$.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{307..310} $
		$.modifiers[#unwrap_uvw].unwrap6.UVLoop -1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedVerticesByNode $
		t1 = val as array
		correctVal = #{308..309}
		assert_array_equal  t1 correctVal
			
		subobjectLevel = 2
		$.modifiers[#unwrap_uvw].unwrap6.selectEdgesByNode #{607, 609, 611} $
		$.modifiers[#unwrap_uvw].unwrap6.UVLoop -1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedEdgesByNode $
		t1 = val as array
		correctVal = #{ 609}
		assert_array_equal  t1 correctVal
		
		subobjectLevel = 3
		$.modifiers[#unwrap_uvw].unwrap6.selectFacesByNode #{239, 271, 303, 335} $
		$.modifiers[#unwrap_uvw].unwrap6.UVLoop -1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedFacesByNode $
		t1 = val as array
		correctVal = #{271, 303}
		assert_array_equal  t1 correctVal				
      
   ),
   function testMeshSelection =
   (
		resetMaxFile #noprompt

Sphere radius:42.7912 smooth:on segs:32 chop:0 slice:off sliceFrom:0 sliceTo:0 mapcoords:on recenter:off pos:[-5.75972,-1.12526,0] isSelected:on
		convertTo $ TriMeshGeometry   
		modPanel.addModToSelection (Unwrap_UVW ()) ui:on
		
		emptyValue = #{}
		emptyArray = emptyValue as array
		val = #{}
--empty tests			
		$.modifiers[#unwrap_uvw].unwrap.edit ()
		subobjectLevel = 1
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedVerticesByNode $
		t1 = val as array
		assert_array_equal  t1 emptyArray
			
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedVerticesByNode $
		t1 = val as array
		assert_array_equal  t1 emptyArray			

		$.modifiers[#unwrap_uvw].unwrap6.UVRing 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedVerticesByNode $
		t1 = val as array
		assert_array_equal  t1 emptyArray			

		subobjectLevel = 2
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedEdgesByNode $
		t1 = val as array
		assert_array_equal  t1 emptyArray
			
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedEdgesByNode $
		t1 = val as array
		assert_array_equal  t1 emptyArray			

		$.modifiers[#unwrap_uvw].unwrap6.UVRing 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedEdgesByNode $
		t1 = val as array
		assert_array_equal  t1 emptyArray
		
		subobjectLevel = 3
			$.modifiers[#unwrap_uvw].unwrap6.UVRing 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedFacesByNode $
		t1 = val as array
		assert_array_equal  t1 emptyArray
			
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedFacesByNode $
		t1 = val as array
		assert_array_equal  t1 emptyArray			

		$.modifiers[#unwrap_uvw].unwrap6.UVRing 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedFacesByNode $
		t1 = val as array
		assert_array_equal  t1 emptyArray
		
--full ring		
		subobjectLevel = 1
		$.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{309..310} $
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedVerticesByNode $
		t1 = val as array
		correctVal = #{45..46, 78..79, 111..112, 144..145, 177..178, 210..211, 243..244, 276..277, 309..310, 342..343, 375..376, 408..409, 441..442, 474..475, 507..508}
		assert_array_equal  t1 correctVal
			
		subobjectLevel = 2
		$.modifiers[#unwrap_uvw].unwrap6.selectEdgesByNode #{971} $
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedEdgesByNode $
		t1 = val as array
		correctVal = #{98, 195, 292, 389, 486, 583, 680, 777, 874, 971, 1068, 1165, 1262, 1359, 1455}
		assert_array_equal  t1 correctVal
		
		subobjectLevel = 3
		$.modifiers[#unwrap_uvw].unwrap6.selectFacesByNode #{573..574, 637..638} $
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedFacesByNode $
		t1 = val as array
		correctVal = #{545..672}
		assert_array_equal  t1 correctVal		
		

	--full loop		
		subobjectLevel = 1
		$.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{309..310} $
		$.modifiers[#unwrap_uvw].unwrap6.UVLoop 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedVerticesByNode $
		t1 = val as array
		correctVal = #{298..330}
		assert_array_equal  t1 correctVal

		subobjectLevel = 2
		$.modifiers[#unwrap_uvw].unwrap6.selectEdgesByNode #{971} $
		$.modifiers[#unwrap_uvw].unwrap6.UVLoop 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedEdgesByNode $
		t1 = val as array
		correctVal = #{938, 941, 944, 947, 950, 953, 956, 959, 962, 965, 968, 971, 974, 977, 980, 983, 986, 989, 992, 995, 998, 1001, 1004, 1007, 1010, 1013, 1016, 1019, 1022, 1025, 1028, 1031}
		assert_array_equal  t1 correctVal
		
		subobjectLevel = 3
		$.modifiers[#unwrap_uvw].unwrap6.selectFacesByNode #{573..574, 637..638} $
		$.modifiers[#unwrap_uvw].unwrap6.UVLoop 0
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedFacesByNode $
		t1 = val as array
		correctVal = #{61..62, 125..126, 189..190, 253..254, 317..318, 381..382, 445..446, 509..510, 573..574, 637..638, 701..702, 765..766, 829..830, 893..894}
		assert_array_equal  t1 correctVal		
		

		--inc ring		
		subobjectLevel = 1
		$.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{309..310} $
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedVerticesByNode $
		t1 = val as array
		correctVal = #{276..277, 309..310, 342..343}
		assert_array_equal  t1 correctVal
			
		subobjectLevel = 2
		$.modifiers[#unwrap_uvw].unwrap6.selectEdgesByNode #{971} $
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedEdgesByNode $
		t1 = val as array
		correctVal = #{874, 971, 1068}
		assert_array_equal  t1 correctVal
		
		subobjectLevel = 3
		$.modifiers[#unwrap_uvw].unwrap6.selectFacesByNode #{573..574, 637..638} $
		$.modifiers[#unwrap_uvw].unwrap6.UVRing 1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedFacesByNode $
		t1 = val as array
		correctVal = #{571..576, 635..640}
		assert_array_equal  t1 correctVal		

--inc loop		
		subobjectLevel = 1
		$.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{309..310} $
		$.modifiers[#unwrap_uvw].unwrap6.UVLoop 1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedVerticesByNode $
		t1 = val as array
		correctVal = #{308..311}
		assert_array_equal  t1 correctVal
			
		subobjectLevel = 2
		$.modifiers[#unwrap_uvw].unwrap6.selectEdgesByNode #{971} $
		$.modifiers[#unwrap_uvw].unwrap6.UVLoop 1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedEdgesByNode $
		t1 = val as array
		correctVal = #{968, 971, 974}
		assert_array_equal  t1 correctVal
		
		subobjectLevel = 3
		$.modifiers[#unwrap_uvw].unwrap6.selectFacesByNode #{573..574, 637..638} $
		$.modifiers[#unwrap_uvw].unwrap6.UVLoop 1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedFacesByNode $
		t1 = val as array
		correctVal = #{509..510, 573..574, 637..638, 701..702}
		assert_array_equal  t1 correctVal	

	--dec loop		
		subobjectLevel = 1
		$.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode #{307..310} $
		$.modifiers[#unwrap_uvw].unwrap6.UVLoop -1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedVerticesByNode $
		t1 = val as array
		correctVal = #{308..309}
		assert_array_equal  t1 correctVal
			
		subobjectLevel = 2
		$.modifiers[#unwrap_uvw].unwrap6.selectEdgesByNode #{968, 971, 974}$
		$.modifiers[#unwrap_uvw].unwrap6.UVLoop -1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedEdgesByNode $
		t1 = val as array
		correctVal = #{ 971}
		assert_array_equal  t1 correctVal
		
		subobjectLevel = 3
		$.modifiers[#unwrap_uvw].unwrap6.selectFacesByNode #{509..510, 573..574, 637..638, 701..702} $ 
		$.modifiers[#unwrap_uvw].unwrap6.UVLoop -1
		val = $.modifiers[#unwrap_uvw].unwrap6.getSelectedFacesByNode $
		t1 = val as array
		correctVal = #{573..574, 637..638}
		assert_array_equal  t1 correctVal	

	   
   ),	   
   
   -- The tests array.  This publishes the test case functions to the
   -- MxsUnitRunner.  It must be named "tests"
   tests = #( testPolySelection , testMeshSelection)
)

-- Optional, but makes the test immediately executable from the editor window.
-- run_test_fixture is implemented to do nothing if executing this test from the
-- command line runner or from the run_test_script utility method, thus preventing
-- the test from being executed twice.
-- run_test_fixture returns the fixture, so the test will work correctly from
-- the command line runner.
run_test_fixture UnwrapSelectionUnitTest


