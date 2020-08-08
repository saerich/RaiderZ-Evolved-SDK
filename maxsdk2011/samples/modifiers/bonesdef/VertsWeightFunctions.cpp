/**********************************************************************

FILE: VertsWeightFunctions.cpp

DESCRIPTION:  Contains functions that set vertex weights

CREATED BY: Peter Watje

HISTORY: 12/12/01


*>	Copyright (c) 1998, All Rights Reserved.


**********************************************************************/
#include "max.h"
#include "mods.h"
#include "iparamm.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "linshape.h"
#include "surf_api.h"
#include "polyobj.h"

// This uses the linked-list class templates
#include "linklist.h"
#include "bonesdef.h"
#include "macrorec.h"
#include "modstack.h"
#include "ISkin.h"
#include "MaxIcon.h"



#ifdef _DEBUG
#undef _DEBUG
#include <atlbase.h>
#define _DEBUG
#else
#include <atlbase.h>
#endif

void WeightTableWindow::UpdateSpinner()
{
	for (int i = 0; i < modDataList.Count(); i++)
	{
		mod->BuildNormalizedWeight(modDataList[i].bmd);
		mod->UpdateEffectSpinner(modDataList[i].bmd);
	}

}

void WeightTableWindow::SetWeight(int currentVert, int currentBone, float value, BOOL update)
{

	if ((currentBone < 0) || (currentBone >= mod->BoneData.Count()))
		return;

	if (currentVert == -1)
	{
		if (mod) mod->weightTableWindow.SetAllWeights( currentBone,value,update);
	}
	else
	{
		//get weight 

		//get index
		BOOL alreadyInList = FALSE;
		int boneID = 0;

		if (currentVert >= vertexPtrList.Count()) return;

		vertexPtrList[currentVert].bmd->rebuildWeights = TRUE;
		VertexListClass *vd = vertexPtrList[currentVert].vertexData;

		//check if modified
		if (!vd->IsModified())
		{
			//normalize out the influence list
			vd->Modified (TRUE);
			//rebalance rest
			vd->NormalizeWeights();
		}

		if (vd->WeightCount()==0)
		{
			VertexInfluenceListClass tempV;
			tempV.Bones = currentBone;
			tempV.Influences = value;

			if (mod->BoneData[currentBone].flags & BONE_SPLINE_FLAG)
			{
				Interval valid;
				Matrix3 ntm = mod->BoneData[currentBone].Node->GetObjTMBeforeWSM(mod->RefFrame,&valid);
				ntm = vertexPtrList[currentVert].bmd->BaseTM * Inverse(ntm);

				float garbage = mod->SplineToPoint(vd->LocalPos,
					&mod->BoneData[currentBone].referenceSpline,
					tempV.u,
					tempV.OPoints,tempV.Tangents,
					tempV.SubCurveIds,tempV.SubSegIds,
					ntm);
			}

			vd->AppendWeight(tempV);
		}
		else
		{
			for (int i =0; i < vd->WeightCount();i++)
			{
				if (vd->GetBoneIndex(i) == currentBone)
				{
					alreadyInList = TRUE;
					boneID = i;
				}
			}
			//if normalize reset rest
			//for right now we will always normalize until we get per verts attributes in
			BOOL normalize = TRUE;
			if (vd->IsUnNormalized())
			{
				if (alreadyInList)
					vd->SetWeight(boneID, value);
				else
				{
					VertexInfluenceListClass tempV;
					tempV.Bones = currentBone;
					tempV.Influences = value;

					if (mod->BoneData[currentBone].flags & BONE_SPLINE_FLAG)
					{
						Interval valid;
						Matrix3 ntm = mod->BoneData[currentBone].Node->GetObjTMBeforeWSM(mod->RefFrame,&valid);
						ntm = vertexPtrList[currentVert].bmd->BaseTM * Inverse(ntm);

						float garbage = mod->SplineToPoint(vd->LocalPos,
							&mod->BoneData[currentBone].referenceSpline,
							tempV.u,
							tempV.OPoints,tempV.Tangents,
							tempV.SubCurveIds,tempV.SubSegIds,
							ntm);
					}

					vd->AppendWeight(tempV);
				}

			}
			else
			{
				if (value >= 1.0f) value = 1.0f;
				if (value < 0.0f) value = 0.0f;
				if (value == 1.0f)
				{
					for (int i=0; i < vd->WeightCount(); i++)
					{
						vd->SetWeight(i, 0.0f);
					}
					if (alreadyInList)
						vd->SetWeight(boneID,1.0f);
					else 
					{
						VertexInfluenceListClass tempV;
						tempV.Bones = currentBone;
						tempV.Influences = 1.0f;

						if (mod->BoneData[currentBone].flags & BONE_SPLINE_FLAG)
						{
							Interval valid;
							Matrix3 ntm = mod->BoneData[currentBone].Node->GetObjTMBeforeWSM(mod->RefFrame,&valid);
							ntm = vertexPtrList[currentVert].bmd->BaseTM * Inverse(ntm);

							float garbage = mod->SplineToPoint(vd->LocalPos,
								&mod->BoneData[currentBone].referenceSpline,
								tempV.u,
								tempV.OPoints,tempV.Tangents,
								tempV.SubCurveIds,tempV.SubSegIds,
								ntm);
						}

						vd->AppendWeight(tempV);
					}
				}
				else if (value == 0.0f)
				{
					if (alreadyInList)
					{
						vd->SetWeight(boneID,0.0f);
						//rebalance rest

						float sum = 0.0f;
						for (int i=0; i < vd->WeightCount(); i++)
							sum += vd->GetWeight(i);

						if (sum != 0.0f)
						{
							for (int i=0; i < vd->WeightCount(); i++)
								vd->SetWeight(i,vd->GetWeight(i)/sum);

						}


					}
				}
				else
				{
					float remainder = 1.0f - value;
					float sum = 0.f;

					if (alreadyInList)
					{
						vd->SetWeight(boneID, 0.0f);
						for (int i=0; i < vd->WeightCount(); i++)
							sum += vd->GetWeight(i);
						if (sum == 0.0f)
							vd->SetWeight(boneID,1.0f);
						else
						{
							float per = remainder/sum;
							for (int i=0; i < vd->WeightCount(); i++)
								vd->SetWeight(i,vd->GetWeight(i) * per);

							vd->SetWeight(boneID,value);
						}

					}
					else
					{
						for (int i=0; i < vd->WeightCount(); i++)
							sum += vd->GetWeight(i);
						if (sum == 0.0f)
						{
							value = 1.0f;
							VertexInfluenceListClass tempV;
							tempV.Bones = currentBone;
							tempV.Influences = value;

							if (mod->BoneData[currentBone].flags & BONE_SPLINE_FLAG)
							{
								Interval valid;
								Matrix3 ntm = mod->BoneData[currentBone].Node->GetObjTMBeforeWSM(mod->RefFrame,&valid);
								ntm = vertexPtrList[currentVert].bmd->BaseTM * Inverse(ntm);

								float garbage = mod->SplineToPoint(vd->LocalPos,
									&mod->BoneData[currentBone].referenceSpline,
									tempV.u,
									tempV.OPoints,tempV.Tangents,
									tempV.SubCurveIds,tempV.SubSegIds,
									ntm);
							}

							vd->AppendWeight(tempV);

						}
						else
						{
							float per = remainder/sum;
							for (int i=0; i < vd->WeightCount(); i++)
								vd->SetWeight(i,vd->GetWeight(i) * per);

							VertexInfluenceListClass tempV;
							tempV.Bones = currentBone;
							tempV.Influences = value;

							if (mod->BoneData[currentBone].flags & BONE_SPLINE_FLAG)
							{
								Interval valid;
								Matrix3 ntm = mod->BoneData[currentBone].Node->GetObjTMBeforeWSM(mod->RefFrame,&valid);
								ntm = vertexPtrList[currentVert].bmd->BaseTM * Inverse(ntm);

								float garbage = mod->SplineToPoint(vd->LocalPos,
									&mod->BoneData[currentBone].referenceSpline,
									tempV.u,
									tempV.OPoints,tempV.Tangents,
									tempV.SubCurveIds,tempV.SubSegIds,
									ntm);
							}

							vd->AppendWeight(tempV);
						}
					}	
				}
			}

		}

	}


	if (update)
	{
		if (mod) mod->weightTableWindow.UpdateSpinner();

		if (mod) mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());	
	}
	BringDownEditField();

}

