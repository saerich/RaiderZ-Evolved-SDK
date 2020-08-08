
#include "unwrap.h"


void UnwrapMod::DrawGizmo(TimeValue t, INode* inode,/*w4 ModContext *mc, */GraphicsWindow *gw)
{
	 	ComputeSelectedFaceData();

		Matrix3 vtm(1);
		Interval iv;
		if (inode) 
			vtm = inode->GetObjectTM(t,&iv);
		Point3 a(-0.5f,-0.5f,0.0f),b(0.5f,-0.5f,0.0f),c(0.5f,0.5f,0.0f),d(-0.5f,0.5f,0.0f);
		
		Matrix3 modmat, ntm = inode->GetObjectTM(t);


		modmat = GetMapGizmoMatrix(t);

		
		gw->setTransform(modmat);	

		if ( (fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == PELTMAP) )
		{
			Point3 line[5];
			line[0] =   a;
			line[1] =   b;
			line[2] =   c;
			line[3] =   d;
			line[4] = line[0];
			gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
			gw->polyline(5, line, NULL, NULL, 0);
		}
		else if (fnGetMapMode() == CYLINDRICALMAP)
		{
			//draw the bottom circle
			int segs = 32;
			float angle = 0.0f;
			float inc = 1.0f/(float)segs * 2 * PI;
			
			Point3 prevVec;
			gw->startSegments();
	 		Point3 pl[3];
			gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
			for (int i = 0; i < (segs+1); i++)
			{
				
				Matrix3 tm(1);
				tm.RotateZ(angle);
				Point3 vec (0.50f,0.0f,0.0f);
				
 				vec = vec * tm;
				if ( i >= 1)
				{
					pl[0] = vec;
					pl[1] = prevVec;
					gw->segment(pl,1);


					pl[0].z = 1.0f;
					pl[1].z = 1.0f;
					gw->segment(pl,1);

					if (((i%4) == 0) && (i != segs))
					{
						pl[0] = vec;
						pl[1] = vec;
						pl[1].z = 1.0f;
						gw->segment(pl,1);
					}

				}
				prevVec = vec;
				angle += inc;
			}


			Color c(openEdgeColor);
			gw->setColor(LINE_COLOR,c);
			
			pl[0] = Point3(0.50f,0.0f,0.0f);
			pl[1] = Point3(0.50f,0.0f,0.0f);
			pl[1].z = 1.0f;
			gw->segment(pl,1);

			gw->endSegments();
		}
		else if (fnGetMapMode() == SPHERICALMAP)
		{
			//draw the bottom circle
			int segs = 32;

			float inc = 1.0f/(float)segs * 2 * PI;
			
			
			gw->startSegments();
	 		Point3 pl[3];
 			Color c(openEdgeColor);
			gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
			pl[0] = Point3(0.0f,0.0f,.6f);
			pl[1] = Point3(0.0f,0.0f,-.5f);
			gw->segment(pl,1);
			
			for (int j = 0; j < 3; j++)
			{
				float angle = 0.0f;
				Point3 prevVec;

 				gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
				for (int i = 0; i < (segs+1); i++)
				{
					
					Matrix3 tm(1);
					Point3 vec (0.50f,0.0f,0.0f);
					if (j == 0)
						tm.RotateZ(angle);
					if (j == 1)
					{
						vec = Point3(0.0f,0.5f,0.0f);
						tm.RotateX(angle);
					}
 					if (j == 2)
					{
						vec = Point3(0.0f,0.0f,-0.5f);
						if (i < ((segs+2)/2))
							gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
						else gw->setColor(LINE_COLOR,c);
						tm.RotateY(angle);
					}
					
					
 					vec = vec * tm;
					if ( i >= 1)
					{
						pl[0] = vec;
						pl[1] = prevVec;
						gw->segment(pl,1);

					}
					prevVec = vec;
					angle += inc;
				}
			}
			gw->endSegments();
		}
		else if (fnGetMapMode() == BOXMAP) 
		{
 			Point3 line[3];
			gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
			gw->startSegments();

			line[0] =  Point3(-0.5f,-0.5f,-0.5f);
			line[1] =  Point3(0.5f,-0.5f,-0.5f);
			gw->segment(line,1);

			line[0] =  Point3(-0.5f,0.5f,-0.5f);
			line[1] =  Point3(0.5f,0.5f,-0.5f);
			gw->segment(line,1);

			line[0] =  Point3(0.5f,0.5f,-0.5f);
			line[1] =  Point3(0.5f,-0.5f,-0.5f);
			gw->segment(line,1);

			line[0] =  Point3(-0.5f,0.5f,-0.5f);
			line[1] =  Point3(-0.5f,-0.5f,-0.5f);
			gw->segment(line,1);




			line[0] =  Point3(-0.5f,-0.5f,0.5f);
		 	line[1] =  Point3(0.5f,-0.5f,0.5f);
			gw->segment(line,1);

			line[0] =  Point3(-0.5f,0.5f,0.5f);
			line[1] =  Point3(0.5f,0.5f,0.5f);
			gw->segment(line,1);

			line[0] =  Point3(0.5f,0.5f,0.5f);
			line[1] =  Point3(0.5f,-0.5f,0.5f);
			gw->segment(line,1);

			line[0] =  Point3(-0.5f,0.5f,0.5f);
			line[1] =  Point3(-0.5f,-0.5f,0.5f);
			gw->segment(line,1);



			line[0] =  Point3(-0.5f,-0.5f,0.5f);
			line[1] =  Point3(-0.5f,-0.5f,-0.5f);			
			gw->segment(line,1);

			line[0] =  Point3(0.5f,-0.5f,0.5f);
			line[1] =  Point3(0.5f,-0.5f,-0.5f);			
			gw->segment(line,1);

			line[0] =  Point3(-0.5f,0.5f,0.5f);
			line[1] =  Point3(-0.5f,0.5f,-0.5f);			
			gw->segment(line,1);

			line[0] =  Point3(0.5f,0.5f,0.5f);
			line[1] =  Point3(0.5f,0.5f,-0.5f);			
			gw->segment(line,1);





			

			gw->endSegments();
		}

}

