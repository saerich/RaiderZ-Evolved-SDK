/********************************************************************** *<
FILE: ToolPelt.cpp

DESCRIPTION: Pelting Methods

HISTORY: 9/25/2006
CREATED BY: Peter Watje




*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/


#include "unwrap.h"
#include "3dsmaxport.h"

#include <process.h>

/*
workflow 
user selects some faces
user hits Pelt Map
user defines the plane
user defines/refines the seams
user brings up pelt dialog

add the display
	add the outer ring list
	add the outer ring edges

	draw the spring
	draw the tension

add the sim run button

create a seam list
add the seam point to point mode
add the seam display
add the hit seam mode

fix the selection of the faces in the dialog
add the UI
add some seam edge tools
	by selection
	by point to point

*/

PeltEdgeMode* PeltData::peltEdgeMode   = NULL;


unsigned __stdcall  CallPeltThread(void *data)
{

	PeltThreadData *threadData = (PeltThreadData *) data;

	threadData->mStarted = TRUE;

	DisableAccelerators();   
	GetCOREInterface()->DisableSceneRedraw();

	//just fire off the pelt run
	
	theHold.Begin();
	threadData->mod->HoldPoints();
	
	theHold.Put (new UnwrapPeltVertVelocityRestore (threadData->mod));
	theHold.Accept(_T(GetString(IDS_PELTDIALOG_RUN)));


	int holdFrames = threadData->mod->peltData.GetFrames();
	threadData->mod->peltData.SetFrames(1000000);
	threadData->mod->peltData.Run(threadData->mod, FALSE);
	threadData->mod->peltData.SetFrames(holdFrames);
	




	for (int ldID = 0; ldID < threadData->mod->GetMeshTopoDataCount(); ldID++)
	{
		threadData->mod->GetMeshTopoData(ldID)->SetUserCancel(FALSE);
	}

	EnableAccelerators();   
	GetCOREInterface()->EnableSceneRedraw();
	threadData->mStarted = FALSE;

	if (threadData->mod->peltData.peltDialog.iRunPeltButton)
	{
		if (threadData->mod->peltData.peltDialog.iRunPeltButton->IsChecked())
		{
			threadData->mod->peltData.peltDialog.iRunPeltButton->SetText(_T(GetString(IDS_SPLINEMAP_STARTPELT)));
			threadData->mod->peltData.peltDialog.iRunPeltButton->SetCheck(FALSE);
		}
	}


	_endthreadex(0);
	return 0;

}

void UnwrapMod::PeltThreadOp(int operation, HWND hWnd)
{


	//get frames and sample?

	mPeltThreadData.mod = this;
	if (operation == KThreadStart)
	{		
		if (peltData.peltDialog.iRunPeltButton)
		{
			peltData.peltDialog.iRunPeltButton->SetCheck(TRUE);
			peltData.peltDialog.iRunPeltButton->SetText(_T(GetString(IDS_SPLINEMAP_ENDPELT)));
		}

		unsigned int threadID;
		mPeltThreadHandle = (HANDLE)_beginthreadex( NULL, 0, &CallPeltThread, (void *) &mPeltThreadData, 0, &threadID );

	}
	else if (operation == KThreadReStart)
	{
		//only restart if we have a current thread running
		if (mPeltThreadHandle && mPeltThreadData.mStarted)
		{

			if (peltData.peltDialog.iRunPeltButton)
			{
				peltData.peltDialog.iRunPeltButton->SetCheck(FALSE);
			}

			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				mMeshTopoData[ldID]->SetUserCancel(TRUE);
			}

			WaitForSingleObject( mPeltThreadHandle, INFINITE );

			CloseHandle( mPeltThreadHandle );

			mPeltThreadHandle = NULL;

			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				mMeshTopoData[ldID]->SetUserCancel(FALSE);
			}	

			if (peltData.peltDialog.iRunPeltButton)
			{
				peltData.peltDialog.iRunPeltButton->SetCheck(TRUE);
			}

			unsigned int threadID;
			mPeltThreadHandle = (HANDLE)_beginthreadex( NULL, 0, &CallPeltThread, (void *) &mPeltThreadData, 0, &threadID );
		}
	}
	else if (operation == KThreadEnd)
	{

		if (peltData.peltDialog.iRunPeltButton)
		{
			peltData.peltDialog.iRunPeltButton->SetCheck(FALSE);
			peltData.peltDialog.iRunPeltButton->SetText(_T(GetString(IDS_SPLINEMAP_STARTPELT)));
		}

		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			mMeshTopoData[ldID]->SetUserCancel(TRUE);
		}

		WaitForSingleObject( mPeltThreadHandle, INFINITE );

		CloseHandle( mPeltThreadHandle );

		mPeltThreadHandle = NULL;

		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			mMeshTopoData[ldID]->SetUserCancel(FALSE);
		}





	}
}


unsigned __stdcall  CallPeltRelaxThread(void *data)
{

	RelaxThreadData *threadData = (RelaxThreadData *) data;

	threadData->mStarted = TRUE;

	DisableAccelerators();   
	GetCOREInterface()->DisableSceneRedraw();

	BOOL rigSelected = FALSE;
	MeshTopoData *peltLD = threadData->mod->peltData.mBaseMeshTopoDataCurrent;
	if (threadData->mod->peltData.peltDialog.hWnd)
	{		
		if (peltLD)
		{
			for (int i = 0; i < threadData->mod->peltData.rigPoints.Count(); i++)
			{
				int index = threadData->mod->peltData.rigPoints[i].lookupIndex;
				if (peltLD->GetTVVertSelected(index))
					rigSelected = TRUE;
			}
		}
	}

	//if we are not instanced we can just start up unwrap relax and go
	if ((threadData->mod->GetMeshTopoDataCount() == 1) && !rigSelected)
	{
		if (threadData->mType == 0)
			threadData->mod->fnRelaxByFaceAngle(threadData->mIterations,threadData->mStretch,threadData->mAmount,threadData->mBoundary,NULL);
		else if (threadData->mType == 1)
			threadData->mod->fnRelaxByEdgeAngle(threadData->mIterations,threadData->mStretch,threadData->mAmount,threadData->mBoundary,NULL);
		else if (threadData->mType == 2)
			threadData->mod->RelaxVerts2(threadData->mAmount,threadData->mIterations,threadData->mBoundary,threadData->mCorner);
	}
	//if we have multiple instance it gets more complicated
	//since only the first relax instance will ever run
	else
	{
		BOOL done = FALSE;
		threadData->mIterations = 5;
		for (int ldID = 0; ldID < threadData->mod->GetMeshTopoDataCount(); ldID++)
		{
			threadData->mod->GetMeshTopoData(ldID)->SetUserCancel(FALSE);
		}
		while(!done)
		{
			for (int ldID = 0; ldID < threadData->mod->GetMeshTopoDataCount(); ldID++)
			{
				if (threadData->mod->GetMeshTopoData(ldID)->GetUserCancel())
					done = TRUE;
			}
			if (threadData->mType == 0)
				threadData->mod->fnRelaxByFaceAngle(threadData->mIterations,threadData->mStretch,threadData->mAmount,threadData->mBoundary,NULL);
			else if (threadData->mType == 1)
				threadData->mod->fnRelaxByEdgeAngle(threadData->mIterations,threadData->mStretch,threadData->mAmount,threadData->mBoundary,NULL);
			else if (threadData->mType == 2)
				threadData->mod->RelaxVerts2(threadData->mAmount,threadData->mIterations,threadData->mBoundary,threadData->mCorner);
			if (rigSelected)
				threadData->mod->peltData.RelaxRig(threadData->mIterations,threadData->mAmount,threadData->mBoundary,threadData->mod);


		}
	}


	for (int ldID = 0; ldID < threadData->mod->GetMeshTopoDataCount(); ldID++)
	{
		threadData->mod->GetMeshTopoData(ldID)->SetUserCancel(FALSE);
	}


	EnableAccelerators();   
	GetCOREInterface()->EnableSceneRedraw();

	if (threadData->mod->peltData.peltDialog.iRunPeltRelaxButton)
	{
		if (threadData->mod->peltData.peltDialog.iRunPeltRelaxButton->IsChecked())
		{
			threadData->mod->peltData.peltDialog.iRunPeltRelaxButton->SetText(_T(GetString(IDS_SPLINEMAP_STARTRELAX)));
			threadData->mod->peltData.peltDialog.iRunPeltRelaxButton->SetCheck(FALSE);
		}
	}


	threadData->mStarted = FALSE;
	_endthreadex(0);
	return 0;
}

void UnwrapMod::PeltRelaxThreadOp(int operation, HWND hWnd)
{

	mRelaxThreadData.mod = this;
	
	mRelaxThreadData.mType = relaxType;
	mRelaxThreadData.mAmount = relaxAmount;
	mRelaxThreadData.mBoundary = relaxBoundary;
	mRelaxThreadData.mStretch = relaxStretch;
	mRelaxThreadData.mCorner = relaxSaddle;

	mRelaxThreadData.mIterations = 1000000;



	if (operation == KThreadStart)
	{		
		unsigned int threadID;

		if (peltData.peltDialog.iRunPeltRelaxButton)
		{
			peltData.peltDialog.iRunPeltRelaxButton->SetCheck(TRUE);
			peltData.peltDialog.iRunPeltRelaxButton->SetText(_T(GetString(IDS_SPLINEMAP_ENDRELAX)));
		}

		mRelaxThreadHandle = (HANDLE)_beginthreadex( NULL, 0, &CallPeltRelaxThread, (void *) &mRelaxThreadData, 0, &threadID );

		

	}
	else if (operation == KThreadReStart)
	{
		//only restart if we have a current thread running
		if (mRelaxThreadHandle && mRelaxThreadData.mStarted)
		{

			if (peltData.peltDialog.iRunPeltRelaxButton)
			{
				peltData.peltDialog.iRunPeltRelaxButton->SetCheck(FALSE);
			}

			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				mMeshTopoData[ldID]->SetUserCancel(TRUE);
			}

			WaitForSingleObject( mRelaxThreadHandle, INFINITE );

			CloseHandle( mRelaxThreadHandle );

			mRelaxThreadHandle = NULL;


			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				mMeshTopoData[ldID]->SetUserCancel(FALSE);
			}		

			if (peltData.peltDialog.iRunPeltRelaxButton)
			{
				peltData.peltDialog.iRunPeltRelaxButton->SetCheck(TRUE);
			}


			unsigned int threadID;
			mRelaxThreadHandle = (HANDLE)_beginthreadex( NULL, 0, &CallPeltRelaxThread, (void *) &mRelaxThreadData, 0, &threadID );
		}
	}
	else if (operation == KThreadEnd && mRelaxThreadHandle && mRelaxThreadData.mStarted)
	{

		if (peltData.peltDialog.iRunPeltRelaxButton)
		{
			peltData.peltDialog.iRunPeltRelaxButton->SetCheck(FALSE);
			peltData.peltDialog.iRunPeltRelaxButton->SetText(_T(GetString(IDS_SPLINEMAP_STARTRELAX)));
		}

		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			mMeshTopoData[ldID]->SetUserCancel(TRUE);
		}

		WaitForSingleObject( mRelaxThreadHandle, INFINITE );

		CloseHandle( mRelaxThreadHandle );

		mRelaxThreadHandle = NULL;

		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			mMeshTopoData[ldID]->SetUserCancel(FALSE);
		}


	}


}