void WeightTableWindow::SetAllWeights(int currentBone, float value, BOOL update)
{
	//	int currentBone = GetCurrentBone(x,y);
	//get weight 

	//get index
	for (int vIndex = 0; vIndex < vertexPtrList.Count(); vIndex++)
	{
		BOOL alreadyInList = FALSE;
		int boneID = 0;

		vertexPtrList[vIndex].bmd->rebuildWeights = TRUE;
		VertexListClass *vd = vertexPtrList[vIndex].vertexData;

		BOOL process = TRUE;

		if (GetAffectSelectedOnly())
		{
			if (!vertexPtrList[vIndex].IsSelected())
				process = FALSE;
		}

		if (process)
		{
			//check if modified
			if (!vd->IsModified())
			{
				//normalize out the influence list
				vd->Modified (TRUE);
				//rebalance rest
				float sum = 0.0f;
				for (int i=0; i < vd->WeightCount(); i++)
					sum += vd->GetWeight(i);
				for (int i=0; i < vd->WeightCount(); i++)
					vd->SetWeight(i, vd->GetWeight(i)/sum);
			}

			for (int i =0; i < vd->WeightCount();i++)
			{
				if (vd->GetBoneIndex(i) == currentBone)
				{
					alreadyInList = TRUE;
					boneID = i;
				}
			}
			//if normalize reset rest
			//for right now we will always normalize until we get per verts attributes in
			if (vd->IsUnNormalized())
			{
				if (alreadyInList)
					vd->SetWeight(boneID, value);
				else
				{
					VertexInfluenceListClass tempV;
					tempV.Bones = currentBone;
					tempV.Influences = value;

					if (mod->BoneData[currentBone].flags & BONE_SPLINE_FLAG)
					{
						Interval valid;
						Matrix3 ntm = mod->BoneData[currentBone].Node->GetObjTMBeforeWSM(mod->RefFrame,&valid);
						ntm = vertexPtrList[vIndex].bmd->BaseTM * Inverse(ntm);

						float garbage = mod->SplineToPoint(vd->LocalPos,
							&mod->BoneData[currentBone].referenceSpline,
							tempV.u,
							tempV.OPoints,tempV.Tangents,
							tempV.SubCurveIds,tempV.SubSegIds,
							ntm);
					}

					vd->AppendWeight(tempV);
				}
			}
			else
			{
				if (value >= 1.0f) value = 1.0f;
				if (value < 0.0f) value = 0.0f;
				if (value == 1.0f)
				{
					for (int i=0; i < vd->WeightCount(); i++)
					{
						vd->SetWeight(i, 0.0f);
					}
					if (alreadyInList)
						vd->SetWeight(boneID, 1.0f);
					else 
					{
						VertexInfluenceListClass tempV;
						tempV.Bones = currentBone;
						tempV.Influences = 1.0f;

						if (mod->BoneData[currentBone].flags & BONE_SPLINE_FLAG)
						{
							Interval valid;
							Matrix3 ntm = mod->BoneData[currentBone].Node->GetObjTMBeforeWSM(mod->RefFrame,&valid);
							ntm = vertexPtrList[vIndex].bmd->BaseTM * Inverse(ntm);

							float garbage = mod->SplineToPoint(vd->LocalPos,
								&mod->BoneData[currentBone].referenceSpline,
								tempV.u,
								tempV.OPoints,tempV.Tangents,
								tempV.SubCurveIds,tempV.SubSegIds,
								ntm);
						}

						vd->AppendWeight(tempV);
					}
				}
				else if (value == 0.0f)
				{
					if (alreadyInList)
					{
						vd->SetWeight(boneID, 0.0f);
						//rebalance rest
						float sum = 0.0f;
						for (int i=0; i < vd->WeightCount(); i++)
							sum += vd->GetWeight(i);

						if (sum != 0.0f)
						{
							for (int i=0; i < vd->WeightCount(); i++)
								vd->SetWeight(i, vd->GetWeight(i)/sum);

						}

					}
				}
				else
				{
					float remainder = 1.0f - value;
					float sum = 0.f;

					if (alreadyInList)
					{
						vd->SetWeight(boneID, 0.0f);
						for (int i=0; i < vd->WeightCount(); i++)
							sum += vd->GetWeight(i);
						if (sum == 0.0f)
							vd->SetWeight(boneID, 1.0f);
						else
						{
							float per = remainder/sum;
							for (int i=0; i < vd->WeightCount(); i++)
								vd->SetWeight(i, vd->GetWeight(i) * per);

							vd->SetWeight(boneID, value);
						}

					}
					else
					{
						for (int i=0; i < vd->WeightCount(); i++)
							sum += vd->GetWeight(i);
						if (sum == 0.0f)
						{
							value = 1.0f;
							VertexInfluenceListClass tempV;
							tempV.Bones = currentBone;
							tempV.Influences = value;

							if (mod->BoneData[currentBone].flags & BONE_SPLINE_FLAG)
							{
								Interval valid;
								Matrix3 ntm = mod->BoneData[currentBone].Node->GetObjTMBeforeWSM(mod->RefFrame,&valid);
								ntm = vertexPtrList[vIndex].bmd->BaseTM * Inverse(ntm);

								float garbage = mod->SplineToPoint(vd->LocalPos,
									&mod->BoneData[currentBone].referenceSpline,
									tempV.u,
									tempV.OPoints,tempV.Tangents,
									tempV.SubCurveIds,tempV.SubSegIds,
									ntm);
							}

							vd->AppendWeight(tempV);

						}
						else
						{
							float per = remainder/sum;
							for (int i=0; i < vd->WeightCount(); i++)
								vd->SetWeight(i, vd->GetWeight(i) * per);

							VertexInfluenceListClass tempV;
							tempV.Bones = currentBone;
							tempV.Influences = value;

							if (mod->BoneData[currentBone].flags & BONE_SPLINE_FLAG)
							{
								Interval valid;
								Matrix3 ntm = mod->BoneData[currentBone].Node->GetObjTMBeforeWSM(mod->RefFrame,&valid);
								ntm = vertexPtrList[vIndex].bmd->BaseTM * Inverse(ntm);

								float garbage = mod->SplineToPoint(vd->LocalPos,
									&mod->BoneData[currentBone].referenceSpline,
									tempV.u,
									tempV.OPoints,tempV.Tangents,
									tempV.SubCurveIds,tempV.SubSegIds,
									ntm);
							}

							vd->AppendWeight(tempV);
						}
					}
				}

			}
		}
	}
	if (update)
	{
		if (mod) mod->weightTableWindow.UpdateSpinner();

		if (mod) mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());	
	}
	BringDownEditField();

}


void BonesDefMod::SetVertex(BoneModData *bmd,int vertID, int BoneID, float amount)