void UnwrapMod::GetOpenEdges(MeshTopoData *ld, Tab<int> &openEdges, Tab<int> &results)
{
	if (openEdges.Count() == 0) return;


	int seedEdgeA = openEdges[0];

	if (openEdges.Count() > 0)
	{
		int initialEdge = seedEdgeA;
		results.Append(1,&seedEdgeA,5000);
		openEdges.Delete(0,1);

		int seedVert = ld->GetGeomEdgeVert(seedEdgeA,0);//TVMaps.gePtrList[seedEdgeA]->a;
		BOOL done = FALSE;
		int ct = openEdges.Count();
		while (!done)
		{
			for (int i = 0; i < openEdges.Count(); i++)
			{
				int edgeIndex = openEdges[i];

				if (edgeIndex != seedEdgeA)
				{
					int a = ld->GetGeomEdgeVert(edgeIndex,0);//TVMaps.gePtrList[edgeIndex]->a;
					int b = ld->GetGeomEdgeVert(edgeIndex,1);//TVMaps.gePtrList[edgeIndex]->b;
					if (a == seedVert)
					{
						seedVert = b;
						seedEdgeA = edgeIndex;
						results.Append(1,&seedEdgeA,5000);
						openEdges.Delete(i,1);
						
						i = openEdges.Count();
					}
					else if  (b == seedVert)
					{
						seedVert = a;
						seedEdgeA = edgeIndex;
						results.Append(1,&seedEdgeA,5000);
						openEdges.Delete(i,1);
						i = openEdges.Count();
					}
				}
			}					
			if (ct == openEdges.Count())
				done = TRUE;
			ct = openEdges.Count();
		}


		seedEdgeA = initialEdge;

		seedVert =  ld->GetGeomEdgeVert(seedEdgeA,1);//TVMaps.gePtrList[seedEdgeA]->b;
		done = FALSE;
		ct = openEdges.Count();
		while (!done)
		{
			for (int i = 0; i < openEdges.Count(); i++)
			{
				int edgeIndex = openEdges[i];

				if (edgeIndex != seedEdgeA)
				{
					int a = ld->GetGeomEdgeVert(edgeIndex,0);//TVMaps.gePtrList[edgeIndex]->a;
					int b = ld->GetGeomEdgeVert(edgeIndex,1);//TVMaps.gePtrList[edgeIndex]->b;
					if (a == seedVert)
					{
						seedVert = b;
						seedEdgeA = edgeIndex;
						results.Append(1,&seedEdgeA,5000);
						openEdges.Delete(seedEdgeA,1);
						i = openEdges.Count();
					}
					else if  (b == seedVert)
					{
						seedVert = a;
						seedEdgeA = edgeIndex;
						results.Append(1,&seedEdgeA,5000);
						openEdges.Delete(seedEdgeA,1);
						i = openEdges.Count();
					}
				}
			}					
			if (ct == openEdges.Count())
				done = TRUE;
			ct = openEdges.Count();
		}
	}

}


void UnwrapMod::fnGizmoReset()
{


	
	theHold.Begin();
	SuspendAnimate();
	AnimateOff();
	TimeValue t = GetCOREInterface()->GetTime();



	
	//get our selection
	Box3 bounds;
	bounds.Init();
	//get the bounding box

	for(int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];

		Matrix3 tm = mMeshTopoData.GetNodeTM(t,ldID);

		for (int k = 0; k < ld->GetNumberFaces(); k++)//gfaces.Count(); k++) 
		{
			if (ld->GetFaceSelected(k))
			{
					// Grap the three points, xformed
				int pcount = 3;
					//				if (gfaces[k].flags & FLAG_QUAD) pcount = 4;
				pcount = ld->GetFaceDegree(k);//gfaces[k]->count;

				Point3 temp_point[4];
				for (int j=0; j<pcount; j++) 
				{
					int index = ld->GetFaceGeomVert(k,j);//gfaces[k]->t[j];
					bounds += ld->GetGeomVert(index) * tm;//gverts.d[index].p;
				}
			}
		}	

	}
	
	Matrix3 tm(1);
	
	//if just a primary axis set the tm;
	Point3 center = bounds.Center();
		// build the scale
 	Point3 scale(bounds.Width().x ,bounds.Width().y , bounds.Width().z);
	if (scale.x == 0.0f) scale.x = 1.0f;
	if (scale.y == 0.0f) scale.y = 1.0f;
 	if (scale.z == 0.0f) scale.z = 1.0f;
	float scl = scale.x;
	if (scale.y > scl) scl = scale.y;
	if (scale.z > scl) scl = scale.z;
	scale.x = scl;
	scale.y = scl;
	scale.z = scl;
	
	tm.SetRow(0,Point3(scale.x,0.0f,0.0f));
	tm.SetRow(1,Point3(0.0f,scale.y,0.0f));
	tm.SetRow(2,Point3(0.0f,0.0f,scale.z));
	if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == PELTMAP)|| (fnGetMapMode() == SPHERICALMAP))
		tm.SetRow(3,center);
	else if (fnGetMapMode() == CYLINDRICALMAP)
	{
		center.z = bounds.pmin.z;
		tm.SetRow(3,center);
	}
		

	Matrix3 ptm(1), id(1);
	tm = tm ;
	SetXFormPacket tmpck(tm,ptm);
	tmControl->SetValue(t,&tmpck,TRUE,CTRL_RELATIVE);
	ResumeAnimate();

	if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == CYLINDRICALMAP) || (fnGetMapMode() == SPHERICALMAP) || (fnGetMapMode() == BOXMAP))
		ApplyGizmo();


	theHold.Accept(GetString(IDS_MAPPING_RESET));
	fnGetGizmoTM();
	if (ip) ip->RedrawViews(ip->GetTime());

}

