/*===========================================================================*\
 | 
 |  FILE:	wM3_core.cpp
 |			Weighted Morpher for MAX R3
 |			ModifyObject
 | 
 |  AUTH:   Harry Denholm
 |			Copyright(c) Kinetix 1999
 |			All Rights Reserved.
 |
 |  HIST:	Started 27-8-98
 | 
\*===========================================================================*/

#include "wM3.h"

void MorphR3::Bez3D(Point3 &b, const Point3 *p, const float &u)
{
	float t01[3], t12[3], t02[3], t13[3];
		
	t01[0] = p[0][0] + (p[1][0] - p[0][0])*u;
	t01[1] = p[0][1] + (p[1][1] - p[0][1])*u;
	t01[2] = p[0][2] + (p[1][2] - p[0][2])*u;
	
	t12[0] = p[1][0] + (p[2][0] - p[1][0])*u;
	t12[1] = p[1][1] + (p[2][1] - p[1][1])*u;
	t12[2] = p[1][2] + (p[2][2] - p[1][2])*u;

	t02[0] = t01[0] + (t12[0] - t01[0])*u;
	t02[1] = t01[1] + (t12[1] - t01[1])*u;
	t02[2] = t01[2] + (t12[2] - t01[2])*u;

	t01[0] = p[2][0] + (p[3][0] - p[2][0])*u;
	t01[1] = p[2][1] + (p[3][1] - p[2][1])*u;
	t01[2] = p[2][2] + (p[3][2] - p[2][2])*u;

	t13[0] = t12[0] + (t01[0] - t12[0])*u;
	t13[1] = t12[1] + (t01[1] - t12[1])*u;
	t13[2] = t12[2] + (t01[2] - t12[2])*u;

	b[0] = t02[0] + (t13[0] - t02[0])*u;
	b[1] = t02[1] + (t13[1] - t02[1])*u;
	b[2] = t02[2] + (t13[2] - t02[2])*u;	
}