{

	if (( BoneID < 0) || (BoneID >= BoneData.Count()))
		return;
	if (BoneData[BoneID].Node == NULL)
		return;

	if (BoneData[BoneID].flags & BONE_LOCK_FLAG)
		return;



	ObjectState os;
	ShapeObject *pathOb = NULL;

	if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
	{
		os = BoneData[BoneID].Node->EvalWorldState(ip->GetTime());
		pathOb = (ShapeObject*)os.obj;
	}

	int k = vertID;

	//bake the closest bone since we are now hand weighting it
	if (bmd->VertexData[k]->GetClosestBone() != -1)
	{
		int closestBoneID = bmd->VertexData[k]->GetClosestBone();
		bmd->VertexData[k]->SetClosestBone(-1);
		VertexInfluenceListClass td;
		td.Bones = closestBoneID;
		td.normalizedInfluences = 1.0f;
		td.Influences = 1.0f;
		bmd->VertexData[k]->AppendWeight(td);
	}

	float effect = 0.0f;
	float originaleffect = 0.0f;
	int found = 0;
	int id = 0;

	//	int k = vsel[i];

	bmd->rebuildWeights = TRUE;
	for (int j =0; j<bmd->VertexData[k]->WeightCount();j++)
	{
		if ( (bmd->VertexData[k]->GetBoneIndex(j) == BoneID)&& (!(BoneData[bmd->VertexData[k]->GetBoneIndex(j)].flags & BONE_LOCK_FLAG)))

		{
			originaleffect = bmd->VertexData[k]->GetNormalizedWeight(j);			
			bmd->VertexData[k]->SetNormalizedWeight(j, amount);
			found = 1;
			id =j;
			effect = bmd->VertexData[k]->GetNormalizedWeight(j);
			j = bmd->VertexData[k]->WeightCount();

		}
	}

	if (bmd->VertexData[k]->IsUnNormalized())
	{
		if (found == 0)
		{
			VertexInfluenceListClass td;
			td.Bones = BoneID;
			td.normalizedInfluences = amount;
			td.Influences = amount;
			bmd->VertexData[k]->AppendWeight(td);
		}
		else
		{
			bmd->VertexData[k]->SetWeight(id, amount);
			bmd->VertexData[k]->SetNormalizedWeight(id,amount);
		}
		return;
	}

	if ((found == 0) && (amount > 0.0f))
	{

		VertexInfluenceListClass td;
		td.Bones = BoneID;
		td.normalizedInfluences = amount;
		//check if spline if so add approriate spline data info also
		// find closest spline

		if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
		{
			Interval valid;
			Matrix3 ntm = BoneData[BoneID].Node->GetObjTMBeforeWSM(RefFrame,&valid);
			ntm = bmd->BaseTM * Inverse(ntm);

			float garbage = SplineToPoint(bmd->VertexData[k]->LocalPos,
				&BoneData[BoneID].referenceSpline,
				td.u,
				td.OPoints,td.Tangents,
				td.SubCurveIds,td.SubSegIds,
				ntm);
		}


		bmd->VertexData[k]->AppendWeight(td);
		effect = amount;
		originaleffect = 0.0f;
		found = 1;
	}

	if (found == 1)
	{
		int bc = bmd->VertexData[k]->WeightCount();




		//rebalance rest
		float remainder = 1.0f - effect;
		originaleffect = 1.0f - originaleffect;
		if (bmd->VertexData[k]->WeightCount() > 1)
		{
			for (int j=0;j<bmd->VertexData[k]->WeightCount();j++)
			{

				if (!(BoneData[bmd->VertexData[k]->GetBoneIndex(j)].flags & BONE_LOCK_FLAG))
				{
					if (bmd->VertexData[k]->GetBoneIndex(j)!=BoneID)
					{
						float per = 0.0f;
						if (remainder == 0.0f)
						{
							bmd->VertexData[k]->SetNormalizedWeight(j, 0.0f);
						}
						else if (originaleffect == 0.0f)
						{
							bmd->VertexData[k]->SetNormalizedWeight(j, remainder/(bmd->VertexData[k]->WeightCount()-1.0f));
						}
						else
						{
							per = bmd->VertexData[k]->GetNormalizedWeight(j)/originaleffect;
							bmd->VertexData[k]->SetNormalizedWeight(j, remainder*per);
						}
					}
				}
			}
			for (int j=0;j<bmd->VertexData[k]->WeightCount();j++)
				bmd->VertexData[k]->SetWeight(j, bmd->VertexData[k]->GetNormalizedWeight(j));
		}
		else
		{
			if (bmd->VertexData[k]->IsUnNormalized())
			{
				bmd->VertexData[k]->SetWeight(0, amount);
				bmd->VertexData[k]->SetNormalizedWeight(0, amount);

			}
			else
			{
				bmd->VertexData[k]->SetWeight(0, 1.0f);
				bmd->VertexData[k]->SetNormalizedWeight(0, 1.0f);
			}
		}




	}
	bmd->VertexData[k]->Modified (TRUE);


	//WEIGHTTABLE
	PaintAttribList();
}



void BonesDefMod::SetVertices(BoneModData *bmd,int vertID, Tab<int> BoneIDList, Tab<float> amountList)

{
	//Tab<int> vsel;


	ObjectState os;
	ShapeObject *pathOb = NULL;



	float effect,originaleffect;
	int k = vertID;
	bmd->rebuildWeights = TRUE;
	for (int i = 0; i < amountList.Count();i++)
	{
		int found = 0;

		float amount = amountList[i];
		int BoneID = BoneIDList[i];
		if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
		{
			os = BoneData[BoneID].Node->EvalWorldState(ip->GetTime());
			pathOb = (ShapeObject*)os.obj;
		}
		int id = 0;
		for (int j =0; j<bmd->VertexData[k]->WeightCount();j++)
		{
			if ( (bmd->VertexData[k]->GetBoneIndex(j) == BoneID)&& (!(BoneData[bmd->VertexData[k]->GetBoneIndex(j)].flags & BONE_LOCK_FLAG)))

			{
				originaleffect = bmd->VertexData[k]->GetWeight(j);
				bmd->VertexData[k]->SetWeight(j, amount);
				found = 1;
				id = j;
				effect = bmd->VertexData[k]->GetWeight(j);
				j = bmd->VertexData[k]->WeightCount();

			}
		}

		if (bmd->VertexData[k]->IsUnNormalized())
		{
			if (found == 0)
			{
				VertexInfluenceListClass td;
				td.Bones = BoneID;
				td.normalizedInfluences = amount;
				td.Influences = amount;
				bmd->VertexData[k]->AppendWeight(td);
			}
			else
			{
				bmd->VertexData[k]->SetNormalizedWeight(id, amount);
				bmd->VertexData[k]->SetWeight(id, amount);
			}

		}
		else
		{
			if ((found == 0) && (amount > 0.0f))
			{

				VertexInfluenceListClass td;
				td.Bones = BoneID;
				td.Influences = amount;
				//check if spline if so add approriate spline data info also
				// find closest spline

				if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
				{
					Interval valid;
					Matrix3 ntm = BoneData[BoneID].Node->GetObjTMBeforeWSM(RefFrame,&valid);
					ntm = bmd->BaseTM * Inverse(ntm);

					float garbage = SplineToPoint(bmd->VertexData[k]->LocalPos,
						&BoneData[BoneID].referenceSpline,
						td.u,
						td.OPoints,td.Tangents,
						td.SubCurveIds,td.SubSegIds,
						ntm);
				}


				bmd->VertexData[k]->AppendWeight(td);
				effect = amount;
				originaleffect = 0.0f;
				found = 1;
			}

			if (found == 1)
			{
				int bc = bmd->VertexData[k]->WeightCount();

				//remove 0 influence bones otherwise they skew the reweigthing
				for (int j=0;j<bmd->VertexData[k]->WeightCount();j++)
				{
					if (bmd->VertexData[k]->GetWeight(j)==0.0f)
					{
						bmd->VertexData[k]->DeleteWeight(j);
						j--;
					}
				}	


				//rebalance rest

			}
		}
	}
	if (!bmd->VertexData[k]->IsUnNormalized())
	{
		bmd->VertexData[k]->Modified (TRUE);
		if (bmd->VertexData[k]->WeightCount() > 1)
		{
			float totalSum = 0.0f;
			for (int j=0;j<bmd->VertexData[k]->WeightCount();j++)
			{
				if (bmd->VertexData[k]->GetWeight(j)==0.0f)
				{
					bmd->VertexData[k]->DeleteWeight(j);
					j--;
				}	
			}

			for (int j=0;j<bmd->VertexData[k]->WeightCount();j++)
				totalSum += bmd->VertexData[k]->GetWeight(j);
			for (int j=0;j<bmd->VertexData[k]->WeightCount();j++)
			{
				bmd->VertexData[k]->SetWeight(j, bmd->VertexData[k]->GetWeight(j)/totalSum);
			}
		}
	}

	//WEIGHTTABLE
	PaintAttribList();

}