INT_PTR CALLBACK UnwrapMapRollupWndProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
 	UnwrapMod *mod = DLGetWindowLongPtr<UnwrapMod*>(hWnd);
	
	static BOOL inEnter = FALSE;

	switch (msg) {
		case WM_INITDIALOG:
			{
			mod = (UnwrapMod*)lParam;
			DLSetWindowLongPtr(hWnd, lParam);

			mod->hMapParams = hWnd;

			ICustButton *iPeltButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_PELT2));
			iPeltButton->SetType(CBT_CHECK);
			iPeltButton->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(iPeltButton);
			

			ICustButton *iPlanarButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_PLANAR));
			iPlanarButton->SetType(CBT_CHECK);
			iPlanarButton->SetHighlightColor(GREEN_WASH);			
			ReleaseICustButton(iPlanarButton);


			ICustButton *iCylindricalButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_CYLINDRICAL));
			iCylindricalButton->SetType(CBT_CHECK);
			iCylindricalButton->SetHighlightColor(GREEN_WASH);			
			ReleaseICustButton(iCylindricalButton);

 			ICustButton *iSphericalButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_SPHERICAL));
			iSphericalButton->SetType(CBT_CHECK);
			iSphericalButton->SetHighlightColor(GREEN_WASH);			
			ReleaseICustButton(iSphericalButton);

			ICustButton *iBoxButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_BOX));
			iBoxButton->SetType(CBT_CHECK);
			iBoxButton->SetHighlightColor(GREEN_WASH);			
			ReleaseICustButton(iBoxButton);

			ICustButton *iSplineButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_SPLINE));
			iSplineButton->SetType(CBT_CHECK);
			iSplineButton->SetHighlightColor(GREEN_WASH);			
			ReleaseICustButton(iSplineButton);

			CheckDlgButton(hWnd,IDC_NORMALIZEMAP_CHECK2,mod->fnGetNormalizeMap());

			
			ICustButton *iEditSeamsButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_EDITSEAMS));
			iEditSeamsButton->SetType(CBT_CHECK);
			iEditSeamsButton->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(iEditSeamsButton);


			ICustButton *iEditSeamsByPointButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_SEAMPOINTTOPOINT));
			iEditSeamsByPointButton->SetType(CBT_CHECK);
			iEditSeamsByPointButton->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(iEditSeamsByPointButton);



			ICustButton *iExpandSeamsButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_EXPANDTOSEAMS2));
			iExpandSeamsButton->SetType(CBT_PUSH);
			ReleaseICustButton(iExpandSeamsButton);


			mod->peltData.ShowUI(hWnd, FALSE);
			mod->EnableMapButtons(FALSE);
			mod->EnableAlignButtons(FALSE);

			mod->peltData.EnablePeltButtons(mod->hMapParams, FALSE);
			mod->peltData.SetPointToPointSeamsMode(mod,FALSE);

			
			int		align = mod->GetQMapAlign();
			CheckDlgButton(hWnd,IDC_RADIO1,FALSE);
			CheckDlgButton(hWnd,IDC_RADIO2,FALSE);
			CheckDlgButton(hWnd,IDC_RADIO3,FALSE);
			CheckDlgButton(hWnd,IDC_RADIO4,FALSE);
			if (align == 0)
				CheckDlgButton(hWnd,IDC_RADIO1,TRUE);
			else if (align == 1)
				CheckDlgButton(hWnd,IDC_RADIO2,TRUE);
			else if (align == 2)
				CheckDlgButton(hWnd,IDC_RADIO3,TRUE);
			else if (align == 3)
				CheckDlgButton(hWnd,IDC_RADIO4,TRUE);

			CheckDlgButton(hWnd,IDC_PREVIEW_CHECK,mod->GetQMapPreview());

			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_PREVIEW_CHECK:

					{
					macroRecorder->Disable();
					theHold.Begin();
					{						
						mod->SetQMapPreview(IsDlgButtonChecked(hWnd,IDC_PREVIEW_CHECK));
					}
					theHold.Accept(_T(GetString(IDS_PREVIEW)));
					macroRecorder->Enable();

					TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.setQuickMapGizmoPreview"));
					macroRecorder->FunctionCall(mstr, 1, 0,
							mr_bool,mod->GetQMapPreview());
					if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
					break;
					}
				case IDC_RADIO1:
				case IDC_RADIO2:
				case IDC_RADIO3:
				case IDC_RADIO4:
					{
						int		align = 0;
						if  (IsDlgButtonChecked(hWnd,IDC_RADIO1))
							align = 0;
						else if  (IsDlgButtonChecked(hWnd,IDC_RADIO2))
							align = 1;
						else if  (IsDlgButtonChecked(hWnd,IDC_RADIO3))
							align = 2;
						else if  (IsDlgButtonChecked(hWnd,IDC_RADIO4))
							align = 3;

						CheckDlgButton(hWnd,IDC_RADIO1,FALSE);
						CheckDlgButton(hWnd,IDC_RADIO2,FALSE);
						CheckDlgButton(hWnd,IDC_RADIO3,FALSE);
						CheckDlgButton(hWnd,IDC_RADIO4,FALSE);
						if (align == 0)
							CheckDlgButton(hWnd,IDC_RADIO1,TRUE);
						else if (align == 1)
							CheckDlgButton(hWnd,IDC_RADIO2,TRUE);
						else if (align == 2)
							CheckDlgButton(hWnd,IDC_RADIO3,TRUE);
						else if (align == 3)
							CheckDlgButton(hWnd,IDC_RADIO4,TRUE);

						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.setProjectionType"));
						macroRecorder->FunctionCall(mstr, 1, 0,mr_int,align+1);

						macroRecorder->Disable();
						theHold.Begin();
						{
							
							mod->SetQMapAlign(align);
						}
						theHold.Accept(_T(GetString(IDS_QMAPALIGN)));						
						macroRecorder->Enable();
						if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
						break;
					}
				case IDC_QMAP:
					{
						theHold.Begin();
						mod->ApplyQMap();
						theHold.Accept(_T(GetString(IDS_PW_PLANARMAP)));
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.quickPlanarMap"));
						macroRecorder->FunctionCall(mstr, 0, 0);
						
						break;
					}
				case IDC_UNWRAP_PELT2:
					{
						mod->WtExecute(ID_PELT_MAP);
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.mappingMode"));
						macroRecorder->FunctionCall(mstr, 1, 0,	mr_int,mod->fnGetMapMode());						
						break;
					}
				case IDC_UNWRAP_PLANAR:
					{
						mod->WtExecute(ID_PLANAR_MAP);
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.mappingMode"));
						macroRecorder->FunctionCall(mstr, 1, 0,	mr_int,mod->fnGetMapMode());						
						break;
					}

				case IDC_UNWRAP_CYLINDRICAL:
					{
						mod->WtExecute(ID_CYLINDRICAL_MAP);
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.mappingMode"));
						macroRecorder->FunctionCall(mstr, 1, 0,	mr_int,mod->fnGetMapMode());						
						break;
					}

				case IDC_UNWRAP_SPHERICAL:
					{
						mod->WtExecute(ID_SPHERICAL_MAP);
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.mappingMode"));
						macroRecorder->FunctionCall(mstr, 1, 0,	mr_int,mod->fnGetMapMode());						
						break;
					}

				case IDC_UNWRAP_BOX:
					{
						mod->WtExecute(ID_BOX_MAP);
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.mappingMode"));
						macroRecorder->FunctionCall(mstr, 1, 0,	mr_int,mod->fnGetMapMode());						
						break;
					}
				case IDC_UNWRAP_SPLINE:
					{
						mod->WtExecute(ID_SPLINE_MAP);
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.mappingMode"));
						macroRecorder->FunctionCall(mstr, 1, 0,	mr_int,mod->fnGetMapMode());						
						break;

//						mod->fnSplineMap_StartMapMode();
//						mod->WtExecute(ID_BOX_MAP);
//						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.mappingMode"));
//						macroRecorder->FunctionCall(mstr, 1, 0,	mr_int,mod->fnGetMapMode());						
						break;

					}
				case IDC_UNWRAP_ALIGNX:
					{
 						mod->WtExecute(ID_MAPPING_ALIGNX);
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.mappingAlign"));
						macroRecorder->FunctionCall(mstr, 1, 0,	mr_int,0);						
					break;
					}
				case IDC_UNWRAP_ALIGNY:
					{
						mod->WtExecute(ID_MAPPING_ALIGNY);
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.mappingAlign"));
						macroRecorder->FunctionCall(mstr, 1, 0,	mr_int,1);						
					break;
					}
				case IDC_UNWRAP_ALIGNZ:
					{
						mod->WtExecute(ID_MAPPING_ALIGNZ);
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.mappingAlign"));
						macroRecorder->FunctionCall(mstr, 1, 0,	mr_int,2);						
						break;
					}
				case IDC_UNWRAP_ALIGNNORMAL:
					{
						mod->WtExecute(ID_MAPPING_NORMALALIGN);
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.mappingAlign"));
						macroRecorder->FunctionCall(mstr, 1, 0,	mr_int,3);						
						break;
					}

				case IDC_UNWRAP_FITMAP:
					{
						mod->WtExecute(ID_MAPPING_FIT);
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.mappingFit"));
						macroRecorder->FunctionCall(mstr, 0, 0);						
						break;
					}
				case IDC_UNWRAP_ALIGNTOVIEW:
					{
						mod->WtExecute(ID_MAPPING_ALIGNTOVIEW);
						Matrix3 macroTM = *mod->fnGetGizmoTM();
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.setGizmoTM"));
						macroRecorder->FunctionCall(mstr, 1, 0,	mr_matrix3,&macroTM);						
					break;
					}
				case IDC_UNWRAP_CENTER:
					{
						mod->WtExecute(ID_MAPPING_CENTER);
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.mappingCenter"));
						macroRecorder->FunctionCall(mstr, 0, 0);						
						break;
					}
				case IDC_UNWRAP_RESET:
					{
						mod->WtExecute(ID_MAPPING_RESET);
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.mappingReset"));
						macroRecorder->FunctionCall(mstr, 0, 0);						
						break;
					}

				case IDC_NORMALIZEMAP_CHECK2:
					{
						//set element mode swtich 
						theHold.Begin();
						{
							mod->fnSetNormalizeMap(IsDlgButtonChecked(hWnd,IDC_NORMALIZEMAP_CHECK2));
						}
						theHold.Accept(_T(GetString(IDS_NORMAL)));							
						//send macro message
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.setNormalizeMap"));
						macroRecorder->FunctionCall(mstr, 1, 0,
							mr_bool,mod->fnGetNormalizeMap());
						macroRecorder->EmitScript();

						break;

					}

				case IDC_UNWRAP_EXPANDTOSEAMS2:
					{
						mod->WtExecute(ID_PELT_EXPANDSELTOSEAM);
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.peltExpandSelectionToSeams"));
						macroRecorder->FunctionCall(mstr, 0, 0);						
						break;
					}

				case IDC_UNWRAP_EDGESELTOSEAMS:
					{
						mod->WtExecute(ID_PW_SELTOSEAM2);
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.peltEdgeSelToSeam"));
						macroRecorder->FunctionCall(mstr, 1, 0,	mr_bool,FALSE);												
						break;
					}

				case IDC_UNWRAP_EDITSEAMS:
					{
	 					mod->WtExecute(ID_PELT_EDITSEAMS);
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.setPeltEditSeamsMode"));
						macroRecorder->FunctionCall(mstr, 1, 0,	mr_bool,mod->fnGetPeltEditSeamsMode());						
						break;
					}


				case IDC_UNWRAP_SEAMPOINTTOPOINT:
					{			
						mod->WtExecute(ID_PELT_POINTTOPOINTSEAMS);
						TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.setPeltPointToPointSeamsMode"));
						macroRecorder->FunctionCall(mstr, 1, 0,	mr_bool,mod->fnGetPeltPointToPointSeamsMode());						
						break;
					}


				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}



	PeltData::~PeltData()
	{
		Free();
	}

	void PeltData::Free()
	{
		verts.ZeroCount();
		initialPointData.ZeroCount();
		springEdges.ZeroCount();
	}

	void PeltData::PeltMode(UnwrapMod *mod, BOOL start)
	{
		if (inPeltMode == start) return;

		ICustButton *iPeltButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_PELT2));
		inPeltMode = start;
			
		if ((!inPeltMode) && peltDialoghWnd)
		{			
			SendMessage(peltDialoghWnd,WM_CLOSE,0,0);
		}
		if (!inPeltMode)
		{
			inPeltPickEdgeMode = FALSE;
			SetPointToPointSeamsMode(mod,FALSE);

			ICustButton *iEditSeamsButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_EDITSEAMS));
			ICustButton *iEditSeamsByPointButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_SEAMPOINTTOPOINT));

			if (iEditSeamsButton)
			{
				iEditSeamsButton->SetCheck(inPeltPickEdgeMode);
				ReleaseICustButton(iEditSeamsButton);
			}
			if (iEditSeamsByPointButton)
			{
				iEditSeamsByPointButton->SetCheck(inPeltPointToPointEdgeMode);
				ReleaseICustButton(iEditSeamsByPointButton);
			}
			
		}



		if (iPeltButton)
		{
			iPeltButton->SetCheck(inPeltMode);
			ShowUI(hMapParams, inPeltMode);
			ReleaseICustButton(iPeltButton);
		}

		if (inPeltMode)
		{
			mod->WtExecute(ID_PELTDIALOG);
		}

	
	}

	void PeltData::StartPeltDialog(UnwrapMod *mod)
	{

		
		HWND hWnd = mod->hMapParams;
		ICustButton *iEditSeamsButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_EDITSEAMS));
		ICustButton *iEditSeamsByPointButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_SEAMPOINTTOPOINT));
		mod->peltData.inPeltPickEdgeMode = FALSE;

		SetPointToPointSeamsMode(mod,FALSE);
//		mod->peltData.inPeltPointToPointEdgeMode = FALSE;

		iEditSeamsButton->SetCheck(mod->peltData.inPeltPickEdgeMode);
		iEditSeamsByPointButton->SetCheck(mod->peltData.inPeltPointToPointEdgeMode);
		
		iEditSeamsButton->Enable(FALSE);
		iEditSeamsByPointButton->Enable(FALSE);
		ReleaseICustButton(iEditSeamsButton);
		ReleaseICustButton(iEditSeamsByPointButton);

		ICustButton *iGetSeamsFromSelButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_EDGESELTOSEAMS));
		iGetSeamsFromSelButton->Enable(FALSE);
		ReleaseICustButton(iGetSeamsFromSelButton);


		ICustButton *iExpandToSeamsButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_EXPANDTOSEAMS2));
		iExpandToSeamsButton->Enable(FALSE);
		ReleaseICustButton(iExpandToSeamsButton);


	}
	void PeltData::EndPeltDialog(UnwrapMod *mod)
	{



		MeshTopoData *ld = mBaseMeshTopoDataCurrent;
		for (int i = 0; i < ld->GetNumberTVVerts(); i++)//mod->vsel.GetSize(); i++)
		{
			if ((i < hiddenVerts.GetSize()) && (!hiddenVerts[i]))
			{
				ld->SetTVVertHidden(i,FALSE);//	mod->TVMaps.v[i].flags &= ~FLAG_HIDDEN;				
			}
		}

		for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
		{
			MeshTopoData *ld = mod->GetMeshTopoData(ldID);
			if (ld != mBaseMeshTopoDataCurrent)
			{
				for (int i = 0; i < ld->GetNumberTVVerts(); i++)
				{
					ld->SetTVVertHidden(i,FALSE);//	mod->TVMaps.v[i].flags |= FLAG_HIDDEN;				
				}
			}
		}



		mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
		if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());

		HWND hWnd = mod->hMapParams;
		ICustButton *iEditSeamsButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_EDITSEAMS));
		ICustButton *iEditSeamsByPointButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_SEAMPOINTTOPOINT));
		mod->peltData.inPeltPickEdgeMode = FALSE;

		SetPointToPointSeamsMode(mod,FALSE);
