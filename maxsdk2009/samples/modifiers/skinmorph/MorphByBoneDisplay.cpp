#include "MorphByBone.h"

static GenSubObjType SOT_Points(13);

void MorphByBone::DrawBox(GraphicsWindow *gw, Box3 box, float size, BOOL makeThin, Matrix3 *tm )
{
	Point3 pt[3];

	if (makeThin)
	{
		Box3 tempBox = box;
		int longAxis = 0;
		float longAxisDist = box.pmax.x - box.pmin.x;

		float dist = box.pmax.y - box.pmin.y;
		if (dist > longAxisDist)
		{
			longAxisDist = dist;
			longAxis = 1;
		}
		dist = box.pmax.z - box.pmin.z;
		if (dist > longAxisDist)
		{
			longAxisDist = dist;
			longAxis = 2;
		}
		for (int i = 0; i < 3; i++)
		{
			if (i != longAxis)
			{
				tempBox.pmax[i] = box.pmin[i] + (box.pmax[i]-box.pmin[i]) *0.5f;
				tempBox.pmin[i] = tempBox.pmax[i];
			}
		}
		if (tm != NULL)
		{
			pt[0] = tempBox.pmax * *tm;
			pt[1] = tempBox.pmin * *tm;
		}
		else
		{
			pt[0] = tempBox.pmax ;
			pt[1] = tempBox.pmin;
		}
		gw->segment(pt,1);

 	}
	else 
	{
		box.Scale(size);

		Point3 boxPts[8];
		for (int i = 0; i < 8; i++)
		{
			if (tm != NULL)
				boxPts[i] = box[i] * *tm;
			else boxPts[i] = box[i];
		}


		pt[0] = boxPts[2];
		pt[1] = boxPts[3];
		gw->segment(pt,1);

		pt[0] = boxPts[3];
		pt[1] = boxPts[1];
		gw->segment(pt,1);

		pt[0] = boxPts[1];
		pt[1] = boxPts[0];
		gw->segment(pt,1);

		pt[0] = boxPts[0];
		pt[1] = boxPts[2];
		gw->segment(pt,1);



		pt[0] = boxPts[2+4];
		pt[1] = boxPts[3+4];
		gw->segment(pt,1);

		pt[0] = boxPts[3+4];
		pt[1] = boxPts[1+4];
		gw->segment(pt,1);

		pt[0] = boxPts[1+4];
		pt[1] = boxPts[0+4];
		gw->segment(pt,1);

		pt[0] = boxPts[0+4];
		pt[1] = boxPts[2+4];
		gw->segment(pt,1);




		pt[0] = boxPts[2];
		pt[1] = boxPts[2+4];
		gw->segment(pt,1);

		pt[0] = boxPts[3];
		pt[1] = boxPts[3+4];
		gw->segment(pt,1);

		pt[0] = boxPts[1];
		pt[1] = boxPts[1+4];
		gw->segment(pt,1);

		pt[0] = boxPts[2];
		pt[1] = boxPts[2+4];
		gw->segment(pt,1);
	}

}


void MorphByBone::Draw3dEdge(GraphicsWindow *gw, float size, Box3 box, Color c, Matrix3 *mtm )