void BonesDefMod::SetSelectedVertices(BoneModData *bmd, int BoneID, float amount)

{
	Tab<int> vsel;

	if (( BoneID < 0) || (BoneID >= BoneData.Count()))
		return;
	if (BoneData[BoneID].Node == NULL)
		return;

	if (BoneData[BoneID].flags & BONE_LOCK_FLAG)
		return;

	ObjectState os;
	ShapeObject *pathOb = NULL;

	if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
	{
		os = BoneData[BoneID].Node->EvalWorldState(ip->GetTime());
		pathOb = (ShapeObject*)os.obj;
	}


	int selcount = bmd->selected.GetSize();

	for (int i = 0 ; i <bmd->VertexData.Count();i++)
	{
		if (bmd->selected[i]) vsel.Append(1,&i,1);
	}

	//float effect,originaleffect;
	for ( int i = 0; i < vsel.Count();i++)
	{
		int found = 0;

		int k = vsel[i];
		SetVertex(bmd,k, BoneID, amount);
		/*
		for (int j =0; j<bmd->VertexData[k]->d.Count();j++)
		{
		if ( (bmd->VertexData[k]->d[j].Bones == BoneID)&& (!(BoneData[bmd->VertexData[k]->d[j].Bones].flags & BONE_LOCK_FLAG)))

		{
		originaleffect = bmd->VertexData[k]->d[j].normalizedInfluences;
		bmd->VertexData[k]->d[j].normalizedInfluences = amount;
		found = 1;
		effect = bmd->VertexData[k]->d[j].normalizedInfluences;
		j = bmd->VertexData[k]->d.Count();

		}
		}

		if ((found == 0) && (amount > 0.0f))
		{

		VertexInfluenceListClass td;
		td.Bones = BoneID;
		td.normalizedInfluences = amount;
		//check if spline if so add approriate spline data info also
		// find closest spline

		if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
		{
		Interval valid;
		Matrix3 ntm = BoneData[BoneID].Node->GetObjTMBeforeWSM(RefFrame,&valid);
		ntm = bmd->BaseTM * Inverse(ntm);

		float garbage = SplineToPoint(bmd->VertexData[k]->LocalPos,
		&BoneData[BoneID].referenceSpline,
		td.u,
		td.OPoints,td.Tangents,
		td.SubCurveIds,td.SubSegIds,
		ntm);
		}


		bmd->VertexData[k]->d.Append(1,&td,1);
		effect = amount;
		originaleffect = 0.0f;
		found = 1;
		}

		if (found == 1)
		{
		int bc = bmd->VertexData[k]->d.Count();



		//rebalance rest
		float remainder = 1.0f - effect;
		originaleffect = 1.0f - originaleffect;
		if (bmd->VertexData[k]->d.Count() > 1)
		{
		for (int j=0;j<bmd->VertexData[k]->d.Count();j++)
		{

		if (!(BoneData[bmd->VertexData[k]->d[j].Bones].flags & BONE_LOCK_FLAG))
		{
		if (bmd->VertexData[k]->d[j].Bones!=BoneID)
		{
		float per = 0.0f;
		if (remainder == 0.0f)
		{
		bmd->VertexData[k]->d[j].normalizedInfluences = 0.0f;
		}
		else if (originaleffect == 0.0f)
		{
		bmd->VertexData[k]->d[j].normalizedInfluences = remainder/(bmd->VertexData[k]->d.Count()-1.0f);
		}
		else
		{
		per = bmd->VertexData[k]->d[j].normalizedInfluences/originaleffect;
		bmd->VertexData[k]->d[j].normalizedInfluences = remainder*per;

		}

		}
		}
		}
		for (int j=0;j<bmd->VertexData[k]->d.Count();j++)
		bmd->VertexData[k]->d[j].Influences = bmd->VertexData[k]->d[j].normalizedInfluences;
		}
		else
		{
		bmd->VertexData[k]->d[0].Influences = amount;
		bmd->VertexData[k]->d[0].normalizedInfluences = amount;
		}

		bmd->VertexData[k]->Modified (TRUE);

		}
		*/
	}
	//WEIGHTTABLE
	PaintAttribList();
}



void BonesDefMod::NormalizeWeight(BoneModData *bmd,int vertID, BOOL unNormalize)
{

	//find vert
	if ((vertID >= 0) && (vertID < bmd->VertexData.Count()))
	{
		//only normalize the data if
		if ((!bmd->VertexData[vertID]->IsModified()) || (!unNormalize))
		{
			float sum = 0.0f;
			for (int j=0;j<bmd->VertexData[vertID]->WeightCount();j++)
			{
				//get sum
				sum += bmd->VertexData[vertID]->GetWeight(j);
			}
			//normalize everything
			for (int j=0;j<bmd->VertexData[vertID]->WeightCount();j++)
			{
				if (sum == 0.0f)
					bmd->VertexData[vertID]->SetWeight(j, 0.0f);
				else bmd->VertexData[vertID]->SetWeight(j, bmd->VertexData[vertID]->GetWeight(j)/sum);
				bmd->VertexData[vertID]->SetNormalizedWeight(j, bmd->VertexData[vertID]->GetWeight(j));
			}
		}
		bmd->VertexData[vertID]->UnNormalized(unNormalize);
		PaintAttribList();
	}
}



float BonesDefMod::RetrieveNormalizedWeight(BoneModData *bmd, int vid, int bid)
{
	return bmd->VertexData[vid]->GetNormalizedWeight(bid);
}


static int InfluSort( const void *elem1, const void *elem2 ) {
	VertexInfluenceListClass *a = (VertexInfluenceListClass *)elem1;
	VertexInfluenceListClass *b = (VertexInfluenceListClass *)elem2;
	if ( a->Influences == b->Influences)
		return 0;
	if ( a->Influences > b->Influences)
		return -1;
	else
		return 1;

}

void VertexListClass::Sort()
{
	d.Sort(InfluSort);
}

