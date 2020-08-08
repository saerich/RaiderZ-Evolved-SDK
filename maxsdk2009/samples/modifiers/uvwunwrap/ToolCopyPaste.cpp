#include "unwrap.h"
/***************************************************************
****************************************************************/
//CopyPasteBuffer    UnwrapMod::copyPasteBuffer;


CopyPasteBuffer::~CopyPasteBuffer()
	{
	for (int i =0; i < faceData.Count(); i++)
		delete faceData[i];
	}


BOOL CopyPasteBuffer::CanPaste()
	{
	if (faceData.Count() == 0) return FALSE;
		return TRUE;
	}
BOOL CopyPasteBuffer::CanPasteInstance(UnwrapMod *mod)
	{
	if (faceData.Count() == 0) return FALSE;
	if (this->mod != mod) return FALSE;
	return TRUE;
	}

void	UnwrapMod::fnCopy()
{
	int ct = 0;
	int currentLDID = -1;
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		BitArray fsel = mMeshTopoData[ldID]->GetFaceSelection();
		if (fsel.NumberSet())
		{
			currentLDID = ldID;
			ct++;
		}
	}
	

	if (ct > 1)
	{
//pop warnign message
		TSTR error,msg;
		error.printf("%s",GetString(IDS_PW_ERROR));
		msg.printf("%s",GetString(IDS_PW_COPYERROR));
		MessageBox(	NULL,msg,error,MB_OK);
		return;
	}
	//check for type


	if ((ct == 0) || (currentLDID==-1))
		return;

	copyPasteBuffer.iRotate = 0;

	
	copyPasteBuffer.copyType = 2;


	//make sure we only have one faces off of one local data selected

	MeshTopoData *ld = mMeshTopoData[currentLDID];

	copyPasteBuffer.mod = this;
	//copy the vertex list over
	copyPasteBuffer.tVertData.SetCount(ld->GetNumberTVVerts());//TVMaps.v.Count());
	for (int i =0; i < ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++)
	{
		copyPasteBuffer.tVertData[i] = ld->GetTVVert(i);//TVMaps.v[i].p;
	}	


//	MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;//copy the face data over that is selected
	copyPasteBuffer.lmd = ld;


	for (int i = 0; i < copyPasteBuffer.faceData.Count(); i++)
	{
		delete copyPasteBuffer.faceData[i];
	}

	BitArray faceSel = ld->GetFaceSelection();
	ct = faceSel.NumberSet();
	if (faceSel.NumberSet() == ld->GetNumberFaces())//TVMaps.f.Count())
		copyPasteBuffer.copyType = 1;
	if (faceSel.NumberSet() == 1)
		copyPasteBuffer.copyType = 0;

	copyPasteBuffer.faceData.SetCount(ct);


	int faceIndex = 0;
	for (int i = 0; i < ld->GetNumberFaces(); i++)//TVMaps.f.Count(); i++)
	{
		if (faceSel[i])
		{
			UVW_TVFaceClass *f = ld->CloneFace(i);//TVMaps.f[i]->Clone();
			copyPasteBuffer.faceData[faceIndex] = f;
			faceIndex++;
		}
	}

}



void	UnwrapMod::fnPaste(BOOL rotate)
	{
	//check for type

	TimeValue t = GetCOREInterface()->GetTime();
	theHold.Begin();
	HoldPointsAndFaces();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];


//check faces if just one selected normal paste with rotates
//or if all faces where selected
//or if first paste