{

	int longAxis = 0;
	float longAxisDist = box.pmax.x - box.pmin.x;

	float dist = box.pmax.y - box.pmin.y;
	if (dist > longAxisDist)
	{
		longAxisDist = dist;
		longAxis = 1;
	}
	dist = box.pmax.z - box.pmin.z;
	if (dist > longAxisDist)
	{
		longAxisDist = dist;
		longAxis = 2;
	}
	for (int i = 0; i < 3; i++)
	{
		if (i != longAxis)
		{
			box.pmax[i] = box.pmin[i] + (box.pmax[i]-box.pmin[i]) *0.5f;
			box.pmin[i] = box.pmax[i];
		}
	}
	Point3 a = box.pmax;
	Point3 b = box.pmin;
	if (mtm != NULL)
	{
		a = a * *mtm;
		b = b * *mtm;
	}

	Matrix3 tm;
	Point3 vec = Normalize(a-b);
	MatrixFromNormal(vec, tm);
	Point3 vecA,vecB,vecC;
	vecA = tm.GetRow(0)*size;
	vecB = tm.GetRow(1)*size;
	Point3 p[3];
	Point3 color[3];
	color[0] = c;
	color[1] = c;
	color[2] = c;
	
	p[0] = a + vecA + vecB;
	p[1] = b + vecA + vecB;
	p[2] = a - vecA + vecB;
	gw->triangle(p,color);

	p[0] = b - vecA + vecB;
	p[1] = a - vecA + vecB;
	p[2] = b + vecA + vecB;
	gw->triangle(p,color);

	p[2] = a + vecA - vecB;
	p[1] = b + vecA - vecB;
	p[0] = a - vecA - vecB;
	gw->triangle(p,color);

	p[2] = b - vecA - vecB;
	p[1] = a - vecA - vecB;
	p[0] = b + vecA - vecB;
	gw->triangle(p,color);


	p[2] = a + vecB + vecA;
	p[1] = b + vecB + vecA;
	p[0] = a - vecB + vecA;
	gw->triangle(p,color);

	p[2] = b - vecB + vecA;
	p[1] = a - vecB + vecA;
	p[0] = b + vecB + vecA;
	gw->triangle(p,color);

	p[0] = a + vecB - vecA;
	p[1] = b + vecB - vecA;
	p[2] = a - vecB - vecA;
	gw->triangle(p,color);

	p[0] = b - vecB - vecA;
	p[1] = a - vecB - vecA;
	p[2] = b + vecB - vecA;
	gw->triangle(p,color);


}
void MorphByBone::DrawCap(GraphicsWindow *gw,Matrix3 tm, int axis, float dist, float angle, Point3 color)
{
	dist *= 0.98f;
	Point3 tmvec = tm.GetRow(axis);
	Matrix3 gwTM(1);
	MatrixFromNormal(tmvec,gwTM);

	gwTM.SetRow(3,tm.GetRow(3));

	gw->setTransform(gwTM);

	Point3 vec = Point3(0.0f,0.0f,dist);
	int uSegs = 32;
	int vSegs = 6;
	Point3 p1[6];
	Point3 p2[6];

	float uAngle = PI*2.0f/32.0f;
	float vAngle = angle/6.0f;


	Material mtl;
	mtl.opacity = 1.0f;
	mtl.selfIllum = 0.85f;
	mtl.Kd = color;

	Point3 fillColor[5];
	fillColor[0] = color;
	fillColor[1] = color;
	fillColor[2] = color;
	fillColor[3] = color;

	mtl.Ka[0] = mtl.Ka[1] = mtl.Ka[2] = 0.0f;
	mtl.Ks[0] = mtl.Ks[1] = mtl.Ks[2] = 0.0f;

	gw->setMaterial(mtl);


	gw->startTriangles();

	for (int i = 0; i < 33; i++)
	{
		Matrix3 uTM(1);
		uTM.RotateZ(uAngle * i);
		for (int j = 0; j < 6; j++)
		{
			Matrix3 vTM(1);
			vTM.RotateX(vAngle *j);
			Point3 drawVec = VectorTransform(vTM*uTM,vec);
			p1[j] = drawVec;
		}

		if (i != 0)
		{
			for (int j = 1; j < 6; j++)
			{
				Point3 pt[4];
				pt[0] = p1[j-1];
				pt[1] = p1[j];
				pt[2] = p2[j-1];
				gw->triangle(pt, fillColor);
				pt[0] = p2[j];
				pt[1] = p2[j-1];
				pt[2] = p1[j];
				gw->triangle(pt, fillColor);
			}
		}
		for (int j = 0; j < 6; j++)
			p2[j] = p1[j];
	}
	gw->endTriangles();

}

void  MorphByBone::DrawArc(GraphicsWindow *gw, float dist, float angle, float thresholdAngle, Point3 color)
{

	Point3 vecX1 = Point3(dist ,0.0f,0.0f);

	float holdAngle = angle;
	if (angle >thresholdAngle) 
		angle = thresholdAngle;
	float angleInc = angle/16.0f;
	Material mtl;
	mtl.opacity = 1.0;
	mtl.selfIllum = 0.85f;
	mtl.Kd = color;

	Point3 fillColor[5];
	fillColor[0] = color;
	fillColor[1] = color;
	fillColor[2] = color;
	fillColor[3] = color;

	mtl.Ka[0] = mtl.Ka[1] = mtl.Ka[2] = 0.0f;
	mtl.Ks[0] = mtl.Ks[1] = mtl.Ks[2] = 0.0f;

	Point3 pt[4];
	Point3 ptb[4];
	pt[0] = vecX1;

	gw->setMaterial(mtl);
	pt[2] = Point3(0.0f,0.0f,0.0f);

	gw->startTriangles();
	for (int j =0; j < 16; j++)
	{
		Matrix3 rtm(1);
		rtm.RotateZ(angleInc * (j+1));
		Point3 vecX2 = VectorTransform(rtm,vecX1);
		pt[1] = vecX2;

		gw->triangle(pt, fillColor);
		ptb[0] = pt[1];
		ptb[1] = pt[0];
		ptb[2] = pt[2];
		gw->triangle(ptb, fillColor);

		pt[0] = pt[1];

	}
	gw->endTriangles();

	if ( holdAngle > thresholdAngle)
	{
		mtl.opacity = 1.0;
		mtl.selfIllum = 0.85f;
		mtl.Kd = color*0.7f;

		Point3 fillColor[5];
		fillColor[0] = color*0.7f;
		fillColor[1] = color*0.7f;
		fillColor[2] = color*0.7f;
		fillColor[3] = color*0.7f;

		mtl.Ka[0] = mtl.Ka[1] = mtl.Ka[2] = 0.0f;
		mtl.Ks[0] = mtl.Ks[1] = mtl.Ks[2] = 0.0f;

//		Point3 pt[4];
//		pt[0] = vecX1;

 		gw->setMaterial(mtl);

		float angleInc = (holdAngle-thresholdAngle)/16.0f;
		gw->startTriangles();
		for (int j =0; j < 16; j++)
		{
			Matrix3 rtm(1);
			float ang = thresholdAngle + angleInc * (j+1);
			rtm.RotateZ(ang);
			Point3 vecX2 = VectorTransform(rtm,vecX1);
			pt[1] = vecX2;

			gw->triangle(pt, fillColor);
			ptb[0] = pt[1];
			ptb[1] = pt[0];
			ptb[2] = pt[2];
			gw->triangle(ptb, fillColor);

			pt[0] = pt[1];
		}
		gw->endTriangles();

	}

}