void BonesDefMod::BuildNormalizedWeight(BoneModData *bmd)
{
	//need to reqeight based on remainder
	//right now if the UI is up redo all verts else uses the cache
	//if ((bmd->nukeValidCache == TRUE) || (ip) /* this can be taken out when we work on the cache*/ || (bmd->validVerts.GetSize() != bmd->VertexData.Count()))
	if(1) // nedd to fix the cache when i get time
	{
		bmd->validVerts.SetSize(bmd->VertexData.Count());
		bmd->validVerts.ClearAll();
		bmd->nukeValidCache = FALSE;
	}

	for (int vid = 0; vid < bmd->VertexData.Count(); vid++)
	{
		float tempdist=0.0f;
		float w;

		if (!bmd->validVerts[vid])
		{
			//		bmd->validVerts.Set(vid,TRUE);

			if (bmd->VertexData[vid]->WeightCount() > boneLimit)
			{
				bmd->VertexData[vid]->Sort();
				if (bmd->VertexData[vid]->WeightCount() > boneLimit) 
				{
					bmd->VertexData[vid]->SetWeightCount(boneLimit);
					/*			for (int j = (boneLimit); j < bmd->VertexData[vid]->d.Count(); j++)
					{
					bmd->VertexData[vid]->d[j].Influences = 0.0f;
					}
					*/
					//				bmd->VertexData[vid]->NormalizeWeights();
				}

			}

			if (bmd->VertexData[vid]->IsRigid())
			{
				//find largest influence set it to 1 and the rest to 0
				float largest = -1.0f;
				int id = -1;
				for (int j =0; j < bmd->VertexData[vid]->WeightCount(); j++)
				{
					if (bmd->VertexData[vid]->GetWeight(j) > largest)
					{
						id = j;
						largest = bmd->VertexData[vid]->GetWeight(j);
					}
				}
				if (id != -1)
				{
					for (int j =0; j < bmd->VertexData[vid]->WeightCount(); j++)
						bmd->VertexData[vid]->SetNormalizedWeight(j, 0.0f);

					bmd->VertexData[vid]->SetNormalizedWeight(id, 1.0f);
				}

			}
			else if (bmd->VertexData[vid]->IsUnNormalized())
			{
				for (int j =0; j < bmd->VertexData[vid]->WeightCount(); j++)
					bmd->VertexData[vid]->SetNormalizedWeight(j, bmd->VertexData[vid]->GetWeight(j));
			}
			else if (bmd->VertexData[vid]->WeightCount() == 1) 
			{
				//if only one bone and absolute control set to it to max control
				if ( (BoneData[bmd->VertexData[vid]->GetBoneIndex(0)].flags & BONE_ABSOLUTE_FLAG) )
				{
					if (bmd->VertexData[vid]->IsModified())
					{
						if (bmd->VertexData[vid]->GetWeight(0) > 0.5f) 
							w = 1.0f;
						else w = 0.0f;
					}
					else
					{
						if (bmd->VertexData[vid]->GetWeight(0) > 0.0f) 
							w = 1.0f;
						else w = 0.0f;
					}
				}
				else w = bmd->VertexData[vid]->GetWeight(0);

				bmd->VertexData[vid]->SetNormalizedWeight(0, w);
			}
			else if (bmd->VertexData[vid]->WeightCount() >= 1) 
			{
				tempdist = 0.0f;
				for (int j =0; j < bmd->VertexData[vid]->WeightCount(); j++)
				{
					int bd = bmd->VertexData[vid]->GetBoneIndex(j);

					if (BoneData[bd].Node == NULL)
					{
						bmd->VertexData[vid]->SetNormalizedWeight(j, 0.0f);
						bmd->VertexData[vid]->SetWeight(j, 0.0f);
					}
					else
					{
						float infl = bmd->VertexData[vid]->GetWeight(j);
						int bone=bmd->VertexData[vid]->GetBoneIndex(j);
						tempdist += infl;
					}
				}
				for (int j=0; j<bmd->VertexData[vid]->WeightCount(); j++) 
				{
					w = (bmd->VertexData[vid]->GetWeight(j))/tempdist;
					bmd->VertexData[vid]->SetNormalizedWeight(j, (bmd->VertexData[vid]->GetWeight(j))/tempdist);
				}


			}

		}

	}

	if (bmd->isPatch)
	{
		for (int vid = 0; vid < bmd->VertexData.Count(); vid++)
		{

			if (!bmd->validVerts[vid])
			{
				if ((bmd->VertexData[vid]->IsRigidHandle()) && (bmd->vecOwnerList[vid]!=-1))
				{
					int ownerID = bmd->vecOwnerList[vid];
					bmd->VertexData[vid]->PasteWeights( bmd->VertexData[ownerID]->CopyWeights());
				}
			}

		}


	}

	bmd->validVerts.SetAll();
}


void BonesDefMod::NormalizeSelected(BOOL norm)
{
	ModContextList mcList;		
	INodeTab nodes;


	if (ip)
	{
		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();

		for (int i =0; i < objects; i++)
		{
			BoneModData *bmd = (BoneModData*)mcList[0]->localData;
			for (int j =0; j < bmd->VertexData.Count(); j++)
			{
				if (bmd->selected[j])
				{
					bmd->VertexData[j]->UnNormalized(!norm);

				}
			}	
		}
	}

}

void BonesDefMod::RigidSelected(BOOL rigid)
{
	ModContextList mcList;		
	INodeTab nodes;


	if (ip)
	{
		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();

		for (int i =0; i < objects; i++)
		{
			BoneModData *bmd = (BoneModData*)mcList[0]->localData;
			for (int j =0; j < bmd->VertexData.Count(); j++)
			{
				if (bmd->selected[j])
				{
					bmd->VertexData[j]->Rigid(rigid);
					bmd->validVerts.Set(j,FALSE);

				}
			}	
		}
	}

}


void BonesDefMod::RigidHandleSelected(BOOL rigid)
{
	ModContextList mcList;		
	INodeTab nodes;


	if (ip)
	{
		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();

		for (int i =0; i < objects; i++)
		{
			BoneModData *bmd = (BoneModData*)mcList[0]->localData;
			for (int j =0; j < bmd->VertexData.Count(); j++)
			{
				if (bmd->selected[j])
				{
					bmd->VertexData[j]->RigidHandle(rigid);
					bmd->validVerts.Set(j,FALSE);

				}
			}	
		}
	}

}

void BonesDefMod::BakeSelectedVertices()
{
	ModContextList mcList;		
	INodeTab nodes;


	if (ip)
	{
		theHold.Begin();

		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();

		for (int i =0; i < objects; i++)
		{
			BoneModData *bmd = (BoneModData*)mcList[0]->localData;

			theHold.Put(new WeightRestore(this,bmd));

			for (int j =0; j < bmd->VertexData.Count(); j++)
			{
				if ((bmd->selected[j]) && (!bmd->VertexData[j]->IsModified()))
				{
					if (bmd->VertexData[j]->GetClosestBone() != -1)
					{
						bmd->VertexData[j]->Modified(TRUE);
						VertexInfluenceListClass w;
						w.Bones = bmd->VertexData[j]->GetClosestBone();
						w.Influences = 1.0f;
						w.normalizedInfluences = 1.0f;							
						bmd->VertexData[j]->AppendWeight(w);
					}
					else
					{
						bmd->VertexData[j]->Modified(TRUE);

					}
					bmd->validVerts.Set(j,FALSE);
				}
			}	
		}
		theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));
	}

}

void BonesDefMod::InvalidateWeightCache()
{
	ModContextList mcList;		
	INodeTab nodes;


	if (ip)
	{

		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();

		for (int i =0; i < objects; i++)
		{
			BoneModData *bmd = (BoneModData*)mcList[0]->localData;
			bmd->nukeValidCache = TRUE;
		}
	}


}


void BonesDefMod::GetClosestPoints(Point3 p, Tab<Point3> &pointList,float threshold,int count,Tab<int> &hitList, Tab<float> &distList)

{
	hitList.ZeroCount();
	distList.ZeroCount();

	//copy any that are exactly on and mark those

	for (int j =0; j < pointList.Count(); j++)
	{
		Point3 oldPos = pointList[j];
		float dist = Length(oldPos-p);
		if (dist < 0.00001f)
		{
			hitList.Append(1,&j);
			float v = 1.0f;
			distList.Append(1,&v);
			return;
		}
	}

	//now look for any that are within a threshold
	for (int i =0; i < pointList.Count(); i++)
	{
		Point3 pos = pointList[i];
		float dist = Length(p-pos);
		if (distList.Count() < count)
		{
			if (dist < threshold)
			{
				hitList.Append(1,&i);
				distList.Append(1,&dist);
			}
		}
		else
		{	
			int id = i;			
			for (int k =0; k < count; k++)
			{
				if (dist < distList[k])
				{
					int tempid = hitList[k];
					float tempdist = distList[k];
					distList[k] = dist;
					hitList[k] = id;
					id = tempid;
					dist = tempdist;
				}
			}			
		}
	}
}