//		mod->peltData.inPeltPointToPointEdgeMode = FALSE;

		iEditSeamsButton->SetCheck(mod->peltData.inPeltPickEdgeMode);
		iEditSeamsByPointButton->SetCheck(mod->peltData.inPeltPointToPointEdgeMode);
		iEditSeamsButton->Enable(TRUE);
		iEditSeamsByPointButton->Enable(TRUE);
		ReleaseICustButton(iEditSeamsButton);
		ReleaseICustButton(iEditSeamsByPointButton);

		ICustButton *iGetSeamsFromSelButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_EDGESELTOSEAMS));
		iGetSeamsFromSelButton->Enable(TRUE);
		ReleaseICustButton(iGetSeamsFromSelButton);


		ICustButton *iExpandToSeamsButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_EXPANDTOSEAMS2));
		iExpandToSeamsButton->Enable(TRUE);
		ReleaseICustButton(iExpandToSeamsButton);
		mod->peltData.SetPeltDialogHandle(NULL);


	}


	void PeltData::EnablePeltButtons(HWND hWnd, BOOL enable)
	{

		ICustButton *iEditSeamsButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_EDITSEAMS));
		ICustButton *iEditSeamsByPointButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_SEAMPOINTTOPOINT));

		if (!enable)
		{
			inPeltPickEdgeMode = FALSE;	
			
	}

		
		iEditSeamsButton->Enable(enable);
		iEditSeamsByPointButton->Enable(enable);
		ReleaseICustButton(iEditSeamsButton);
		ReleaseICustButton(iEditSeamsByPointButton);

		ICustButton *iGetSeamsFromSelButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_EDGESELTOSEAMS));
		iGetSeamsFromSelButton->Enable(enable);
		ReleaseICustButton(iGetSeamsFromSelButton);


		ICustButton *iExpandToSeamsButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_EXPANDTOSEAMS2));
		iExpandToSeamsButton->Enable(enable);
		ReleaseICustButton(iExpandToSeamsButton);
	}

	void PeltData::ShowUI(HWND hWnd, BOOL show)
	{
		//get the edit button

		HWND hButton = GetDlgItem(hWnd, IDC_UNWRAP_EDITSEAMS);
		ShowWindow(hButton,TRUE);


		hButton = GetDlgItem(hWnd, IDC_UNWRAP_SEAMPOINTTOPOINT);
		ShowWindow(hButton,TRUE);

		hButton = GetDlgItem(hWnd, IDC_UNWRAP_EDGESELTOSEAMS);
		ShowWindow(hButton,TRUE);


		hButton = GetDlgItem(hWnd, IDC_UNWRAP_EXPANDTOSEAMS2);
		ShowWindow(hButton,TRUE);

		hButton = GetDlgItem(hParams, IDC_UNWRAP_RESET);
		EnableWindow(hButton,!show);

		hButton = GetDlgItem(hParams, IDC_UNWRAP_SAVE);
		EnableWindow(hButton,!show);

		hButton = GetDlgItem(hParams, IDC_UNWRAP_LOAD);
		EnableWindow(hButton,!show);

		hButton = GetDlgItem(hParams, IDC_MAP_CHAN1);
		EnableWindow(hButton,!show);

		hButton = GetDlgItem(hParams, IDC_MAP_CHAN2);
		EnableWindow(hButton,!show);

		hButton = GetDlgItem(hParams, IDC_MAP_CHAN);
		EnableWindow(hButton,!show);

		hButton = GetDlgItem(hParams, IDC_MAP_CHAN_SPIN);
		EnableWindow(hButton,!show);

	}

	void PeltData::NukeRig()
	{

		MeshTopoData *ld = mBaseMeshTopoDataCurrent;
		for (int i = 0; i < rigPoints.Count(); i++)
		{
			int vIndex = rigPoints[i].lookupIndex;
			if ((vIndex >= 0) && (vIndex < ld->GetNumberTVVerts()))//mod->TVMaps.v.Count()))
			{
				ld->SetTVVertDead(vIndex,TRUE);//mod->TVMaps.v[vIndex].flags |= FLAG_DEAD;
				int flag = ld->GetTVVertFlag(vIndex);
				flag = flag & ~FLAG_RIGPOINT;
				ld->SetTVVertFlag(vIndex,flag);
			}
		}

		verts.ZeroCount();
		springEdges.ZeroCount();
		rigPoints.ZeroCount();
	}

	void PeltData::SetRigStrength(float str)
	{
		rigStrength = str;
		for (int i = 0; i < rigPoints.Count(); i++)
		{
			int springIndex = rigPoints[i].springIndex;
			if ((springIndex >= 0) && (springIndex < springEdges.Count()))
			{
				springEdges[springIndex].str = rigStrength;
			}
		}
	}


	float PeltData::GetRigStrength()
	{
		return rigStrength;
	}

	void PeltData::SetSamples(int samp)
	{
		samples = samp;
	}
	int PeltData::GetSamples()
	{
		return samples;
	}
	void PeltData::SetFrames(int fr)
	{
		frames = fr;
	}
	int PeltData::GetFrames()
	{
		return frames;
	}

	void PeltData::SetStiffness(float stiff) { stiffness = stiff; }
	float PeltData::GetStiffness() { return stiffness; }

	void PeltData::SetDampening(float damp) { dampening = damp; }
	float PeltData::GetDampening() { return dampening; }

	void PeltData::SetDecay(float dec) { decay = dec; }
	float PeltData::GetDecay() { return decay; }

	void PeltData::SetMirrorAngle(float ang) { rigMirrorAngle = ang; }
	float PeltData::GetMirrorAngle() { return rigMirrorAngle; }
	
	Point3  PeltData::GetMirrorCenter() {return rigCenter;}

	BOOL PeltData::GetEditSeamsMode()
	{
		return inPeltPickEdgeMode;
	}
	void PeltData::SetEditSeamsMode(UnwrapMod *mod,BOOL mode)
	{
		if (inPeltPickEdgeMode == mode) return;

		inPeltPickEdgeMode = mode;

//		if (inPeltMode)
		{
			ICustButton *iEditSeamsButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_EDITSEAMS));
			ICustButton *iEditSeamsByPointButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_SEAMPOINTTOPOINT));
			if (!inPeltPickEdgeMode)
				inPeltPickEdgeMode = FALSE;
			else
			{
				inPeltPickEdgeMode = TRUE;
				SetPointToPointSeamsMode(mod,FALSE);
//				inPeltPointToPointEdgeMode = FALSE;

			}
			iEditSeamsButton->SetCheck(inPeltPickEdgeMode);
			iEditSeamsByPointButton->SetCheck(inPeltPointToPointEdgeMode);
			ReleaseICustButton(iEditSeamsButton);
			ReleaseICustButton(iEditSeamsByPointButton);
		}
	}

	BOOL PeltData::GetPointToPointSeamsMode()
	{
		return inPeltPointToPointEdgeMode;
	}
	void PeltData::SetPointToPointSeamsMode(UnwrapMod *mod,BOOL mode)
	{
//		if (mode == inPeltPointToPointEdgeMode) return;
		inPeltPointToPointEdgeMode = mode;
/*
		ICustButton *iEditSeamsButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_EDITSEAMS));
	 	ICustButton *iEditSeamsByPointButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_SEAMPOINTTOPOINT));
*/
		if (inPeltPointToPointEdgeMode)
		{
			SetEditSeamsMode(mod,FALSE);
		}
		currentPointHit = -1;
		previousPointHit = -1;
/*
		if (iEditSeamsButton)
		{
		iEditSeamsButton->SetCheck(GetEditSeamsMode());
			ReleaseICustButton(iEditSeamsButton);
		}
		if (iEditSeamsByPointButton)
		{
		iEditSeamsByPointButton->SetCheck(GetPointToPointSeamsMode());
		ReleaseICustButton(iEditSeamsByPointButton);
		}
*/
		if (inPeltPointToPointEdgeMode)
		{
			
			GetCOREInterface()->PushCommandMode(mod->peltPointToPointMode);
		}
		else
		{
			if (GetCOREInterface()->GetCommandMode() == mod->peltPointToPointMode)
			{
				if (GetCOREInterface()->GetCommandStackSize() == 1)
					GetCOREInterface()->SetStdCommandMode(CID_OBJMOVE);			
				else GetCOREInterface()->PopCommandMode();
			}
		}

	}


	BOOL PeltData::GetPeltMapMode()
	{
		return inPeltMode;
	}
	void PeltData::SetPeltMapMode(UnwrapMod *mod,BOOL mode)
	{
		if (mode == inPeltMode) return;
		PeltMode(mod,mode);
	}



	void PeltData::SnapRig(UnwrapMod *mod)
	{
		MeshTopoData *ld = mBaseMeshTopoDataCurrent;

		int vCount = ld->GetNumberTVVerts();// mod->TVMaps.v.Count()
		TimeValue t = GetCOREInterface()->GetTime();

		for (int i = 0; i < rigPoints.Count(); i++)
		{
			int selfIndex = rigPoints[i].lookupIndex;
			int targetIndex = rigPoints[i].index;

			if ((selfIndex >= 0) && (selfIndex < vCount) && 
				(targetIndex >= 0) && (targetIndex < vCount) )
			{
				Point3 targetP =  ld->GetTVVert(targetIndex);//mod->TVMaps.v[targetIndex].p;
				ld->SetTVVert(t,selfIndex,targetP,mod);
//				mod->TVMaps.v[selfIndex].p = targetP;
//				if (mod->TVMaps.cont[selfIndex]) mod->TVMaps.cont[selfIndex]->SetValue(0,&mod->TVMaps.v[selfIndex].p);		
			}
		}

		mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
		mod->InvalidateView();
	
	}

	void PeltData::ValidateSeams(/*w4UnwrapMod *mod,*/BitArray &newSeams)
	{
		//loop through our spring list

		newSeams = mBaseMeshTopoDataCurrent->mSeamEdges;
		
		for (int i = 0; i < newSeams.GetSize(); i++)
		{
			if (newSeams[i])
			{


				if (i < mBaseMeshTopoDataCurrent->GetNumberGeomEdges())//mod->TVMaps.gePtrList.Count())
				{
					int faceCount = mBaseMeshTopoDataCurrent->GetGeomEdgeNumberOfConnectedFaces(i);//mod->TVMaps.gePtrList[i]->faceList.Count();
					BOOL sel = FALSE;
					for (int j = 0; j< faceCount; j++)
					{
		//make sure each spring edge is part of selection
						int faceIndex = mBaseMeshTopoDataCurrent->GetGeomEdgeConnectedFace(i,j);//mod->TVMaps.gePtrList[i]->faceList[j];
						if ((faceIndex < peltFaces.GetSize()) && peltFaces[faceIndex])
							sel = TRUE;
	
					}
					//if not remove it
					if (!sel)
					{
						newSeams.Set(i,FALSE);
					}
					//check to see if this is a degenerate seams
//					if (mod->TVMaps.gePtrList[i]->a == mod->TVMaps.gePtrList[i]->b)
					if (mBaseMeshTopoDataCurrent->GetGeomEdgeVert(i,0) == mBaseMeshTopoDataCurrent->GetGeomEdgeVert(i,1))//(mod->TVMaps.gePtrList[i]->a == mod->TVMaps.gePtrList[i]->b)
						newSeams.Set(i,FALSE);
				}

			}
		}
	}


	void PeltData::CleanUpIsolatedClusters(UnwrapMod *mod, BOOL byTVs )
	{
		//now get our largest cluster and only use that cluster
		theHold.Suspend();
		BitArray holdFaces = mBaseMeshTopoDataCurrent->GetFaceSelection();  //original selection

		peltFaces = mBaseMeshTopoDataCurrent->GetFaceSelection(); //were we are going to store off our final selection
		peltFaces.ClearAll();

		BitArray touchingVerts; //list of all verts that touch a selection
		if (byTVs)
			touchingVerts.SetSize(mBaseMeshTopoDataCurrent->GetNumberTVVerts());
		else
			touchingVerts.SetSize(mBaseMeshTopoDataCurrent->GetNumberGeomVerts());

		Tab<int> checkVerts;

		//loop through our faces
		for (int i  = 0; i < mBaseMeshTopoDataCurrent->GetNumberFaces(); i++)
		{
			if (holdFaces[i]) //see if it is part of the original selection
			{
				int faceID = i;
				BOOL done = FALSE;
				BitArray subSel = holdFaces;  //our current cluster
				subSel.ClearAll();
				touchingVerts.ClearAll();
				//get a list of verts for this face and add to our touching list
				if (byTVs)
					mBaseMeshTopoDataCurrent->GetFaceTVPoints(faceID, touchingVerts);
				else
					mBaseMeshTopoDataCurrent->GetFaceGeomPoints(faceID, touchingVerts);

				//build the cluster for this faces
				while (!done)
				{
					int ct = subSel.NumberSet();
					//loop through our faces
					for (int j = 0; j < mBaseMeshTopoDataCurrent->GetNumberFaces();j++)
					{
						//make sure it was part of our original selection
						if (holdFaces[j])
						{
							if (byTVs)
								mBaseMeshTopoDataCurrent->GetFaceTVPoints(j, checkVerts);
							else
								mBaseMeshTopoDataCurrent->GetFaceGeomPoints(j, checkVerts);
							BOOL touching = FALSE;
							//see if this face touches any parts of the cluster
							for (int k = 0; k < checkVerts.Count(); k++)
							{
								int vIndex = checkVerts[k];
								if (touchingVerts[vIndex])
								{
									touching = TRUE;
									k =  checkVerts.Count();
								}
							}

							//if it does
							if (touching)
							{
								//add the face to our cluster
								subSel.Set(j,TRUE);
								//add that faces verts to our touching list
								for (int k = 0; k < checkVerts.Count(); k++)
								{
									int vIndex = checkVerts[k];
									touchingVerts.Set(vIndex,TRUE);
								}
								//remove this face since it has been processed
								holdFaces.Clear(j);
							}
						}
					}

					//if the cluster did not grow we are done for this cluster					
					if (ct == subSel.NumberSet())
						done = TRUE;
				}
				//if this cluster has more faces than the pelt cluster
				//make the pelt cluster equal to this one
				if (subSel.NumberSet() > peltFaces.NumberSet())
					peltFaces = subSel;
				//remove any processed faces
				holdFaces &= ~subSel;
			}
		}
		//set our selection back
		mBaseMeshTopoDataCurrent->SetFaceSelection(peltFaces);
		theHold.Resume();

	}

	void PeltData::SetupSprings(UnwrapMod *mod )
	{



 		theHold.Begin();
		mod->HoldPointsAndFaces(true);	
		theHold.Accept("Run");

		for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
		{
			MeshTopoData *ld = mod->GetMeshTopoData(ldID);
			for (int i = 0; i < ld->GetNumberTVVerts(); i++)
			{
				ld->SetTVVertHidden(i,TRUE);//	mod->TVMaps.v[i].flags |= FLAG_HIDDEN;				
			}
		}


		//find out which local data we are working on 
		int currentLD = 0;
		int faceCount = -1;
		mBaseMeshTopoDataCurrent = NULL;
		for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
		{
			MeshTopoData *ld = mod->GetMeshTopoData(ldID);
			if (ld->GetFaceSelection().NumberSet() > faceCount)
			{
				currentLD = ldID;
				mBaseMeshTopoDataCurrent = ld;
				faceCount = ld->GetFaceSelection().NumberSet();
			}
		}

		for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
		{
			MeshTopoData *ld = mod->GetMeshTopoData(ldID);
			if (ld != mBaseMeshTopoDataCurrent)
			{
				ld->ClearTVVertSelection();
				ld->ClearTVEdgeSelection();
				ld->ClearFaceSelection();
			}

		}


		CleanUpIsolatedClusters(mod,FALSE);


		BitArray tempSeams; 
		ValidateSeams(tempSeams);

		theHold.Suspend();
		//fix up x,y,z so they are square 

		Matrix3 tm(1);
		TimeValue t = GetCOREInterface()->GetTime();

		mod->tmControl->GetValue(t,&tm,FOREVER,CTRL_RELATIVE);

		if (Length(tm.GetRow(2)) == 0.0f)
		{
			tm.SetRow(2,Point3(0.0f,0.0f,1.0f));
		}


		float len = Length(tm.GetRow(0));

		tm.SetRow(1,Normalize(tm.GetRow(1))*len);
		tm.SetRow(2,Normalize(tm.GetRow(2))*len);




		Matrix3 ptm(1), id(1);
		SetXFormPacket tmpck(tm,ptm);
		mod->tmControl->SetValue(t,&tmpck,TRUE,CTRL_RELATIVE);

		mod->fnPlanarMap();
		theHold.Resume();
		
 		mBaseMeshTopoDataCurrent->SetTVEdgeInvalid();//tvData->edgesValid = FALSE;		
 		mBaseMeshTopoDataCurrent->BuildTVEdges();//RebuildEdges();

 		CutSeams(mod,tempSeams);
 		
		mBaseMeshTopoDataCurrent->SetTVEdgeInvalid();//tvData->edgesValid = FALSE;		
 		mBaseMeshTopoDataCurrent->BuildTVEdges();//RebuildEdges();
		mBaseMeshTopoDataCurrent->BuildVertexClusterList();


		CleanUpIsolatedClusters(mod,TRUE);
		
		mBaseMeshTopoDataCurrent->BuildSpringData(springEdges,verts, rigCenter,rigPoints,initialPointData,rigStrength);
		
		mBaseMeshTopoDataCurrent->SetTVEdgeInvalid();
		mBaseMeshTopoDataCurrent->BuildTVEdges();//RebuildTVEdges();
		mBaseMeshTopoDataCurrent->BuildVertexClusterList();

		BitArray vsel ;
		vsel.SetSize(mBaseMeshTopoDataCurrent->GetNumberTVVerts());//mod->TVMaps.v.Count());
		mBaseMeshTopoDataCurrent->SetTVVertSelection(vsel);
		
		mBaseMeshTopoDataCurrent->SetFaceSelection(peltFaces);//md->faceSel = peltFaces;
		mod->SyncTVToGeomSelection(mBaseMeshTopoDataCurrent);
		
		

		vsel.SetSize(mBaseMeshTopoDataCurrent->GetNumberTVVerts());//tvData->v.Count());
		vsel.ClearAll();
		mBaseMeshTopoDataCurrent->SetTVVertSelection(vsel);
		mod->RebuildDistCache();

		SelectRig(mod);
		SelectPelt(mod);

		vsel = mBaseMeshTopoDataCurrent->GetTVVertSelection();
		hiddenVerts = vsel;
		for (int i = 0; i < vsel.GetSize(); i++)
		{
			if (!hiddenVerts[i])
			{
				mBaseMeshTopoDataCurrent->SetTVVertHidden(i,TRUE);//	mod->TVMaps.v[i].flags |= FLAG_HIDDEN;				
			}
			else
				mBaseMeshTopoDataCurrent->SetTVVertHidden(i,FALSE);//	mod->TVMaps.v[i].flags |= FLAG_HIDDEN;				
		}
		

 		vsel.ClearAll();
		for (int i = 0; i < rigPoints.Count(); i++)
		{
			int index = rigPoints[i].lookupIndex;
			vsel.Set(index,TRUE);
		}
		mBaseMeshTopoDataCurrent->SetTVVertSelection(vsel);


		if (mBaseMeshTopoDataCurrent->mSeamEdges.GetSize() != mBaseMeshTopoDataCurrent->GetNumberGeomEdges())//mod->TVMaps.gePtrList.Count())
		{
			mBaseMeshTopoDataCurrent->mSeamEdges.SetSize(mBaseMeshTopoDataCurrent->GetNumberGeomEdges());//mod->TVMaps.gePtrList.Count());
			mBaseMeshTopoDataCurrent->mSeamEdges.ClearAll();
		}

		for (int i = 0; i < mBaseMeshTopoDataCurrent->GetNumberGeomEdges(); i++)//mod->TVMaps.gePtrList.Count(); i++)
		{
				int numberSelected = 0;
				int ct = mBaseMeshTopoDataCurrent->GetGeomEdgeNumberOfConnectedFaces(i);//mod->TVMaps.gePtrList[i]->faceList.Count();
				
				for (int j = 0; j < ct; j++)
				{
					int faceIndex = mBaseMeshTopoDataCurrent->GetGeomEdgeConnectedFace(i,j);//mod->TVMaps.gePtrList[i]->faceList[j];
					//draw this edge
					if (mBaseMeshTopoDataCurrent->GetFaceSelected(faceIndex))//mod->TVMaps.f[faceIndex]->flags & FLAG_SELECTED)
					{
						numberSelected++;
					}
				}
				if ((numberSelected==1) )
				{
					mBaseMeshTopoDataCurrent->mSeamEdges.Set(i,TRUE);
				}
		}

		mBaseMeshTopoDataCurrent->GetTVVertSelectionPtr()->ClearAll();

	}

