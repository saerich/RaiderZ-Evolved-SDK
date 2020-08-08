#include "unwrap.h"

#include "3dsmaxport.h"


/*

find the best guess edge lengths


*/

int UnwrapMod::fnGetRelaxBySpringIteration()
{
	return relaxBySpringIteration;
}
void UnwrapMod::fnSetRelaxBySpringIteration(int iteration)
{
	relaxBySpringIteration = iteration;
	if (relaxBySpringIteration < 1) relaxBySpringIteration = 1;
}

float UnwrapMod::fnGetRelaxBySpringStretch()
{
	return relaxBySpringStretch;
}
void UnwrapMod::fnSetRelaxBySpringStretch(float stretch)
{
	relaxBySpringStretch = stretch;
}

BOOL UnwrapMod::fnGetRelaxBySpringVEdges()
{
	return relaxBySpringUseOnlyVEdges;
}
void UnwrapMod::fnSetRelaxBySpringVEdges(BOOL useVEdges)
{
	relaxBySpringUseOnlyVEdges = useVEdges;
}


void UnwrapMod::fnRelaxByFaceAngleNoDialog(int frames, float stretch, float str, BOOL lockOuterVerts)
{
	fnRelaxByFaceAngle(frames, stretch, str, lockOuterVerts);
}
void UnwrapMod::fnRelaxByEdgeAngleNoDialog(int frames, float stretch,  float str, BOOL lockOuterVerts)
{
	fnRelaxByEdgeAngle(frames, stretch, str, lockOuterVerts);
}


void UnwrapMod::fnRelaxByFaceAngle(int frames, float stretch, float str, BOOL lockEdges,HWND statusHWND)

{

	bool anythingSelected = AnyThingSelected();
	BOOL applyToAll = TRUE;
	if (anythingSelected)
		applyToAll = FALSE;

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		mMeshTopoData[ldID]->RelaxByFaceAngle(fnGetTVSubMode(), frames, stretch, str, lockEdges, statusHWND, this,applyToAll);
		if (mMeshTopoData[ldID]->GetUserCancel())
			ldID = mMeshTopoData.Count();
	}

	if (statusHWND)
	{
		TSTR progress;
		progress.printf("    ");
		SetWindowText(statusHWND,progress);

		NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
		InvalidateView();
		if (ip) ip->RedrawViews(ip->GetTime());
	}
}

void UnwrapMod::fnRelaxByEdgeAngle(int frames, float stretch,float str, BOOL lockEdges,HWND statusHWND)

{

	bool anythingSelected = AnyThingSelected();
	BOOL applyToAll = TRUE;
	if (anythingSelected)
		applyToAll = FALSE;

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		mMeshTopoData[ldID]->RelaxByEdgeAngle(fnGetTVSubMode(), frames, stretch, str, lockEdges, statusHWND, this,applyToAll);
		if (mMeshTopoData[ldID]->GetUserCancel())
			ldID = mMeshTopoData.Count();
	}

	if (statusHWND)
	{
		TSTR progress;
		progress.printf("    ");
		SetWindowText(statusHWND,progress);

		NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
		InvalidateView();
		if (ip) ip->RedrawViews(ip->GetTime());
	}

}


void UnwrapMod::fnRelaxBySprings(int frames, float stretch, BOOL lockEdges)

{
//    int frames = 10;
 //	float stretch = 0.87f;

	BOOL accept = FALSE;
	if (!theHold.Holding())
	{
		theHold.Begin();
		HoldPoints();	
		accept = TRUE;
	}

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		mMeshTopoData[ldID]->RelaxBySprings(TVSubObjectMode, frames, stretch, lockEdges, this);
	}

	if (accept)
		theHold.Accept(_T(GetString(IDS_PW_MOVE_UVW)));

	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	InvalidateView();
	if (ip) ip->RedrawViews(ip->GetTime());

}

void	UnwrapMod::SetRelaxBySpringDialogPos()
	{
	if (relaxWindowPos.length != 0) 
		SetWindowPlacement(relaxBySpringHWND,&relaxWindowPos);
	}

void	UnwrapMod::SaveRelaxBySpringDialogPos()
	{
	relaxWindowPos.length = sizeof(WINDOWPLACEMENT); 
	GetWindowPlacement(relaxBySpringHWND,&relaxWindowPos);
	}