void BonesDefMod::RemapVertData(BoneModData *bmd, float threshold, int loadID, Object *obj)
{
	//if obj then use that point data
	if (obj)
	{
		bmd->rebuildWeights = TRUE;
		Tab<VertexListClass*> tempData;
		Tab<Point3> posList;
		posList.SetCount(bmd->VertexData.Count());

		//copy the old vertex data
		tempData.SetCount(bmd->VertexData.Count());
		for (int i =0; i < bmd->VertexData.Count(); i++)
		{
			tempData[i]  = new(VertexListClass);
			*tempData[i] = *bmd->VertexData[i];
			posList[i] = bmd->VertexData[i]->LocalPos;
			delete bmd->VertexData[i];
		}


		bmd->VertexData.SetCount(obj->NumPoints());
		//reset the bmd vertex data to empty
		for ( int i =0; i < bmd->VertexData.Count(); i++)
		{
			bmd->VertexData[i] = new VertexListClass();
		}
		BitArray processedVerts;
		processedVerts.SetSize(bmd->VertexData.Count());
		processedVerts.ClearAll();


		//copy any that are exactly on and mark those
		Tab<int> hitList;
		Tab<float> distList;

		for ( int i =0; i < bmd->VertexData.Count(); i++)
		{

			Point3 pos = obj->GetPoint(i);
			GetClosestPoints(pos, posList,threshold,4,hitList, distList);

			if (hitList.Count() == 1)
			{
				int index = hitList[0];
				*bmd->VertexData[i] = *tempData[index];
				bmd->VertexData[i]->LocalPos = pos;
				processedVerts.Set(i);
			}
			else
			{
				int modCount = 0;
				for (int j = 0; j < hitList.Count(); j++)
				{
					int index = hitList[j];
					if (tempData[index]->IsModified()) modCount++;
				}
				if ( (hitList.Count() > 0) && (modCount>=2))
				{
					bmd->VertexData[i]->Modified(TRUE);
					float sum = 0.0f;
					for (int j =0; j < hitList.Count(); j++)
					{
						if (tempData[hitList[j]]->IsModified())
							sum += distList[j];
					}
					for (int j =0; j < hitList.Count(); j++)
					{
						int hitID = hitList[j];
						if (tempData[hitID]->IsModified())
						{
							int tweightCount = tempData[hitID]->WeightCount();
							int oweightCount = bmd->VertexData[i]->WeightCount();

							//if multiples then average
							for (int k =0; k < tweightCount; k++)
							{
								int id = tempData[hitID]->GetBoneIndex(k);
								float weight = tempData[hitID]->GetNormalizedWeight(k);
								float per = 1.0f - (distList[j]/sum);
								weight *= per;
								BOOL found = FALSE;
								for (int m =0; m < oweightCount; m++)
								{
									if (bmd->VertexData[i]->GetBoneIndex(m) == id)
									{
										found = TRUE;
										float wt = bmd->VertexData[i]->GetWeight(m);
										bmd->VertexData[i]->SetWeight(m,  wt + weight);
									}

								}
								if (!found)
								{
									VertexInfluenceListClass temp;
									temp.Bones = id;
									temp.Influences = weight;
									bmd->VertexData[i]->AppendWeight(temp);
								}
							}
						}

					}
					bmd->VertexData[i]->NormalizeWeights();

				}

			}
		}
	}
	else if ((loadID >=0) && (loadID < loadBaseNodeData.Count()) && (loadBaseNodeData[loadID].matchData))
		//else use the load data
	{
		LoadVertexDataClass *matchData = loadBaseNodeData[loadID].matchData;

		BitArray processedVerts;
		processedVerts.SetSize(bmd->VertexData.Count());
		processedVerts.ClearAll();

		//build a remap list instead

		Tab<Point3> posList;
		posList.SetCount(matchData->vertexData.Count());
		for (int i =0; i < matchData->vertexData.Count(); i++)
		{
			Point3 p = matchData->vertexData[i]->LocalPos;

			posList[i] = p;
		}


		//copy any that are exactly on and mark those
		Tab<int> hitList;
		Tab<float> distList;

		//copy any that are exactly on and mark those
		for (int i =0; i < bmd->VertexData.Count(); i++)
		{
			Point3 pos = bmd->VertexData[i]->LocalPos;
			//5.1.02
			if (this->loadByIndex)
			{
				if (i < posList.Count())
				{
					hitList.SetCount(1);
					hitList[0] = i;
				}
				else hitList.ZeroCount();
			}	
			else GetClosestPoints(pos, posList,threshold,4,hitList, distList);

			if (hitList.Count() == 1)
			{
				int index = hitList[0];
				*bmd->VertexData[i] = *matchData->vertexData[index];
				bmd->VertexData[i]->LocalPos = pos;
				processedVerts.Set(i);				
			}
			else
			{
				int modCount = 0;
				for (int j = 0; j < hitList.Count(); j++)
				{
					int index = hitList[j];
					if (matchData->vertexData[index]->IsModified()) modCount++;
				}
				if ( (hitList.Count() > 0) && (modCount>=2))
				{
					bmd->VertexData[i]->Modified(TRUE);
					bmd->VertexData[i]->ZeroWeights();
					float sum = 0.0f;
					for (int j =0; j < hitList.Count(); j++)
					{
						if (matchData->vertexData[hitList[j]]->IsModified())
							sum += distList[j];
					}
					for (int j =0; j < hitList.Count(); j++)
					{

						int hitID = hitList[j];
						if (matchData->vertexData[hitID]->IsModified())
						{
							int tweightCount = matchData->vertexData[hitID]->WeightCount();
							int oweightCount = bmd->VertexData[i]->WeightCount();

							//if multiples then average
							for (int k =0; k < tweightCount; k++)
							{
								int id = matchData->vertexData[hitID]->GetBoneIndex(k);
								float weight = matchData->vertexData[hitID]->GetNormalizedWeight(k);

								float per = 1.0f - (distList[j]/sum);
								weight *= per;
								BOOL found = FALSE;
								for (int m =0; m < oweightCount; m++)
								{
									if (bmd->VertexData[i]->GetBoneIndex(m) == id)
									{
										found = TRUE;
										float wt = bmd->VertexData[i]->GetWeight(m);
										bmd->VertexData[i]->SetWeight(m, wt + weight);
									}

								}
								if (!found)
								{
									VertexInfluenceListClass temp;
									temp.Bones = id;
									temp.Influences = weight;
									bmd->VertexData[i]->AppendWeight(temp);
								}
							}

						}

					}
					bmd->VertexData[i]->NormalizeWeights();

				}
			}
		}
	}


}