void PeltData::SelectRig(UnwrapMod *mod, BOOL replace)
{
	

	if (replace) 
	{
		mBaseMeshTopoDataCurrent->ClearTVVertSelection();//mod->vsel.ClearAll();
	}
	for (int i = 0; i < rigPoints.Count(); i++)
	{
		int index = rigPoints[i].lookupIndex;
		if ((index >= 0) && (index < mBaseMeshTopoDataCurrent->GetNumberTVVerts()))//mod->vsel.GetSize()))
			mBaseMeshTopoDataCurrent->SetTVVertSelected(index,TRUE);//mod->vsel.Set(index,TRUE);
	}

	mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	mod->InvalidateView();

	if (mod->ip) 
	{
		mod->ip->RedrawViews(mod->ip->GetTime());
	}


}
void PeltData::SelectPelt(UnwrapMod *mod, BOOL replace)
{


	if (replace) 
		mBaseMeshTopoDataCurrent->ClearTVVertSelection();//	mod->vsel.ClearAll();
	//loopt through the rigs
	BitArray vsel = mBaseMeshTopoDataCurrent->GetTVVertSelection();
	BitArray rigPointsSel;
	rigPointsSel.SetSize(vsel.GetSize());
	rigPointsSel.ClearAll();

	for (int i = 0; i < rigPoints.Count(); i++)
	{
		int index = rigPoints[i].lookupIndex;
		rigPointsSel.Set(index,TRUE);
	}
	for (int i = 0; i < springEdges.Count(); i++)
	{
		int index = springEdges[i].v1;
		if (index >= 0)
		{
			if ( (index < rigPointsSel.GetSize()) && (index < vsel.GetSize()))
			{
				if (!rigPointsSel[index])
					vsel.Set(index,TRUE);
			}
		}

		index = springEdges[i].v2;
		if (index >= 0)
		{
			if ( (index < rigPointsSel.GetSize()) && (index < vsel.GetSize()) )
			{
				if (!rigPointsSel[index])
					vsel.Set(index,TRUE);
			}
		}
	}

	mBaseMeshTopoDataCurrent->SetTVVertSelection(vsel);

	mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	mod->InvalidateView();
	if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
}


void PeltData::ResetRig(UnwrapMod *mod)
{
	theHold.Begin();
	mod->HoldPointsAndFaces();	

	TimeValue t = GetCOREInterface()->GetTime();

	for (int i=0;i<verts.Count();i++)
	{
		verts[i].vel = Point3(0.0f,0.0f,0.0f);
		for (int j = 0; j < 6; j++)
		{
			verts[i].tempVel[j] = Point3(0.0f,0.0f,0.0f);
			verts[i].tempPos[j] = Point3(0.0f,0.0f,0.0f);
		}			
	}
	MeshTopoData *ld = mBaseMeshTopoDataCurrent;
	int vCount = ld->GetNumberTVVerts();
	for (int i = 0; i < springEdges.Count(); i++)
	{
		int index = springEdges[i].v1;
		if (index >= 0)
		{

			if (( index < vCount) && (index < initialPointData.Count()))
			{
				ld->SetTVVert(t,index,initialPointData[index],mod);//mod->TVMaps.v[index].p = initialPointData[index];
			}

		}

		index = springEdges[i].v2;
		if (index >= 0)
		{
			if (( index < vCount) && (index < initialPointData.Count()))
			{
				ld->SetTVVert(t,index,initialPointData[index],mod);//mod->TVMaps.v[index].p = initialPointData[index];
			}
		}

		index = springEdges[i].vec1;
		if (index >= 0)
		{
			if (( index < vCount) && (index < initialPointData.Count()))
			{
				ld->SetTVVert(t,index,initialPointData[index],mod);//mod->TVMaps.v[index].p = initialPointData[index];
			}
		}

		index = springEdges[i].vec2;
		if (index >= 0)
		{

			if (( index < vCount) && (index < initialPointData.Count()))
			{
				ld->SetTVVert(t,index,initialPointData[index],mod);//mod->TVMaps.v[index].p = initialPointData[index];
			}
		}
	}
	theHold.Accept(GetString(IDS_PELTDIALOG_RESETRIG));

	mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	mod->InvalidateView();
	if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
}

void PeltData::RunRelax(UnwrapMod *mod, int relaxLevel)
{
	float  holdStr = rigStrength;
	int holdFrames = GetFrames();
	int holdSamples = GetSamples();
	
	if (relaxLevel == 0)
		SetRigStrength(0.005f);
	else if (relaxLevel == 1)
		SetRigStrength(0.0005f);

	SetSamples(5);
	SetFrames(5);

	Run(mod);
	SetRigStrength(holdStr);

	SetSamples(holdSamples);
	SetFrames(holdFrames);

}