void UnwrapMod::fnAlignAndFit(int axis)
{



	//get our selection
	Box3 bounds;
	bounds.Init();
	//get the bounding box
	Point3 pnorm(0.0f,0.0f,0.0f);
	int ct = 0;
	TimeValue t = GetCOREInterface()->GetTime();
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		Matrix3 tm = mMeshTopoData.GetNodeTM(t,ldID);
		for (int k = 0; k < ld->GetNumberFaces(); k++) 
		{
			if (ld->GetFaceSelected(k))
			{
					// Grap the three points, xformed
				int pcount = 3;
					//				if (gfaces[k].flags & FLAG_QUAD) pcount = 4;
				pcount = ld->GetFaceDegree(k);//gfaces[k]->count;

				Point3 temp_point[4];
				for (int j=0; j<pcount; j++) 
				{
					int index = ld->GetFaceGeomVert(k,j);//gfaces[k]->t[j];
					bounds += ld->GetGeomVert(index) *tm;//gverts.d[index].p;
					if (j < 4)
						temp_point[j] = ld->GetGeomVert(index);//gverts.d[index].p;
				}
				pnorm += VectorTransform(Normalize(temp_point[1]-temp_point[0]^temp_point[2]-temp_point[1]),tm);
				ct++;
			}
		}	
	}

	if (ct == 0) return;

	theHold.Begin();
	SuspendAnimate();
	AnimateOff();

	pnorm = pnorm / (float) ct;
	Matrix3 tm(1);
	
	//if just a primary axis set the tm;
	Point3 center = bounds.Center();
		// build the scale
	Point3 scale(bounds.Width().x ,bounds.Width().y , bounds.Width().z);
	if (scale.x == 0.0f) scale.x = 1.0f;
	if (scale.y == 0.0f) scale.y = 1.0f;
 	if (scale.z == 0.0f) scale.z = 1.0f;
	
 	if (axis == 0) // x axi
	{

  		tm.SetRow(0,Point3(0.0f,-scale.y,0.0f));
		tm.SetRow(1,Point3(0.0f,0.0f,scale.z));
		tm.SetRow(2,Point3(scale.x,0.0f,0.0f));
		if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == PELTMAP)  || (fnGetMapMode() == SPHERICALMAP) || (fnGetMapMode() == BOXMAP))
			tm.SetRow(3,center);
		else if (fnGetMapMode() == CYLINDRICALMAP)
		{
			center.x = bounds.pmin.x;
			tm.SetRow(3,center);
		}		

		Matrix3 ptm(1), id(1);
		tm = tm ;
		SetXFormPacket tmpck(tm,ptm);
		tmControl->SetValue(t,&tmpck,TRUE,CTRL_RELATIVE);
	}
	else if (axis == 1) // y axi
	{
  		tm.SetRow(0,Point3(scale.x,0.0f,0.0f));
 		tm.SetRow(1,Point3(0.0f,0.0f,scale.z));
		tm.SetRow(2,Point3(0.0f,scale.y,0.0f));
		if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == PELTMAP)|| (fnGetMapMode() == SPHERICALMAP) || (fnGetMapMode() == BOXMAP))
			tm.SetRow(3,center);
		else if (fnGetMapMode() == CYLINDRICALMAP)
		{
			center.y = bounds.pmin.y;
			tm.SetRow(3,center);
		}
		

		Matrix3 ptm(1), id(1);
		tm = tm;
		SetXFormPacket tmpck(tm,ptm);
		tmControl->SetValue(t,&tmpck,TRUE,CTRL_RELATIVE);
	}
	else if (axis == 2) //z axi
	{
		tm.SetRow(0,Point3(scale.x,0.0f,0.0f));
		tm.SetRow(1,Point3(0.0f,scale.y,0.0f));
		tm.SetRow(2,Point3(0.0f,0.0f,scale.z));
		if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == PELTMAP)|| (fnGetMapMode() == SPHERICALMAP) || (fnGetMapMode() == BOXMAP))
			tm.SetRow(3,center);
		else if (fnGetMapMode() == CYLINDRICALMAP)
		{
			center.z = bounds.pmin.z;
			tm.SetRow(3,center);
		}
		

		Matrix3 ptm(1), id(1);
		tm = tm;
		SetXFormPacket tmpck(tm,ptm);
		tmControl->SetValue(t,&tmpck,TRUE,CTRL_RELATIVE);
	}
	else if (axis == 3) // normal
	{
		int numberOfSelectionGroups = 0;
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			if (ld->GetFaceSelection().NumberSet())
				numberOfSelectionGroups++;
		}
		if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == PELTMAP) || (numberOfSelectionGroups > 1))
		{
			//get our tm
			Matrix3 tm;
			UnwrapMatrixFromNormal(pnorm,tm);
 			Matrix3 itm = Inverse(tm);
			//find our x and y scale
			float xmax = 0.0f;
			float ymax = 0.0f;
			float zmax = 0.0f;
			Box3 localBounds;
			localBounds.Init();
			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				MeshTopoData *ld = mMeshTopoData[ldID];
				Matrix3 tm = mMeshTopoData.GetNodeTM(t,ldID);
				for (int k = 0; k < ld->GetNumberFaces(); k++) 
				{
					if (ld->GetFaceSelected(k))
					{
							// Grap the three points, xformed
						int pcount = 3;
							//				if (gfaces[k].flags & FLAG_QUAD) pcount = 4;
						pcount = ld->GetFaceDegree(k);//gfaces[k]->count;

						Point3 temp_point[4];
						for (int j=0; j<pcount; j++) 
						{
							int index = ld->GetFaceGeomVert(k,j);//gfaces[k]->t[j];
							Point3 p = ld->GetGeomVert(index) * tm * itm;//gverts.d[index].p * itm;
							localBounds += p;
						}
					}
				}
			}

