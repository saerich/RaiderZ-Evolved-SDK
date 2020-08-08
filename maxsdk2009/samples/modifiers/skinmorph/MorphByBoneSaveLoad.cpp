
#include "MorphByBone.h"

#define BONE_COUNT				0x2000
#define BONE_INITIALNODETM		0x2010
#define BONE_INITIALOBJECTTM	0x2020
#define BONE_INITIALPARENTNODETM 0x2025
#define BONE_LOCALBOUNDS		0x2030

#define BONE_MORPHCOUNT			0x2040

#define BONE_MORPHPOINTCOUNT	0x2050
#define BONE_MORPHPOINTDATA		0x2055
#define BONE_MORPHTM			0x2057
#define BONE_MORPHPARENTTM		0x2058
#define BONE_MORPHINFLUENCE		0x2060
#define BONE_MORPHFALLOFF		0x2070
#define BONE_MORPHNAMECOUNT		0x2080
#define BONE_MORPHNAME			0x2090
#define BONE_MORPHFLAGS			0x2100
#define BONE_MORPHEXNODE		0x2110
#define BONE_MORPHEXGRAPH		0x2120
#define BONE_MORPHISPLANARJOINT	0x2130


IOResult MorphByBone::Load(ILoad *iload)
{
	//TODO: Add code to allow plugin to load its data
	//TODO: Add code to allow plugin to load its data
	IOResult	res;
	ULONG		nb;

	int boneIndex = -1;
	int morphIndex = -1;
	int nameLength = -1;
	while (IO_OK==(res=iload->OpenChunk())) 
		{
		switch(iload->CurChunkID())  
			{
			case BONE_COUNT:
				{
				int boneCount = 0;
				iload->Read(&boneCount,sizeof(int), &nb);
				boneData.SetCount(boneCount);
				for (int i = 0; i < boneCount; i++)
				{
					boneData[i] = new MorphByBoneData();
					boneData[i]->SetBallJoint();
				}

				break;
				}
			case BONE_INITIALNODETM:
				{
				boneIndex++;
				res = boneData[boneIndex]->intialBoneNodeTM.Load(iload);
				break;
				}

			case BONE_INITIALOBJECTTM:
				{
				res = boneData[boneIndex]->intialBoneObjectTM.Load(iload);
				break;
				}

			case BONE_INITIALPARENTNODETM:
				{
				res = boneData[boneIndex]->intialParentNodeTM.Load(iload);
				break;
				}

			case BONE_LOCALBOUNDS:
				{
				Point3 min,max;
				iload->Read(&min,sizeof(min), &nb);
				iload->Read(&max,sizeof(max), &nb);

				boneData[boneIndex]->localBounds.Init();
				boneData[boneIndex]->localBounds += min;
				boneData[boneIndex]->localBounds += max;
				break;
				}
			case BONE_MORPHISPLANARJOINT:
				{
					boneData[boneIndex]->SetPlanarJoint();
				}
			case BONE_MORPHCOUNT:
				{
				int morphCount = 0;
				iload->Read(&morphCount,sizeof(int), &nb);
				boneData[boneIndex]->morphTargets.SetCount(morphCount);
				for (int i = 0; i < morphCount; i++)
					boneData[boneIndex]->morphTargets[i] = new MorphTargets();

				morphIndex = -1;
				break;
				}
			case BONE_MORPHPOINTCOUNT:
				{
				morphIndex++;
				int pointCount;
				iload->Read(&pointCount,sizeof(int), &nb);
				boneData[boneIndex]->morphTargets[morphIndex]->d.SetCount(pointCount);
				break;
				}

			case BONE_MORPHPOINTDATA:
				{

				int ct = boneData[boneIndex]->morphTargets[morphIndex]->d.Count();
				if (ct)
					iload->Read(boneData[boneIndex]->morphTargets[morphIndex]->d.Addr(0),sizeof(MorphTargets)*ct, &nb);
				
				break;
				}
			case BONE_MORPHTM:
				{
				res = boneData[boneIndex]->morphTargets[morphIndex]->boneTMInParentSpace.Load(iload);
				break;
				}
			case BONE_MORPHPARENTTM:
				{
				res = boneData[boneIndex]->morphTargets[morphIndex]->parentTM.Load(iload);
				break;
				}


			case BONE_MORPHINFLUENCE:
				{
				float influence;
				iload->Read(&influence,sizeof(influence), &nb);
				boneData[boneIndex]->morphTargets[morphIndex]->influenceAngle = influence;
				break;
				}
			case BONE_MORPHFALLOFF:
				{
				int falloff;
				iload->Read(&falloff,sizeof(falloff), &nb);
				boneData[boneIndex]->morphTargets[morphIndex]->falloff = falloff;
				break;
				}

/*			case BONE_MORPHNAMECOUNT:
				{
				iload->Read(&nameLength,sizeof(nameLength), &nb);
				break;
				}
*/
			case BONE_MORPHNAME:
				{
				TCHAR *name;
				iload->ReadWStringChunk(&name);
				boneData[boneIndex]->morphTargets[morphIndex]->name.printf("%s",name);
				break;
				}
			case BONE_MORPHFLAGS:
				{
				int flags;
				iload->Read(&flags,sizeof(flags), &nb);
				boneData[boneIndex]->morphTargets[morphIndex]->flags = flags;
				break;
				}
			case BONE_MORPHEXNODE:
				{
				int exNodeID;
				iload->Read(&exNodeID,sizeof(exNodeID), &nb);
				boneData[boneIndex]->morphTargets[morphIndex]->externalNodeID = exNodeID;
				break;
				}

			case BONE_MORPHEXGRAPH:
				{
				int exGraphID;
				iload->Read(&exGraphID,sizeof(exGraphID), &nb);
				boneData[boneIndex]->morphTargets[morphIndex]->externalFalloffID = exGraphID;
				break;
				}




			}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
		}

	for (int i = 0;i < boneData.Count(); i++)
	{
		int numberOfMorphs = boneData[i]->morphTargets.Count();
		for (int j = 0; j < numberOfMorphs; j++)
		{
			if (boneData[i]->morphTargets[j]->IsDead())
				boneData[i]->morphTargets[j]->d.ZeroCount();

		}

	}

	for (int i = 0;i < boneData.Count(); i++)
	{
		int numberOfMorphs = boneData[i]->morphTargets.Count();
		BOOL validVec = TRUE;
		for (int j = 0; j < numberOfMorphs; j++)
		{
			MorphTargets *morph = boneData[i]->morphTargets[j];
			if (!morph->IsDead())
			{

				int largestInt = 0;
				for (int k = 0; k < morph->d.Count(); k++)
				{
					if (morph->d[k].vertexID > largestInt)
						largestInt = morph->d[k].vertexID;
				}
				Tab<int> hits;
				largestInt++;
				hits.SetCount(largestInt);
				for (int k = 0; k < largestInt; k++)
				{
					hits[k] = 0;
				}
				for (int k = 0; k < morph->d.Count(); k++)
				{

					hits[morph->d[k].vertexID] += 1;
				}
				
				for (int k = 0; k < largestInt; k++)
				{
					
					if (hits[k] > 1)
					{
						validVec = FALSE;
						int firstIndex = -1;
						for (int m = 0; m < morph->d.Count(); m++)
						{
							if (morph->d[m].vertexID == k)
							{
								if (firstIndex == -1)
									firstIndex = m;
								else
								{
									morph->d[firstIndex].vec += morph->d[m].vec;
									morph->d[firstIndex].vecParentSpace += morph->d[m].vecParentSpace;
									morph->d.Delete(m,1);
									m--;
								}
							}
						}
					}
				}
				

			}

		}

	}

	return IO_OK;

}