//hold the points and faces


		BitArray holdFaceSel(ld->GetFaceSelection());

		BitArray subFaceSel;
		if ( ip && (ip->GetSubObjectLevel() == 0) )
			{
//convert our current selection into faces			
			if (fnGetTVSubMode() == TVVERTMODE)
				ld->GetFaceSelFromVert(subFaceSel,FALSE);
			else if (fnGetTVSubMode() == TVEDGEMODE)
				{
				BitArray tempVSel;
				ld->GetVertSelFromEdge(tempVSel);
				BitArray vsel = ld->GetTVVertSelection();
				BitArray holdVSel(vsel);
				ld->SetTVVertSelection(tempVSel);//vsel = tempVSel;
				ld->GetFaceSelFromVert(subFaceSel,FALSE);
				ld->SetTVVertSelection(holdVSel);//vsel = holdVSel;

				}
			else 
				{
				subFaceSel = ld->GetFaceSelection();//.SetSize(fsel.GetSize());
				//subFaceSel = fsel;
				}
			}
		else
			{
			if (fnGetTVSubMode() == TVFACEMODE)
				{
				subFaceSel = ld->GetFaceSelection();//.SetSize(fsel.GetSize());
				//subFaceSel = fsel;
				}

			}

		if ( (copyPasteBuffer.copyType == 0) || (copyPasteBuffer.copyType == 1) || (copyPasteBuffer.iRotate==0))
			{
			int copyIndex = 0;
			Tab<int> vertexLookUpList;

			vertexLookUpList.SetCount(copyPasteBuffer.tVertData.Count());

			BitArray faceSel = ld->GetFaceSelection();

			for (int i =0; i < vertexLookUpList.Count(); i++)
				vertexLookUpList[i] = -1;
			if (copyPasteBuffer.copyType == 1)
				copyPasteBuffer.iRotate = 0;
			else 
				{
				if (copyPasteBuffer.lastSel.GetSize() == faceSel.GetSize())
					{
					if (copyPasteBuffer.lastSel == faceSel) 
						{

						if (rotate)
							{
							copyPasteBuffer.iRotate++;
							}
						else copyPasteBuffer.iRotate = 0;

						
						}
					}
				}
			if (copyPasteBuffer.copyType == 2) copyPasteBuffer.iRotate = 0;
			copyPasteBuffer.lastSel = faceSel;

//loop through selected faces
			for (int i =0; i < faceSel.GetSize(); i++)
				{
				if (faceSel[i])
					{
//make sure selected faces count = buffer face
					if (( i < ld->GetNumberFaces()/*TVMaps.f.Count()*/) && (copyIndex < copyPasteBuffer.faceData.Count()))
						{
						int degree = ld->GetFaceDegree(i);
						if (/*TVMaps.f[i]->count*/ degree == copyPasteBuffer.faceData[copyIndex]->count)
							{
//if so	set the face data indices as the same
							for (int j = 0; j < degree/*TVMaps.f[i]->count*/; j++)
								{
							//index into the texture vertlist
							
								int vid = (j + copyPasteBuffer.iRotate)%degree;//TVMaps.f[i]->count;
								int vertexIndex = copyPasteBuffer.faceData[copyIndex]->t[vid];

								if (vertexLookUpList[vertexIndex] == -1)
									{
									Point3 p = copyPasteBuffer.tVertData[vertexIndex];
									ld->AddTVVert(t, p, i, j, this,FALSE);//ld->AddPoint(p, i, j,FALSE);
									vertexLookUpList[vertexIndex] = ld->GetFaceTVVert(i,j);//TVMaps.f[i]->t[j];
									}
								else 
									 ld->SetFaceTVVert(i,j,vertexLookUpList[vertexIndex]);//TVMaps.f[i]->t[j] = vertexLookUpList[vertexIndex];

								if ((ld->GetFaceHasVectors(i)/*TVMaps.f[i]->vecs*/) && (copyPasteBuffer.faceData[copyIndex]->vecs) && (j < 4))
									{
									int hid = (j*2 + (copyPasteBuffer.iRotate*2))%(/*TVMaps.f[i]->count*/degree*2);
									int handleIndex = copyPasteBuffer.faceData[copyIndex]->vecs->handles[hid];
									if ((handleIndex >= 0) && (vertexLookUpList[handleIndex] == -1))
										{
										Point3 p = copyPasteBuffer.tVertData[handleIndex];
										ld->AddTVHandle(t,p, i, j*2,this,FALSE);
										vertexLookUpList[handleIndex] = ld->GetFaceTVHandle(i,j*2);// TVMaps.f[i]->vecs->handles[j*2];
										}
									else 
										ld->SetFaceTVHandle(i,j*2,vertexLookUpList[handleIndex]);//TVMaps.f[i]->vecs->handles[j*2] = vertexLookUpList[handleIndex];
	
									hid = (j*2 + (copyPasteBuffer.iRotate*2))%(degree/*TVMaps.f[i]->count*/*2)+1;
									handleIndex = copyPasteBuffer.faceData[copyIndex]->vecs->handles[hid];
									if ((handleIndex >= 0) && (vertexLookUpList[handleIndex] == -1))
										{
										Point3 p = copyPasteBuffer.tVertData[handleIndex];
										ld->AddTVHandle(t,p, i, j*2+1,this,FALSE);
										vertexLookUpList[handleIndex] = ld->GetFaceTVHandle(i,j*2+1);//TVMaps.f[i]->vecs->handles[j*2+1];
										}
									else 
										ld->SetFaceTVHandle(i,j*2+1,vertexLookUpList[handleIndex]);//TVMaps.f[i]->vecs->handles[j*2+1] = vertexLookUpList[handleIndex];
	
									int iid = (j + (copyPasteBuffer.iRotate))%degree;//(TVMaps.f[i]->count);
									int interiorIndex = copyPasteBuffer.faceData[copyIndex]->vecs->interiors[iid];
									if ((interiorIndex >= 0) && (vertexLookUpList[interiorIndex] == -1))
										{
										Point3 p = copyPasteBuffer.tVertData[interiorIndex];
										ld->AddTVInterior(t,p, i, j,this,FALSE);
										vertexLookUpList[interiorIndex] = ld->GetFaceTVInterior(i,j);//TVMaps.f[i]->vecs->handles[j];
										}
									else 
										ld->SetFaceTVInterior(i,j,vertexLookUpList[interiorIndex]);//TVMaps.f[i]->vecs->interiors[j] = vertexLookUpList[interiorIndex];
									}
								}
							copyIndex++;
							if (copyIndex >= copyPasteBuffer.faceData.Count()) copyIndex = 0;
							}
						}	
					}

				}

			}
		
		ld->SetTVEdgeInvalid();//RebuildEdges();

		if ( ip && (ip->GetSubObjectLevel() == 0) )
			{
			if (fnGetTVSubMode() == TVVERTMODE)
				{
				BitArray fsel = ld->GetFaceSelection();
				BitArray vsel = ld->GetTVVertSelection();
				BitArray holdFSel(fsel);
				fsel = subFaceSel;
				ld->GetVertSelFromFace(vsel);
				ld->SetTVVertSelection(vsel);
				ld->SetFaceSelection(holdFSel);//fsel = holdFSel;
				}
			else if (fnGetTVSubMode() == TVEDGEMODE)
				{
				BitArray fsel = ld->GetFaceSelection();
				BitArray esel = ld->GetTVEdgeSelection();
				BitArray holdFSel(fsel);
				fsel = subFaceSel;
				ld->GetVertSelFromFace(fsel);
				ld->SetFaceSelection(fsel);//
				ld->GetEdgeSelFromVert(esel,FALSE);
				ld->SetTVEdgeSelection(esel);
				ld->SetFaceSelection(holdFSel);//fsel = holdFSel;
				}
			else 
				{				
				ld->SetFaceSelection(subFaceSel);//fsel = subFaceSel;
				}

			}
		else
			{
//			md->faceSel = holdFaceSel;
			ld->SetFaceSelection(subFaceSel);
/*
			if (fnGetTVSubMode() == TVFACEMODE)
				{
				ld->SetFaceSelection(subFaceSel);//fsel = subFaceSel;
				}
*/
			}

		}
		CleanUpDeadVertices();
		theHold.Accept(_T(GetString(IDS_PW_PASTE)));

		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		InvalidateView();
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());		

	}

	void	UnwrapMod::fnPasteInstance()
	{
		//make sure mods are the same
		theHold.Begin();
		HoldPointsAndFaces();

		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			if ((this == copyPasteBuffer.mod) && (ld == copyPasteBuffer.lmd))
			{

				BitArray faceSel = ld->GetFaceSelection();

				//loop through selected faces
				int copyIndex = 0;
				for (int i =0; i < faceSel.GetSize(); i++)
				{
					if (faceSel[i])
					{
						//make sure selected faces count = buffer face
						if (( i < ld->GetNumberFaces()/*TVMaps.f.Count()*/) && (copyIndex < copyPasteBuffer.faceData.Count()))
						{
							int degree = ld->GetFaceDegree(i);
							if (degree == copyPasteBuffer.faceData[copyIndex]->count)
							{
								//if so set the face data indices as the same
								for (int j = 0; j < degree; j++)
								{
									//index into the texture vertlist
									ld->SetFaceTVVert(i,j,copyPasteBuffer.faceData[copyIndex]->t[j]);//TVMaps.f[i]->t[j] = copyPasteBuffer.faceData[copyIndex]->t[j];
									//index into the geometric vertlist
									if ((ld->GetFaceHasVectors(i)/*TVMaps.f[i]->vecs*/) && (j < 4))
									{
										ld->SetFaceTVInterior(i,j,copyPasteBuffer.faceData[copyIndex]->vecs->interiors[j]);//TVMaps.f[i]->vecs->interiors[j] = copyPasteBuffer.faceData[copyIndex]->vecs->interiors[j];

										ld->SetFaceTVHandle(i,j*2,copyPasteBuffer.faceData[copyIndex]->vecs->handles[j*2]);//TVMaps.f[i]->vecs->handles[j*2] = copyPasteBuffer.faceData[copyIndex]->vecs->handles[j*2];
										ld->SetFaceTVHandle(i,j*2+1,copyPasteBuffer.faceData[copyIndex]->vecs->handles[j*2+1]);//TVMaps.f[i]->vecs->handles[j*2+1] = copyPasteBuffer.faceData[copyIndex]->vecs->handles[j*2+1];

									}
								}
								copyIndex++;
							}
						}
					}
				}				
				ld->SetTVEdgeInvalid();

			}

		}
		CleanUpDeadVertices();
		theHold.Accept(_T(GetString(IDS_PW_PASTE)));
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		InvalidateView();
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

	}



	void	UnwrapMod::CleanUpDeadVertices()
	{

		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			BitArray usedList;


			usedList.SetSize(ld->GetNumberTVVerts());//TVMaps.v.Count());
			usedList.ClearAll();

			for (int i = 0; i < ld->GetNumberFaces(); i++)//TVMaps.f.Count(); i++)
			{
				if (!ld->GetFaceDead(i))
				{			
					int degree = ld->GetFaceDegree(i);
					for (int j = 0; j < degree; j++)
					{
						int vertIndex = ld->GetFaceTVVert(i,j);//TVMaps.f[i]->t[j];
						usedList.Set(vertIndex);

						//index into the geometric vertlist
						if ((ld->GetFaceHasVectors(i)/*TVMaps.f[i]->vecs*/) && (j < 4))
						{
							vertIndex = ld->GetFaceTVInterior(i,j);//TVMaps.f[i]->vecs->interiors[j];
							if ((vertIndex>=0) && (vertIndex < usedList.GetSize()))
								usedList.Set(vertIndex);

							vertIndex = ld->GetFaceTVHandle(i,j*2);//TVMaps.f[i]->vecs->handles[j*2];
							if ((vertIndex>=0) && (vertIndex < usedList.GetSize()))
								usedList.Set(vertIndex);
							vertIndex = ld->GetFaceTVHandle(i,j*2+1);//TVMaps.f[i]->vecs->handles[j*2+1];
							if ((vertIndex>=0) && (vertIndex < usedList.GetSize()))
								usedList.Set(vertIndex);
						}

					}
				}
			}
			int vTotalCount = ld->GetNumberTVVerts();//TVMaps.v.Count();
			int vInitalDeadCount = 0;
			int vFinalDeadCount = 0;

			for (int i =0; i < ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++)
			{
				if (ld->GetTVVertDead(i))//TVMaps.v[i].flags & FLAG_DEAD)
					vInitalDeadCount++;
			}

			for (int i =0; i < usedList.GetSize(); i++)
			{
				BOOL isRigPoint = ld->GetTVVertFlag(i) & FLAG_RIGPOINT;
				if (!usedList[i] && (!isRigPoint))
				{
					ld->DeleteTVVert(i,this);
//					TVMaps.v[i].flags |= FLAG_DEAD;
				}
			}

			for (int i =0; i < ld->GetNumberTVVerts(); i++)
			{
				if (ld->GetTVVertDead(i))//TVMaps.v[i].flags & FLAG_DEAD)
					vFinalDeadCount++;
			}


#ifdef DEBUGMODE 

			if (gDebugLevel >= 3)
				ScriptPrint("Cleaning Dead Verts Total Verts %d Initial Dead Verts %d Final Dead Verts %d \n",vTotalCount,vInitalDeadCount,vFinalDeadCount); 

#endif
		}
	}



