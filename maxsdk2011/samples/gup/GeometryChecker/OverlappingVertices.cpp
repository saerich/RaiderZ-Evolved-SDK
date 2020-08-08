/*==============================================================================
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Overlapping Vertices Checker
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/
#include<3dsmaxport.h>
#include "checkerimplementations.h"
#include <omp.h>


//default GUP implementation
int GUPChecker::GeometryCheck_FPS(TimeValue t, INode *node, Tab<int> &results)
{
	IGeometryChecker::ReturnVal val;
	IGeometryChecker::OutputVal oval;
	val = GeometryCheck(t,node,oval);
	results =  oval.mIndex;
	return  (int)val;
}


OverlappingVerticesChecker OverlappingVerticesChecker::_theInstance(OVERLAPPING_VERTICES_INTERFACE, _T("OverlappingVertices"), 0, NULL, FP_CORE,
	GUPChecker::kGeometryCheck, _T("Check"), 0,TYPE_ENUM, GUPChecker::kOutputEnum,0,3,
		_T("time"), 0, TYPE_TIMEVALUE,
		_T("nodeToCheck"), 0, TYPE_INODE,
		_T("results"), 0, TYPE_INDEX_TAB_BR,
	GUPChecker::kHasPropertyDlg, _T("hasPropertyDlg"),0,TYPE_bool,0,0,
	GUPChecker::kShowPropertyDlg, _T("showPropertyDlg"),0,TYPE_VOID,0,0,
    properties,
		OverlappingVerticesChecker::kGetTolerance,  OverlappingVerticesChecker::kSetTolerance, _T("tolerance"), 0,TYPE_FLOAT,
	enums,
		GUPChecker::kOutputEnum, 4,	
		_T("Failed"), GUPChecker::kFailed,
		_T("Vertices"), GUPChecker::kVertices,
		_T("Edges"), GUPChecker::kEdges,
		_T("Faces"), GUPChecker::kFaces,
	end	
																			
  );





class OverlappingVerticesClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) {return &OverlappingVerticesChecker::_theInstance; }
	const TCHAR *	ClassName() { return GetString(IDS_OVERLAPPING_VERTICES); }
	SClass_ID		SuperClassID() { return GUP_CLASS_ID; }
	Class_ID		ClassID() { return OVERLAPPING_VERTICES_CLASS_ID; }
	const TCHAR* 	Category() { return _T(""); }

	const TCHAR*	InternalName() { return _T("OverlappingVerticesClass"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};

static  OverlappingVerticesClassDesc classDesc;
ClassDesc2* GetOverlappingVerticesClassDesc() { return &classDesc; }

void setupSpin(HWND hDlg, int spinID, int editID, float val)
{
	ISpinnerControl *spin;

	spin = GetISpinner(GetDlgItem(hDlg, spinID));
	spin->SetLimits( (float)0.0, (float)10000000, FALSE );
	spin->SetScale( 0.1f );
	spin->SetAutoScale(TRUE);
	spin->LinkToEdit( GetDlgItem(hDlg, editID), EDITTYPE_POS_UNIVERSE );
	spin->SetValue( val, FALSE );
	ReleaseISpinner( spin );
}

float getSpinVal(HWND hDlg, int spinID)
{
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hDlg, spinID));
	float res = spin->GetFVal();
	ReleaseISpinner(spin);
	return res;
}

void PositionWindowNearCursor(HWND hwnd)
{
    RECT rect;
    POINT cur;
    GetWindowRect(hwnd,&rect);
    GetCursorPos(&cur);
	int scrwid = GetScreenWidth();
	int scrhgt = GetScreenHeight();
    int winwid = rect.right - rect.left;
    int winhgt = rect.bottom - rect.top;
    cur.x -= winwid/3; // 1/3 of the window should be to the left of the cursor
    cur.y -= winhgt/3; // 1/3 of the window should be above the cursor
    if (cur.x + winwid > scrwid - 10) cur.x = scrwid - winwid - 10; // check too far right 
	if (cur.x < 10) cur.x = 10; // check too far left
    if (cur.y + winhgt> scrhgt - 50) cur.y = scrhgt - 50 - winhgt; // check if too low
	if (cur.y < 10) cur.y = 10; // check for too high s45
    MoveWindow(hwnd, cur.x, cur.y, rect.right - rect.left, rect.bottom - rect.top, TRUE);
}

static INT_PTR CALLBACK OverlappingVerticesDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	OverlappingVerticesChecker *ov = DLGetWindowLongPtr<OverlappingVerticesChecker *>(hDlg);

	switch (msg) {
	case WM_INITDIALOG:
		PositionWindowNearCursor(hDlg);
		DLSetWindowLongPtr(hDlg, lParam);
		ov = (OverlappingVerticesChecker *)lParam;
		setupSpin(hDlg, IDC_TOLERANCE_SPIN, IDC_TOLERANCE, ov->GetTolerance());
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
            goto done;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			return FALSE;
		}
		break;

	case WM_CLOSE:
done:
		theHold.Begin();
		ov->SetTolerance(getSpinVal(hDlg, IDC_TOLERANCE_SPIN));
		theHold.Accept(GetString(IDS_PARAMETER_CHANGED));

		EndDialog(hDlg, TRUE);
		return FALSE;
	}
	return FALSE;
}


void OverlappingVerticesChecker::ShowPropertyDlg()
{
	HWND hDlg = GetCOREInterface()->GetMAXHWnd();
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_OVERLAPPING_VERTICES),
				hDlg, OverlappingVerticesDlgProc, (LPARAM)this);


}

bool OverlappingVerticesChecker::IsSupported(INode* node)
{
	if(node)
	{
		ObjectState os  = node->EvalWorldState(GetCOREInterface()->GetTime());
		Object *obj = os.obj;
		if(obj&&obj->IsSubClassOf(triObjectClassID)||
			obj->IsSubClassOf(polyObjectClassID))
			return true;
	}
	return false;
}
IGeometryChecker::ReturnVal OverlappingVerticesChecker::TypeReturned()
{
	return IGeometryChecker::eVertices;
}
IGeometryChecker::ReturnVal OverlappingVerticesChecker::GeometryCheck(TimeValue t,INode *nodeToCheck, IGeometryChecker::OutputVal &val)
{
	val.mIndex.ZeroCount();
	if(IsSupported(nodeToCheck))
	{
		LARGE_INTEGER	ups,startTime;
		QueryPerformanceFrequency(&ups);
		QueryPerformanceCounter(&startTime); //used to see if we need to pop up a dialog 
		bool checkTime = GetIGeometryCheckerManager()->GetAutoUpdate();//only check time for the dialog if auto update is active!
		ObjectState os  = nodeToCheck->EvalWorldState(t);
		Object *obj = os.obj;
		BitArray arrayOfVerts; //we fill this up with the verts, if not empty then we have isolated vets
		Interface *ip = GetCOREInterface();
		TCHAR buf[256];
		TCHAR tmpBuf[64];
		_tcscpy(tmpBuf,GetString(IDS_CHECK_TIME_USED));
		_tcscpy(buf,GetString(IDS_RUNNING_GEOMETRY_CHECKER));
		if(os.obj->IsSubClassOf(triObjectClassID))
		{
			TriObject *tri = dynamic_cast<TriObject *>(os.obj);
			if(tri)
			{
				ip->PushPrompt(buf);
				Mesh mesh = tri->GetMesh();
				arrayOfVerts.SetSize(mesh.numVerts);
				arrayOfVerts.ClearAll();
				float tolSquared = GetTolerance()*GetTolerance();
				int total = 0;
				EscapeChecker::theEscapeChecker.Setup();
				for (int i=0; i<mesh.numVerts; i++)
				{
					#pragma omp parallel for 
					for(int j=i+1;j<mesh.numVerts;++j)
					{
						Point3 diff(mesh.verts[i] - mesh.verts[j]);
						if(diff.LengthSquared()<tolSquared)
						{
							#pragma omp critical
							{
								arrayOfVerts.Set(i);
								arrayOfVerts.Set(j);
							}
						}
					}
                    total += 1;
					
					LARGE_INTEGER endTime;
					QueryPerformanceCounter(&endTime);
					float sec = (float)((double)(endTime.QuadPart-startTime.QuadPart)/(double)ups.QuadPart);
					if(sec>ESC_TIME)
					{
						if(EscapeChecker::theEscapeChecker.Check())
						{
							_tcscpy(buf,GetString(IDS_EXITING_GEOMETRY_CHECKER));
							ip->ReplacePrompt(buf);
							break;
						}
					}
					if((total%100)==0)
					{
						{
							float percent = (float)total/(float)mesh.numVerts*100.0f;;
							_stprintf(buf,tmpBuf,percent);
							ip->ReplacePrompt(buf);
						}     
					}
					
					if(checkTime==true)
					{
						bool compute = true;
						DialogChecker::Check(checkTime,compute,mesh.numFaces,startTime,ups);
						if(compute==false)
							break;
					}
				}
				ip->PopPrompt();
			}
			else return IGeometryChecker::eFail;
		}
		else if(os.obj->IsSubClassOf(polyObjectClassID))
		{
			PolyObject *poly = dynamic_cast<PolyObject *>(os.obj);
			if(poly)
			{
				ip->PushPrompt(buf);
				MNMesh mnMesh = poly->GetMesh();
				arrayOfVerts.SetSize(mnMesh.numv);
				arrayOfVerts.ClearAll();
				float tolSquared = GetTolerance()*GetTolerance();
				int total = 0;
				EscapeChecker::theEscapeChecker.Setup();
				for (int i=0; i<mnMesh.numv; i++)
				{
					if(mnMesh.v[i].GetFlag(MN_DEAD)==0)
					{
						#pragma omp parallel for 
						for(int j=i+1;j<mnMesh.numv;++j)
						{
							if(mnMesh.v[j].GetFlag(MN_DEAD)==0)
							{
								Point3 diff(mnMesh.v[i].p - mnMesh.v[j].p);
								if(diff.LengthSquared()<tolSquared)
								{
									#pragma omp critical
									{
										arrayOfVerts.Set(i);
										arrayOfVerts.Set(j);
									}
								}
							}
						}
					}
                    total += 1;

					LARGE_INTEGER endTime;
					QueryPerformanceCounter(&endTime);
					float sec = (float)((double)(endTime.QuadPart-startTime.QuadPart)/(double)ups.QuadPart);
					if(sec>ESC_TIME)
					{
						if(EscapeChecker::theEscapeChecker.Check())
						{
							_tcscpy(buf,GetString(IDS_EXITING_GEOMETRY_CHECKER));
							ip->ReplacePrompt(buf);
							break;
						}
					}
					if((total%100)==0)
					{
						float percent = (float)total/(float)mnMesh.numv*100.0f;
						_stprintf(buf,tmpBuf,percent);
						ip->ReplacePrompt(buf);
					}
					if(checkTime==true)
					{
						bool compute = true;
						DialogChecker::Check(checkTime,compute,mnMesh.numf,startTime,ups);
						if(compute==false)
							break;
					}
				}
				ip->PopPrompt();
			}
			else return IGeometryChecker::eFail;
		}
		if(arrayOfVerts.IsEmpty()==false) //we have an isolated vierts
		{
			int localsize= arrayOfVerts.GetSize();
			for(int i=0;i<localsize;++i)
			{
				if(arrayOfVerts[i])
					val.mIndex.Append(1,&i);
			}
		}
		return TypeReturned();
	}
	return IGeometryChecker::eFail;
}

MSTR OverlappingVerticesChecker::GetName()
{
	return MSTR(_T(GetString(IDS_OVERLAPPING_VERTICES)));
}