IOResult MorphByBone::Save(ISave *isave)
{
	//TODO: Add code to allow plugin to save its data
//save bone data
	IOResult	res;
	ULONG		nb;

	int boneCount = boneData.Count();

	isave->BeginChunk(BONE_COUNT);
	res = isave->Write(&boneCount, sizeof(boneCount), &nb);
	isave->EndChunk();

	for (int i = 0; i < boneCount; i++)
		{
	
		isave->BeginChunk(BONE_INITIALNODETM);
		res = boneData[i]->intialBoneNodeTM.Save(isave);
		isave->EndChunk();

		isave->BeginChunk(BONE_INITIALOBJECTTM);
		res = boneData[i]->intialBoneObjectTM.Save(isave);
		isave->EndChunk();

		isave->BeginChunk(BONE_INITIALPARENTNODETM);
		res = boneData[i]->intialParentNodeTM.Save(isave);
		isave->EndChunk();


		if (boneData[i]->IsPlanarJoint())
		{			
			isave->BeginChunk(BONE_MORPHISPLANARJOINT);
			isave->EndChunk();
		}

		isave->BeginChunk(BONE_LOCALBOUNDS);
		Point3 min,max;
		min = boneData[i]->localBounds.pmin;
		max = boneData[i]->localBounds.pmax;
		res = isave->Write(&min, sizeof(min), &nb);
		res = isave->Write(&max, sizeof(max), &nb);
		isave->EndChunk();


		int morphCount = boneData[i]->morphTargets.Count();
		isave->BeginChunk(BONE_MORPHCOUNT);
		res = isave->Write(&morphCount, sizeof(morphCount), &nb);
		isave->EndChunk();

		for (int j = 0; j < morphCount; j++)
			{
			int pointCount = boneData[i]->morphTargets[j]->d.Count();

			isave->BeginChunk(BONE_MORPHPOINTCOUNT);
			res = isave->Write(&pointCount, sizeof(pointCount), &nb);
			isave->EndChunk();

			if (pointCount > 0)
			{
				isave->BeginChunk(BONE_MORPHPOINTDATA);
				res = isave->Write(boneData[i]->morphTargets[j]->d.Addr(0), sizeof(MorphData)*pointCount, &nb);
				isave->EndChunk();
			}

			isave->BeginChunk(BONE_MORPHTM);
			res = boneData[i]->morphTargets[j]->boneTMInParentSpace.Save(isave);
			isave->EndChunk();

			isave->BeginChunk(BONE_MORPHPARENTTM);
			res = boneData[i]->morphTargets[j]->parentTM.Save(isave);
			isave->EndChunk();

			

			
			float influence = boneData[i]->morphTargets[j]->influenceAngle;
			isave->BeginChunk(BONE_MORPHINFLUENCE);
			res = isave->Write(&influence, sizeof(influence), &nb);
			isave->EndChunk();

			int falloff = boneData[i]->morphTargets[j]->falloff;
			isave->BeginChunk(BONE_MORPHFALLOFF);
			res = isave->Write(&falloff, sizeof(falloff), &nb);
			isave->EndChunk();

//			int nameLength = boneData[i]->morphTargets[j]->name.Length();
			TCHAR *name = boneData[i]->morphTargets[j]->name.data();
//			isave->BeginChunk(BONE_MORPHNAMECOUNT);
//			res = isave->Write(&nameLength, sizeof(nameLength), &nb);
//			isave->EndChunk();
			isave->BeginChunk(BONE_MORPHNAME);
			res = isave->WriteWString(name);
//			res = isave->Write(&name, sizeof(TCHAR)*nameLength, &nb);
			isave->EndChunk();



			int flags = boneData[i]->morphTargets[j]->flags;
			isave->BeginChunk(BONE_MORPHFLAGS);
			res = isave->Write(&flags, sizeof(flags), &nb);
			isave->EndChunk();

			int exNodeID = boneData[i]->morphTargets[j]->externalNodeID;
			isave->BeginChunk(BONE_MORPHEXNODE);
			res = isave->Write(&exNodeID, sizeof(exNodeID), &nb);
			isave->EndChunk();

			int exGraphID = boneData[i]->morphTargets[j]->externalFalloffID;
			isave->BeginChunk(BONE_MORPHEXGRAPH);
			res = isave->Write(&exGraphID, sizeof(exGraphID), &nb);
			isave->EndChunk();



			}


		}

	
	return IO_OK;
}