//			center = localBounds.Center();
			xmax = localBounds.pmax.x - localBounds.pmin.x;
			ymax = localBounds.pmax.y - localBounds.pmin.y;
			zmax = localBounds.pmax.z - localBounds.pmin.z;

			if (xmax < 0.001f)
				xmax = 1.0f;
			if (ymax < 0.001f)
				ymax = 1.0f;
			if (zmax < 0.001f)
				zmax = 1.0f;

			Point3 vec;
			vec = Normalize(tm.GetRow(0)) * xmax;
			tm.SetRow(0,vec);

			vec = Normalize(tm.GetRow(1)) * ymax;
			tm.SetRow(1,vec);

			vec = Normalize(tm.GetRow(2)) * zmax;
			tm.SetRow(2,vec);


			tm.SetRow(3,center);
			

			Matrix3 ptm(1), id(1);
			tm = tm ;
			SetXFormPacket tmpck(tm,ptm);
			tmControl->SetValue(t,&tmpck,TRUE,CTRL_RELATIVE);
		}		
		else if ((fnGetMapMode() == CYLINDRICALMAP) || (fnGetMapMode() == SPHERICALMAP)|| (fnGetMapMode() == BOXMAP))
		{

			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{


				//get our first 2 rings
				Tab<int> openEdges;
				Tab<int> startRing;
				Tab<int> endRing;

				MeshTopoData *ld =  mMeshTopoData[ldID];

				//skip any local data that has no selections
				if (ld->GetFaceSelection().NumberSet() == 0)
					continue;

				Matrix3 nodeTM = mMeshTopoData.GetNodeTM(t,ldID);

				for (int i = 0; i < ld->GetNumberGeomEdges(); i++)//TVMaps.gePtrList.Count(); i++)
				{
					int numberSelectedFaces = 0;
					int ct = ld->GetGeomEdgeNumberOfConnectedFaces(i);//TVMaps.gePtrList[i]->faceList.Count();
					for (int j = 0; j < ct; j++)
					{
						int faceIndex = ld->GetGeomEdgeConnectedFace(i,j);//TVMaps.gePtrList[i]->faceList[j];
						if (ld->GetFaceSelected(faceIndex))//fsel[faceIndex])
							numberSelectedFaces++;
					}
					if (numberSelectedFaces == 1)
					{
						openEdges.Append(1,&i,1000);
						
					}
				}

				GetOpenEdges(ld,openEdges, startRing);
				GetOpenEdges(ld,openEdges, endRing);
				Point3 zVec = pnorm;

				Point3 centerS(0.0f,0.0f,0.0f), centerE;
				if ((startRing.Count() != 0) && (endRing.Count() != 0))
				{
					//get the center start
					Box3 BoundsS, BoundsE;
					BoundsS.Init();
					BoundsE.Init();

					//get the center end
					for (int i = 0; i < startRing.Count(); i++)
					{
						int eIndex = startRing[i];
						int a = ld->GetGeomEdgeVert(eIndex,0);//TVMaps.gePtrList[eIndex]->a;
						int b = ld->GetGeomEdgeVert(eIndex,1);//TVMaps.gePtrList[eIndex]->b;

						BoundsS += ld->GetGeomVert(a) * nodeTM;//TVMaps.geomPoints[a];
						BoundsS += ld->GetGeomVert(b) * nodeTM;//TVMaps.geomPoints[b];
					}


					for (int i = 0; i < endRing.Count(); i++)
					{
						int eIndex = endRing[i];
						int a = ld->GetGeomEdgeVert(eIndex,0);//TVMaps.gePtrList[eIndex]->a;
						int b = ld->GetGeomEdgeVert(eIndex,1);//TVMaps.gePtrList[eIndex]->b;

						BoundsE += ld->GetGeomVert(a) * nodeTM;//TVMaps.geomPoints[a];
						BoundsE += ld->GetGeomVert(b) * nodeTM;//TVMaps.geomPoints[b];
					}

					
					centerS = BoundsS.Center();
					centerE = BoundsE.Center();
					//create the vec
					zVec = centerE - centerS;

				}
				else if ((startRing.Count() != 0) && (endRing.Count() == 0))
				{
					//get the center start
					Box3 BoundsS;
					BoundsS.Init();
					

					//get the center end
					for (int i = 0; i < startRing.Count(); i++)
					{
						int eIndex = startRing[i];
						int a =  ld->GetGeomEdgeVert(eIndex,0);//TVMaps.gePtrList[eIndex]->a;
						int b =  ld->GetGeomEdgeVert(eIndex,1);//TVMaps.gePtrList[eIndex]->b;

						BoundsS += ld->GetGeomVert(a) * nodeTM;//TVMaps.geomPoints[a];
						BoundsS += ld->GetGeomVert(b) * nodeTM;//TVMaps.geomPoints[b];
					}

					centerS = BoundsS.Center();
					

					int farthestPoint= -1;
					Point3 fp;
					float farthestDist= 0.0f;
					for (int k=0; k < ld->GetNumberFaces(); k++) 
					{
						if (ld->GetFaceSelected(k))
						{
								// Grap the three points, xformed
							int pcount = 3;
								//				if (gfaces[k].flags & FLAG_QUAD) pcount = 4;
							pcount = ld->GetFaceDegree(k);//gfaces[k]->count;

							
							for (int j=0; j<pcount; j++) 
							{
								int index = ld->GetFaceGeomVert(k,j);//gfaces[k]->t[j];
								
								Point3 p = ld->GetGeomVert(index)* nodeTM;//gverts.d[index].p;
								float d = LengthSquared(p-centerS);
								if ((d > farthestDist) || (farthestPoint == -1))
								{
									farthestDist = d;
									farthestPoint = index;
									fp = p;
								}							
							}
						}
					}

					
					
					centerE = fp;
					//create the vec
					zVec = centerE - centerS;

				}
				else
				{
					zVec = Point3(0.0f,0.0f,1.0f);
				}


				//get our tm
				Matrix3 tm;
				UnwrapMatrixFromNormal(zVec,tm);
				tm.SetRow(3,centerS);
 				Matrix3 itm = Inverse(tm);
				//find our x and y scale
				float xmax = 0.0f;
				float ymax = 0.0f;
				float zmax = 0.0f;
				Box3 localBounds;
				localBounds.Init();
				for (int k = 0; k < ld->GetNumberFaces(); k++)//gfaces.Count(); k++) 
				{
					if (ld->GetFaceSelected(k))
					{
							// Grap the three points, xformed
						int pcount = 3;
							//				if (gfaces[k].flags & FLAG_QUAD) pcount = 4;
						pcount = ld->GetFaceDegree(k);//gfaces[k]->count;

						Point3 temp_point[4];
						for (int j=0; j<pcount; j++) 
						{
							int index = ld->GetFaceGeomVert(k,j);//gfaces[k]->t[j];
							Point3 p = ld->GetGeomVert(index) * nodeTM * itm;//gverts.d[index].p * itm;
							localBounds += p;
						}
					}
				}

				center = localBounds.Center() * tm;

				if (fnGetMapMode() == CYLINDRICALMAP)
				{
					if ((startRing.Count() == 0) && (endRing.Count() == 0))
					{
						centerS = center;
						centerS.z = localBounds.pmin.z;					
					}
					else
					{

						centerS = centerS * itm;
						centerS.z = localBounds.pmin.z;
						centerS = centerS * tm;
					}
				}
				else if ((fnGetMapMode() == SPHERICALMAP) || (fnGetMapMode() == BOXMAP))
				{
					centerS = center;
				}

				Point3 bc = localBounds.Center();
				bc.z = localBounds.pmin.z;
				bc = bc * tm;

				xmax = localBounds.pmax.x - localBounds.pmin.x;
				ymax = localBounds.pmax.y - localBounds.pmin.y;
				zmax = localBounds.pmax.z - localBounds.pmin.z;

				Point3 vec;
				vec = Normalize(tm.GetRow(0)) * xmax;
				tm.SetRow(0,vec);

				vec = Normalize(tm.GetRow(1)) * ymax;
				tm.SetRow(1,vec);

				vec = Normalize(tm.GetRow(2)) * zmax;
				tm.SetRow(2,vec);


				
				tm.SetRow(3,centerS);
				

				Matrix3 ptm(1), id(1);
				tm = tm;
				SetXFormPacket tmpck(tm,ptm);
				tmControl->SetValue(t,&tmpck,TRUE,CTRL_RELATIVE);
			}
			
		}
	}
	ResumeAnimate();

	if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == CYLINDRICALMAP) || (fnGetMapMode() == SPHERICALMAP) || (fnGetMapMode() == BOXMAP))
		ApplyGizmo();

	theHold.Accept(GetString(IDS_MAPPING_ALIGN));

	fnGetGizmoTM();

	if (ip) ip->RedrawViews(ip->GetTime());

}


