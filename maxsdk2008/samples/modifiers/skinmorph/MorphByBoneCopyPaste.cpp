
#include "MorphByBone.h"


void CopyBuffer::CopyToBuffer(MorphByBoneData *data)
{
//make sure we have a valid bone

	if (copyData != NULL)
		copyData->DeleteThis();

	copyData = data->Clone(TRUE);


}

int MorphByBone::GetMirrorBone(int id)
{

	int whichBone = 0;
	if (id == -1)
		whichBone = currentBone;
	else whichBone = id;

//make sure it is a valid node
	if ((whichBone < 0) || (whichBone >= boneData.Count()) 	)
			{
			return -1;
			}
	if (GetNode(whichBone) == NULL) return -1;


	Matrix3 itm = Inverse(localDataList[0]->selfObjectTM);

	Point3 mirrorMin, mirrorMax;

	Box3 mbox;
	mbox.Init();
	mbox += boneData[whichBone]->localBounds.pmin * boneData[whichBone]->currentBoneObjectTM * itm;
	mbox += boneData[whichBone]->localBounds.pmax * boneData[whichBone]->currentBoneObjectTM * itm;
	
	mirrorMin = mbox.pmin;
	mirrorMax = mbox.pmax;


	int closestBone = -1;
	float closestDist = 0.0f;
	for (int i = 0; i < boneData.Count(); i++)
		{
		INode *node = GetNode(i);
		if (node)
			{
			Box3 box;
			box.Init();
			box += boneData[i]->localBounds.pmin * boneData[i]->currentBoneObjectTM * itm * mirrorTM;
			box += boneData[i]->localBounds.pmax * boneData[i]->currentBoneObjectTM * itm * mirrorTM;
			Point3 min, max;

			min = box.pmin;// * boneData[i]->currentBoneObjectTM * itm * mirrorTM;
			max = box.pmax;// * boneData[i]->currentBoneObjectTM * itm * mirrorTM;

			float dist = Length(mirrorMin - min) + Length(mirrorMax - max);

			if ((closestBone == -1) || (dist < closestDist))
				{
				closestBone = i;
				closestDist = dist;
				}
			}
		}
	return closestBone;

}