void BonesDefMod::RemapExclusionData(BoneModData *bmd, float threshold, int loadID, Object *obj)
{
	Tab<int> hitList;
	Tab<float> distList;

	//if obj then use that point data
	if (obj)
	{

		Tab<Point3> posList;
		posList.SetCount(bmd->VertexData.Count());

		//copy the old vertex data
		int i;
		for (i =0; i < bmd->VertexData.Count(); i++)
		{
			posList[i] = bmd->VertexData[i]->LocalPos;
		}


		//need to clone exclusions
		Tab<ExclusionListClass*> holdExclusionList;
		holdExclusionList.SetCount(bmd->exclusionList.Count());
		for (i = 0; i < bmd->exclusionList.Count(); i++)
		{
			if (bmd->exclusionList[i])
			{
				holdExclusionList[i] = new ExclusionListClass();
				*holdExclusionList[i] = *bmd->exclusionList[i];
			}
			else holdExclusionList[i]=NULL;
			delete bmd->exclusionList[i];
			bmd->exclusionList[i] = NULL;
		}

		for (int j=0; j< obj->NumPoints(); j++)
		{
			Point3 pos = obj->GetPoint(j);
			//copy any that are exactly on and mark those
			GetClosestPoints(pos, posList,threshold,4,hitList, distList);

			for (i = 0; i < holdExclusionList.Count(); i++)
			{
				if (holdExclusionList[i])
				{


					if (hitList.Count() == 1)
					{
						int index = hitList[0];
						Tab<int> tempList;
						tempList.Append(1,&j);
						int where;
						if (holdExclusionList[i]->isInList(index,where))
						{
							bmd->ExcludeVerts(i, tempList,FALSE);
						}
					}
					else
					{
						int exCount=0;
						for (int k = 0; k < hitList.Count();k++)
						{
							int index = hitList[k];
							int where;
							if (holdExclusionList[i]->isInList(index,where))
								exCount++;

						}
						if (exCount >=2)
						{	
							Tab<int> tempList;
							tempList.Append(1,&j);
							bmd->ExcludeVerts(i, tempList,FALSE);

						}

					}
				}
			}
		}

		for (i = 0; i < holdExclusionList.Count(); i++)
		{
			if (holdExclusionList[i])
				delete holdExclusionList[i];
		}

	}

	else if ((loadID >=0) && (loadID < loadBaseNodeData.Count()) && (loadBaseNodeData[loadID].matchData))
	{
		//else use the load data
		Tab<Point3> posList;
		posList.SetCount(loadBaseNodeData[loadID].matchData->vertexData.Count());

		int i;
		//copy the old vertex data
		for (i =0; i < loadBaseNodeData[loadID].matchData->vertexData.Count(); i++)
		{
			posList[i] = loadBaseNodeData[loadID].matchData->vertexData[i]->LocalPos;
		}


		//need to clone exclusions
		Tab<ExclusionListClass*> holdExclusionList;
		holdExclusionList.SetCount(loadBaseNodeData[loadID].matchData->exclusionList.Count());
		for (i = 0; i < loadBaseNodeData[loadID].matchData->exclusionList.Count(); i++)
		{
			if (loadBaseNodeData[loadID].matchData->exclusionList[i])
			{
				holdExclusionList[i] = new ExclusionListClass();
				*holdExclusionList[i] = *loadBaseNodeData[loadID].matchData->exclusionList[i];
			}
			else holdExclusionList[i]=NULL;
			delete loadBaseNodeData[loadID].matchData->exclusionList[i]; 
			//			delete bmd->exclusionList[i]; 
			//			bmd->exclusionList[i] = NULL;
		}
		//nuke old exclusion list
		for (i = 0; i < bmd->exclusionList.Count(); i++)
		{
			if (bmd->exclusionList[i])
			{
				delete bmd->exclusionList[i]; 
				bmd->exclusionList[i] = NULL;
			}
		}


		for (int j=0; j< bmd->VertexData.Count(); j++)
		{
			Point3 pos = bmd->VertexData[j]->LocalPos;
			//copy any that are exactly on and mark those
			//5.1.02
			if (this->loadByIndex)
			{
				if (i < posList.Count())
				{
					hitList.SetCount(1);
					hitList[0] = j;
				}
				else hitList.ZeroCount();
			}	
			else GetClosestPoints(pos, posList,threshold,4,hitList, distList);


			for (i = 0; i < holdExclusionList.Count(); i++)
			{
				if (holdExclusionList[i])
				{


					if (hitList.Count() == 1)
					{
						int index = hitList[0];
						Tab<int> tempList;
						tempList.Append(1,&j);
						int where;
						if (holdExclusionList[i]->isInList(index,where))
						{
							bmd->ExcludeVerts(i, tempList,FALSE);
						}
					}
					else
					{
						int exCount=0;
						for (int k = 0; k < hitList.Count();k++)
						{
							int index = hitList[k];
							int where;
							if (holdExclusionList[i]->isInList(index,where))
								exCount++;

						}
						if (exCount >=2)
						{	
							Tab<int> tempList;
							tempList.Append(1,&j);
							bmd->ExcludeVerts(i, tempList,FALSE);

						}

					}
				}
			}
		}

		for (i = 0; i < holdExclusionList.Count(); i++)
		{
			if (holdExclusionList[i])
				delete holdExclusionList[i];
		}
	}


}


void BonesDefMod::RemapLocalGimzoData(BoneModData *bmd,  float threshold, int loadID ,Object *obj)
{
	Tab<int> hitList;
	Tab<float> distList;

	//if obj then use that point data
	if (obj)
	{

		Tab<Point3> posList;
		posList.SetCount(obj->NumPoints());

		//copy the old vertex data
		for (int i =0; i < obj->NumPoints(); i++)
		{
			posList[i] = obj->GetPoint(i);
		}



		for (int j=0; j< bmd->gizmoData.Count(); j++)
		{
			if (bmd->gizmoData[j])
			{
				for (int i = 0; i < bmd->gizmoData[j]->Count(); i++)
				{
					int index = bmd->gizmoData[j]->GetVert(i);
					if ((index >=0) && (index < bmd->VertexData.Count()))
					{
						Point3 pos = bmd->VertexData[index]->LocalPos;
						//copy any that are exactly on and mark those
						GetClosestPoints(pos, posList,threshold,1,hitList, distList);
						if (hitList.Count() == 1)
						{
							bmd->gizmoData[j]->Replace(i,hitList[0]);
						}
						else bmd->gizmoData[j]->Replace(i,-1);

					}

				}
			}
		}
	}

	else if ((loadID >=0) && (loadID < loadBaseNodeData.Count()) && (loadBaseNodeData[loadID].matchData))
	{
	}

}

void BonesDefMod::RemoveZeroWeights()
{

	ModContextList mcList;		
	INodeTab nodes;


	float limit = 0.0f;
	pblock_advance->GetValue(skin_advance_clearzerolimit,0,limit,FOREVER);

	theHold.Begin();



	if (ip == NULL) return;

	ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	for (int i = 0; i < objects; i++)
	{

		BoneModData *bmd = (BoneModData*)mcList[i]->localData;
		theHold.Put(new WeightRestore(this,bmd));

		for (int j = 0; j < bmd->VertexData.Count(); j++)
		{

			for (int k = 0; k < bmd->VertexData[j]->WeightCount(); k++)
			{
				float weight =  bmd->VertexData[j]->GetNormalizedWeight(k);
				if (weight <= 0.0f)
				{
					bmd->VertexData[j]->DeleteWeight(k);
					k--;
				}
			}
			if ( bmd->VertexData[j]->WeightCount() > 1 )
			{
				for (int k = 0; k < bmd->VertexData[j]->WeightCount(); k++)
				{
					float weight =  bmd->VertexData[j]->GetNormalizedWeight(k);
					if (weight <= limit)
					{
						bmd->VertexData[j]->DeleteWeight(k);
						k--;
					}
				}
			}
			if (!bmd->VertexData[j]->IsUnNormalized())
				bmd->VertexData[j]->NormalizeWeights();
		}
	}
	theHold.Accept(GetString(IDS_PW_REMOVEZEROWEIGHTS));

	if (weightTableWindow.hWnd)
		weightTableWindow.FillOutVertexList();

	PaintAttribList();

	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());	

}



#define LOOP_SEL 1
#define RING_SEL 2
#define GROW_SEL 3
#define SHRINK_SEL 4

