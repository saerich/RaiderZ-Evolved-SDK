--##################################################
-- Modifiers UI simplification Unit Test
-- Author:	Etienne Clement
-- Created:	Nov 21, 2002
--##################################################
-- The following macros are testing the changes
-- made to the modifier UI in Kahn. For more
-- information view spec #1159b
--##################################################
fileIn "MxsUnit/MxsUnit.ms"

struct Obj_Prop_UnitTest
(
private
	--##################################################
	-- Expected values
	--##################################################

	-- Smooth modidifer
	function getSmoothProperties =
	(
		#(false,	\ -- Auto smooth
		 false,	\ -- Prevent indirect
		"30.0",	\ -- Threshold
		0) 		-- Smoothing bits
	),
	testprops = #(
		"autosmooth",
		"preventindirect",
		"threshold",
		"smoothingbits"
	),

	--##################################################
	-- Helper functions
	--##################################################

	function fetchSmoothProperties mod properties =
	(
		append properties mod.autoSmooth
		append properties mod.preventIndirect
		append properties (mod.threshold as string)
		append properties mod.smoothingBits
	),

	function validateProperties experimental expected =
	(
		if (experimental.count != expected.count) then
		(
			assert_equal expected.count experimental.count message:"Incorrect number of properties"
		)
		else
		(
			for i in 1 to experimental.count do
			(
				assert_equal expected[i] experimental[i] message:("Incorrect Value for " + testprops[i])
			)
		)
	),

	function changeValue valueName pvalue properties =
	(
		for i in 1 to properties.count do
		(
			if testprops[i] == valueName do
			(
				properties[i] = pvalue
				return true
			)
		)
		local str = stringstream ""
		format "Value(%): %, not found in property array" valueName pvalue to:str
		assert_true false message:(str as string)
	),

	--##################################################
	-- Unit tests
	--##################################################
public
	function setup =
	(
		resetMaxFile #noPrompt
	),

	function teardown =
	(
		
	),

	function createSmooth =
	(
		box1 = box name:#box1
		smooth1 = smoothModifier()
		addModifier box1 smooth1

		index = modPanel.getModifierIndex box1 box1.smooth
		smooth1 = box1.modifiers[index]

		expected = getSmoothProperties()
		experimental = #()
		fetchSmoothProperties smooth1 experimental
		validateProperties experimental expected
		forcecompleteredraw()
	),

	function loadSmooth =
	(
		local box1 = box name:#box1
		local smooth1 = smoothModifier()
		addModifier box1 smooth1

		local box2 = box name:#box2 pos:[20, 20, 20]
		local smooth2 = smoothModifier()
		smooth2.autoSmooth 		= true
		smooth2.preventIndirect 	= true
		smooth2.threshold 		= 15.0
		smooth2.smoothingBits 	= 2
		addModifier box2 smooth2

		saveMaxFile "toload.max"

		resetMaxFile #noPrompt

		loadMaxFile "toload.max"

		box1 = getNodeByName "box1" exact:true
		index = modPanel.getModifierIndex box1 box1.smooth
		smooth1 = box1.modifiers[index]

		local expected = getSmoothProperties()
		experimental = #()
		fetchSmoothProperties smooth1 experimental
		validateProperties experimental expected

		box2 = getNodeByName "box2" exact:true
		index = modPanel.getModifierIndex box2 box2.smooth
		smooth2 = box2.modifiers[index]

		expected = getSmoothProperties()
		changevalue "autosmooth" 	true expected
		changevalue "preventindirect" true expected
		changevalue "threshold" 		(15.0 as string) expected
		changevalue "smoothingbits" 	2 	expected

		experimental = #()
		fetchSmoothProperties smooth2 experimental
		validateProperties experimental expected
		forcecompleteredraw()
	),

	function mergeSmooth =
	(
		box1 = box name:#box1
		smooth1 = smoothModifier()
		addModifier box1 smooth1

		box2 = box name:#box2 pos:[20, 20, 20]
		smooth2 = smoothModifier()
		smooth2.autoSmooth = true
		smooth2.preventIndirect = true
		smooth2.threshold = 15.0
		smooth2.smoothingBits = 2
		addModifier box2 smooth2

		saveMaxFile "tomerge.max"

		resetMaxFile #noPrompt

		loadMaxFile "tomerge.max"

		box1 = getNodeByName "box1" exact:true
		index = modPanel.getModifierIndex box1 box1.smooth
		smooth1 = box1.modifiers[index]

		expected = getSmoothProperties()
		experimental = #()
		fetchSmoothProperties smooth1 experimental
		validateProperties experimental expected

		box2 = getNodeByName "box2" exact:true
		index = modPanel.getModifierIndex box2 box2.smooth
		smooth2 = box2.modifiers[index]

		expected = getSmoothProperties()
		changevalue "autosmooth" true expected
		changevalue "preventindirect" true expected
		changevalue "threshold" (15.0 as string) expected
		changevalue "smoothingbits" 2 expected

		experimental = #()
		fetchSmoothProperties smooth2 experimental
		validateProperties experimental expected
		forcecompleteredraw()
	),

	Tests =
	#(
		createSmooth,
		loadSmooth,
		mergeSmooth
	)
)
/* Useful for debugging. Do not remove.
foo = Obj_Prop_UnitTest()
foo.setup()
foo.createSmooth()
resetmaxfile(#noprompt)
foo.loadSmooth()
resetmaxfile(#noprompt)
foo.mergeSmooth()
*/
run_test_fixture Obj_Prop_UnitTest script:(getThisScriptFilename())