void MorphR3::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{
	Update_channelValues();
	// This will see if the local cached object is valid and update it if not
	// It will now also call a full channel rebuild to make sure their deltas are
	// accurate to the new cached object
	if(!MC_Local.AreWeCached())
	{
		UI_MAKEBUSY

		MC_Local.MakeCache(os->obj);

		for(int i=0;i<100;i++)
		{
			if(chanBank[i].mActive)
			{
				chanBank[i].rebuildChannel();
			}
		}

		UI_MAKEFREE
	}

	Interval valid=FOREVER;


	// AUTOLOAD
	int itmp; 
	pblock->GetValue(PB_CL_AUTOLOAD, 0, itmp, valid);

	if(itmp==1)
	{
			for(int k=0;k<100;k++)
			{
				if(chanBank[k].mConnection) {
					chanBank[k].buildFromNode(chanBank[k].mConnection,FALSE,t);
					for(int i=0; i<chanBank[k].mNumProgressiveTargs; i++) { chanBank[k].mTargetCache[i].Init(chanBank[k].mTargetCache[i].mTargetINode); }
				}
			}
	}



	// Get count from host
	int hmCount = os->obj->NumPoints();

	int i, pointnum;
	// to hold percentage
	float fChannelPercent;

	// some worker variables
	float deltX,deltY,deltZ;
	float tmX,tmY,tmZ;
	Point3 vert,fVert;
	double weight,deltW,decay;

	// These are our morph deltas / point
	// They get built by cycling through the points and generating
	// the difference data, summing it into these tables and then
	// appling the changes at the end.
	// This will leave us with the total differences per point on the 
	// local mesh. We can then rip through and apply them quickly
	float *difX = new float[hmCount];
	float *difY = new float[hmCount];
	float *difZ = new float[hmCount];
	double *wgts = new double[hmCount];

	// this is the indicator of what points on the host
	// to update after all deltas have been summed
	BitArray PointsMOD(hmCount);
	PointsMOD.ClearAll();

	int glUsesel;
	pblock->GetValue( PB_OV_USESEL, t, glUsesel, valid);

	BOOL glUseLimit; float glMAX,glMIN;
	pblock->GetValue( PB_OV_USELIMITS, t, glUseLimit, valid);
	pblock->GetValue( PB_OV_SPINMAX, t, glMAX, valid);
	pblock->GetValue( PB_OV_SPINMIN, t, glMIN, valid);


	// --------------------------------------------------- MORPHY BITS
	// cycle through channels, searching for ones to use
	for(i=0;i<100;i++)
	{
		if( chanBank[i].mActive )
		{
			// temp fix for diff. pt counts
			if(chanBank[i].mNumPoints!=hmCount) 
			{
				chanBank[i].mInvalid = TRUE;
				continue;
			};

			// This channel is considered okay to use
			chanBank[i].mInvalid = FALSE;
	
			// Is this channel flagged as inactive?
			if(chanBank[i].mActiveOverride==FALSE) continue;

			// get morph percentage for this channel
			chanBank[i].cblock->GetValue(0,t,fChannelPercent,valid);

			// Clamp the channel values to the limits
			if(chanBank[i].mUseLimit||glUseLimit)
			{
				int Pmax; int Pmin;
				if(glUseLimit)  { Pmax = glMAX; Pmin = glMIN; }
				else
				{
					Pmax = chanBank[i].mSpinmax;
					Pmin = chanBank[i].mSpinmin;
				}
				if(fChannelPercent>Pmax) fChannelPercent = Pmax;
				if(fChannelPercent<Pmin) fChannelPercent = Pmin;
			}


			
			// cycle through all morphable points, build delta arrays
			for(pointnum=0; pointnum < chanBank[i].mNumPoints; pointnum++)
			{
				morphChannel& currentMorChannel = chanBank[i];
				BOOL pointSelected = FALSE;
				if (pointnum < chanBank[i].mSel.GetSize())
					pointSelected = chanBank[i].mSel[pointnum];
				if ( (!chanBank[i].mUseSel && glUsesel==0)  || pointSelected)
				{
					// get the points to morph between
					//vert = os->obj->GetPoint(mIndex);
					//weight = os->obj->GetWeight(mIndex);
					vert = MC_Local.oPoints[pointnum];
					weight = MC_Local.oWeights[pointnum];

					// Get softselection, if applicable
					decay = 1.0f;
					if(os->obj->GetSubselState()!=0) decay = os->obj->PointSelection(pointnum);

					tmX = vert.x; tmY = vert.y; tmZ = vert.z;

					// Add the previous point data into the delta table
					// if its not already been done
					if(!PointsMOD[pointnum])
					{
						difX[pointnum]=vert.x;
						difY[pointnum]=vert.y;
						difZ[pointnum]=vert.z;
						wgts[pointnum]=weight;
					}
		
					// calculate the differences
					// decay by the weighted vertex amount, to support soft selection
					deltW=(chanBank[i].mWeights[pointnum]-weight)/100.0f*(double)fChannelPercent;
					wgts[pointnum]+=deltW;

					if(!chanBank[i].mNumProgressiveTargs)
					{
						deltX=((chanBank[i].mDeltas[pointnum].x)*fChannelPercent)*decay;
						deltY=((chanBank[i].mDeltas[pointnum].y)*fChannelPercent)*decay;
						deltZ=((chanBank[i].mDeltas[pointnum].z)*fChannelPercent)*decay;
					}
					else  
					{	//DO PROGRESSIVE MORPHING

						// worker variables for progressive modification
						float fProgression, legnth, totaltargs;
						int segment;
						Point3 endpoint[4];
						Point3 splinepoint[4];
						Point3 temppoint[2];
						Point3 progession;

						totaltargs = chanBank[i].mNumProgressiveTargs +1;

						fProgression = fChannelPercent;
						if (fProgression<0) fProgression=0;
						if(fProgression>100) fProgression=100;
						segment=1; 
						while(segment<=totaltargs && fProgression >= chanBank[i].GetTargetPercent(segment-2)) segment++;
						//figure out which segment we are in
						//on the target is the next segment
						//first point (object) MC_Local.oPoints
						//second point (first target) is chanBank[i].mPoints
						//each additional target is targetcache starting from 0
						if(segment==1) {
							endpoint[0]= MC_Local.oPoints[pointnum]; 
							endpoint[1]= chanBank[i].mPoints[pointnum];
							endpoint[2]= chanBank[i].mTargetCache[0].GetPoint(pointnum);
						}
						else if(segment==totaltargs) {
							int targnum= totaltargs-1;
							for(int j=2; j>=0; j--) {
								targnum--;
								if(targnum==-2) temppoint[0]=MC_Local.oPoints[pointnum];
								else if(targnum==-1) temppoint[0]=chanBank[i].mPoints[pointnum];
								else temppoint[0]=chanBank[i].mTargetCache[targnum].GetPoint(pointnum);
								endpoint[j]= temppoint[0]; 
							}
						}
						else {
							int targnum= segment;
							for( int j=3; j>=0; j--) {
								targnum--;
								if(targnum==-2) temppoint[0]=MC_Local.oPoints[pointnum];
								else if(targnum==-1) temppoint[0]=chanBank[i].mPoints[pointnum];
								else temppoint[0]=chanBank[i].mTargetCache[targnum].GetPoint(pointnum);
								endpoint[j]= temppoint[0]; 
							}
						}

						//set the middle knot vectors
						if(segment==1) {
							splinepoint[0] = endpoint[0];
							splinepoint[3] = endpoint[1];
							temppoint[1] = endpoint[2] - endpoint[0];
							temppoint[0] = endpoint[1] - endpoint[0];
							legnth = FLength(temppoint[1]); legnth *= legnth;
							if(legnth==0.0f) {
								splinepoint[1] = endpoint[0];
								splinepoint[2] = endpoint[1];
							}
							else { 
								splinepoint[2] = endpoint[1] - 
									(DotProd(temppoint[0], temppoint[1]) * chanBank[i].mCurvature / legnth) * temppoint[1];
								splinepoint[1] = endpoint[0] +  
									chanBank[i].mCurvature * (splinepoint[2]-endpoint[0]);
							}

						}
						else if (segment==totaltargs) {
							splinepoint[0] = endpoint[1];
							splinepoint[3] = endpoint[2];
							temppoint[1] = endpoint[2] - endpoint[0];
							temppoint[0] = endpoint[1] - endpoint[2];
							legnth = FLength(temppoint[1]); legnth *= legnth;
							if(legnth==0.0f) {
								splinepoint[1] = endpoint[0];
								splinepoint[2] = endpoint[1];
							}
							else {
								Point3 p = (DotProd(temppoint[1], temppoint[0]) * chanBank[i].mCurvature / legnth) * temppoint[1];
								splinepoint[1] = endpoint[1] - 
									(DotProd(temppoint[1], temppoint[0]) * chanBank[i].mCurvature / legnth) * temppoint[1];
								splinepoint[2] = endpoint[2] +  
									chanBank[i].mCurvature * (splinepoint[1]-endpoint[2]);
							}
						}
						else {
							temppoint[1] = endpoint[2] - endpoint[0];
							temppoint[0] = endpoint[1] - endpoint[0];
							legnth = FLength(temppoint[1]); legnth *= legnth;
							splinepoint[0] = endpoint[1];
							splinepoint[3] = endpoint[2];
							if(legnth==0.0f) { splinepoint[1] = endpoint[0]; }
							else {
								splinepoint[1] = endpoint[1] + 
								(DotProd(temppoint[0], temppoint[1]) * chanBank[i].mCurvature / legnth) * temppoint[1];
							}
							temppoint[1] = endpoint[3] - endpoint[1];
							temppoint[0] = endpoint[2] - endpoint[1];
							legnth = FLength(temppoint[1]); legnth *= legnth;
							if(legnth==0.0f) { splinepoint[2] = endpoint[1]; }
							else {
								splinepoint[2] = endpoint[2] - 
									(DotProd(temppoint[0], temppoint[1]) * chanBank[i].mCurvature / legnth) * temppoint[1];
							}
						}

						// this is the normalizing equation
						float targetpercent1, targetpercent2, u;
						targetpercent1 = chanBank[i].GetTargetPercent(segment-3);
						targetpercent2 = chanBank[i].GetTargetPercent(segment-2);
						
						float top = fProgression-targetpercent1;
						float bottom = targetpercent2-targetpercent1;
						u = top/bottom;

						////this is just the bezier calculation
						Bez3D(progession, splinepoint, u);
						deltX= (progession[0]-vert.x)*decay;
						deltY= (progession[1]-vert.y)*decay;
						deltZ= (progession[2]-vert.z)*decay;

					}
					
					difX[pointnum]+=deltX;
					difY[pointnum]+=deltY;
					difZ[pointnum]+=deltZ;
					// We've modded this point
					PointsMOD.Set(pointnum);

				} // msel check

			} // nmPts cycle
		}
	}

	// Cycle through all modified points and apply delta arrays
	for(int k=0;k<hmCount;k++)
	{
		if(PointsMOD[k]&&(MC_Local.sel[k]||os->obj->GetSubselState()==0))
		{
			fVert.x = difX[k];
			fVert.y = difY[k];
			fVert.z = difZ[k];
			os->obj->SetPoint(k,fVert);
			os->obj->SetWeight(k,wgts[k]);
		}


		// Captain Hack Returns...
		// Support for saving of modifications to a channel
		// Most of this is just duped from buildFromNode (delta/point/calc)
		if (recordModifications)
		{
			int tChan = recordTarget;

			int tPc = hmCount;

			// Prepare the channel
			chanBank[tChan].AllocBuffers(tPc, tPc);
			chanBank[tChan].mNumPoints = 0;

			int id = 0;
			Point3 DeltP;
			double wtmp;
			Point3 tVert;

			for(int x=0;x<tPc;x++)
			{

				if(PointsMOD[x])
				{
					tVert.x = difX[x];
					tVert.y = difY[x];
					tVert.z = difZ[x];

					wtmp = wgts[x];
				}
				else
				{
					tVert = os->obj->GetPoint(x);
					wtmp = os->obj->GetWeight(x);
				}
				// calculate the delta cache
				DeltP.x=(tVert.x-MC_Local.oPoints[x].x)/100.0f;
				DeltP.y=(tVert.y-MC_Local.oPoints[x].y)/100.0f;
				DeltP.z=(tVert.z-MC_Local.oPoints[x].z)/100.0f;
				chanBank[tChan].mDeltas[x] = DeltP;

				chanBank[tChan].mWeights[x] = wtmp;

				chanBank[tChan].mPoints[x] = tVert;
				chanBank[tChan].mNumPoints++;
			}

			recordModifications = FALSE;
			recordTarget = 0;
			chanBank[tChan].mInvalid = FALSE;

		}
		// End of record

	}

	// clean up
	if(difX) delete [] difX;
	if(difY) delete [] difY;
	if(difZ) delete [] difZ;
	if(wgts) delete [] wgts;

	if(itmp==1) valid = Interval(t,t);

	// Update all the caches etc
	os->obj->UpdateValidity(GEOM_CHAN_NUM,valid);
	os->obj->PointsWereChanged();
}