void MorphByBone::DrawMirrorData(GraphicsWindow *gw, MorphByBoneLocalData *ld)
{

	Point3 red(1.0f,0.0f,0.0f);
	gw->setColor(LINE_COLOR,red);
	if (this->previewVertices)
	{
		gw->startMarkers();
		for (int i = 0; i < ld->postDeform.Count(); i++)
		{

			Point3 p = ld->postDeform[i] * mirrorTM;
			gw->marker(&p,POINT_MRKR);
		}
		gw->endMarkers();
	}

	Point3 pmin,pmax;
	pmin = bounds.pmin;
	pmax = bounds.pmax;
	pmin[mirrorPlane] = mirrorOffset;
	pmax[mirrorPlane] = mirrorOffset;




	Box3 mirrorBox;
	mirrorBox.Init();
	mirrorBox += pmin;
	mirrorBox += pmax;

	gw->startSegments();
	DrawBox(gw, mirrorBox, 1.0f);
	gw->endSegments();

}

int MorphByBone::Display(
						 TimeValue t, INode* inode, ViewExp *vpt, 
						 int flagst, ModContext *mc)
{

	//Defect 727409 If a modifier is disabled and then an user apply some changes on it,
	//the changes should not be displayable in the viewport. In this crash case, the 
	//display method was nevertheless called when the modifier was disabled.
	if ( TestAFlag(A_MOD_DISABLED) || localDataList.Count() == 0) 
	{
		return 0;
	}	  

	GraphicsWindow *gw = vpt->getGW();
	Point3 pt[4];
	int savedLimits;
	savedLimits = gw->getRndLimits();
	int newLimits = savedLimits & ~GW_ILLUM;
	
	
	pblock->GetValue(pb_mirrorplane,t,mirrorPlane,FOREVER);

	MorphByBoneLocalData *ld = (MorphByBoneLocalData *) mc->localData;

	if (ld != NULL)
	{




		//draw mirror plane


		/*		for (i = 0; i < boneData.Count(); i++)
		mirrorBones[i] = -1;
		*/
		//		int mirrorBone = -1;
		BitArray mirrorBones;
		mirrorBones.SetSize(boneData.Count());
		mirrorBones.ClearAll();

		//draw our mirror data
		if (showMirror)
		{
			if (this->previewBones)
			{
				for (int i = 0; i < boneData.Count(); i++)
				{
					if ((i==currentBone) || (boneData[i]->IsMultiSelected()))
					{
						int id = GetMirrorBone(i);
						if ((id!=-1) && (id!=currentBone) && (!boneData[id]->IsMultiSelected()))
							mirrorBones.Set(id); 
					}
				}
			}

			DrawMirrorData(gw, ld);
		}

		//draw current morph 
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));

		BOOL showDriver, showMorph;
		BOOL showCurrentAngle, showLimitAngle;
		float vecSize;

		pblock->GetValue(pb_showdriverbone,0,showDriver,FOREVER);
		pblock->GetValue(pb_showmorphbone,0,showMorph,FOREVER);

		pblock->GetValue(pb_showcurrentangle,0,showCurrentAngle,FOREVER);
		pblock->GetValue(pb_showanglelimit,0,showLimitAngle,FOREVER);

		pblock->GetValue(pb_matrixsize,0,vecSize,FOREVER);

		BOOL showDeltas;
		pblock->GetValue(pb_showdeltas,0,showDeltas,FOREVER);

		BOOL showEdges;
		pblock->GetValue(pb_showedges,0,showEdges,FOREVER);

		float boneSize;
		pblock->GetValue(pb_bonesize,0,boneSize,FOREVER);

		Matrix3 boneTM ;

		Point3 vecX(vecSize,0.0f,0.0f),vecY(0.0f,vecSize,0.0f),vecZ(0.0f,0.0f,vecSize);
		Point3 Zero(0.0f,0.0f,0.0f);

		Point3 vecXBase(vecSize*1.2f,0.0f,0.0f),vecYBase(0.0f,vecSize*1.2f,0.0f),vecZBase(0.0f,0.0f,vecSize*1.2f);
		Point3 ZeroBase(0.0f,0.0f,0.0f);

		if ((currentBone != -1)&& (currentBone < boneData.Count()) )
		{

			boneTM = boneData[currentBone]->currentBoneObjectTM;
			boneTM.SetRow(0,Normalize(boneTM.GetRow(0)));
			boneTM.SetRow(1,Normalize(boneTM.GetRow(1)));
			boneTM.SetRow(2,Normalize(boneTM.GetRow(2)));
			vecX = vecX * boneTM * Inverse(ld->selfObjectCurrentTM);
			vecY = vecY * boneTM * Inverse(ld->selfObjectCurrentTM);
			vecZ = vecZ * boneTM * Inverse(ld->selfObjectCurrentTM);	
			Zero = Zero * boneTM * Inverse(ld->selfObjectCurrentTM);

			if ((currentMorph != -1) && (currentMorph < boneData[currentBone]->morphTargets.Count()))
			{
				Matrix3 tmBase = boneData[currentBone]->morphTargets[currentMorph]->boneTMInParentSpace * boneData[currentBone]->parentBoneObjectCurrentTM;
				tmBase.SetRow(0,Normalize(tmBase.GetRow(0)));
				tmBase.SetRow(1,Normalize(tmBase.GetRow(1)));
				tmBase.SetRow(2,Normalize(tmBase.GetRow(2)));
				tmBase.SetRow(3,boneData[currentBone]->currentBoneObjectTM.GetRow(3));
				
				vecXBase = vecXBase * tmBase * Inverse(ld->selfObjectCurrentTM);
				vecYBase = vecYBase * tmBase * Inverse(ld->selfObjectCurrentTM);
				vecZBase = vecZBase * tmBase * Inverse(ld->selfObjectCurrentTM);	
				ZeroBase = ZeroBase * tmBase * Inverse(ld->selfObjectCurrentTM);

			} 

		}