//tm the mirror tm 
void CopyBuffer::PasteToBuffer(MorphByBone *mod, MorphByBoneData *targ, Matrix3 mirrorTM, float threshold, BOOL flipTM)
{
	if (copyData == NULL) return;
	if (copyData->morphTargets.Count() == 0) return;


	if (targ == NULL) return;

//delete any existing morphs
	for (int i = 0; i < targ->morphTargets.Count(); i++)
		delete targ->morphTargets[i];

	targ->morphTargets.ZeroCount();
	int realCount= 0;
	for (int i = 0; i < copyData->morphTargets.Count(); i++)
		{
		if (!copyData->morphTargets[i]->IsDead()) realCount++;
		}
	targ->morphTargets.SetCount(realCount);

	for (int i = 0; i < mod->localDataList.Count(); i++)
		{
		MorphByBoneLocalData *ld = mod->localDataList[i];
		if (ld)
			{
			ld->tempPoints = ld->preDeform;
			ld->tempMatchList.SetCount(ld->tempPoints.Count());
			for (int j = 0; j < ld->preDeform.Count(); j++)
				{
				Matrix3 tm(1);
				//need to put our original points
				ld->tempPoints[j] = ld->tempPoints[j] * mirrorTM;
				ld->tempMatchList[j] = -1;
				}
			}
		}
//now find closest pairs
	for (int i = 0; i < mod->localDataList.Count(); i++)
		{
		MorphByBoneLocalData *ld = mod->localDataList[i];
		if (ld)
			{			
			for (int j = 0; j < ld->tempPoints.Count(); j++)
				{
				Point3 sourcePoint = ld->preDeform[j];
				float closest = 100000.0f;
				int closestID = -1;
				for (int k = 0; k < ld->tempPoints.Count(); k++)
					{
					Point3 mirrorPoint = ld->tempPoints[k];
					float dist = Length(sourcePoint-mirrorPoint);
					if ((dist < threshold) && (dist < closest))
						{
						closest = dist;
						closestID = k;
						}

					}
				ld->tempMatchList[closestID] = j;
				}
			}
		}
//create a blank set of morphs
	int currentMorph = 0;
	for (int i = 0; i < copyData->morphTargets.Count(); i++)
		{
		if (!copyData->morphTargets[i]->IsDead())
			{
			targ->morphTargets[currentMorph] = new MorphTargets();
			int currentVert=0;
			for (int j = 0; j < mod->localDataList.Count(); j++)
				{
				targ->morphTargets[currentMorph]->d.SetCount(mod->localDataList[j]->preDeform.Count());
				for (int k =0; k < mod->localDataList[j]->preDeform.Count(); k++)
					{
					targ->morphTargets[currentMorph]->d[currentVert].vertexID = k;
					targ->morphTargets[currentMorph]->d[currentVert].vec = Point3(0.0f,0.0f,0.0f);
					targ->morphTargets[currentMorph]->d[currentVert].vecParentSpace = Point3(0.0f,0.0f,0.0f);
					targ->morphTargets[currentMorph]->d[currentVert].originalPt = mod->localDataList[j]->preDeform[k];//*tm;
					targ->morphTargets[currentMorph]->d[currentVert].localOwner = j;
					currentVert++;
					}
				}
			currentMorph++;
			}
		
		}
//this only works if it is not instanced
//loop through morphs
	currentMorph = 0;
	for (int i = 0; i < copyData->morphTargets.Count(); i++)
		{
		if (!copyData->morphTargets[i]->IsDead())
			{

		//loop through points
			for (int j = 0; j <targ->morphTargets[currentMorph]->d.Count(); j++)
				{

		//get the mirror index if there is one
				int ldIndex = targ->morphTargets[currentMorph]->d[j].localOwner;
				MorphByBoneLocalData *ld = mod->localDataList[ldIndex];

				Matrix3 boneTM = copyData->currentBoneNodeTM;
				Matrix3 parentTM = copyData->parentBoneNodeCurrentTM;
				Matrix3 mirror = Inverse(ld->selfNodeTM) * mirrorTM * ld->selfNodeTM;
				Matrix3 mirrorBoneTM = targ->currentBoneNodeTM;
				Matrix3 mirrorParentTM = targ->parentBoneNodeCurrentTM;

				Matrix3 lTM, pTM;
				lTM = boneTM * mirror * Inverse(mirrorBoneTM);
				pTM = parentTM * mirror * Inverse(mirrorParentTM);


				if (ld)
					{
					int  mirrorID = targ->morphTargets[currentMorph]->d[j].vertexID;
					int sourceID = ld->tempMatchList[mirrorID];
					//now look though our list and swap the 2
					if (sourceID != -1)
						{

	////transform the vec into local space, flip them, then back into bone space
	//					targ->morphTargets[i]->d[j].vec = VectorTransform(copyData->morphTargts[i]->d[sourceID].vec,mirrorTM);
						if (1)//(flipTM)
							{
							Point3 vec = copyData->morphTargets[i]->d[sourceID].vec; // in local bone space
							targ->morphTargets[currentMorph]->d[j].vec = VectorTransform(lTM,vec);

							vec = copyData->morphTargets[i]->d[sourceID].vecParentSpace; // in local bone space
							targ->morphTargets[currentMorph]->d[j].vecParentSpace = VectorTransform(pTM,vec);
							//current bone tm
							//current parent tm

							//invserse of the self
							//mirror
							//self tm

							//inverse of mirror bone
							//inverse of mirror parent bone

							}
						else 
							{
							targ->morphTargets[currentMorph]->d[j].vec = copyData->morphTargets[i]->d[sourceID].vec;
							targ->morphTargets[currentMorph]->d[j].vecParentSpace = copyData->morphTargets[i]->d[sourceID].vecParentSpace;
							}


//						Point3 pvec = copyData->morphTargets[i]->d[sourceID].vecParentSpace;

//						Matrix3 selfTM = ld->selfNodeTM;
//						Matrix3 parentTM = copyData->parentBoneNodeCurrentTM;
//						Matrix3 tm = parentTM * Inverse(selfTM) * mirrorTM * selfTM * Inverse(parentTM);

//						pvec = VectorTransform(tm,pvec);

						//do we need to flip the vecs?
						targ->morphTargets[currentMorph]->d[j].originalPt = copyData->morphTargets[i]->d[sourceID].originalPt * mirrorTM;

						}
					}
				}

	//resolve tms
			if (flipTM)
				{
				MorphByBoneLocalData *ld = mod->localDataList[0];
				Matrix3 mirror = Inverse(ld->selfNodeTM) * mirrorTM * ld->selfNodeTM;

		//mirror the initial copy node tm
				Matrix3 copyTM = copyData->currentBoneNodeTM;
				copyTM *= mirror;
				copyTM.SetRow(3,targ->currentBoneNodeTM.GetRow(3));

	//			targ->morphTargets[currentMorph]->boneTMInParentSpace = copyTM * ;


		//put our current targ node tm into the copyTM space
				Matrix3 targTM = targ->currentBoneNodeTM;
				targTM *= Inverse(copyTM);

		//put the morph tm in worldspace
				Matrix3 morphTM = copyData->morphTargets[i]->boneTMInParentSpace * copyData->parentBoneNodeCurrentTM;
		//mirror it
	//			morphTM *= mirror;
	//			morphTM.SetRow(3,targ->currentBoneNodeTM.GetRow(3));

		//now animate it back to world using the morph target tm
				targTM *= morphTM;
		//jam it back into our parent space
				targTM *= Inverse(targ->currentBoneNodeTM);

				targ->morphTargets[currentMorph]->boneTMInParentSpace = targTM;
				}
			else targ->morphTargets[currentMorph]->boneTMInParentSpace = copyData->morphTargets[i]->boneTMInParentSpace;


//puts it in world space
/*
			morphTM = copyData->morphTargets[i]->boneTMInParentSpace * copyData->parentBoneNodeCurrentTM;
			morphTM.SetRow(3,targ->currentBoneNodeTM.GetRow(3));
			morphTM = morphTM * mirror;
			morphTM = morphTM * Inverse(targ->parentBoneNodeCurrentTM);
			targ->morphTargets[currentMorph]->boneTMInParentSpace = morphTM;
*/


/*
	//mirror all the morph tms
			MorphByBoneLocalData *ld = mod->localDataList[0];
			Matrix3 mirror = Inverse(ld->selfNodeTM) * mirrorTM * ld->selfNodeTM;
			Matrix3 ptm = copyData->parentBoneNodeCurrentTM;
			Matrix3 mtm = copyData->morphTargets[i]->boneTMInParentSpace;
			Matrix3 morphWorldSpaceTM = mtm * ptm *mirror;
			morphWorldSpaceTM.SetRow(3,targ->currentBoneNodeTM.GetRow(3));
	

	//link the target bone initial to the mirrored initial copy node tm
			//remcompute the mirror tms
			//but pack into our targ space


			if (flipTM)
				{
				MorphByBoneLocalData *ld = mod->localDataList[0];

				Matrix3 mirror = Inverse(ld->selfNodeTM) * mirrorTM * ld->selfNodeTM;

				//transform the tm into local space
				//flip it thenback in parent space
				Matrix3 mtm = copyData->morphTargets[i]->boneTMInParentSpace;
				mtm = mtm * copyData->parentBoneNodeCurrentTM; //this puts in world space
				Point3 x,y,z;
				x = VectorTransform(mirror,mtm.GetRow(0));
				y = VectorTransform(mirror,mtm.GetRow(1));
				z = VectorTransform(mirror,mtm.GetRow(2));
				mtm.SetRow(0,x);
				mtm.SetRow(1,y);
				mtm.SetRow(2,z);
				mtm = mtm * Inverse(copyData->parentBoneNodeCurrentTM);
				mtm.SetRow(3,copyData->morphTargets[i]->boneTMInParentSpace.GetRow(3));
				//put in world space, then by the mirror then back into parent
				targ->morphTargets[currentMorph]->boneTMInParentSpace = copyData->morphTargets[i]->boneTMInParentSpace;
				}
			else targ->morphTargets[currentMorph]->boneTMInParentSpace = copyData->morphTargets[i]->boneTMInParentSpace;

  */
			targ->morphTargets[currentMorph]->influenceAngle = copyData->morphTargets[i]->influenceAngle;
			targ->morphTargets[currentMorph]->falloff = copyData->morphTargets[i]->falloff;

			targ->morphTargets[currentMorph]->name = copyData->morphTargets[i]->name;
			targ->morphTargets[currentMorph]->treeHWND = NULL;
			targ->morphTargets[currentMorph]->weight = copyData->morphTargets[i]->weight;
			targ->morphTargets[currentMorph]->flags = copyData->morphTargets[i]->flags;
			currentMorph++;
			}
		

		}



//clean up temp data
	for (int i = 0; i < mod->localDataList.Count(); i++)
		{
		MorphByBoneLocalData *ld = mod->localDataList[i];
		if (ld)
			{			
			ld->tempPoints.Resize(0);
			ld->tempMatchList.Resize(0);
			}
		}
}