void MorphR3::DisplayMemoryUsage(void )
{
	if(!hwAdvanced) return;
	float tmSize = sizeof(*this);
	for(int i=0;i<100;i++) tmSize += chanBank[i].getMemSize();
	char s[20];
	sprintf(s,"%i KB", (int)tmSize/1000);
	SetWindowText(GetDlgItem(hwAdvanced,IDC_MEMSIZE),s);	
}


BOOL IMorphClass::AddProgessiveMorph(MorphR3 *mp,int morphIndex, INode *node)
{
	Interval valid; 
	
	if (mp->ip == NULL) return FALSE;

	ObjectState os = node->GetObjectRef()->Eval(mp->ip->GetTime());

	if( os.obj->IsDeformable() == FALSE ) return FALSE;

	// Check for same-num-of-verts-count
	if( os.obj->NumPoints()!= mp->MC_Local.Count) return FALSE;

	node->BeginDependencyTest();
	mp->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) {		
		return FALSE;
	}

	// check to make sure that the max number of progressive targets will not be exceeded
	//
	morphChannel &bank = mp->chanBank[morphIndex];
	if(bank.mConnection ) 
	{
		if ( bank.mNumProgressiveTargs >= MAX_TARGS )
			return FALSE;
	}


	if( mp->CheckMaterialDependency() ) return FALSE;
	// Make the node reference, and then ask the channel to load itself
		
	if (theHold.Holding())
		theHold.Put(new Restore_FullChannel(mp, morphIndex));

	if(bank.mConnection ) 
	{
		int refnum = (morphIndex*MAX_TARGS) + 201 + bank.mNumProgressiveTargs;
        mp->ReplaceReference(refnum,node);
		bank.InitTargetCache(bank.mNumProgressiveTargs,node);
		bank.mNumProgressiveTargs++;
		assert(bank.mNumProgressiveTargs<=MAX_TARGS);
		bank.ReNormalize();
		mp->Update_channelParams();
	}
	return TRUE;
}