//markers
		//draw selection
		
		Box3 meshSize;
		meshSize.Init();
		Color selColor = GetUIColor(COLOR_SUBSELECTION);
		
		gw->startMarkers();
		for (int i = 0; i < ld->postDeform.Count(); i++)
		{
			meshSize += ld->postDeform[i];
			if (ld->softSelection[i]>0.0f)
			{
				Color c = selColor * ld->softSelection[i];
				gw->setColor(LINE_COLOR,c);		
				gw->marker(&ld->postDeform[i],ASTERISK_MRKR);
			}
		}
		gw->endMarkers();


		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
		float lineSize = Length(meshSize.pmax-meshSize.pmin)/700.0f;
		//see if we have an active morph
		if ((currentMorph != -1) && (currentBone != -1) && (currentMorph < boneData[currentBone]->morphTargets.Count()))
		{
			//draw our base morph points
			INode *node = GetNode(currentBone);
			if (node)
			{
				if (!boneData[currentBone]->morphTargets[currentMorph]->IsDead())
				{
					Matrix3 currentBoneTM = boneData[currentBone]->currentBoneNodeTM;
					Matrix3 currentParentBoneTM = boneData[currentBone]->parentBoneNodeCurrentTM;
					currentBoneTM *= Inverse(currentParentBoneTM);
					Matrix3 defTM = currentParentBoneTM * Inverse(ld->selfNodeCurrentTM);
					gw->startMarkers();
					for (int k =0; k < boneData[currentBone]->morphTargets[currentMorph]->d.Count();k++)
					{
						int id = boneData[currentBone]->morphTargets[currentMorph]->d[k].vertexID;
						if ( (id < ld->softSelection.Count() ) && (ld->softSelection[id] == 0.0f) )
						{
							
							Point3 vec = boneData[currentBone]->morphTargets[currentMorph]->d[k].vecParentSpace;
							vec = VectorTransform(defTM,vec);
							Point3 p[3];
							p[0] = ld->postDeform[id];
							p[1] = ld->preDeform[id] + vec;
							if (Length(vec) > 0.001f)
								gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
							else gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));

							gw->marker(&p[0],POINT_MRKR);
						}
					}
					gw->endMarkers();

					if ((showEdges) && (ip))
					{
						gw->startSegments();
						BitArray processVerts;
						processVerts.SetSize(ld->postDeform.Count());
						processVerts.ClearAll();
						gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
						for (int k =0; k < boneData[currentBone]->morphTargets[currentMorph]->d.Count();k++)
						{
							int id = boneData[currentBone]->morphTargets[currentMorph]->d[k].vertexID;
							if (id < ld->postDeform.Count())
							{
							Point3 p[3];
							p[0] = ld->postDeform[id];
							for (int m = 0; m < ld->connectionList[id]->verts.Count(); m++)
							{
								int endID = ld->connectionList[id]->verts[m];
								p[1] = ld->postDeform[endID];
								if (!processVerts[endID])
								{
									gw->segment(p,1);
								}
							}
							processVerts.Set(id,TRUE);
							}
						}

						gw->endSegments();
					}
				}

			}

		}
		gw->startMarkers();
		if (showMorph)
		{
			//get current matrix
			pt[0] = Zero;
			Point3 red(0.70f,0.0f,0.0f);
			gw->setColor(LINE_COLOR,red);
			pt[1] = vecX;
			gw->marker(&pt[1],SM_DOT_MRKR);


			Point3 green(0.0f,0.70f,0.0f);
			gw->setColor(LINE_COLOR,green);
			pt[1] = vecY;
			gw->marker(&pt[1],SM_DOT_MRKR);


			Point3 blue(0.0f,0.0f,0.70f);
			pt[1] = vecZ;
			gw->setColor(LINE_COLOR,blue);
			gw->marker(&pt[1],SM_DOT_MRKR);
		}

		if (showDriver)
		{
			pt[0] = ZeroBase;
			Point3 red(1.0f,0.0f,0.0f);
			gw->setColor(LINE_COLOR,red);
			pt[1] = vecXBase;
			gw->marker(&pt[1],SM_DOT_MRKR);


			Point3 green(0.0f,01.0f,0.0f);
			gw->setColor(LINE_COLOR,green);
			pt[1] = vecYBase;
			gw->marker(&pt[1],SM_DOT_MRKR);


			Point3 blue(0.0f,0.0f,01.0f);
			pt[1] = vecZBase;
			gw->setColor(LINE_COLOR,blue);
			gw->marker(&pt[1],SM_DOT_MRKR);
		}


		gw->endMarkers();


		gw->setRndLimits(newLimits );
		