void PeltData::Run(UnwrapMod *mod, BOOL updateViewport)
{


	MeshTopoData *ld = mBaseMeshTopoDataCurrent;

	//see if we have selected verts
	BOOL selectedVerts = FALSE;
	for (int i = 0; i < ld->GetNumberFaces(); i++)
	{
		if (!ld->GetFaceDead(i))
		{
			int deg = ld->GetFaceDegree(i);
			for (int j = 0; j < deg; j++)
			{
				int index = ld->GetFaceTVVert(i,j);
				if (ld->GetTVVertSelected(index) || ld->GetTVVertInfluence(index) > 0.0f)
				{
					selectedVerts = TRUE;
				}
			}
		}
	}


	//update our vertex data pos/vel
 	for (int j = 0; j < verts.Count(); j++)
		{
			Point3 p = ld->GetTVVert(j);
			verts[j].pos = p;
			if (!selectedVerts)
				verts[j].weight = 1.0f;

			else
			{
				verts[j].weight = 0.0f;
				if (ld->GetTVVertSelected(j))
					verts[j].weight = 1.0f;					
				else verts[j].weight = ld->GetTVVertInfluence(j);
					selectedVerts = TRUE;
			}

			if (!ld->IsTVVertVisible(j))
				verts[j].weight = 0.0f;			
			
		}

	Solver solver;

	
	//create our solver
	
 	Tab<EdgeBondage> tempSpringEdges;
	for (int i = 0; i < springEdges.Count(); i++)
	{
		//copy rig springs
		if (springEdges[i].isEdge)
		{
			tempSpringEdges.Append(1,&springEdges[i],10000);
		}
		else
		{
			int a = springEdges[i].v1;
			int b = springEdges[i].v2;
			if (mod->fnGetLockSpringEdges())
			{
				if ((verts[a].weight > 0.0f) || (verts[b].weight > 0.0f))
					tempSpringEdges.Append(1,&springEdges[i],10000);
			}
			else
			{
				if ((verts[a].weight > 0.0f) && (verts[b].weight > 0.0f))
					tempSpringEdges.Append(1,&springEdges[i],10000);
			}

		}
	}
	//solve
	solver.Solve(0, frames, samples,
				tempSpringEdges, verts,
			   GetStiffness(),GetDampening(),GetDecay(),mod,ld,updateViewport);

	
	if (mod)
	{
	//put the data back
			TimeValue t = GetCOREInterface()->GetTime();

			for (int j = 0; j < verts.Count(); j++)
			{
				Point3 p = verts[j].pos	;		
				ld->SetTVVert(t,j,p,mod);							
			}
			ResolvePatchHandles(mod);
			mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
			mod->InvalidateView();
			if (updateViewport)
			{
				if (mod->ip) mod->ip->RedrawViews(t);
			}
	}
}


void PeltData::CutSeams(UnwrapMod *mod, BitArray seams)
{
 
	mBaseMeshTopoDataCurrent->CutSeams(mod,seams);
	

}


void  UnwrapMod::SetPeltDialogPos()
{
	if (peltWindowPos.length != 0) 
		SetWindowPlacement(peltData.GetPeltDialogHandle(),&peltWindowPos);
}

void  UnwrapMod::SavePeltDialogPos()
{
	peltWindowPos.length = sizeof(WINDOWPLACEMENT); 
	GetWindowPlacement(peltData.GetPeltDialogHandle(),&peltWindowPos);
}

static void SlideChildWindow(HWND hWnd, HWND hwPar,  int deltay)
{
	if (hWnd != NULL ) 
	{
		RECT rect;
		GetWindowRect(hWnd,&rect);
		MapWindowPoints(NULL, hwPar, (POINT*)&rect, 2);
		//		GetClientRectP(hWnd,&rect);
		SetWindowPos(hWnd,NULL,rect.left,deltay-35,0,0,
			SWP_NOSIZE|SWP_NOZORDER);
	}	
}


static void ResizeWindows(HWND hWnd, int bottomHeight)
{
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
	HWND commitHWND = GetDlgItem(hWnd, IDC_COMMIT_BUTTON);
	SlideChildWindow(commitHWND,hWnd,bottomHeight);
	HWND cancelHWND = GetDlgItem(hWnd, IDC_CANCEL_BUTTON);
	SlideChildWindow(cancelHWND,hWnd,bottomHeight);

	HWND rollupWindow = GetDlgItem(hWnd,IDC_PELTDIALOG_ROLLOUT);
	x = 3;
	y = 1;
	width = 302;
	height = bottomHeight - 40;
	MoveWindow(rollupWindow,x,y,width,height,TRUE);
}
INT_PTR CALLBACK UnwrapPeltFloaterDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	UnwrapMod *mod = DLGetWindowLongPtr<UnwrapMod*>(hWnd);
	static int bottomHeight = 0;

	switch (msg) {
		case WM_INITDIALOG:
			{


			mod = (UnwrapMod*)lParam;

			DLSetWindowLongPtr(hWnd, lParam);
			::SetWindowContextHelpId(hWnd, idh_unwrap_peltmap);
			SendMessage(hWnd, WM_SETICON, ICON_SMALL, GetClassLongPtr(mod->ip->GetMAXHWnd(), GCLP_HICONSM)); // mjm - 3.12.99


//HOLD OUR UVWs
			mod->HoldCancelBuffer();

			mod->peltData.peltDialog.SetUpDialog(hWnd);
			mod->peltData.SetPeltDialogHandle(hWnd);
			mod->peltData.StartPeltDialog(mod);

			//restore window pos
			mod->SetPeltDialogPos();

			Rect rectp;
			GetClientRect(hWnd,&rectp);			
			bottomHeight = rectp.bottom - rectp.top;
//			ResizeWindows(hWnd,newWidth,newHeight,bottomHeight);
			ResizeWindows(hWnd,bottomHeight);

			break;
			}
		case WM_WINDOWPOSCHANGING: 
			{
				WINDOWPOS *wp = (WINDOWPOS*)lParam;
				wp->cx = 315;
				break;
			}
		case WM_SIZE:
			{				
				int newHeight = HIWORD(lParam);
				ResizeWindows(hWnd,newHeight);
			}
			break;
		case WM_SYSCOMMAND:
			if ((wParam & 0xfff0) == SC_CONTEXTHELP) 
			{
				DoHelp(HELP_CONTEXT, idh_unwrap_peltmap); 
			}
			return FALSE;
			break;

		case CC_SPINNER_CHANGE:
			mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);
			mod->peltData.peltDialog.SpinnerChange(mod->peltData,LOWORD(wParam));
			mod->InvalidateView();
			mod->PeltThreadOp(UnwrapMod::KThreadReStart,hWnd);
			break;
		case WM_DESTROY:
			mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);
			mod->PeltThreadOp(UnwrapMod::KThreadEnd,hWnd);
 			mod->peltData.peltDialog.DestroyDialog();
			mod->peltData.EndPeltDialog(mod);
			mod->peltData.NukeRig();
			mod->peltData.SetPeltDialogHandle( NULL);
			mod->fnSetMapMode(NOMAP);
			break;
		case WM_CLOSE:
			{
				mod->SavePeltDialogPos();
				mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);
				mod->PeltThreadOp(UnwrapMod::KThreadEnd,hWnd);
 				mod->peltData.peltDialog.DestroyDialog();
				mod->peltData.EndPeltDialog(mod);

				mod->peltData.NukeRig();
				if (!mod->peltData.IsSubObjectUpdateSuspended())
					mod->fnSetTVSubMode(TVFACEMODE);

				mod->InvalidateView();
				EndDialog(hWnd,1);

				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.peltDialog"));
				macroRecorder->FunctionCall(mstr, 0, 0);
				mod->peltData.SetPeltDialogHandle( NULL);
				mod->fnSetMapMode(NOMAP);
				mod->RestoreCancelBuffer();
				mod->FreeCancelBuffer();

				break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_COMMIT_BUTTON:
				{
					
					mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);
					mod->PeltThreadOp(UnwrapMod::KThreadEnd,hWnd);
					mod->SavePeltDialogPos();
					mod->peltData.peltDialog.DestroyDialog();
					mod->peltData.EndPeltDialog(mod);

					mod->peltData.NukeRig();
					if (!mod->peltData.IsSubObjectUpdateSuspended())
						mod->fnSetTVSubMode(TVFACEMODE);

					mod->InvalidateView();
					EndDialog(hWnd,1);

					TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.peltDialog"));
					macroRecorder->FunctionCall(mstr, 0, 0);
					mod->peltData.SetPeltDialogHandle( NULL);
					mod->fnSetMapMode(NOMAP);
					EndDialog(hWnd,1);
					mod->FreeCancelBuffer();
					break;
				}
			case IDC_CANCEL_BUTTON:
				{
					mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);
					mod->PeltThreadOp(UnwrapMod::KThreadEnd,hWnd);
					mod->SavePeltDialogPos();
					mod->peltData.peltDialog.DestroyDialog();
					mod->peltData.EndPeltDialog(mod);

					mod->peltData.NukeRig();
					if (!mod->peltData.IsSubObjectUpdateSuspended())
						mod->fnSetTVSubMode(TVFACEMODE);

					mod->InvalidateView();
					EndDialog(hWnd,1);

					TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.peltDialog"));
					macroRecorder->FunctionCall(mstr, 0, 0);
					mod->peltData.SetPeltDialogHandle( NULL);
					mod->fnSetMapMode(NOMAP);

//RESTORE OUR UVs

					EndDialog(hWnd,0);
					mod->RestoreCancelBuffer();
					mod->FreeCancelBuffer();
					break;
				}
			}


		case WM_ACTIVATE:
			mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);
			mod->PeltThreadOp(UnwrapMod::KThreadEnd,hWnd);
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}


	//The Window handler for the Dynamics rollout in the modeless propertis dialog
	//******************************************************************************
INT_PTR CALLBACK QuickPeltRollupDialogProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{


		UnwrapMod *mod = DLGetWindowLongPtr<UnwrapMod*>(hWnd);
		if ( !mod && message != WM_INITDIALOG ) return FALSE;

		TimeValue t = GetCOREInterface()->GetTime();

		switch ( message ) 
		{
		case WM_INITDIALOG:
			{
				DLSetWindowLongPtr(hWnd, lParam);
				mod = (UnwrapMod*)lParam;
				
				mod->peltData.peltDialog.iSpinSamples = SetupIntSpinner(
					hWnd,IDC_PELT_SAMPLESSPIN,IDC_PELT_SAMPLES,
					1,50,mod->peltData.GetSamples());	
				mod->peltData.peltDialog.iSpinSamples->SetScale(1.0f);


				mod->peltData.peltDialog.iRunPeltButton = GetICustButton(GetDlgItem(hWnd, IDC_RUNPELT_BUTTON));
				mod->peltData.peltDialog.iRunPeltButton->SetType(CBT_CHECK);
				mod->peltData.peltDialog.iRunPeltButton->SetHighlightColor(GREEN_WASH);

				mod->peltData.peltDialog.iRunPeltRelaxButton = GetICustButton(GetDlgItem(hWnd, IDC_RUNRELAX_BUTTON));
				mod->peltData.peltDialog.iRunPeltRelaxButton->SetType(CBT_CHECK);
				mod->peltData.peltDialog.iRunPeltRelaxButton->SetHighlightColor(GREEN_WASH);

				BOOL showLocal = FALSE;
				mod->pblock->GetValue(unwrap_localDistorion,t,showLocal,FOREVER);
				CheckDlgButton(hWnd,IDC_SHOW_LOCAL_DISTORTIONS_CHECK,showLocal);
			}

			return TRUE;

		case WM_DESTROY:
		case WM_MOUSEACTIVATE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:            
			return FALSE;
		case CC_SPINNER_CHANGE:
			mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);
			mod->peltData.peltDialog.SpinnerChange(mod->peltData,LOWORD(wParam));
			mod->InvalidateView();
			mod->PeltThreadOp(UnwrapMod::KThreadReStart,hWnd);
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_RUNPELT_BUTTON:
				{
					ICustButton *iButton = mod->peltData.peltDialog.iRunPeltButton;
					mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);
					if (iButton && iButton->IsChecked())
					{
						mod->PeltThreadOp(UnwrapMod::KThreadStart,hWnd);
					}
					else
						mod->PeltThreadOp(UnwrapMod::KThreadEnd,hWnd);

					mod->InvalidateView();
					TimeValue t = GetCOREInterface()->GetTime();
					mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
					break;
				}
			case IDC_RUNRELAX_BUTTON:
				{
					mod->PeltThreadOp(UnwrapMod::KThreadEnd,hWnd);
					ICustButton *iButton = mod->peltData.peltDialog.iRunPeltRelaxButton;
					if (iButton && iButton->IsChecked())
					{
						theHold.Begin();
						mod->HoldPoints();
						theHold.Accept(_T(GetString(IDS_PW_RELAX)));
						mod->PeltRelaxThreadOp(UnwrapMod::KThreadStart,hWnd);
					}
					else
						mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);

					mod->InvalidateView();
					TimeValue t = GetCOREInterface()->GetTime();
					mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
					break;
				}
			case IDC_RELAXSETTINGS_BUTTON:
				{
					mod->fnRelax2Dialog();
					mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);
					mod->PeltThreadOp(UnwrapMod::KThreadEnd,hWnd);
					break;
				}
			case IDC_RESETRIG_BUTTON:
				{
					mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);
					mod->PeltThreadOp(UnwrapMod::KThreadEnd,hWnd);
					mod->WtExecute(ID_PELTDIALOG_RESETRIG);
					TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.peltDialogResetStretcher"));
					macroRecorder->FunctionCall(mstr, 0, 0);						
					break;
				}
			case IDC_SHOW_LOCAL_DISTORTIONS_CHECK:
				{
					BOOL show = IsDlgButtonChecked(hWnd,IDC_SHOW_LOCAL_DISTORTIONS_CHECK);
					mod->pblock->SetValue(unwrap_localDistorion,t,show);
					mod->fnSetShowEdgeDistortion(show);
				}
			}
		}
	return FALSE;
	}