BOOL IMorphClass::DeleteProgessiveMorph(MorphR3 *mp,int morphIndex, int progressiveMorphIndex)
{

	morphChannel &bank = mp->chanBank[morphIndex];
	

	if(!&bank || !bank.mNumProgressiveTargs) return FALSE;

	int targetnum = progressiveMorphIndex;

	if(bank.iTargetListSelection<0) return FALSE;
	if (theHold.Holding())
		theHold.Put(new Restore_FullChannel(mp, morphIndex) );

	if(targetnum==0)
	{
		if(bank.mNumProgressiveTargs)
		{
			bank = bank.mTargetCache[0];
			for(int i=0; i<bank.mNumProgressiveTargs-1; i++) {
				bank.mTargetCache[i] = bank.mTargetCache[i+1];
			}
			bank.mTargetCache[bank.mNumProgressiveTargs-1].Clear();
		}
	}
	else if(targetnum>0 && bank.mNumProgressiveTargs && targetnum<=bank.mNumProgressiveTargs)
	{
		for(int i=targetnum; i<bank.mNumProgressiveTargs; i++) {
				bank.mTargetCache[i-1] = bank.mTargetCache[i];
		}
		bank.mTargetCache[bank.mNumProgressiveTargs-1].Clear();
	}

	//reset the references
	bank.ResetRefs(mp, progressiveMorphIndex);

	bank.mNumProgressiveTargs--;

	bank.ReNormalize();
	if(bank.iTargetListSelection) bank.iTargetListSelection--;

	bank.rebuildChannel();
	mp->Update_channelFULL();

	if (theHold.Holding())
	{
		theHold.Put( new Restore_Display( mp ) );
	}

	return TRUE;	
}