//segments
		//draw all our regular bones
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
		gw->setRndLimits(gw->getRndLimits() &  ~GW_Z_BUFFER);
		gw->startSegments();
		for (int i = 0; i < boneData.Count(); i++)
		{
			if (i != currentBone)
			{
				INode *node = GetNode(i);
				if (node)
				{
					//set our transform
//					gw->setTransform();
					//draw a line
					Matrix3 tm =  boneData[i]->currentBoneObjectTM * Inverse(ld->selfObjectCurrentTM);
					DrawBox(gw, boneData[i]->localBounds, boneSize,TRUE,&tm);
					
				}
			}
		}
		//draw the selected driver axis segments
		if (showMorph)
		{
			//get current matrix
			pt[0] = Zero;
			Point3 red(0.70f,0.0f,0.0f);
			gw->setColor(LINE_COLOR,red);
			pt[1] = vecX;
			gw->segment(pt,1);


			Point3 green(0.0f,0.70f,0.0f);
			gw->setColor(LINE_COLOR,green);
			pt[1] = vecY;
			gw->segment(pt,1);


			Point3 blue(0.0f,0.0f,0.70f);
			pt[1] = vecZ;
			gw->setColor(LINE_COLOR,blue);
			gw->segment(pt,1);
		}

		if (showDriver)
		{
			pt[0] = ZeroBase;
			Point3 red(1.0f,0.0f,0.0f);
			gw->setColor(LINE_COLOR,red);
			pt[1] = vecXBase;
			gw->segment(pt,1);


			Point3 green(0.0f,01.0f,0.0f);
			gw->setColor(LINE_COLOR,green);
			pt[1] = vecYBase;
			gw->segment(pt,1);


			Point3 blue(0.0f,0.0f,01.0f);
			pt[1] = vecZBase;
			gw->setColor(LINE_COLOR,blue);
			gw->segment(pt,1);
		}

		//draw the selected moprh axis egements

		//draw our morph deltas
		gw->endSegments();

		//triangles
		
		//draw our selected bones
		int slimits = gw->getRndLimits();

		gw->setRndLimits(gw->getRndLimits() |  GW_TWO_SIDED);
		gw->setRndLimits(gw->getRndLimits() |  GW_FLAT);
		gw->setRndLimits(gw->getRndLimits() &  ~GW_WIREFRAME);
		gw->setRndLimits(gw->getRndLimits() |  GW_ILLUM);
		

		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
		Color c= GetUIColor(COLOR_SEL_GIZMOS);


		Material m;
		m.Kd = c;
		m.dblSided = 1;
		m.opacity = 1.0f;
		m.selfIllum = 1.0f;
		gw->setMaterial(m) ;

		gw->startTriangles();

		if ((currentBone != -1)&& (currentBone < boneData.Count()) )

		{
			INode *node = GetNode(currentBone);
			if (node)
			{
				Matrix3 tm =  boneData[currentBone]->currentBoneObjectTM * Inverse(ld->selfObjectCurrentTM);
				Draw3dEdge(gw, lineSize*boneSize, boneData[currentBone]->localBounds, c, &tm);				
			}
		}
		gw->endTriangles();	

		c.r = 1.0f;
		c.g = 0.0f;
		c.b = 0.0f;


		m;
		m.Kd = c;
		m.dblSided = 1;
		m.opacity = 1.0f;
		m.selfIllum = 1.0f;
		gw->setMaterial(m) ;

		gw->startTriangles();

		for (int i = 0; i < boneData.Count(); i++)
		{
			if ((i != currentBone) && (mirrorBones[i]))
			{
				INode *node = GetNode(i);
				if (node)
				{
					//set our transform

					Matrix3 tm =  boneData[i]->currentBoneObjectTM * Inverse(ld->selfObjectCurrentTM);
					Draw3dEdge(gw, lineSize*0.5f*boneSize, boneData[i]->localBounds, c, &tm);
//					DrawBox(gw, boneData[i]->localBounds, boneSize,TRUE,&tm);
					
				}
			}
		}
		