void UnwrapMod::fnGizmoFit()
{
	//get our tm
	//set the tm scale


	TimeValue t = GetCOREInterface()->GetTime();



	
	//get our selection
	Box3 bounds;
	bounds.Init();
	//get the bounding box
	Point3 pnorm(0.0f,0.0f,0.0f);
	int ct = 0;
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		Matrix3 wtm = mMeshTopoData.GetNodeTM(t,ldID);
		for (int k = 0; k < ld->GetNumberFaces(); k++) 
		{
			if (ld->GetFaceSelected(k))		// Grap the three points, xformed
			{
				int pcount = 3;
				//				if (gfaces[k].flags & FLAG_QUAD) pcount = 4;
				pcount = ld->GetFaceDegree(k);//gfaces[k]->count;

				Point3 temp_point[4];
				for (int j=0; j<pcount; j++) 
				{
					int index = ld->GetFaceGeomVert(k,j);//gfaces[k]->t[j];
					bounds += ld->GetGeomVert(index) * wtm;//gverts.d[index].p;
					if (j < 4)
						temp_point[j] = ld->GetGeomVert(index) * wtm;//
				}
				pnorm += Normalize(temp_point[1]-temp_point[0]^temp_point[2]-temp_point[1]);
				ct++;
			}
		}
	}	

	if (ct == 0) return;

	theHold.Begin();
	SuspendAnimate();
	AnimateOff();


	pnorm = pnorm / (float) ct;//gfaces.Count();
	

	//if just a primary axis set the tm;
	Point3 center = bounds.Center();
	// build the scale

	//get our tm
	Matrix3 tm(1);
	tm = *fnGetGizmoTM();






	Point3 vec2;
	vec2 = Normalize(tm.GetRow(0)); 
	tm.SetRow(0,vec2);

	vec2 = Normalize(tm.GetRow(1)) ;
	tm.SetRow(1,vec2);

	vec2 = Normalize(tm.GetRow(2)); 
	tm.SetRow(2,vec2);

	tm.SetRow(3,center);


 	Matrix3 itm = Inverse(tm);
	//find our x and y scale
	float xmax = 0.0f;
	float ymax = 0.0f;
	float zmax = 0.0f;
	Box3 localBounds;
	localBounds.Init();

	ct = 0;
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		Matrix3 wtm = mMeshTopoData.GetNodeTM(t,ldID);
		for (int k = 0; k < ld->GetNumberFaces(); k++)
		{
			if (ld->GetFaceSelected(k))
			{
				// Grap the three points, xformed
				int pcount = 3;
				//				if (gfaces[k].flags & FLAG_QUAD) pcount = 4;
				pcount = ld->GetFaceDegree(k);//gfaces[k]->count;

				Point3 temp_point[4];
				for (int j=0; j<pcount; j++) 
				{
					int index =ld->GetFaceGeomVert(k,j);// gfaces[k]->t[j];
					Point3 p = /*gverts.d[index].p*/ ld->GetGeomVert(index)* wtm * itm;
					localBounds += p;
				}
			}
		}
	}

	center = localBounds.Center() * tm;



	xmax = localBounds.pmax.x - localBounds.pmin.x;
	ymax = localBounds.pmax.y - localBounds.pmin.y;
	zmax = localBounds.pmax.z - localBounds.pmin.z;
	Point3 vec;
	vec = Normalize(tm.GetRow(0)) * xmax;
	tm.SetRow(0,vec);

	vec = Normalize(tm.GetRow(1)) * ymax;
	tm.SetRow(1,vec);
 
	vec = Normalize(tm.GetRow(2)) * zmax;
	tm.SetRow(2,vec);

	if (fnGetMapMode() == CYLINDRICALMAP)
	{
//		Point3 zvec = tm.GetRow(2);
		center -= (vec * 0.5f);
	}

	tm.SetRow(3,center);


	Matrix3 ptm(1), id(1);
	tm = tm;/// * mctm ;
	SetXFormPacket tmpck(tm,ptm);
	tmControl->SetValue(t,&tmpck,TRUE,CTRL_RELATIVE);

	ResumeAnimate();

	if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == CYLINDRICALMAP) || (fnGetMapMode() == BOXMAP) || (fnGetMapMode() == SPHERICALMAP))
		ApplyGizmo();

	theHold.Accept(GetString(IDS_MAPPING_FIT));

	fnGetGizmoTM();

	if (ip) ip->RedrawViews(ip->GetTime());
}