void IMorphClass::SwapMorphs(MorphR3 *mp,const int from, const int to, BOOL swap)
{

	if (theHold.Holding())
	{
		theHold.Put( new Restore_Display( mp ) );
		theHold.Put(new Restore_FullChannel(mp, from, FALSE));
		theHold.Put(new Restore_FullChannel(mp, to, FALSE));
	}
	
	if (swap)
		mp->ChannelOp(from,to,OP_SWAP);
	else mp->ChannelOp(from,to,OP_MOVE);

	if (theHold.Holding())
	{
		theHold.Put( new Restore_Display( mp ) );
	}

}


void IMorphClass::SwapPTargets(MorphR3 *mp,const int morphIndex, const int from, const int to, const bool isundo)
{
	/////////////////////////////////
	int currentChan = morphIndex;
	morphChannel &cBank =  mp->chanBank[currentChan];
	if(from<0 || to<0 || from>cBank.NumProgressiveTargets() || to>cBank.NumProgressiveTargets()) return;


	if(from!=0 && to!=0) 
	{
		TargetCache toCache(cBank.mTargetCache[to-1]);
		
		float wa,wb;
		wa = cBank.mTargetCache[to-1].mTargetPercent;
		wb = cBank.mTargetCache[from-1].mTargetPercent;

		cBank.mTargetCache[to-1] = cBank.mTargetCache[from-1]; 
		cBank.mTargetCache[from-1] = toCache; 

		cBank.mTargetCache[to-1].mTargetPercent = wb;
		cBank.mTargetCache[from-1].mTargetPercent = wa;

		mp->ReplaceReference(mp->GetRefNumber(currentChan, from), cBank.mTargetCache[from-1].RefNode() );
		mp->ReplaceReference(mp->GetRefNumber(currentChan, to), cBank.mTargetCache[to-1].RefNode());
	}
	else //switch channel and first targetcache
	{

		float wa,wb;
		wa = cBank.mTargetCache[0].mTargetPercent;
		wb = cBank.mTargetPercent;

		TargetCache tempCache(cBank.mTargetCache[0]);
		cBank.mTargetCache[0] = cBank;
		cBank = tempCache;

		cBank.mTargetCache[0].mTargetPercent = wb;
		cBank.mTargetPercent = wa;


		mp->ReplaceReference(101+currentChan, cBank.mConnection );
		mp->ReplaceReference(mp->GetRefNumber(currentChan, 0), cBank.mTargetCache[0].RefNode() );		
	}


}


void IMorphClass::SetTension(MorphR3 *mp, int morphIndex, float tension )
{
	if (theHold.Holding())
		theHold.Put( new Restore_FullChannel(mp, morphIndex ));

	mp->chanBank[morphIndex].mCurvature = tension;

	if (theHold.Holding())
		theHold.Put( new Restore_Display( mp ) );
	
 	mp->NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	mp->ip->RedrawViews(mp->ip->GetTime(),REDRAW_INTERACTIVE,NULL);
	
}

void IMorphClass::HoldMarkers(MorphR3 *mp) 
{
	if (theHold.Holding()) theHold.Put(new Restore_Marker(mp)); 
};

void IMorphClass::HoldChannel(MorphR3 *mp, int channel) 
{
	if (theHold.Holding()) 
		theHold.Put(new Restore_FullChannel(mp, channel) );
}