INT_PTR CALLBACK UnwrapRelaxBySpringFloaterDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	UnwrapMod *mod = DLGetWindowLongPtr<UnwrapMod*>(hWnd);
	//POINTS p = MAKEPOINTS(lParam);	commented out by sca 10/7/98 -- causing warning since unused.
	
	static ISpinnerControl *iStretch = NULL;
	static ISpinnerControl *iIterations = NULL;

	static float stretch = 0.0f;
	static int iterations = 10;
	static BOOL useOnlyVEdges = FALSE;

	switch (msg) {
		case WM_INITDIALOG:
			{


			mod = (UnwrapMod*)lParam;
			mod->relaxBySpringHWND = hWnd;

			DLSetWindowLongPtr(hWnd, lParam);

//save our points

//create relax amount spinner and set value
			iStretch = SetupFloatSpinner(
				hWnd,IDC_RELAX_STRETCHSPIN,IDC_RELAX_STRETCH,
				0.0f,1.0f,mod->fnGetRelaxBySpringStretch());	
			iStretch->SetScale(0.01f);
			stretch = mod->fnGetRelaxBySpringStretch();

			iIterations = SetupIntSpinner(
				hWnd,IDC_RELAX_ITERATIONSSPIN,IDC_RELAX_ITERATIONS,
				0,1000,mod->fnGetRelaxBySpringIteration());	
			iIterations->SetScale(1.f);
			iterations = mod->fnGetRelaxBySpringIteration();

			
//restore window pos
			mod->SetRelaxBySpringDialogPos();
//start the hold begin
			if (!theHold.Holding())
				{
				theHold.SuperBegin();
				theHold.Begin();
				}
//hold the points 
			mod->HoldPoints();

			float str = 1.0f - (stretch*0.01f);
			mod->fnRelaxBySprings(iterations, str,mod->fnGetRelaxBySpringVEdges());
//stitch initial selection
//			mod->RelaxVerts2(amount,iterations,bBoundary,bCorner);

			mod->InvalidateView();
			TimeValue t = GetCOREInterface()->GetTime();
			mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(t);

			break;
			}



		case CC_SPINNER_CHANGE:
			if ( (LOWORD(wParam) == IDC_RELAX_STRETCHSPIN) ||
				 (LOWORD(wParam) == IDC_RELAX_ITERATIONSSPIN) )
				{
				stretch = iStretch->GetFVal();
				iterations = iIterations->GetIVal();
				theHold.Restore();

				float str = 1.0f - (stretch);
				mod->fnRelaxBySprings(iterations, str,mod->fnGetRelaxBySpringVEdges());

//				mod->RelaxVerts2(amount,iterations,bBoundary,bCorner);

				mod->InvalidateView();
				TimeValue t = GetCOREInterface()->GetTime();
				mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
				GetCOREInterface()->RedrawViews(t);
				}
			break;

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			if ( (LOWORD(wParam) == IDC_RELAX_STRETCHSPIN) || (LOWORD(wParam) == IDC_RELAX_ITERATIONSSPIN) )
				{
				mod->InvalidateView();
				TimeValue t = GetCOREInterface()->GetTime();
				mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
				GetCOREInterface()->RedrawViews(t);

				}
			break;


		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_APPLY:
					{
					theHold.Accept(_T(GetString(IDS_PW_RELAX)));
					theHold.SuperAccept(_T(GetString(IDS_PW_RELAX)));

					theHold.SuperBegin();
					theHold.Begin();
					mod->HoldPoints();

					float str = 1.0f - (stretch*0.01f);
					mod->fnRelaxBySprings(iterations, str,mod->fnGetRelaxBySpringVEdges());

//					mod->RelaxVerts2(amount,iterations,bBoundary,bCorner);
					mod->InvalidateView();
					TimeValue t = GetCOREInterface()->GetTime();
					mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
					GetCOREInterface()->RedrawViews(t);

					break;
					}
				case IDC_APPLYOK:
					{
					theHold.Accept(_T(GetString(IDS_PW_RELAX)));
					theHold.SuperAccept(_T(GetString(IDS_PW_RELAX)));
					mod->SaveRelaxBySpringDialogPos();

					ReleaseISpinner(iStretch);
					iStretch = NULL;
					ReleaseISpinner(iIterations);
					iIterations = NULL;

					 EndDialog(hWnd,1);
					
					break;
					}
				case IDC_REVERT:
					{
					theHold.Restore();
					theHold.Cancel();
					theHold.SuperCancel();
				
					mod->InvalidateView();

					ReleaseISpinner(iStretch);
					iStretch = NULL;
					ReleaseISpinner(iIterations);
					iIterations = NULL;
					EndDialog(hWnd,0);

					break;
					}
				case IDC_DEFAULT:
					{
//get bias
					stretch = iStretch->GetFVal();
					iterations = iIterations->GetIVal();

					mod->fnSetRelaxBySpringStretch(stretch);
					mod->fnSetRelaxBySpringIteration(iterations);

					break;
					}

				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}


void UnwrapMod::fnRelaxBySpringsDialog()
{

//bring up the dialog
	DialogBoxParam(	hInstance,
							MAKEINTRESOURCE(IDD_RELAXBYSPRINGDIALOG),
							GetCOREInterface()->GetMAXHWnd(),
//							hWnd,
							UnwrapRelaxBySpringFloaterDlgProc,
							(LPARAM)this );


}