//now draw fans
		gw->endTriangles();

 		if ((showCurrentAngle ) && (currentBone != -1) && (currentBone < boneData.Count()) && ((currentMorph >=0) && (currentMorph < boneData[currentBone]->morphTargets.Count())) )
		{


			Matrix3 tm;
			Matrix3 mtm = boneData[currentBone]->morphTargets[currentMorph]->boneTMInParentSpace;
			Matrix3 ptm = boneData[currentBone]->parentBoneObjectCurrentTM;

			float thresholdAngle = boneData[currentBone]->morphTargets[currentMorph]->influenceAngle;
			tm =  mtm * ptm;

			//get a matrix for the x vec
			Point3 vecX1 = Normalize(tm.GetRow(0));
			//cross the X
			Point3 vecX2 = Normalize(boneData[currentBone]->currentBoneObjectTM.GetRow(0));
			//cross the Y
			//gets your Z
			Point3 vecX = vecX1 ;
			Point3 vecZ = (Normalize(CrossProd(vecX1,vecX2)));

			//cross your X and Z to get Y
			//set your X as the x Vec
			//set your Z as the z Vec
			Point3 vecY = (Normalize(CrossProd(vecZ,vecX)));


			Matrix3 ringTM;
			ringTM.SetRow(0,vecX);
			ringTM.SetRow(1,vecY);
			ringTM.SetRow(2,vecZ);
			ringTM.SetRow(3,boneData[currentBone]->currentBoneObjectTM.GetRow(3));
			tm.SetRow(3,boneData[currentBone]->currentBoneObjectTM.GetRow(3));

			gw->setTransform(ringTM);



			float dot = DotProd(vecX1,vecX2);
			float currentAngle = 0.0f;
			if (dot < 0.9995f)
				currentAngle = acos(dot);
			float angleInc = currentAngle/24;

			vecX1 = Point3(vecSize *1.f,0.0f,0.0f);
			pt[0] = vecX1;
			Point3 red(1.0f,0.0f,0.0f);
			gw->setColor(LINE_COLOR,red);


			if (showCurrentAngle )
			{
				DrawArc(gw, vecSize, currentAngle,thresholdAngle, red);
			}


			vecX1 = Point3(vecSize *.95f,0.0f,0.0f);
			pt[0] = vecX1;

			//get a matrix for the x vec
			vecX1 = Normalize(tm.GetRow(1));
			//cross the X
			vecX2 = Normalize(boneData[currentBone]->currentBoneObjectTM.GetRow(1));
			//cross the Y
			//gets your Z
			vecX = vecX1 ;
			vecZ = (Normalize(CrossProd(vecX1,vecX2)));

			//cross your X and Z to get Y
			//set your X as the x Vec
			//set your Z as the z Vec
			vecY = (Normalize(CrossProd(vecZ,vecX)));


			ringTM;
			ringTM.SetRow(0,vecX);
			ringTM.SetRow(1,vecY);
			ringTM.SetRow(2,vecZ);
			ringTM.SetRow(3,boneData[currentBone]->currentBoneObjectTM.GetRow(3));

			gw->setTransform(ringTM);

			dot = DotProd(vecX1,vecX2);
			currentAngle = 0.0f;
			if (dot < 0.9995f)
				currentAngle = acos(dot);
//			currentAngle = acos(DotProd(vecX1,vecX2));
			angleInc = currentAngle/24;

			vecX1 = Point3(vecSize *1.f,0.0f,0.0f);
			pt[0] = vecX1;
			Point3 green(0.0f,1.0f,0.0f);
			gw->setColor(LINE_COLOR,green);

			if (showCurrentAngle)
			{
				DrawArc(gw, vecSize, currentAngle,thresholdAngle, green);

			}


			vecX1 = Point3(vecSize *.8f,0.0f,0.0f);
			pt[0] = vecX1;


			//get a matrix for the x vec
			vecX1 = Normalize(tm.GetRow(2));
			//cross the X
			vecX2 = Normalize(boneData[currentBone]->currentBoneObjectTM.GetRow(2));
			//cross the Y
			//gets your Z
			vecX = vecX1 ;
			vecZ = (Normalize(CrossProd(vecX1,vecX2)));

			//cross your X and Z to get Y
			//set your X as the x Vec
			//set your Z as the z Vec
			vecY = (Normalize(CrossProd(vecZ,vecX)));


			ringTM;
			ringTM.SetRow(0,vecX);
			ringTM.SetRow(1,vecY);
			ringTM.SetRow(2,vecZ);
			ringTM.SetRow(3,boneData[currentBone]->currentBoneObjectTM.GetRow(3));
			tm.SetRow(3,boneData[currentBone]->currentBoneObjectTM.GetRow(3));

			gw->setTransform(ringTM);

			dot = DotProd(vecX1,vecX2);
			currentAngle = 0.0f;
			if (dot < 0.9995f)
				currentAngle = acos(dot);
//			currentAngle = acos(DotProd(vecX1,vecX2));
			angleInc = currentAngle/24;

			vecX1 = Point3(vecSize *1.f,0.0f,0.0f);
			pt[0] = vecX1;
			Point3 blue(0.0f,0.0f,1.0f);
			gw->setColor(LINE_COLOR,blue);

			if (showCurrentAngle)
			{
				DrawArc(gw, vecSize, currentAngle,thresholdAngle, blue);
			}

		}

		gw->setRndLimits(slimits);


	}


	gw->setRndLimits(savedLimits);
	return 0;
}