void BonesDefMod::EdgeSel(int mode)
{
	//get mesh

	if (ip == NULL) return;

	MyEnumProc dep;              
	DoEnumDependents(&dep);

	if (dep.Nodes.Count() == 0) return;

	//hold our selection


	for (int i = 0; i < dep.Nodes.Count(); i++)
	{
		BoneModData *bmd = GetBMD(dep.Nodes[i]);

		if (theHold.Holding() && bmd) 
			theHold.Put(new SelectionRestore(this,bmd));
	}	

	for (int i = 0; i < dep.Nodes.Count(); i++)
	{
		BoneModData *bmd = GetBMD(dep.Nodes[i]);

		if (bmd==NULL) continue;

		int numberVerts = bmd->selected.GetSize();


		//get the mesh
		//check if mesh or poly
		if (bmd->mesh)
		{

			Mesh *tmsh = bmd->mesh;


			MNMesh *msh = new MNMesh();
			msh->Init();
			msh->AddTri(*tmsh);
			msh->FillInMesh();
			msh->EliminateBadVerts ();
			msh->MakePolyMesh (0, false);

			msh->FillInVertEdgesFaces();


			if (tmsh->numVerts != numberVerts) return;


			Tab<int> vertIndexIntoPolyMesh;
			vertIndexIntoPolyMesh.SetCount(msh->numv);


			for (int i = 0; i < msh->numv; i++)
			{
				Point3 mnP = msh->v[i].p;
				for (int j = 0; j < tmsh->numVerts; j++)
				{
					Point3 p = tmsh->verts[j];
					if (p == mnP)
					{
						vertIndexIntoPolyMesh[i] = j;
					}
				}
			}

			//convert vert sel to edge sel
			BitArray vsel, esel;

			vsel.SetSize(numberVerts);
			vsel.ClearAll();
			//loop through the selection
			for (int i =0; i < numberVerts; i++)
			{
				if (bmd->selected[i])
					vsel.Set(i);
			}

			esel.SetSize(msh->nume);
			esel.ClearAll();

			for (int i = 0; i < msh->nume; i++)
			{
				int a,b;
				a = msh->e[i].v1;
				b = msh->e[i].v2;
				a = vertIndexIntoPolyMesh[a];
				b = vertIndexIntoPolyMesh[b];
				if (vsel[a] && vsel[b])
					esel.Set(i);
			}


			//do loop
			if (mode == LOOP_SEL)
				msh->SelectEdgeLoop (esel);
			else if (mode == RING_SEL)
				msh->SelectEdgeRing (esel);


			//convert back to vert sel

			//pass that back
			if (esel.NumberSet() > 0)
			{
				bmd->selected.ClearAll();
				for (int i = 0; i < msh->nume; i++)
				{
					if (esel[i])
					{
						int a,b;
						a = msh->e[i].v1;
						b = msh->e[i].v2;

						a = vertIndexIntoPolyMesh[a];
						b = vertIndexIntoPolyMesh[b];

						bmd->selected.Set(a);
						bmd->selected.Set(b);
					}
				}
			}

			delete msh;


		}
		//if poly object we can use that
		else if (bmd->mnMesh)
		{

			MNMesh *msh = bmd->mnMesh;

			//convert vert sel to edge sel
			BitArray vsel, esel;

			if (msh->numv != numberVerts) return;

			vsel.SetSize(numberVerts);
			vsel.ClearAll();
			//loop through the selection
			for (int i =0; i < numberVerts; i++)
			{
				if (bmd->selected[i])
					vsel.Set(i);
			}

			esel.SetSize(msh->nume);
			esel.ClearAll();

			for (int i = 0; i < msh->nume; i++)
			{
				int a,b;
				a = msh->e[i].v1;
				b = msh->e[i].v2;
				if (vsel[a] && vsel[b])
					esel.Set(i);
			}


			//do loop
			if (mode == LOOP_SEL)
				msh->SelectEdgeLoop (esel);
			else if (mode == RING_SEL)
				msh->SelectEdgeRing (esel);

			//convert back to vert sel

			//pass that back
			if (esel.NumberSet() > 0)
			{
				bmd->selected.ClearAll();
				for (int i = 0; i < msh->nume; i++)
				{
					if (esel[i])
					{
						int a,b;
						a = msh->e[i].v1;
						b = msh->e[i].v2;

						bmd->selected.Set(a);
						bmd->selected.Set(b);
					}
				}
			}
		}

	}

	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());


	PaintAttribList();

}




void BonesDefMod::GrowVertSel( int mode)
{
	if (ip == NULL) return;
	//get mesh
	MyEnumProc dep;              
	DoEnumDependents(&dep);

	if (dep.Nodes.Count() == 0) return;

	//hold our selection


	for (int i = 0; i < dep.Nodes.Count(); i++)
	{
		BoneModData *bmd = GetBMD(dep.Nodes[i]);

		if (theHold.Holding() && bmd) 
			theHold.Put(new SelectionRestore(this,bmd));
	}	

	for (int i = 0; i < dep.Nodes.Count(); i++)
	{
		BoneModData *bmd = GetBMD(dep.Nodes[i]);

		if (bmd==NULL) continue;

		int numberVerts = bmd->selected.GetSize();


		//get the mesh
		//check if mesh or poly
		if (bmd->mesh)
		{

			Mesh *tmsh = bmd->mesh;

			if (tmsh->numVerts != numberVerts) return;

			BitArray origSel(bmd->selected);

			for (int i = 0; i < tmsh->numFaces; i++)
			{
				int deg = 3;
				int hit = 0;
				for (int j = 0; j < deg; j++)
				{	
					int a = tmsh->faces[i].v[j];
					if (origSel[a])
						hit ++;
				}
				if ((hit>0) && (mode == GROW_SEL))
				{
					for (int j = 0; j < deg; j++)
					{	
						int a = tmsh->faces[i].v[j];

						bmd->selected.Set(a,TRUE);	

					}			
				}
				else if ((hit!= deg) && (mode == SHRINK_SEL))
				{
					for (int j = 0; j < deg; j++)
					{	
						int a = tmsh->faces[i].v[j];

						bmd->selected.Set(a,FALSE);						
					}			
				}			

			}
		}
		//if poly object we can use that
		else if (bmd->mnMesh)
		{

			MNMesh *msh = bmd->mnMesh;

			//convert vert sel to edge sel
			BitArray vsel, esel;

			if (msh->numv != numberVerts) return;

			BitArray origSel(bmd->selected);

			for (int i = 0; i < msh->numf; i++)
			{
				int deg = msh->f[i].deg;
				int hit = 0;
				for (int j = 0; j < deg; j++)
				{	
					int a = msh->f[i].vtx[j];
					if (origSel[a])
						hit++;
				}
				if ((hit>0) && (mode == GROW_SEL))
				{
					for (int j = 0; j < deg; j++)
					{	
						int a = msh->f[i].vtx[j];

						bmd->selected.Set(a,TRUE);

					}			
				}
				else if ((hit!= deg) && (mode == SHRINK_SEL))
				{
					for (int j = 0; j < deg; j++)
					{	
						int a = msh->f[i].vtx[j];

						bmd->selected.Set(a,FALSE);						
					}			
				}				


			}
		}
	}
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());

	PaintAttribList();

}


void BonesDefMod::SelectVerticesByBone(int index)
{
	if (ip == NULL) return;
	//get mesh
	MyEnumProc dep;              
	DoEnumDependents(&dep);

	if (dep.Nodes.Count() == 0) return;

	//hold our selection


	for (int i = 0; i < dep.Nodes.Count(); i++)
	{
		BoneModData *bmd = GetBMD(dep.Nodes[i]);

		if (theHold.Holding()&& bmd ) 
			theHold.Put(new SelectionRestore(this,bmd));
	}	

	for (int i = 0; i < dep.Nodes.Count(); i++)
	{
		BoneModData *bmd = GetBMD(dep.Nodes[i]);

		if (bmd==NULL) continue;

		int numberVerts = bmd->VertexData.Count();
		bmd->selected.ClearAll();
		for (int i = 0; i < numberVerts; i++)
		{
			int weightCount = bmd->VertexData[i]->WeightCount();
			for (int j = 0; j < weightCount; j++)
			{
				int bid = bmd->VertexData[i]->GetBoneIndex(j);
				float w = bmd->VertexData[i]->GetNormalizedWeight(j);
				if ((bid == index) && (i < bmd->selected.GetSize()))
				{
					if (w > 0.0f)
						bmd->selected.Set(i,TRUE);
				}
			}
		}

	}
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());

	PaintAttribList();

}