void MorphByBone::CopyCurrent(int id)
{
//get current sel
	int whichBone = 0;
	if (id == -1) 
		whichBone = currentBone;
	else whichBone = id;
//make sure it is a valid node
	if ((whichBone < 0) || (whichBone >= boneData.Count()) ||
		(boneData[whichBone]->morphTargets.Count()==0))
			{
			return;
			}

//send it to the copy buffer
	copyBuffer.CopyToBuffer(boneData[whichBone]);

DebugPrint("copy to buffer %d\n",whichBone);

}

void MorphByBone::PasteMirrorCurrent()
{
//get current sel
	int whichBone = currentBone;
//make sure it is a valid node
	if ((whichBone < 0) || (whichBone >= boneData.Count()) )
			{
			return;
			}

	float threshold = 0.0f;
	pblock->GetValue(pb_mirrorthreshold,0,threshold,FOREVER);

	BOOL flipTM=TRUE;
	pblock->GetValue(pb_mirrortm,0,flipTM,FOREVER);

DebugPrint("paste to buffer %d\n",whichBone);


	copyBuffer.PasteToBuffer(this,boneData[whichBone],mirrorTM,threshold,flipTM);
	
}


void MorphByBone::PasteToMirror()
{
//get current sel
	int whichBone = GetMirrorBone();
	if (whichBone != currentBone)
		this->MirrorMorph(currentBone,whichBone,TRUE);
/*
	if (multiSelectMode)
		{
		for (int i = 0; i < boneData.Count(); i++)
			{
			if (boneData[i]->IsMultiSelected())
				{
				CopyCurrent(i);
				int whichBone = GetMirrorBone(i);
			//make sure it is a valid node
				if ((whichBone < 0) || (whichBone >= boneData.Count()) )
						{
						return;
						}

				float threshold = 0.0f;
				pblock->GetValue(pb_mirrorthreshold,0,threshold,FOREVER);

				BOOL flipTM=TRUE;
				pblock->GetValue(pb_mirrortm,0,flipTM,FOREVER);

				DebugPrint("paste to buffer %d\n",whichBone);


				copyBuffer.PasteToBuffer(this,boneData[whichBone],mirrorTM,threshold,flipTM);
				}

			}
		}
	else
		{
		CopyCurrent();
		int whichBone = GetMirrorBone();
	//make sure it is a valid node
		if ((whichBone < 0) || (whichBone >= boneData.Count()) )
				{
				return;
				}

		float threshold = 0.0f;
		pblock->GetValue(pb_mirrorthreshold,0,threshold,FOREVER);

		BOOL flipTM=TRUE;
		pblock->GetValue(pb_mirrortm,0,flipTM,FOREVER);

		DebugPrint("paste to buffer %d\n",whichBone);


		copyBuffer.PasteToBuffer(this,boneData[whichBone],mirrorTM,threshold,flipTM);
		}
*/	
}