int MorphByBone::NumSubObjTypes() 
{ 
	return 1;
}

ISubObjType *MorphByBone::GetSubObjType(int i) 
{

	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_Points.SetName(GetString(IDS_POINTS));
	}

	switch(i)
	{
	case 0:
		return &SOT_Points;
	}
	return NULL;
}


void MorphByBone::ActivateSubobjSel(int level, XFormModes& modes)
{
	switch (level) {
		case 0:
			ip->DeleteMode(moveMode);
			ip->DeleteMode(rotMode);
			ip->DeleteMode(uscaleMode);
			ip->DeleteMode(nuscaleMode);
			ip->DeleteMode(squashMode);
			ip->DeleteMode(selectMode);	
			break;
		case 1: // Points
			modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,selectMode);
			break;		
	}
	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
}

class MorphHitData : public HitData
{
public:
	MorphByBoneLocalData *ld;
	int vertexID;


};


void MorphByBone::ClearSelection(int selLevel)
{
	if (theHold.Holding()) 
		HoldSelections();
	for (int i = 0; i < localDataList.Count(); i++)
	{
		if (localDataList[i])
		{
			for (int j = 0; j < localDataList[i]->softSelection.Count(); j++)
				localDataList[i]->softSelection[j] = 0.0f;
		}	

	}

	for (int i = 0; i < boneData.Count(); i++)
		boneData[i]->SetMultiSelected(FALSE);



	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
}

void MorphByBone::SelectAll(int selLevel)
{
	if (theHold.Holding()) 
		HoldSelections();
	for (int i = 0; i < localDataList.Count(); i++)
	{
		if (localDataList[i])
		{
			for (int j = 0; j < localDataList[i]->softSelection.Count(); j++)
				localDataList[i]->softSelection[j] = 1.0f;

//			localDataList[i]->selection.SetAll();
		}

	}
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
}

void MorphByBone::InvertSelection(int selLevel)
{
	if (theHold.Holding()) 
		HoldSelections();
	for (int i = 0; i < localDataList.Count(); i++)
	{
		if (localDataList[i])
		{
			for (int j = 0; j < localDataList[i]->softSelection.Count(); j++)
			{
				if (localDataList[i]->softSelection[j] == 1.0f)
					localDataList[i]->softSelection[j] = 0.0f;
				else localDataList[i]->softSelection[j] = 1.0f;
			}
		}

//			localDataList[i]->selection = ~localDataList[i]->selection;

	}
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
}