INT_PTR CALLBACK PeltOptionsRollupDialogProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{


	UnwrapMod *mod = DLGetWindowLongPtr<UnwrapMod*>(hWnd);
	if ( !mod && message != WM_INITDIALOG ) return FALSE;

	switch ( message ) 
	{
	case WM_INITDIALOG:
		{
			DLSetWindowLongPtr(hWnd, lParam);
			mod = (UnwrapMod*)lParam;

			CheckDlgButton(hWnd,IDC_LOCKOPENEDGESCHECK,mod->fnGetLockSpringEdges());


			mod->peltData.peltDialog.iSpinRigStrength = SetupFloatSpinner(
				hWnd,IDC_PELT_RIGSTRENGTHSPIN,IDC_PELT_RIGSTRENGTH,
				0.0f,0.5f,mod->peltData.GetRigStrength());	
			mod->peltData.peltDialog.iSpinRigStrength->SetScale(0.1f);


			mod->peltData.peltDialog.iSpinStiffness = SetupFloatSpinner(
				hWnd,IDC_PELT_STIFFNESSSPIN,IDC_PELT_STIFFNESS,
				0.0f,0.5f,mod->peltData.GetStiffness());	
			mod->peltData.peltDialog.iSpinStiffness->SetScale(0.1f);

			mod->peltData.peltDialog.iSpinDampening = SetupFloatSpinner(
				hWnd,IDC_PELT_DAMPENINGSPIN,IDC_PELT_DAMPENING,
				0.0f,0.5f,mod->peltData.GetDampening());	
			mod->peltData.peltDialog.iSpinDampening->SetScale(0.1f);

			mod->peltData.peltDialog.iSpinDecay = SetupFloatSpinner(
				hWnd,IDC_PELT_DECAYSPIN,IDC_PELT_DECAY,
				0.0f,0.5f,mod->peltData.GetDecay());	
			mod->peltData.peltDialog.iSpinDecay->SetScale(0.1f);

			mod->peltData.peltDialog.iSpinMirrorAngle = SetupFloatSpinner(
				hWnd,IDC_PELT_MIRRORAXISSPIN,IDC_PELT_MIRRORAXIS,
				0.0f,360.0f,mod->peltData.GetMirrorAngle()*180.0f/PI);	
			mod->peltData.peltDialog.iSpinMirrorAngle->SetScale(1.0f);

			mod->peltData.peltDialog.iPeltStraightenButton = GetICustButton(GetDlgItem(hWnd, IDC_STRAIGHTEN_BUTTON2));
			mod->peltData.peltDialog.iPeltStraightenButton->SetType(CBT_CHECK);
			mod->peltData.peltDialog.iPeltStraightenButton->SetHighlightColor(GREEN_WASH);
		}
		return TRUE;

	case WM_DESTROY:
		return FALSE;

	case WM_MOUSEACTIVATE:
		return FALSE;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MOUSEMOVE:            
		//          cont->ip->RollupMouseMessage(hDlg,message,wParam,lParam);
		return FALSE;
	case CC_SPINNER_CHANGE:
		mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);
		mod->peltData.peltDialog.SpinnerChange(mod->peltData,LOWORD(wParam));
		mod->InvalidateView();
		mod->PeltThreadOp(UnwrapMod::KThreadReStart,hWnd);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{
		case IDC_LOCKOPENEDGESCHECK:
			{
				mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);
				//set element mode swtich 
				//					CheckDlgButton(hWnd,IDC_SELECTELEMENT_CHECK,mod->fnGetGeomElemMode());
				mod->fnSetLockSpringEdges(IsDlgButtonChecked(hWnd,IDC_LOCKOPENEDGESCHECK));
				//send macro message
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.setPeltLockOpenEdges"));
				macroRecorder->FunctionCall(mstr, 1, 0,
					mr_bool,mod->fnGetLockSpringEdges());
				macroRecorder->EmitScript();

				mod->PeltThreadOp(UnwrapMod::KThreadReStart,hWnd);
				break;

			}
		case IDC_SELECTRIG_BUTTON:
			{
				mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);
				mod->PeltThreadOp(UnwrapMod::KThreadEnd,hWnd);
				mod->WtExecute(ID_PELTDIALOG_SELECTRIG);
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.peltDialogSelectStretcher"));
				macroRecorder->FunctionCall(mstr, 0, 0);						
				break;
			}
		case IDC_SELECTPELT_BUTTON:
			{
				mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);
				mod->PeltThreadOp(UnwrapMod::KThreadEnd,hWnd);
				mod->WtExecute(ID_PELTDIALOG_SELECTPELT);
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.peltDialogSelectPelt"));
				macroRecorder->FunctionCall(mstr, 0, 0);						
				break;
			}
		case IDC_SNAPRIG_BUTTON:
			{
				mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);
				mod->PeltThreadOp(UnwrapMod::KThreadEnd,hWnd);
				mod->WtExecute(ID_PELTDIALOG_SNAPRIG);
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.peltDialogSnapToSeams"));
				macroRecorder->FunctionCall(mstr, 0, 0);						
				break;
			}
		case IDC_RESETRIG_BUTTON:
			{
				mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);
				mod->PeltThreadOp(UnwrapMod::KThreadEnd,hWnd);
				mod->WtExecute(ID_PELTDIALOG_RESETRIG);
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.peltDialogResetStretcher"));
				macroRecorder->FunctionCall(mstr, 0, 0);						
				break;
			}
		case IDC_MIRRORRIG_BUTTON:
			{
				mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);
				mod->PeltThreadOp(UnwrapMod::KThreadEnd,hWnd);
				mod->WtExecute(ID_PELTDIALOG_MIRRORRIG);
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.peltDialogMirrorStretcher"));
				macroRecorder->FunctionCall(mstr, 0, 0);						
				break;
			}

		case IDC_STRAIGHTEN_BUTTON2:
			{
				mod->PeltRelaxThreadOp(UnwrapMod::KThreadEnd,hWnd);
				mod->PeltThreadOp(UnwrapMod::KThreadEnd,hWnd);
				mod->WtExecute(ID_PELTDIALOG_STRAIGHTENSEAMS);
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.setPeltDialogStraightenMode"));
				macroRecorder->FunctionCall(mstr, 1, 0,mr_bool,mod->peltData.peltDialog.GetStraightenMode());					
				break;
			}
		}
	}
	return FALSE;
}


void UnwrapMod::fnPeltDialog()
{

 	if ((GetPeltMapMode())&&(peltData.peltDialog.hWnd == NULL))
	{
		
		fnEdit();
		peltData.peltDialog.mod = this;
		peltData.peltDialog.hWnd = CreateDialogParam(	hInstance,
							MAKEINTRESOURCE(IDD_UNWRAP_PELTDIALOG),
							hWnd,
							UnwrapPeltFloaterDlgProc,
							(LPARAM)this );
		ShowWindow(peltData.peltDialog.hWnd ,SW_SHOW);

		peltData.SetupSprings(this);

		int oCount = peltData.mBaseMeshTopoDataCurrent->GetNumberTVVerts();//vsel.GetSize();
		peltData.startPoint = oCount;
		fnSetTVSubMode(TVVERTMODE);
		

		fnFit();
		NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);

	}
	else
	{
		SendMessage(peltData.peltDialog.hWnd,WM_CLOSE,0,0);
	}



}


void PeltDialog::SetUpDialog(HWND hWnd)
{










	IRollupWindow *irollup = GetIRollup(GetDlgItem(hWnd,IDC_PELTDIALOG_ROLLOUT));
	irollup->AppendRollup(hInstance,MAKEINTRESOURCE(IDD_UNWRAP_PELTROLLUP1_DIALOG),QuickPeltRollupDialogProc, _T(GetString(IDS_PW_QUICK_PELT)),(LPARAM)mod );

	irollup->AppendRollup(hInstance,MAKEINTRESOURCE(IDD_UNWRAP_PELTROLLUP2_DIALOG),PeltOptionsRollupDialogProc, _T(GetString(IDS_PW_PELT_OPTIONS)),(LPARAM)mod );
	irollup->Show(0);
	irollup->Show(1);
	irollup->SetPanelOpen(1,FALSE);
	ReleaseIRollup(irollup);
	this->hWnd = hWnd;

}

void PeltDialog::DestroyDialog() 
{
	
	ReleaseISpinner(iSpinRigStrength);
	iSpinRigStrength = NULL;

	ReleaseISpinner(iSpinSamples);
	iSpinSamples = NULL;

	ReleaseISpinner(iSpinStiffness);
	iSpinStiffness = NULL;

	ReleaseISpinner(iSpinDampening);
	iSpinDampening = NULL;

	ReleaseISpinner(iSpinDecay);
	iSpinDecay = NULL;

	ReleaseISpinner(iSpinMirrorAngle);
	iSpinMirrorAngle = NULL;

	ReleaseICustButton(iRunPeltButton);
	iRunPeltButton = NULL;

	ReleaseICustButton(iRunPeltRelaxButton);
	iRunPeltRelaxButton = NULL;

	ReleaseICustButton(iPeltStraightenButton);
	iPeltStraightenButton = NULL;

	this->hWnd = NULL;

}

void PeltDialog::SetStraightenMode(BOOL on)
{
	if (!on)
	{
		if (mod->oldMode == ID_TOOL_PELTSTRAIGHTEN)
			mod->SetMode(ID_TOOL_MOVE);
		else mod->SetMode(mod->oldMode);		
	}
	else
	{
		mod->peltData.currentPointHit = -1;
		mod->peltData.previousPointHit = -1;
		mod->SetMode(ID_TOOL_PELTSTRAIGHTEN);

		if (mod->fnGetTVSubMode() == TVVERTMODE)
			mod->peltData.mBaseMeshTopoDataCurrent->ClearTVVertSelection();//			mod->vsel.ClearAll();

		
	}

	if (iPeltStraightenButton)
	{
		if (on)
			iPeltStraightenButton->SetCheck(TRUE);
		else iPeltStraightenButton->SetCheck(FALSE);
	}

}

void PeltDialog::SetStraightenButton(BOOL on)
{
	if (iPeltStraightenButton)
	{
		if (on)
			iPeltStraightenButton->SetCheck(TRUE);
		else iPeltStraightenButton->SetCheck(FALSE);
	}
}

BOOL PeltDialog::GetStraightenMode()
{
	if (mod->mode == ID_TOOL_PELTSTRAIGHTEN)
		return TRUE;
	else return FALSE;
}

void PeltDialog::UpdateSpinner(int id, int value)
{
	if ((id == IDC_PELT_SAMPLESSPIN) && iSpinSamples)
		iSpinSamples-> SetValue(value, 0);
}

void PeltDialog::UpdateSpinner(int id, float value)
{
	if ((id == IDC_PELT_RIGSTRENGTHSPIN) && iSpinRigStrength)
		iSpinRigStrength-> SetValue(value, 0);
	else if ((id == IDC_PELT_STIFFNESSSPIN) && iSpinStiffness)
		iSpinStiffness-> SetValue(value, 0);
	else if ((id == IDC_PELT_DAMPENINGSPIN) && iSpinDampening)
		iSpinDampening-> SetValue(value, 0);
	else if ((id == IDC_PELT_DECAYSPIN) && iSpinDecay)
		iSpinDecay-> SetValue(value, 0);
	else if ((id == IDC_PELT_MIRRORAXISSPIN) && iSpinMirrorAngle)
		iSpinMirrorAngle-> SetValue(value, 0);

}

void PeltDialog::SpinnerChange(PeltData &peltData, int controlID)
{
	if (controlID == IDC_PELT_RIGSTRENGTHSPIN)
	{
		if (iSpinRigStrength)
		{
			peltData.SetRigStrength(iSpinRigStrength->GetFVal());

			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.setPeltDialogStretcherStrength"));
			macroRecorder->FunctionCall(mstr, 1, 0,
				mr_float,iSpinRigStrength->GetFVal());		
		}
	}
	else if (controlID == IDC_PELT_SAMPLESSPIN)
	{
		if (iSpinSamples)
		{
			peltData.SetSamples(iSpinSamples->GetIVal());
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.setPeltDialogSamples"));
			macroRecorder->FunctionCall(mstr, 1, 0,
				mr_int,iSpinSamples->GetIVal());						
		}
	}
	else if (controlID == IDC_PELT_STIFFNESSSPIN)
	{
		if (iSpinStiffness)
		{
			peltData.SetStiffness(iSpinStiffness->GetFVal());
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.setPeltDialogStiffness"));
			macroRecorder->FunctionCall(mstr, 1, 0,
				mr_float,iSpinStiffness->GetFVal());		

		}
	}
	else if (controlID == IDC_PELT_DAMPENINGSPIN)
	{
		if (iSpinDampening)
		{
			peltData.SetDampening(iSpinDampening->GetFVal());
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.setPeltDialogDampening"));
			macroRecorder->FunctionCall(mstr, 1, 0,
				mr_float,iSpinDampening->GetFVal());
		}
	}
	else if (controlID == IDC_PELT_DECAYSPIN)
	{
		if (iSpinDecay)
		{
			peltData.SetDecay(iSpinDecay->GetFVal());
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.setPeltDialogDecay"));
			macroRecorder->FunctionCall(mstr, 1, 0,
				mr_float,iSpinDecay->GetFVal());

		}
	}
	else if (controlID == IDC_PELT_MIRRORAXISSPIN)
	{
		if (iSpinMirrorAngle)
		{
			peltData.SetMirrorAngle(iSpinMirrorAngle->GetFVal()*PI/180.0f);
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.setPeltDialogMirrorAxis"));
			macroRecorder->FunctionCall(mstr, 1, 0,
				mr_float,iSpinMirrorAngle->GetFVal());
		}
	}


}