void UnwrapMod::fnGizmoCenter()
{
	//get our tm
	//set the tm scale



	TimeValue t = GetCOREInterface()->GetTime();



	
	//get our selection
	Box3 bounds;
	bounds.Init();
	//get the bounding box
	Point3 pnorm(0.0f,0.0f,0.0f);
	int ct = 0;
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		Matrix3 tm = mMeshTopoData.GetNodeTM(t,ldID);

		for (int k = 0; k < ld->GetNumberFaces(); k++) 
		{
			if (ld->GetFaceSelected(k))
			{
				// Grap the three points, xformed
				int pcount = 3;
				//				if (gfaces[k].flags & FLAG_QUAD) pcount = 4;
				pcount = ld->GetFaceDegree(k);//gfaces[k]->count;

				Point3 temp_point[4];
				for (int j=0; j<pcount; j++) 
				{
					int index = ld->GetFaceGeomVert(k,j);//gfaces[k]->t[j];
					bounds += ld->GetGeomVert(index) * tm; //gverts.d[index].p * tm;
					if (j < 4)
						temp_point[j] = ld->GetGeomVert(index) * tm; //gverts.d[index].p;
				}
				pnorm += Normalize(temp_point[1]-temp_point[0]^temp_point[2]-temp_point[1]);
				ct++;
			}
		}	
	}


	if (ct == 0) return;

	theHold.Begin();
	SuspendAnimate();
	AnimateOff();
	pnorm = pnorm / (float) ct;//gfaces.Count();
	

	//if just a primary axis set the tm;
	Point3 center = bounds.Center();
	// build the scale



	//get our tm
	Matrix3 tm(1);
	tm = *fnGetGizmoTM();
	Matrix3 initialTM = tm;

	Point3 vec2;
	vec2 = Normalize(tm.GetRow(0)); 
	tm.SetRow(0,vec2);

	vec2 = Normalize(tm.GetRow(1)) ;
	tm.SetRow(1,vec2);

	vec2 = Normalize(tm.GetRow(2)); 
	tm.SetRow(2,vec2);

	tm.SetRow(3,center);


 	Matrix3 itm = Inverse(tm);
	//find our x and y scale
	Box3 localBounds;
	localBounds.Init();
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		Matrix3 tm = mMeshTopoData.GetNodeTM(t,ldID);

		for (int k = 0; k < ld->GetNumberFaces(); k++) 
		{
			if (ld->GetFaceSelected(k))
			{		// Grap the three points, xformed
				int pcount = 3;
				//				if (gfaces[k].flags & FLAG_QUAD) pcount = 4;
				pcount = ld->GetFaceDegree(k);//gfaces[k]->count;

				Point3 temp_point[4];
				for (int j=0; j<pcount; j++) 
				{
					int index = ld->GetFaceGeomVert(k,j);//gfaces[k]->t[j];
					Point3 p = ld->GetGeomVert(index) * tm * itm;//gverts.d[index].p * tm * itm;
					localBounds += p;

		//			if (fabs(p.x) > xmax) xmax = fabs(p.x);
		//			if (fabs(p.y) > ymax) ymax = fabs(p.y);
		//			if (fabs(p.z) > zmax) zmax = fabs(p.z);
				}
			}
		}
	}

	if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == PELTMAP))
		center = localBounds.Center() * tm;
	else if (fnGetMapMode() == CYLINDRICALMAP)
	{
		Point3 zvec = initialTM.GetRow(2);
		
//		center = center * tm;

		center = localBounds.Center() * tm - (zvec * 0.5f);
		

	}


	initialTM.SetRow(3,center);


	Matrix3 ptm(1), id(1);
	initialTM = initialTM ;
	SetXFormPacket tmpck(initialTM,ptm);
	tmControl->SetValue(t,&tmpck,TRUE,CTRL_RELATIVE);

	ResumeAnimate();

	if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == CYLINDRICALMAP) || (fnGetMapMode() == SPHERICALMAP) || (fnGetMapMode() == BOXMAP))
		ApplyGizmo();

	theHold.Accept(GetString(IDS_MAPPING_FIT));
	fnGetGizmoTM();
	if (ip) ip->RedrawViews(ip->GetTime());

}