void MorphByBone::SelectSubComponent(
									 HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert)
{
	if (theHold.Holding()) 
		HoldSelections();
//		theHold.Put(new SelRestore(this));
	int tempBone=-1;
	BOOL vertHits = FALSE;
	while (hitRec) 
	{
		if (hitRec->hitData == NULL)
		{
			tempBone = hitRec->hitInfo ;
			if (multiSelectMode)
				boneData[tempBone]->SetMultiSelected(TRUE);
		}
		else
		{
			MorphHitData *hd = (MorphHitData *) hitRec->hitData;

			if (hd)
			{
				BOOL state = selected;
				float v = 0.0f;
				if (state) v = 1.0f;

				if (invert) 
				{
					if (hd->ld->softSelection[hd->vertexID] == 0.0f)
						v = 1.0f;
					else v = 0.0f;//state = !hd->ld->selection[hd->vertexID];
				}	

				hd->ld->softSelection[hd->vertexID]= v;
				delete hd;
				hitRec->hitData = NULL;
				vertHits = TRUE;
				if (!all) break;			
			}

		}
		hitRec = hitRec->Next();
	}	


	BOOL useSoftSelection;
	pblock->GetValue(pb_usesoftselection,0,useSoftSelection,FOREVER);
	float radius;
	pblock->GetValue(pb_selectionradius,0,radius,FOREVER);
	
	BOOL useEdgeLimit;
	int edgeLimit;
	pblock->GetValue(pb_useedgelimit,0,useEdgeLimit,FOREVER);
	pblock->GetValue(pb_edgelimit,0,edgeLimit,FOREVER);

	if (useSoftSelection)
	{
		for (int i = 0; i < this->localDataList.Count(); i++)
		{
			if (localDataList[i])
			{
				localDataList[i]->ComputeSoftSelection(radius,pCurve,useEdgeLimit,edgeLimit);
			}
		}
	}
	if ((tempBone >= 0) && (!vertHits))
	{	
		HoldBoneSelections();
		SetSelection(tempBone, TRUE);
		macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.selectBone"), 2, 0,
												mr_reftarg,GetNode(currentBone),
												mr_string,GetMorphName(GetNode(currentBone),currentMorph));							

	}
	else
	{
		for (int i = 0; i < this->localDataList.Count(); i++)
		{
			if (localDataList[i])
			{
				BitArray sel;
				sel.SetSize(localDataList[i]->softSelection.Count());
				sel.ClearAll();
				for (int j = 0; j < localDataList[i]->softSelection.Count(); j++)
				{
					if (localDataList[i]->softSelection[j] == 1.0f)
						sel.Set(j,TRUE);
				}
				macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.selectVertices"), 2, 0,
							mr_reftarg,localDataList[i]->selfNode,
							mr_bitarray,&sel);
			}
		}
	}
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
}



int MorphByBone::HitTest(
						 TimeValue t, INode* inode, 
						 int type, int crossing, int flags, 
						 IPoint2 *p, ViewExp *vpt, ModContext* mc)
{
	GraphicsWindow *gw = vpt->getGW();
	Point3 pt;
	HitRegion hr;
	int savedLimits, res = 0;

	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);	

	MorphByBoneLocalData *ld = (MorphByBoneLocalData *) mc->localData;

	if (ld == NULL) return res;


	gw->setTransform(ld->selfObjectTM);

	for (int i = 0; i < ld->postDeform.Count(); i++)
	{
		if ((flags&HIT_SELONLY   &&  (ld->softSelection[i]==1.0f)) ||
			(flags&HIT_UNSELONLY && (ld->softSelection[i]<1.0f)) ||
			!(flags&(HIT_UNSELONLY|HIT_SELONLY)) ) 
		{
			gw->clearHitCode();

			gw->marker(&ld->postDeform[i],HOLLOW_BOX_MRKR);

			if (gw->checkHitCode()) 
			{
				MorphHitData *hd = new MorphHitData();
				hd->ld = ld;
				hd->vertexID = i;
				vpt->LogHit(inode, mc, gw->getHitDistance(), i, (HitData*)hd); 
				res = 1;
			}

		}

	}

	if (!editMorph)
	{
		float boneSize;
		pblock->GetValue(pb_bonesize,0,boneSize,FOREVER);

		for (int i = 0; i < boneData.Count(); i++)
		{
			INode *node = GetNode(i);
			if (node)
			{	
				Point3 pt[3];
//				gw->setTransform(boneData[i]->currentBoneObjectTM);


				if ((flags&HIT_SELONLY   &&  (currentBone ==i)) ||
					(flags&HIT_UNSELONLY && (currentBone !=i)) ||
					!(flags&(HIT_UNSELONLY|HIT_SELONLY)) ) 
				{

					gw->clearHitCode();

					Matrix3 tm =  boneData[i]->currentBoneObjectTM * Inverse(ld->selfNodeCurrentTM);
					DrawBox(gw, boneData[i]->localBounds, boneSize,TRUE,&tm);
//					DrawBox(gw, boneData[i]->localBounds, boneSize);



					// Hit test start point

					if (gw->checkHitCode()) 
					{
						vpt->LogHit(inode, mc, gw->getHitDistance(), i, NULL); 
						res = 1;
					}
				}
			}
		}
	}


	gw->setRndLimits(savedLimits);
	return res;
}


void MorphByBone::GetWorldBoundBox(	TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc)

{
	MorphByBoneLocalData *ld = (MorphByBoneLocalData *) mc->localData;

	if (ld != NULL)
	{
		// do points
		for (int i = 0; i < ld->postDeform.Count(); i++)
		{
			box += ld->postDeform[i];
		}
		//now do bones
		for (int i = 0; i < boneData.Count(); i++)
		{
			INode *node = GetNode(i);
			if (node)
			{
				box += boneData[i]->localBounds * boneData[i]->currentBoneObjectTM;
			}
		}
		float vecSize;
		pblock->GetValue(pb_matrixsize,0,vecSize,FOREVER);

		box.EnlargeBy(vecSize);
		box = box * inode->GetObjTMBeforeWSM(t,&FOREVER);


	}

}