UnwrapPeltSeamRestore::UnwrapPeltSeamRestore(UnwrapMod *m, MeshTopoData *d, Point3 *anchorPoint)
{
	mod = m;
	ld = d;
	useams = d->mSeamEdges;
	up1 = mod->peltData.previousPointHit;
	up2 = mod->peltData.currentPointHit;
	ucurrentP = mod->peltData.currentMouseClick;
	ulastP = mod->peltData.lastMouseClick;

	//watje 685880 might need to restore the anchor point
	mRestoreAnchorPoint = FALSE;
	if (anchorPoint)
	{
		mUAnchorPoint = *anchorPoint;
		mRestoreAnchorPoint = TRUE;
	}
}
	
void UnwrapPeltSeamRestore::Restore(int isUndo)
{
	
	if (isUndo)
	{
		rseams = ld->mSeamEdges;
		rp1 = mod->peltData.previousPointHit;
		rp2 = mod->peltData.currentPointHit;

		rcurrentP = mod->peltData.currentMouseClick;
		rlastP = mod->peltData.lastMouseClick;

		//watje 685880 might need to restore the anchor point
		if (mRestoreAnchorPoint)
			mRAnchorPoint = mod->peltData.pointToPointAnchorPoint;


	}
	mod->peltData.basep = ulastP;
	ld->mSeamEdges = useams;
	mod->peltData.previousPointHit = -1;
	mod->peltData.currentPointHit = up1;
	mod->peltData.basep = ulastP;

	//watje 685880 might need to restore the anchor point
	if (mRestoreAnchorPoint)
		mod->peltData.pointToPointAnchorPoint = mUAnchorPoint;


	mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);

}
void UnwrapPeltSeamRestore::Redo()
{
	

	mod->peltData.previousPointHit = -1;
	mod->peltData.currentPointHit = rp1;
	mod->peltData.basep = rlastP;

	ld->mSeamEdges = rseams;

	//watje 685880 might need to restore the anchor point
	if (mRestoreAnchorPoint)
		mod->peltData.pointToPointAnchorPoint = mRAnchorPoint;

	mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	
}

UnwrapPeltVertVelocityRestore::UnwrapPeltVertVelocityRestore(UnwrapMod *m)
{
	mod = m;
	uverts = mod->peltData.verts;

}
	
void UnwrapPeltVertVelocityRestore::Restore(int isUndo)
{
	if (isUndo)
	{
		rverts = mod->peltData.verts;
	}
	mod->peltData.verts = uverts;
}
void UnwrapPeltVertVelocityRestore::Redo()
{
	mod->peltData.verts = rverts;
}


void PeltData::MirrorRig(UnwrapMod *mod)
{

	TimeValue t = GetCOREInterface()->GetTime();
	if (rigPoints.Count() == 0) return;
	//get our mirror angle
	//build our total rig initial distance
	float d= 0.0f;
	for (int i = 0; i < rigPoints.Count(); i++)
	{
		d += rigPoints[i].elen;
	}
	float mirrorAngle = rigMirrorAngle;
	

	Tab<float> mAngles;
 	mAngles.SetCount(rigPoints.Count());
	for (int i = 0; i < rigPoints.Count(); i++)
	{
		float rigAngle = (rigPoints[i].angle - mirrorAngle);
		if (rigAngle < 0.0f)
			rigAngle = (PI*2.0f) + rigAngle;
		mAngles[i] = rigAngle;

	}
	//build our mirror tm
	Matrix3 tm(1);
	tm.RotateZ(mirrorAngle);
	tm.SetRow(3,rigCenter);

	Matrix3 itm = Inverse(tm);
	

	//now start mirroring the data

	//get our start source point list
	int startSource = 0;
	int startMirror = 0;
	//get our start source mirror list
	Point3 lastPoint = Point3(0.0f,0.0f,0.0f);
	MeshTopoData *ld = mBaseMeshTopoDataCurrent;
	lastPoint = ld->GetTVVert(rigPoints[0].lookupIndex);//mod->TVMaps.v[rigPoints[0].lookupIndex].p;
	lastPoint = lastPoint * itm;
	Point3 nextPoint;
	for (int i = 1; i < rigPoints.Count(); i++)
	{		
		int rigIndex = rigPoints[i].lookupIndex;
		Point3 testPoint = ld->GetTVVert(rigIndex);//mod->TVMaps.v[rigIndex].p;
		testPoint = testPoint * itm;
		if ((testPoint.x < 0.0f) && (lastPoint.x > 0.0f))
		{
			startSource = i - 1;
			startMirror = i;
		}
		lastPoint = testPoint;
		
	}
	//build our mirror list
	nextPoint = Point3(1.0f,0.0f,0.0f);
  	int currentSourcePoint = startSource;
	while (nextPoint.x > 0.0f)
	{	
		int id = rigPoints[currentSourcePoint].lookupIndex;
		Point3 p = ld->GetTVVert(id);//mod->TVMaps.v[id].p;
		p = p * itm;
		nextPoint = p;

		currentSourcePoint--;
		if (currentSourcePoint < 0)
			currentSourcePoint = rigPoints.Count()-1;			
	}

//now walk down the target verts

	currentSourcePoint = startSource;
	int prevSourcePoint = startSource;
	int currentTargetPoint = startMirror;
	//set the first one then walk the path
	int id =  rigPoints[currentTargetPoint].lookupIndex;
	int sid = rigPoints[currentSourcePoint].lookupIndex;
	Point3 p = ld->GetTVVert(sid);//mod->TVMaps.v[sid].p;

	p = p*itm;
	p.x *= -1.0f;
	ld->SetTVVert(t,id,p*tm,mod);//mod->TVMaps.v[id].p = p*tm;
	float runningSourceD = 0.0f;
	float runningLastSourceD = 0.0f;
	float runningTargetD = 0.0f;
	Point3 lastMP = Point3(1.0f,0.0f,0.0f);
	while (lastMP.x > 0.0f)
	{	
		//get the next target point
		currentTargetPoint++;
			if (currentTargetPoint>=rigPoints.Count())
				currentTargetPoint = 0;
			runningTargetD += rigPoints[currentTargetPoint].elen;
			//now find the spot on the mirror
			while (runningSourceD < runningTargetD)
			{
				prevSourcePoint = currentSourcePoint;
				runningLastSourceD = runningSourceD;
				currentSourcePoint--;
				if (currentSourcePoint < 0)
					currentSourcePoint = rigPoints.Count()-1;	
				runningSourceD += rigPoints[currentSourcePoint].elen;
			}
			float remainder = runningTargetD - runningLastSourceD;
			float per = remainder/(runningSourceD-runningLastSourceD);

			Point3 prevp = ld->GetTVVert(rigPoints[prevSourcePoint].lookupIndex);//mod->TVMaps.v[rigPoints[prevSourcePoint].lookupIndex].p;
			Point3 currentp = ld->GetTVVert(rigPoints[currentSourcePoint].lookupIndex);//mod->TVMaps.v[rigPoints[currentSourcePoint].lookupIndex].p;
			Point3 vec = (currentp - prevp) * per;
			Point3 p = prevp + vec;
			p = p * itm;
			lastMP = p;
			p.x *= -1.0f;		
			p = p * tm;
		
			ld->SetTVVert(t,rigPoints[currentTargetPoint].lookupIndex,p,mod);//mod->TVMaps.v[rigPoints[currentTargetPoint].lookupIndex].p = p;
		}

	


	mod->InvalidateView();

}



void PeltData::StraightenSeam(UnwrapMod *mod, int a, int b)

{

	//get our sel


	BitArray seamPoints;
	MeshTopoData *ld = mBaseMeshTopoDataCurrent;
	seamPoints.SetSize(ld->GetNumberTVVerts());//mod->vsel.GetSize());
	seamPoints.ClearAll();
	for (int i = 0; i < rigPoints.Count(); i++)
	{
		seamPoints.Set(rigPoints[i].lookupIndex,TRUE);
	}


	if (a == b) return;
	int ra = -1;
	int rb = -1;

	for (int i = 0; i < rigPoints.Count(); i++)
	{
		if ((a == rigPoints[i].lookupIndex) && (ra == -1))
			ra = i;
		if ((b == rigPoints[i].lookupIndex) && (rb == -1))
			rb = i;
	}

	//make sure we have 2 rig points
	if ((ra == -1) || (rb == -1))
		return;

	int aDist;
	int bDist;
	if (rb < ra)
	{
		int temp = ra;
		ra = rb;
		rb = temp;
	}

	aDist = rb - ra;
	bDist = ra;
	bDist += rigPoints.Count() - rb;

	int startPoint = ra;
	int endPoint = rb;

	if (bDist < aDist)
	{
		startPoint = rb;
		endPoint = ra;
	}

	int currentPoint = startPoint;
	float goalDist = 0.0f;
	float seamDist = 0.0f;

	Tab<int> edgeList;

	while (currentPoint != endPoint)
	{
		edgeList.Append(1,&currentPoint);
		currentPoint++;
		if (currentPoint >= rigPoints.Count())
			currentPoint = 0;
	}

	edgeList.Append(1,&endPoint);
	Point3 pa,pb;

	pa = ld->GetTVVert(rigPoints[startPoint].lookupIndex);//mod->TVMaps.v[rigPoints[startPoint].lookupIndex].p;
	pb = ld->GetTVVert(rigPoints[endPoint].lookupIndex);//mod->TVMaps.v[rigPoints[endPoint].lookupIndex].p;

	goalDist = Length(pb-pa);
	Point3 vec = (pb-pa);

	for (int i = 0; i < (edgeList.Count()-1); i++)
	{
		seamDist += rigPoints[i].elen;
	}


	float runningDist = rigPoints[0].elen;
	TimeValue t = GetCOREInterface()->GetTime();
	for (int i = 1; i < (edgeList.Count()-1); i++)
	{
		int id = edgeList[i];
		float per = runningDist/seamDist;
		int currentPoint = rigPoints[id].lookupIndex;
		Point3 p = pa + (vec*per);
		ld->SetTVVert(t,currentPoint,p,mod);//mod->TVMaps.v[currentPoint].p = pa + (vec*per);
		runningDist += rigPoints[i].elen; 
	} 

}



HCURSOR PeltStraightenMode::GetXFormCur() 
	{
	return mod->selCur;
	}

int PeltStraightenMode::subproc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
	{
	switch (msg) {
		case MOUSE_POINT:
			if (point==0) {
 				theHold.SuperBegin();
//		 		mod->PlugControllers();
				theHold.Begin();
				mod->HoldPoints();
				om = m;
				mod->tempVert = Point3(0.0f,0.0f,0.0f);


			} else {
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.MoveSelected"));
				macroRecorder->FunctionCall(mstr, 1, 0,
					mr_point3,&mod->tempVert);

				if (mod->peltData.previousPointHit!= -1)
				{
					
					mod->peltData.StraightenSeam(mod,mod->peltData.previousPointHit,mod->peltData.currentPointHit);
					TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.peltDialogStraighten"));
					macroRecorder->FunctionCall(mstr, 2, 0,
						mr_int,mod->peltData.previousPointHit+1,
						mr_int,mod->peltData.currentPointHit+1
						);
				}
				mod->peltData.previousPointHit = mod->peltData.currentPointHit;
			 	mod->ip->RedrawViews(mod->ip->GetTime());

				theHold.Accept(_T(GetString(IDS_PW_MOVE_UVW)));
				theHold.SuperAccept(_T(GetString(IDS_PW_MOVE_UVW)));
				om = m;
				}
			break;

		case MOUSE_MOVE: {
			theHold.Restore();

			Tab<int> hits;
			Rect rect;
			rect.left = m.x;
			rect.right = m.x;
			rect.top = m.y;
			rect.bottom = m.y;

			MeshTopoData *ld = mod->peltData.mBaseMeshTopoDataCurrent;
			for (int i = 0; i < ld->GetNumberTVVerts(); i++)//mod->vsel.GetSize(); i++)
			{
				if (ld->GetTVVertSelected(i))//if (mod->vsel[i])
					mod->peltData.currentPointHit = i;
			}


			float xzoom, yzoom;
			int width, height;
			IPoint2 delta = m-om;
			if (flags&MOUSE_SHIFT && mod->move==0) {
				if (abs(delta.x) > abs(delta.y)) delta.y = 0;
				else delta.x = 0;
			} else if (mod->move==1) {
				delta.y = 0;
			} else if (mod->move==2) {
				delta.x = 0;
				}
			mod->ComputeZooms(hWnd,xzoom,yzoom,width,height);
			Point2 mv;
			mv.x = delta.x/xzoom;
			mv.y = -delta.y/yzoom;
			
			mod->tempVert.x = mv.x;
			mod->tempVert.y = mv.y;
			mod->tempVert.z = 0.0f;

			mod->TransferSelectionStart();
			mod->MovePoints(mv);
			mod->TransferSelectionEnd(FALSE,FALSE);

	 	 	if (mod->peltData.previousPointHit!= -1)
				mod->peltData.StraightenSeam(mod,mod->peltData.previousPointHit,mod->peltData.currentPointHit);

			if (mod->update) mod->ip->RedrawViews(mod->ip->GetTime());
			UpdateWindow(hWnd);
			break;		
			}

		case MOUSE_ABORT:
			theHold.Cancel();
			theHold.SuperCancel();
			mod->ip->RedrawViews(mod->ip->GetTime());
			break;
		}
	return 1;
	}




void PeltData::ExpandSelectionToSeams(UnwrapMod *mod)
{

	for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
	{
		MeshTopoData *ld = mod->GetMeshTopoData(ldID);


		ld->ExpandSelectionToSeams();

		mod->SyncTVToGeomSelection(ld);
	}

	
	mod->InvalidateView();
	mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (mod->ip) 
	{
		mod->ip->RedrawViews(mod->ip->GetTime());
	}


}