void UnwrapMod::fnGizmoAlignToView()
{

	if (ip == NULL) return;

	ViewExp *vpt = ip->GetActiveViewport();
	if (!vpt) return;





	//get our tm
	//set the tm scale
	theHold.Begin();
	SuspendAnimate();
	AnimateOff();
	TimeValue t = GetCOREInterface()->GetTime();
	


	// Viewport tm
	Matrix3 vtm;
	vpt->GetAffineTM(vtm);
	vtm = Inverse(vtm);

	// Node tm
	Matrix3 ntm(1);// = nodeList[0]->GetObjectTM(ip->GetTime());

	Matrix3 destTM = vtm * Inverse(ntm);
	//get our tm
	
	
	Matrix3 initialTM = *fnGetGizmoTM();


	

	Point3 center = Normalize(initialTM.GetRow(3)); 
	initialTM.SetRow(3,center);

	for (int i = 0; i < 3; i++)
	{
		Point3 vec = destTM.GetRow(i);
		float l  = Length(initialTM.GetRow(i));
		vec = Normalize(vec) * l;
		initialTM.SetRow(i,vec);
	}



	initialTM.SetRow(3,center);


	Matrix3 ptm(1), id(1);
	initialTM = initialTM ;
	SetXFormPacket tmpck(initialTM,ptm);
	tmControl->SetValue(t,&tmpck,TRUE,CTRL_RELATIVE);

	ResumeAnimate();

	fnGizmoFit();

	if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == CYLINDRICALMAP)  || (fnGetMapMode() == SPHERICALMAP) || (fnGetMapMode() == BOXMAP))
		ApplyGizmo();

	theHold.Accept(GetString(IDS_MAPPING_ALIGNTOVIEW));
	fnGetGizmoTM();
	if (ip) ip->RedrawViews(ip->GetTime());

}