void MorphByBone::MirrorMorph(int sourceIndex, int targetIndex, BOOL mirrorData)
{
	//validate the indices
	if ((sourceIndex < 0) || (sourceIndex >= boneData.Count())) return;
	if ((targetIndex < 0) || (targetIndex >= boneData.Count())) return;

	float threshold = 0.0f;
	pblock->GetValue(pb_mirrorthreshold,0,threshold,FOREVER);

	//get our mirror tm

	theHold.Begin();
	theHold.Put(new MorphUIRestore(this));  //stupid Hack to handle UI update after redo
	

	for (int i = 0; i < localDataList.Count(); i++)
	{

		MorphByBoneLocalData *ld = localDataList[i];
		if (ld)
		{
			//build our vertex look up index
			ld->tempPoints = ld->preDeform;
			ld->tempMatchList.SetCount(ld->tempPoints.Count());
			for (int j = 0; j < ld->preDeform.Count(); j++)
			{
				Matrix3 tm(1);
				//need to put our original points
				ld->tempPoints[j] = ld->tempPoints[j] * mirrorTM;
				ld->tempMatchList[j] = -1;
			}
		
			for (int j = 0; j < ld->tempPoints.Count(); j++)
			{
				Point3 sourcePoint = ld->preDeform[j];
				float closest = 100000.0f;
				int closestID = -1;
				for (int k = 0; k < ld->tempPoints.Count(); k++)
				{
					Point3 mirrorPoint = ld->tempPoints[k];
					float dist = Length(sourcePoint-mirrorPoint);
					if ((dist < threshold) && (dist < closest))
					{
						closest = dist;
						closestID = k;
					}

				}
				ld->tempMatchList[closestID] = j;
			}
	//loop through our targets
			int numberOfNewMorphs = boneData[sourceIndex]->morphTargets.Count();
	
			int currentSourceMorph = 0;
			Matrix3 initialDriverTargetTM = GetNode(targetIndex)->GetNodeTM(GetCOREInterface()->GetTime());
//			Matrix3 initialDriverTargetTM = boneData[targetIndex]->intialBoneNodeTM;
			Matrix3 initialDriverSourceTM = GetNode(sourceIndex)->GetNodeTM(GetCOREInterface()->GetTime());// boneData[sourceIndex]->intialBoneNodeTM;

			Matrix3 tm(1);
			//from world to local
			tm = Inverse(ld->selfNodeCurrentTM); 
			//morph tm in local object space and mirrored
			tm = tm * mirrorTM;
			//back to world space
			tm  = tm  * ld->selfNodeCurrentTM; 
			//now in local space of the target bone
//			tm = tm * Inverse(boneData[targetIndex]->parentBoneNodeCurrentTM);
			
			//now intitial driver tm in world space
			initialDriverSourceTM *= tm;


			for (int j = 0; j < numberOfNewMorphs; j++)
			{
	//create a new morph 
				MorphTargets *morphTarget = boneData[sourceIndex]->morphTargets[currentSourceMorph]->Clone();

		//remap the indices
				
				morphTarget->d.ZeroCount();
				for (int k = 0; k < boneData[sourceIndex]->morphTargets[currentSourceMorph]->d.Count(); k++)
				{
					int vertID = boneData[sourceIndex]->morphTargets[currentSourceMorph]->d[k].vertexID;
					int index = ld->tempMatchList[vertID];
					if (index != -1)
					{
						morphTarget->d.Append(1,&boneData[sourceIndex]->morphTargets[currentSourceMorph]->d[k],500);		
						morphTarget->d[morphTarget->d.Count()-1].vertexID = index;
						morphTarget->d[morphTarget->d.Count()-1].originalPt = morphTarget->d[morphTarget->d.Count()-1].originalPt * mirrorTM;
					}
				}
				if (mirrorData)		
				{
					
					morphTarget->parentTM = boneData[targetIndex]->parentBoneNodeCurrentTM;


	//mirror the source tms
					Matrix3 morphTM = morphTarget->boneTMInParentSpace;
					//put in world space
					morphTM = morphTM * boneData[sourceIndex]->parentBoneNodeCurrentTM;
					//now in world space
					morphTM *= tm;										
	//rebuild the tms
					//put the driver tm in the morph tm space
					Matrix3 newTM = initialDriverTargetTM;
					newTM = newTM * Inverse(initialDriverSourceTM);
					newTM = newTM * morphTM;
					newTM = newTM * Inverse(boneData[targetIndex]->parentBoneNodeCurrentTM);
					
					morphTarget->boneTMInParentSpace = newTM;
					
	//mirror the source deltas
					for (int k = 0; k < morphTarget->d.Count(); k++)
					{
						Point3 vec = morphTarget->d[k].vec; // current bone space
						vec = VectorTransform(vec,boneData[sourceIndex]->intialBoneNodeTM); //world space
						vec = VectorTransform(vec,tm); //world space after mirror
						vec = VectorTransform(vec,Inverse(boneData[targetIndex]->intialBoneNodeTM)); //bone space
						morphTarget->d[k].vec = vec;

						vec = morphTarget->d[k].vecParentSpace; //parent space
						vec = VectorTransform(vec,boneData[sourceIndex]->intialParentNodeTM); //world space
						vec = VectorTransform(vec,tm); //world space after mirror
						vec = VectorTransform(vec,Inverse(boneData[targetIndex]->intialParentNodeTM)); //bone space
						morphTarget->d[k].vecParentSpace = vec;
						
					}


				}

				float angle = 0.0f;

				Matrix3 currentBoneTM = boneData[targetIndex]->currentBoneNodeTM;
				Matrix3 currentParentBoneTM = boneData[targetIndex]->parentBoneNodeCurrentTM;
				currentBoneTM *= Inverse(currentParentBoneTM);
				angle = AngleBetweenTMs(currentBoneTM,morphTarget->boneTMInParentSpace);
				float per = 1.0f - angle/morphTarget->influenceAngle;
				per = GetFalloff(per, morphTarget->falloff,morphTarget->curve);
				if (per < 0.0f) 
					per = 0.0f;
				morphTarget->weight = per;	


				boneData[targetIndex]->morphTargets.Append(1,&morphTarget);
				theHold.Put(new CreateMorphRestore(this,morphTarget));
				currentSourceMorph++;
			}

		}
	}
	theHold.Put(new MorphUIRestore(this));  //stupid Hack to handle UI update after redo

	theHold.Accept(GetString(IDS_MIRRORMORPHS));

	BuildTreeList();

}