int PeltData::HitTestPointToPointMode(UnwrapMod *mod, MeshTopoData *ld, ViewExp *vpt,GraphicsWindow *gw,IPoint2 *p,HitRegion hr, INode* inode,ModContext* mc)
{
	int res = 0;
	int closestIndex = -1;

	static IPoint2 basep;
	static IPoint2 cp,pp;

	if (currentPointHit == -1) 
	{
		basep = p[0];
	}
	pp = cp;
	cp = p[0];
	//xor our last line
	//draw our new one


	gw->setRndLimits(gw->getRndLimits() |  GW_BACKCULL);
	BitArray visibleFaces;
	mod->BuildVisibleFaces(gw, ld, visibleFaces);

	Tab<UVWHitData> hitVerts;
	mod->HitGeomVertData(ld,hitVerts,gw,  hr);
	res = 0;

	TimeValue t = GetCOREInterface()->GetTime();
	Matrix3 ntm = inode->GetObjectTM(t);

	Matrix3 vtm; 
	vpt->GetAffineTM(vtm );
	vtm = Inverse(vtm);

	viewZVec = vtm.GetRow(2);
	ntm = Inverse(ntm);
	viewZVec = VectorTransform(ntm,viewZVec);


	Tab<int> lookup;
	lookup.SetCount(ld->GetNumberGeomVerts());//mod->TVMaps.geomPoints.Count());
	for (int i = 0;  i < ld->GetNumberGeomVerts(); i++)//mod->TVMaps.geomPoints.Count(); i++)
	{
		lookup[i] = -1;
	}

	for (int i = 0;  i < hitVerts.Count(); i++)
	{
		lookup[hitVerts[i].index] = i;
	}
	float closest = -1.0f;
	

	Matrix3 toView(1);
	vpt->GetAffineTM(toView);
	Matrix3 mat = inode->GetObjectTM(GetCOREInterface()->GetTime());
	toView = mat * toView;

	for (int i = 0;  i < visibleFaces.GetSize(); i++)
	{

		if (visibleFaces[i])
		{
			int deg = ld->GetFaceDegree(i);//mod->TVMaps.f[i]->count;
			for (int j = 0; j < deg; j++)
			{
				int index = ld->GetFaceGeomVert(i,j);//mod->TVMaps.f[i]->v[j];

				if (lookup[index]!=-1)
				{
					

					Point3 p = ld->GetGeomVert(index) * toView;//mod->TVMaps.geomPoints[index] * toView;
					if ((p.z > closest) || (closestIndex==-1))
					{
						closest = p.z ;
						closestIndex = index;
					}

				}
			}

		}
	}
	if (closestIndex != -1)
	{
		res = 1;
		vpt->LogHit(inode,mc,0.0f,closestIndex,NULL);
	}

	return closestIndex;
}


int PeltData::HitTestEdgeMode(UnwrapMod *mod,MeshTopoData *ld , ViewExp *vpt,GraphicsWindow *gw/*w4,IPoint2 *p*/,HitRegion hr,INode* inode,ModContext *mc, int flags, int type)
{

	//build our visible face
	int res = 0;

		BitArray visibleFaces;
		mod->BuildVisibleFaces(gw, ld, visibleFaces);
		//hit test our geom edges
		Tab<UVWHitData> hitEdges;
		

		mod->HitGeomEdgeData(ld, hitEdges,gw,  hr);

		res = hitEdges.Count();

		if (type == HITTYPE_POINT)
		{
			int closestIndex = -1;
			float closest=  0.0f;
			Matrix3 toView(1);
			vpt->GetAffineTM(toView);
			Matrix3 mat = inode->GetObjectTM(GetCOREInterface()->GetTime());
			toView = mat * toView;

			for (int hi = 0; hi < hitEdges.Count(); hi++)
			{

				int eindex = hitEdges[hi].index;
				
				int a = ld->GetGeomEdgeVert(eindex,0);//mod->TVMaps.gePtrList[eindex]->a;

				Point3 p = ld->GetGeomVert(a) * toView;//mod->TVMaps.geomPoints[a] * toView;
				if ((p.z > closest) || (closestIndex==-1))
				{
					closest = p.z ;
					closestIndex = hi;
				}

				a = ld->GetGeomEdgeVert(eindex,1);//mod->TVMaps.gePtrList[eindex]->b;

				p = ld->GetGeomVert(a) * toView;//mod->TVMaps.geomPoints[a] * toView;
				if ((p.z > closest) || (closestIndex==-1))
				{
					closest = p.z ;
					closestIndex = hi;
				}
			}
			if (closestIndex != -1)
			{
				Tab<UVWHitData> tempHitEdge;
				tempHitEdge.Append(1,&hitEdges[closestIndex],1);
				hitEdges = tempHitEdge;
			}
		}


		if (ld->mSeamEdges.GetSize() != ld->GetNumberGeomEdges())//mod->TVMaps.gePtrList.Count())
		{
			ld->mSeamEdges.SetSize(ld->GetNumberGeomEdges());//mod->TVMaps.gePtrList.Count());
			ld->mSeamEdges.ClearAll();
		}
		for (int hi = 0; hi < hitEdges.Count(); hi++)
		{
			int i = hitEdges[hi].index;
			//						if (hitEdges[i])
			{
				BOOL visibleFace = FALSE;
				BOOL selFace = TRUE;
				int ct = ld->GetGeomEdgeNumberOfConnectedFaces(i);//mod->TVMaps.gePtrList[i]->faceList.Count();
				for (int j = 0; j < ct; j++)
				{
					int faceIndex = ld->GetGeomEdgeConnectedFace(i,j);//mod->TVMaps.gePtrList[i]->faceList[j];
	//				if ((faceIndex < peltFaces.GetSize()))
	//					selFace = TRUE;
					if ((faceIndex < visibleFaces.GetSize()) && (visibleFaces[faceIndex]))
						visibleFace = TRUE;

				}

				if (mod->fnGetBackFaceCull())
				{
					if (selFace && visibleFace)
					{
						if ( (ld->mSeamEdges[i] && (flags&HIT_SELONLY)) ||
							!(flags&(HIT_UNSELONLY|HIT_SELONLY)))
							vpt->LogHit(inode,mc,0.0f,i,NULL);
						else if ( (!ld->mSeamEdges[i] && (flags&HIT_UNSELONLY)) ||
							!(flags&(HIT_UNSELONLY|HIT_SELONLY)))
							vpt->LogHit(inode,mc,0.0f,i,NULL);

					}
				}
				else
				{
					if (selFace )
					{
						if ( (ld->mSeamEdges[i] && (flags&HIT_SELONLY)) ||
							!(flags&(HIT_UNSELONLY|HIT_SELONLY)))
							vpt->LogHit(inode,mc,0.0f,i,NULL);
						else if ( (!ld->mSeamEdges[i] && (flags&HIT_UNSELONLY)) ||
							!(flags&(HIT_UNSELONLY|HIT_SELONLY)))
							vpt->LogHit(inode,mc,0.0f,i,NULL);

					}
				}
			}
		}


	return res;

}


void PeltData::ResolvePatchHandles(UnwrapMod *mod)
{

	MeshTopoData *ld = mBaseMeshTopoDataCurrent;

	if (ld == NULL) return;
	TimeValue t = GetCOREInterface()->GetTime();

	//loop through our edge pointer list
	for (int i = 0; i < ld->GetNumberTVEdges(); i++)//mod->TVMaps.ePtrList.Count(); i++)
	{
	//find ones with handles 
		int va = ld->GetTVEdgeVec(i,0);//mod->TVMaps.ePtrList[i]->avec;
		int vb = ld->GetTVEdgeVec(i,1);//mod->TVMaps.ePtrList[i]->bvec;
		int a = ld->GetTVEdgeVert(i,0);//mod->TVMaps.ePtrList[i]->a;
		int b = ld->GetTVEdgeVert(i,1);//mod->TVMaps.ePtrList[i]->b;

//make them 1/3 way between the end points
		if (ld->GetTVVertSelected(a) || ld->GetTVVertSelected(b))//mod->vsel[a] || mod->vsel[b])
		{
			Point3 pa,pb;
			
			pa = ld->GetTVVert(a);
			pb = ld->GetTVVert(b);

			Point3 vec = (pb - pa)*0.33f;//(mod->TVMaps.v[b].p - mod->TVMaps.v[a].p)*0.33f;
			if (va != -1)
			{
				ld->SetTVVert(t,va,pa + vec,mod);//mod->TVMaps.v[va].p = mod->TVMaps.v[a].p + vec;
			}
			if (vb != -1)
			{
				ld->SetTVVert(t,vb,pb - vec,mod);//mod->TVMaps.v[vb].p = mod->TVMaps.v[b].p - vec;
			}
		}	
	}

}



BOOL UnwrapMod::fnGetAlwayShowPeltSeams()
{
	return alwaysShowSeams;
}
void UnwrapMod::fnSetAlwayShowPeltSeams(BOOL show)
{
	if (theHold.Holding())
	{
		theHold.Put(new UnwrapSeamAttributesRestore(this));
	}

	alwaysShowSeams = show;
	CheckDlgButton(hParams,IDC_ALWAYSSHOWPELTSEAMS_CHECK,show);
	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());
}

void  UnwrapMod::BuildEdgeDistortionData()
{

	//loop through our geo/uvw edges
	float sumUVLengths = 0;
	float sumGLengths = 0;
	int ect = 0;
	BOOL localDistortion = TRUE;
	TimeValue t = GetCOREInterface()->GetTime();
	pblock->GetValue(unwrap_localDistorion,t,localDistortion,FOREVER);
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		for (int i = 0; i < ld->GetNumberTVEdges(); i++)//TVMaps.ePtrList.Count(); i++)
		{
		//find ones with handles 

			int a = ld->GetTVEdgeVert(i,0);//TVMaps.ePtrList[i]->a;
			int b = ld->GetTVEdgeVert(i,1);//TVMaps.ePtrList[i]->b;

			BOOL useEdge = FALSE;
			if (localDistortion)			
			{
				if (ld->IsTVVertVisible(a) || ld->IsTVVertVisible(b)) 
				{
					useEdge = TRUE;
				}
			}
			else
			{
				useEdge = TRUE;
			}
			if (useEdge)
			{
				int ga = ld->GetTVEdgeGeomVert(i,0);//TVMaps.ePtrList[i]->ga;
				int gb = ld->GetTVEdgeGeomVert(i,1);//TVMaps.ePtrList[i]->gb;

				Point3 pa = ld->GetTVVert(a);//TVMaps.v[a].p;
				Point3 pb = ld->GetTVVert(b);//TVMaps.v[b].p;

				Point3 pga = ld->GetGeomVert(ga);//TVMaps.geomPoints[ga];
				Point3 pgb = ld->GetGeomVert(gb);//TVMaps.geomPoints[gb];

				sumUVLengths += Length(pa-pb);
				sumGLengths += Length(pga-pgb);
				ect++;
			}

		}
	}

	sumUVLengths = sumUVLengths/(float)ect;//TVMaps.ePtrList.Count();
	sumGLengths = sumGLengths/(float)ect;//TVMaps.ePtrList.Count();


	edgeScale = sumUVLengths/sumGLengths;
	//get there average lengths

}


void PeltData::RelaxRig(int iterations, float str, BOOL lockBoundaries, UnwrapMod *mod)
{

	if (mod == NULL) return;
	Tab<Point3> op;

	MeshTopoData *ld = mBaseMeshTopoDataCurrent;

 	op.SetCount(ld->GetNumberTVVerts());//mod->TVMaps.v.Count());
	str = str * 0.05f;
	TimeValue t = GetCOREInterface()->GetTime();

	for (int i = 0; i < iterations; i++)
	{

		for (int j = 0; j < op.Count(); j++)
			op[j] = ld->GetTVVert(j);//mod->TVMaps.v[j].p;
		for (int j = 0; j < rigPoints.Count(); j++)
		{
			int prevID = j - 1;
			int currentID = j;
			int nextID = j + 1;
			if (prevID < 0) prevID = rigPoints.Count()-1;
			if (nextID >= rigPoints.Count()) nextID = 0;


			Point3 pp,cp,np;
			pp = op[rigPoints[prevID].lookupIndex];
			cp = op[rigPoints[currentID].lookupIndex];
			np = op[rigPoints[nextID].lookupIndex];

			Point3 newPoint = (pp + np) *0.5f;
			newPoint = cp + (newPoint - cp) * str;

			int selID = rigPoints[currentID].lookupIndex;

			if (ld->GetTVVertSelected(selID))//mod->vsel[selID])
			{
				if (!lockBoundaries)
				{
					ld->SetTVVert(t,selID,newPoint,mod);//mod->TVMaps.v[selID].p = newPoint;
				}
				else
				{
					if (ld->GetTVVertSelected(rigPoints[prevID].lookupIndex) && //mod->vsel[rigPoints[prevID].lookupIndex] && 
						ld->GetTVVertSelected(rigPoints[nextID].lookupIndex) )//mod->vsel[rigPoints[nextID].lookupIndex])
					{
						ld->SetTVVert(t,selID,newPoint,mod);//mod->TVMaps.v[selID].p = newPoint;
					}
				}
			}
		}
		if (ld->GetUserCancel())
			i = iterations;


	}

	if (mod)
		mod->InvalidateView();

	
/*	for (int i = 0; i < mod->TVMaps.v.Count(); i++)
	{
		if (mod->TVMaps.cont[i]) mod->TVMaps.cont[i]->SetValue(t,&mod->TVMaps.v[i].p);
	}
*/

}