void UnwrapMod::ApplyGizmo()
{

	if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == PELTMAP) ||
		(fnGetMapMode() == SPHERICALMAP) || (fnGetMapMode() == CYLINDRICALMAP))
	{
		ApplyGizmoPrivate();
	}
	else
	{
  		theHold.Begin();
		//compute the center
			//get our normal list
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			ld->HoldFaceSel();
		}

		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			ld->HoldFaceSel();

			Tab<Point3> fnorms;
			fnorms.SetCount(ld->GetNumberFaces());
			for (int k=0; k< fnorms.Count(); k++) 
				fnorms[k] = Point3(0.0f,0.0f,0.0f);
				
			//get our projection normal
			Point3 projectionNorm(0.0f,0.0f,0.0f);

			//build normals
			for (int k = 0; k < fnorms.Count(); k++) 
			{
				if (ld->GetFaceSelected(k))
				{
							// Grap the three points, xformed
					int pcount = 3;
					//				if (gfaces[k].flags & FLAG_QUAD) pcount = 4;
					pcount = ld->GetFaceDegree(k);//gfaces[k]->count;

					Point3 temp_point[4];
					for (int j=0; j<pcount; j++) 
					{
						int index = ld->GetFaceGeomVert(k,j);//gfaces[k]->t[j];							
						if (j < 4)
							temp_point[j] = ld->GetGeomVert(index);//gverts.d[index].p;
					}
					
					fnorms[k] = Normalize(temp_point[1]-temp_point[0]^temp_point[2]-temp_point[1]);
				}
			}
				
			BitArray front,back,left,right,top,bottom;
			front.SetSize(ld->GetNumberFaces());
			front.ClearAll();
			back = front;
			left = front;
			right = front;
			top = front;
			bottom = front;

			Tab<Point3> norms;

			Matrix3 gtm(1);
			TimeValue t = 0;
			if (ip) t = ip->GetTime();
			if (tmControl)
				tmControl->GetValue(t,&gtm,FOREVER,CTRL_RELATIVE);

			norms.SetCount(6);
			for (int i = 0; i < 3; i++)
			{
				Point3 v = gtm.GetRow(i);
				norms[i*2] = Normalize(v);
				norms[i*2+1] = norms[i*2] * -1.0f;
			}
				
			for (int k=0; k< ld->GetNumberFaces(); k++) 
			{
				if (ld->GetFaceSelected(k))
				{
					int closestFace = -1;
					float closestAngle = -10.0f;
					for (int j = 0; j < 6; j++)
					{
						float dot = DotProd(norms[j],fnorms[k]);
						if (dot > closestAngle)
						{
							closestAngle = dot;
							closestFace = j;
						}
					}
					if (closestFace == 0)
						front.Set(k,TRUE);
					else if (closestFace == 1)
						back.Set(k,TRUE);
					else if (closestFace == 2)
						left.Set(k,TRUE);
					else if (closestFace == 3)
						right.Set(k,TRUE);
					else if (closestFace == 4)
						top.Set(k,TRUE);
					else if (closestFace == 5)
						bottom.Set(k,TRUE);
				}
			}


			gtm.IdentityMatrix();
			if (tmControl)
				tmControl->GetValue(t,&gtm,FOREVER,CTRL_RELATIVE);

			Point3 xvec,yvec,zvec;
			xvec = gtm.GetRow(0);
			yvec = gtm.GetRow(1);
			zvec = gtm.GetRow(2);
  			Point3 center = gtm.GetRow(3);


			for (int k = 0; k < 6; k++)
			{

				Matrix3 tm(1);
				if (k == 0)
				{
					tm.SetRow(0,yvec);
					tm.SetRow(1,zvec);
					tm.SetRow(2,xvec);
					ld->SetFaceSelection(front);
				}
				else if (k == 1)
				{
					tm.SetRow(0,yvec);
					tm.SetRow(1,zvec);
					tm.SetRow(2,(xvec*-1.0f));

					ld->SetFaceSelection(back);
				}
				else if (k == 2)
				{
					tm.SetRow(0,xvec);
					tm.SetRow(1,zvec);
					tm.SetRow(2,yvec);

					ld->SetFaceSelection(left);
				}
				else if (k == 3)
				{
					tm.SetRow(0,xvec);
					tm.SetRow(1,zvec);
					tm.SetRow(2,(yvec *-1.0f));

					ld->SetFaceSelection(right);
				}
				else if (k == 4)
				{
					tm.SetRow(0,xvec);
					tm.SetRow(1,yvec);
					tm.SetRow(2,zvec);

					ld->SetFaceSelection(top);
				}
				else if (k == 5)
				{
					tm.SetRow(0,xvec);
					tm.SetRow(1,yvec);
					tm.SetRow(2,(zvec*-1.0f));

					ld->SetFaceSelection(bottom);
				}

				

				tm.SetRow(3,center);

				if (!fnGetNormalizeMap())
				{
					for (int i = 0; i < 3; i++)
					{
						Point3 vec = tm.GetRow(i);
						vec = Normalize(vec);
						tm.SetRow(i,vec);
					}
				}

				tm = mMeshTopoData.GetNodeTM(t,ldID) * Inverse(tm);
				ld->ApplyMap(fnGetMapMode(), fnGetNormalizeMap(), tm, this);				
			}
			ld->RestoreFaceSel();
		}

		theHold.Accept(_T(GetString(IDS_PW_PLANARMAP)));
		
	}

}
void UnwrapMod::ApplyGizmoPrivate(Matrix3 *defaultTM)
{
	BOOL wasHolding = FALSE;
 	if (theHold.Holding())
		wasHolding = TRUE;


	if (!theHold.Holding())
	{
		theHold.Begin();
	}
	HoldPointsAndFaces();	


	//add vertices to our internal vertex list filling in dead spots where appropriate

	//get align normal
	//get fit data


	Matrix3 gtm(1);
	TimeValue t = 0;
	if (ip) t = ip->GetTime();
	if (defaultTM)
		gtm = *defaultTM;
	else
	{
		if (tmControl)
		{
			
			gtm = GetMapGizmoMatrix(t);

			if (!fnGetNormalizeMap())
			{
				for (int i = 0; i < 3; i++)
				{
					Point3 vec = gtm.GetRow(i);
					vec = Normalize(vec);
					gtm.SetRow(i,vec);
				}
			}

		}

	}

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		Matrix3 tm(1);
		tm = mMeshTopoData.GetNodeTM(t,ldID)* Inverse(gtm);
		ld->ApplyMap(fnGetMapMode(), fnGetNormalizeMap(), tm, this);
		ld->SetTVEdgeInvalid();
		ld->BuildTVEdges();
		ld->BuildVertexClusterList();
		
	}


	if (!wasHolding)
	{
		theHold.Accept(_T(GetString(IDS_PW_PLANARMAP)));
	}

	RebuildEdges();
	theHold.Suspend();
	fnFaceToEdgeSelect();
	theHold.Resume();

//	ConvertFaceToEdgeSel();
//	TVMaps.edgesValid= FALSE;
	//update our views to show new faces

	InvalidateView();

}
