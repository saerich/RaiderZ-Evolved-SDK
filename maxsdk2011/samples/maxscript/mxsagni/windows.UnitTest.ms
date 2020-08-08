fileIn "MxsUnit/MxsUnit.ms"

struct Windows_Test
(
	WM_LBUTTONDOWN = 0x0201,  -- the windows message send on a left mouse button down
	WM_LBUTTONUP = 0x0202,  -- the windows message send on a left mouse button up
	-- the function simulates a left mouse click on an hwnd via postmessage
	fn simulateLMouseClick_post hwnd =
	(
		windows.postmessage hwnd WM_LBUTTONDOWN 0 0
		windows.postmessage hwnd WM_LBUTTONUP 0 0
	),
	-- the function simulates a left mouse click on an hwnd via sendmessage
	fn simulateLMouseClick_send hwnd =
	(
		windows.sendmessage hwnd WM_LBUTTONDOWN 0 0
		windows.sendmessage hwnd WM_LBUTTONUP 0 0
	),
	fn test_methods =
	(
		rollout test "testXXXX"
		(
			button b "b"
			spinner s "s" type:#integer
		)
		createdialog test

		max_hwnd = windows.getMAXHWND()
		ro_hwnd = test.hwnd
		button_hwnd = test.b.hwnd[1]
		button_hwnd_data = windows.getHWNDData button_hwnd
		assert_true (button_hwnd == button_hwnd_data[1])
		assert_true (ro_hwnd == button_hwnd_data[2])
		assert_true (ro_hwnd == button_hwnd_data[3])
		assert_true ("CustButton" == button_hwnd_data[4])
		assert_true (test.b.text == button_hwnd_data[5])
		assert_true (0 == button_hwnd_data[6])
		assert_true (ro_hwnd == button_hwnd_data[7])
		assert_true (max_hwnd == button_hwnd_data[8])

		res = windows.getChildHWND test.hwnd "b"
		assert_true (button_hwnd == res[1])

		res = windows.getChildHWND test.hwnd "z"
		assert_true (undefined == res)

		res = windows.getChildrenHWND ro_hwnd
		assert_true (res.count == 6)

		res = windows.getChildHWND 0 test.title parent:#max
		assert_true (ro_hwnd == res[1])
		res = windows.getChildHWND 0 test.title parent:(windows.getMAXHWND())
		assert_true (ro_hwnd == res[1])
		res = windows.getChildHWND (windows.getDesktopHWND()) test.title parent:#max
		assert_true (ro_hwnd == res[1])
			
		res = windows.getChildrenHWND #max -- not sure what to test for here.....

		res = windows.getChildrenHWND 0
		found = false
		for w in res while (not found) do found = (w[1] == ro_hwnd)
		assert_true (found)
		
		spinner_hwnd_data = for d in (windows.getChildrenHWND test.hwnd) where d[4] == "SpinnerControl" collect d
		spinner_hwnd =spinner_hwnd_data[1][1]
		assert_true (spinner_hwnd == test.s.hwnd[1])
		assert_true (test.s.value == 0)
		simulateLMouseClick_send spinner_hwnd
		assert_true (test.s.value == 1)
		simulateLMouseClick_post spinner_hwnd
		assert_true (test.s.value == 1)
		windows.processPostedMessages()
		assert_true (test.s.value == 2)
		destroydialog test
	),

	Tests = 
	#(
		test_methods
	)
)

run_test_fixture Windows_Test script:(getThisScriptFilename())