#define LD_ID					0x2300
#define LD_SELFNODETM			0x2310
#define LD_SELFOBJECTTM			0x2320
#define LD_INITIALPOINTS		0x2330


IOResult MorphByBone::SaveLocalData(ISave *isave, LocalModData *pld)
{
	//save morphs
	MorphByBoneLocalData *p;
	IOResult	res;
	ULONG		nb;

	p = (MorphByBoneLocalData*)pld;

	isave->BeginChunk(LD_ID);
	res = isave->Write(&p->id, sizeof(p->id), &nb);
	isave->EndChunk();


	isave->BeginChunk(LD_SELFNODETM);
	res = p->selfNodeTM.Save(isave);
	isave->EndChunk();


	isave->BeginChunk(LD_SELFOBJECTTM);
	res = p->selfObjectTM.Save(isave);
	isave->EndChunk();

	if (p->originalPoints.Count())
	{
		isave->BeginChunk(LD_INITIALPOINTS);
		int ct = p->originalPoints.Count();
		res = isave->Write(&ct, sizeof(ct), &nb);
		res = isave->Write(p->originalPoints.Addr(0), sizeof(Point3)*ct, &nb);
		isave->EndChunk();
	}



	return IO_OK;

}
IOResult MorphByBone::LoadLocalData(ILoad *iload, LocalModData **pld)
{
	IOResult	res;
	ULONG		nb;

	MorphByBoneLocalData *p= new MorphByBoneLocalData();
	while (IO_OK==(res=iload->OpenChunk())) 
		{
		switch(iload->CurChunkID())  
			{

			case LD_ID:
				{
				iload->Read(&p->id,sizeof(p->id), &nb);
				break;
				}
			case LD_SELFNODETM:
				{
				p->selfNodeTM.Load(iload);
				
				break;
				}
			case LD_SELFOBJECTTM:
				{
				p->selfObjectTM.Load(iload);

				break;
				}
			case LD_INITIALPOINTS:
				{
				int ct = 0;
				iload->Read(&ct,sizeof(ct), &nb);
				p->originalPoints.SetCount(ct);
				iload->Read(p->originalPoints.Addr(0),sizeof(Point3)*ct, &nb);

				break;
				}


			}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
		}

	*pld = p;


	return IO_OK;
}
