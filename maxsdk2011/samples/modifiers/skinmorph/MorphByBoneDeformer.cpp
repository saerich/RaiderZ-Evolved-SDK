#include "MorphByBone.h"

void MorphByBone::Deform(Object *obj, int ldID)
{
	MorphByBoneLocalData *ld = this->localDataList[ldID];
//build weights per bone
	Tab<Point3> vecs;
	int pointCount = obj->NumPoints();
	vecs.SetCount(pointCount);
//build a list of vecs
//zero them out
	for (int i = 0; i < pointCount; i++)
		vecs[i] = Point3(0.0f,0.0f,0.0f);

//update our curve controls
	for (int i = 0; i < boneData.Count(); i++)
		{
		INode *node = GetNode(i);

		if (node)
			{
			for (int j = 0; j< boneData[i]->morphTargets.Count(); j++)
				{
				MorphTargets *morph = boneData[i]->morphTargets[j];
				int graphID = morph->externalFalloffID;
				morph->curve = NULL; 
				if ((graphID >= 0) && (graphID < pblock->Count(pb_falloffgraphlist)))
					{
					ReferenceTarget *ref = NULL;
					pblock->GetValue(pb_falloffgraphlist,0,ref,FOREVER,graphID);
					if (ref)
						{
						ICurveCtl *graph = (ICurveCtl *) ref;
						morph->curve = graph->GetControlCurve(0);
						}	
					}
				}
			
			}
		}


	if (0)//(editMorph)
		{
		for (int i = 0; i < boneData.Count(); i++)
			{
			INode *node = GetNode(i);
			if ((node) && (i == currentBone))
				{

				Matrix3 currentBoneTM = boneData[i]->currentBoneNodeTM;
				Matrix3 currentParentBoneTM = boneData[i]->parentBoneNodeCurrentTM;
				currentBoneTM *= Inverse(currentParentBoneTM);

				Matrix3 defTM = currentParentBoneTM * Inverse(ld->selfNodeCurrentTM);
			
				for (int j = 0; j < boneData[i]->morphTargets.Count(); j++)
					{
					if (!boneData[i]->morphTargets[j]->IsDead() && (j == currentMorph))
						{
						MorphTargets *morph = boneData[i]->morphTargets[j];
						float angle = 0.0f;//this->AngleBetweenTMs(currentBoneTM,morph->boneTMInParentSpace);
						morph->weight = 0.0f;
						if (angle < morph->influenceAngle)
							{
							float per = 1.0f - angle/morph->influenceAngle;
							per = GetFalloff(per, morph->falloff,morph->curve);
							morph->weight = per;
							
							for (int k = 0; k < morph->d.Count(); k++)
								{
								int id = morph->d[k].vertexID;
								int owner = morph->d[k].localOwner;
								if (owner == ldID)
									{
										if (id < vecs.Count())
										{
										vecs[id] = morph->d[k].originalPt - ld->preDeform[id];
										vecs[id] += VectorTransform(defTM,morph->d[k].vecParentSpace)*per;
										}
									}
									
								}
							}
						}
					}
				}
			}

		}
	else
		{
		for (int i = 0; i < boneData.Count(); i++)
			{
			INode *node = GetNode(i);
			BOOL useNode = TRUE;
			if (editMorph)
				{
				if (i==currentBone) useNode = TRUE;
				else useNode = FALSE;
				}
			if ((node) && (useNode))
				{

				BOOL ballJoint = boneData[i]->IsBallJoint();

				Matrix3 currentBoneTM = boneData[i]->currentBoneNodeTM;
				Matrix3 currentParentBoneTM = boneData[i]->parentBoneNodeCurrentTM;
				currentBoneTM *= Inverse(currentParentBoneTM);

				Matrix3 defTM = currentParentBoneTM * Inverse(ld->selfNodeCurrentTM);
				Matrix3 initialTMInParentSpace(1);
				if (!ballJoint)
					{
					initialTMInParentSpace = boneData[i]->intialBoneNodeTM * Inverse(boneData[i]->intialParentNodeTM);
					}

			
				for (int j = 0; j < boneData[i]->morphTargets.Count(); j++)
					{
					if ((!boneData[i]->morphTargets[j]->IsDead()))
						{
						MorphTargets *morph = boneData[i]->morphTargets[j];
						float angle = 0.0f;
						if (ballJoint)
							angle = this->AngleBetweenTMs(currentBoneTM,morph->boneTMInParentSpace);
						else 
							{
							float morphAngle = this->AngleBetweenTMs(initialTMInParentSpace,morph->boneTMInParentSpace);
							float currentAngle = this->AngleBetweenTMs(initialTMInParentSpace,currentBoneTM);
//DebugPrint(_T("Morph angle %3.2f current Angle %3.2f\n"),morphAngle*180.0f/PI,currentAngle*180.0f/PI);
							angle = morphAngle - currentAngle;
							
							}


						morph->weight = 0.0f;
						if ((angle < morph->influenceAngle) || (editMorph))
							{
							float per = 1.0f - angle/morph->influenceAngle;
							if ((!ballJoint) && (per > 1.0f))
								per = 1.0f;
							per = GetFalloff(per, morph->falloff,morph->curve);
							if (per < 0.0f) per = 0.0f;
							morph->weight = per;
//							morph->weight = GetFalloff(morph->weight, morph->falloff,morph->curve);
							if  (!boneData[i]->morphTargets[j]->IsDisabled())
								{
								for (int k = 0; k < morph->d.Count(); k++)
									{
									int id = morph->d[k].vertexID;
									int owner = morph->d[k].localOwner;
									if ((owner == ldID)  && (id < vecs.Count()) )
										{
										if ((editMorph) && (j==currentMorph))
											{										
											vecs[id] += morph->d[k].originalPt - ld->preDeform[id];
											vecs[id] += VectorTransform(defTM,morph->d[k].vecParentSpace)*per;
											}
										else vecs[id] += VectorTransform(defTM,morph->d[k].vecParentSpace)*per;

										}
									}
								}
							}
						}
					}
				}
			}
		}

	if (vecs.Count() > 0)
	{
		MorphByBoneDeformer def(ld,vecs.Addr(0));
		obj->Deform(&def, TRUE);
	}

}


MorphByBoneDeformer::MorphByBoneDeformer(MorphByBoneLocalData *ld,Point3 *pList)
{
this->ld = ld;
this->pList = pList;

}
Point3 MorphByBoneDeformer::Map(int i, Point3 p)
{
ld->postDeform[i] = p+pList[i];
return ld->postDeform[i];
}
