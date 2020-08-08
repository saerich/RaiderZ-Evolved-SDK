#include "unwrap.h"

#include "3dsmaxport.h"

HCURSOR SketchMode::GetXFormCur()   
{
	return mod->selCur;
}

void SketchMode::GetIndexListFromSelection()
{

	//clear out indexList
	indexList.ZeroCount();

	for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
	{
		//save esel
		//save vsel

		MeshTopoData *ld = mod->GetMeshTopoData(ldID);
		BitArray vsel = ld->GetTVVertSelection();
		BitArray esel = ld->GetTVEdgeSelection();

		BitArray originalVSel(vsel);
		BitArray originalESel(esel);

		//transfer selection to egde
		ld->GetEdgeSelFromVert(esel,FALSE);
		//get first open vertex

		int firstVertex = -1;
		Tab<int> selCount;
		selCount.SetCount(ld->GetNumberTVVerts());//mod->TVMaps.v.Count());

		for (int i =0; i < ld->GetNumberTVVerts(); i++)//mod->TVMaps.v.Count(); i++)
			selCount[i] = 0;

		for (int i =0; i < esel.GetSize(); i++)
		{
			if (esel[i] )
			{
				int a = ld->GetTVEdgeVert(i,0);//mod->TVMaps.ePtrList[i]->a; 
				int b = ld->GetTVEdgeVert(i,1);//mod->TVMaps.ePtrList[i]->b; 

				selCount[a] += 1;
				selCount[b] += 1;
			}
		}

		for (int i =0; i < ld->GetNumberTVVerts(); i++)//mod->TVMaps.v.Count(); i++)
		{
			if (selCount[i] == 1)
			{
				firstVertex = i;
				i = ld->GetNumberTVVerts(); //mod->TVMaps.v.Count();
			}
		}

		if (firstVertex == -1) //is a loop or non conitgous just pick an arboitray start point
		{

			//then just add the remaining arbitrarily
			for (int i =0; i < vsel.GetSize(); i++)
			{
				if (vsel[i]) 
				{
					firstVertex = i;
					i = vsel.GetSize(); 
				}
			}
		}

		//walk this edge
		BOOL done = FALSE;
		BitArray processedEdges(esel);
		processedEdges.ClearAll();
		BitArray processedVerts(vsel);
		processedVerts.ClearAll();

		SketchIndexData skData;
		skData.mIndex = firstVertex;
		skData.mLD = ld;
		indexList.Append(1,&skData);
		processedVerts.Set(firstVertex);

		int edgeListSize = -1;
		BOOL addFirstHandle = TRUE;
		while (!done)  //ugliness below since this is not very efficient
		{
			edgeListSize = processedEdges.NumberSet();
			for (int i =0; i < esel.GetSize(); i++)
			{
				if (esel[i] && (!(ld->GetTVEdgeHidden(i))))//mod->TVMaps.ePtrList[i]->flags & FLAG_HIDDENEDGEA)) )
				{
					if (!processedEdges[i])
					{
						int a = ld->GetTVEdgeVert(i,0);//mod->TVMaps.ePtrList[i]->a; 
						int b = ld->GetTVEdgeVert(i,1);//mod->TVMaps.ePtrList[i]->b; 
						int va = ld->GetTVEdgeVec(i,0);//mod->TVMaps.ePtrList[i]->avec; 
						int vb = ld->GetTVEdgeVec(i,1);//mod->TVMaps.ePtrList[i]->bvec; 
						if ( (a == firstVertex) && (!processedVerts[b]))
						{
							if ((addFirstHandle) && (va != -1))
							{
								SketchIndexData skData;
								skData.mIndex = va;
								skData.mLD = ld;
								indexList.Append(1,&skData);
								processedVerts.Set(va);
							}
							if (vb != -1)     
							{
								SketchIndexData skData;
								skData.mIndex = vb;
								skData.mLD = ld;
								indexList.Append(1,&skData);
//								indexList.Append(1,&vb);
								processedVerts.Set(vb);
							}
							SketchIndexData skData;
							skData.mIndex = b;
							skData.mLD = ld;
							indexList.Append(1,&skData);
//							indexList.Append(1,&b);
							processedVerts.Set(b);
							firstVertex = b;
							processedEdges.Set(i);
							addFirstHandle = FALSE;
						}
						else if ((b == firstVertex) && (!processedVerts[a]))
						{
							if ((addFirstHandle) && (vb != -1))
							{
								SketchIndexData skData;
								skData.mIndex = vb;
								skData.mLD = ld;
								indexList.Append(1,&skData);
//								indexList.Append(1,&vb);
								processedVerts.Set(vb);
							}
							if (va != -1)
							{
								SketchIndexData skData;
								skData.mIndex = va;
								skData.mLD = ld;
								indexList.Append(1,&skData);
//								indexList.Append(1,&va);
								processedVerts.Set(va);
							}
							SketchIndexData skData;
							skData.mIndex = a;
							skData.mLD = ld;
							indexList.Append(1,&skData);
//							indexList.Append(1,&a);
							processedVerts.Set(a);
							firstVertex = a;
							processedEdges.Set(i);
							addFirstHandle = FALSE;
						}
					}
				}
			}
			if (edgeListSize == processedEdges.NumberSet()) done = TRUE;
		}

		//then just add the remaining arbitrarily
		for (int i =0; i < vsel.GetSize(); i++)
		{
			if ( (!processedVerts[i]) && (vsel[i]) )
			{
				SketchIndexData skData;
				skData.mIndex = i;
				skData.mLD = ld;
				indexList.Append(1,&skData);
//				indexList.Append(1,&i);
			}
		}

		//restore selections
		ld->SetTVEdgeSelection(originalESel);//mod->esel = originalESel;
		ld->SetTVVertSelection(originalVSel);//mod->vsel = originalVSel;	
	}
}

void SketchMode::Apply(HWND hWnd, IPoint2 m)
{

	mod->BuildSnapBuffer();
	if (mod->sketchType == SKETCH_LINE)
	{
		//apply the sketch
		float xzoom, yzoom;
		int width, height;
		mod->ComputeZooms(hWnd,xzoom,yzoom,width,height);
		Point2 center = mod->UVWToScreen(Point3(0.0f,0.0f,0.0f),xzoom,yzoom,width,height);
		//convert points from screen to UVW Space
		pointList.SetCount(2);

		Point3 p;
		p.x = ((float)(prevPoint.x-center.x))/xzoom;
		p.y = -((float)(prevPoint.y-center.y))/yzoom;
		p.z = 0.0f;

		pointList[0] = new Point3(mod->SnapPoint(p,indexList[0].mLD,indexList[0].mIndex));

		p.x = ((float)(m.x-center.x))/xzoom;
		p.y = -((float)(m.y-center.y))/yzoom;
		p.z = 0.0f;
		pointList[1] =  new Point3(mod->SnapPoint(p,indexList[indexList.Count()-1].mLD,indexList[indexList.Count()-1].mIndex));

		mod->Sketch(&indexList,&pointList);
		delete pointList[0];
		delete pointList[1];


	}
	else if (mod->sketchType == SKETCH_FREEFORM)
	{
		//apply the sketch
		float xzoom, yzoom;
		int width, height;
		mod->ComputeZooms(hWnd,xzoom,yzoom,width,height);
		Point2 center = mod->UVWToScreen(Point3(0.0f,0.0f,0.0f),xzoom,yzoom,width,height);
		//convert points from screen to UVW Space
		pointList.SetCount(ipointList.Count());

		for (int i = 0; i < pointList.Count(); i++)
		{
			Point3 p;
			p.x = ((float)(ipointList[i].x-center.x))/xzoom;
			p.y = -((float)(ipointList[i].y-center.y))/yzoom;
			p.z = 0.0f;

			pointList[i] = new Point3(mod->SnapPoint(p,NULL,-1));
		}



		mod->Sketch(&indexList,&pointList);
		for (int i = 0; i < pointList.Count(); i++)
		{
			delete pointList[i];
		}


	}
	else if (mod->sketchType == SKETCH_BOX)
	{
		//apply the sketch
		float xzoom, yzoom;
		int width, height;
		mod->ComputeZooms(hWnd,xzoom,yzoom,width,height);
		Point2 center = mod->UVWToScreen(Point3(0.0f,0.0f,0.0f),xzoom,yzoom,width,height);
		//convert points from screen to UVW Space
		pointList.SetCount(5);


		Point3 p;
		p.x = ((float)(prevPoint.x-center.x))/xzoom;
		p.y = -((float)(prevPoint.y-center.y))/yzoom;
		p.z = 0.0f;

		Point3 c0 = mod->SnapPoint(p,NULL,-1);


		p.x = ((float)(m.x-center.x))/xzoom;
		p.y = -((float)(m.y-center.y))/yzoom;
		p.z = 0.0f;

		Point3 c2 = mod->SnapPoint(p,NULL,-1);


		pointList[0] = new Point3(c0);


		Point3 c1 = c2;
		c1.y = c0.y;

		pointList[1] = new Point3(c1);

		pointList[2] =  new Point3(c2);

		Point3 c3 = c0;
		c3.y = c2.y;


		pointList[3] =  new Point3(c3);

		pointList[4] = new Point3(c0);



		mod->Sketch(&indexList,&pointList,TRUE);

		delete pointList[0];
		delete pointList[1];
		delete pointList[2];
		delete pointList[3];

	}
	else if (mod->sketchType == SKETCH_CIRCLE)
	{

		//apply the sketch
		float xzoom, yzoom;
		int width, height;
		mod->ComputeZooms(hWnd,xzoom,yzoom,width,height);
		Point2 center = mod->UVWToScreen(Point3(0.0f,0.0f,0.0f),xzoom,yzoom,width,height);

		Point3 snapCenter;
		snapCenter.x = ((float)(prevPoint.x-center.x))/xzoom;
		snapCenter.y = -((float)(prevPoint.y-center.y))/yzoom;
		snapCenter.z = 0.0f;
		snapCenter = mod->SnapPoint(snapCenter,NULL,-1);


		Point3 snapEdge;

		snapEdge.x = ((float)(m.x-center.x))/xzoom;
		snapEdge.y = -((float)(m.y-center.y))/yzoom;
		snapEdge.z = 0.0f;
		snapEdge = mod->SnapPoint(snapEdge,NULL,-1);

		//convert points from screen to UVW Space
		pointList.SetCount(indexList.Count()*2);
		float radius = 0.0f;

		radius = Length(snapEdge-snapCenter);

		if (radius == 0.0f) radius = 0.000001f;

		Point3 vec;
		vec.x = snapEdge.x - snapCenter.x;
		vec.y = snapEdge.y - snapCenter.y;
		vec.z = 0;
		vec = Normalize(vec) * radius;


		float angleInc = 0.0f;
		float angle = 0.0f;
		angleInc = (2.0f*PI)/(float)(pointList.Count());
		Point3 start;
		for (int i = 0; i < (pointList.Count()-1); i++)
		{
			Point3 p;
			p = vec;
			Matrix3 tempMat(1);  
			tempMat.RotateZ(angle); 
			p = (p * tempMat);
			p.x += snapCenter.x;
			p.y += snapCenter.y;

			p.z = 0.0f;


			if (i==0) start = p;
			angle += angleInc;

			pointList[i] = new Point3(p);
		}
		if (pointList.Count() > 0)
		{
			pointList[pointList.Count()-1] = new Point3(start);


			mod->Sketch(&indexList,&pointList,TRUE);
		}

		for (int i = 0; i < pointList.Count(); i++)
		{
			delete pointList[i];
		}


	}
}



int SketchMode::proc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{

	static IPoint2 om;

	switch (msg) {
	case MOUSE_POINT:
		{
			//this is needed because we can flip flop pick/drag modes in the middle of a sketch and just removes a point message on mouse up on the last ALT click
			if ( (inPickLineMode) && (mod->sketchSelMode != SKETCH_DRAWMODE))
			{


				int ct = ipointList.Count()-1;
				inPickLineMode = FALSE;
				IPoint2 points[2];
				points[0] = ipointList[ct];
				points[1] = om;
				XORDottedPolyline(hWnd, 2, points);
				ipointList.Append(1,&m);


				return 1;
			}


			if ((flags & MOUSE_CTRL) && (mod->sketchSelMode == SKETCH_SELCURRENT))//we are in a draw mode
			{
				// Hit test
				Tab<TVHitData> hits;
				Rect rect;
				rect.left = m.x-2;
				rect.right = m.x+2;
				rect.top = m.y-2;
				rect.bottom = m.y+2;
				// First hit test sel only
				if (mod->HitTest(rect,hits,TRUE))
				{  
					mod->fnSketchSetFirst(hits[0].mID);

				}
				mod->ip->ReplacePrompt( GetString(IDS_PW_SKETCHPROMPT_FREEFORM));

			}
			else if ( (mod->sketchSelMode == SKETCH_DRAWMODE)  || (mod->sketchSelMode == SKETCH_SELCURRENT))//we are in a draw mode
			{

				mod->sketchSelMode = SKETCH_DRAWMODE;
				if ( (mod->sketchType == SKETCH_LINE) || (mod->sketchType == SKETCH_FREEFORM)  || (mod->sketchType == SKETCH_BOX) || (mod->sketchType == SKETCH_CIRCLE) ) 
				{
					if (drawPointCount == 0)
					{



						prevPoint = m;
						om = m;
						ipointList.ZeroCount();
						ipointList.Append(1,&m);
						theHold.Begin();
						mod->HoldPoints();
//						theHold.Put(new TVertRestore(mod,FALSE));
						if (!mod->sketchInteractiveMode)
						{
							//need to pkug the controllers also so they get put on the undo stack

							theHold.Accept(_T(GetString(IDS_PW_MOVE_UVW)));
						}
						drawPointCount++;
					}

					else if ((drawPointCount == 1) && (flags & MOUSE_ALT) && (mod->sketchType == SKETCH_FREEFORM))
					{
						if (mod->sketchInteractiveMode)
							theHold.Restore();


						//draw a line from our pev point to our current point
						//                if ((m.x != ipointList[ct].x) && (m.y != ipointList[ct].y))
						{
							ipointList.Append(1,&m);
							if ((mod->sketchInteractiveMode) && (drawPointCount == 1))
							{
								Apply(hWnd,m);
								if (mod->sketchInteractiveMode)
									mod->InvalidateView();
							}
						}
						om = m;
						inPickLineMode = TRUE;
					}


					else if ((drawPointCount == 1) && (!inPickLineMode))
					{
						drawPointCount++;
						if (mod->sketchType == SKETCH_LINE)
						{
							if (flags & MOUSE_SHIFT)
							{
								float xdist = abs(prevPoint.x - m.x);
								float ydist = abs(prevPoint.y - m.y);
								if (xdist < ydist)
									m.x = prevPoint.x;
								else m.y = prevPoint.y;
							}
						}
						mod->sketchSelMode = SKETCH_APPLYMODE;
					}
					else inPickLineMode = FALSE;


				}
			}
			//we are now in the apply mode
			if (mod->sketchSelMode == SKETCH_APPLYMODE)  //we are in a pick mode
			{
				if (mod->sketchInteractiveMode)
					theHold.Accept(_T(GetString(IDS_PW_MOVE_UVW)));

				Apply(hWnd,m);

				mod->sketchSelMode = oldMode;


				pointCount = 0;
				drawPointCount =0;
				if (mod->sketchSelMode != SKETCH_SELCURRENT)
				{
					for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
					{
						mod->GetMeshTopoData(ldID)->ClearSelection(TVVERTMODE);//mod->vsel.ClearAll();
					}
					
					mod->RebuildDistCache();
				}
				mod->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				mod->ip->RedrawViews(mod->ip->GetTime());
				mod->InvalidateView();

				if (mod->sketchSelMode == SKETCH_SELPICK)
					mod->ip->ReplacePrompt( GetString(IDS_PW_SKETCHPROMPT_PICKSEL));
				else if (mod->sketchSelMode == SKETCH_SELDRAG)
					mod->ip->ReplacePrompt( GetString(IDS_PW_SKETCHPROMPT_DRAGSEL));           

			}

			else if (mod->sketchSelMode == SKETCH_SELPICK)  //we are in a pick mode
			{
				//see if that point hit a vert if so add it and select it
				if (pointCount == 0) 
				{  
					drawPointCount = 0;
					indexList.ZeroCount();
					for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
					{
						mod->GetMeshTopoData(ldID)->ClearSelection(TVVERTMODE);//mod->vsel.ClearAll();
					} //	mod->vsel.ClearAll();

					mod->RebuildDistCache();
					oldMode = SKETCH_SELPICK;
				}
				// Hit test
				Tab<TVHitData> hits;
				Rect rect;
				rect.left = m.x-2;
				rect.right = m.x+2;
				rect.top = m.y-2;
				rect.bottom = m.y+2;
				// First hit test sel only
				BOOL hitVert = FALSE;
				if (mod->HitTest(rect,hits,FALSE))
				{
					for (int i =0; i < hits.Count(); i++)
					{
						int index = hits[i].mID;
						MeshTopoData *ld = mod->GetMeshTopoData(hits[i].mLocalDataID);

						if (ld && (index >=0) && (index < ld->GetNumberTVVerts()))//mod->TVMaps.v.Count()))
						{
							if (!ld->GetTVVertSelected(index))//mod->vsel[index])
							{
								SketchIndexData skData;
								skData.mIndex = index;
								skData.mLD = ld;
								indexList.Append(1,&skData);
								ld->SetTVVertSelected(index,TRUE);//mod->vsel.Set(index);
								hitVert = TRUE;
							}
						}

					}
				}
				if (hitVert) 
					mod->RebuildDistCache();
				lastPoint = pointCount;
				prevPoint = m;
				mod->InvalidateView();

				pointCount++;  
			}



			else if (mod->sketchSelMode == SKETCH_SELDRAG)  //we are in a drag sel mode
			{
				if (pointCount == 0) 
				{
					drawPointCount = 0;
					indexList.ZeroCount();
					for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
					{
						mod->GetMeshTopoData(ldID)->ClearSelection(TVVERTMODE);//mod->vsel.ClearAll();
					}
					mod->RebuildDistCache();
					oldMode = SKETCH_SELDRAG;
					// Hit test
					Tab<TVHitData> hits;
					Rect rect;

					rect.right = m.x+mod->sketchCursorSize;
					rect.left = m.x-mod->sketchCursorSize;
					rect.bottom = m.y+mod->sketchCursorSize;
					rect.top = m.y-mod->sketchCursorSize;

					SetCursor(mod->circleCur[mod->sketchCursorSize]);

					// First hit test sel only
					if (mod->HitTest(rect,hits,FALSE,TRUE))
					{
						for (int i =0; i < hits.Count(); i++)
						{
							int index = hits[i].mID;
							MeshTopoData *ld = mod->GetMeshTopoData(hits[i].mLocalDataID);

							if (ld && (index >=0) && (index < ld->GetNumberTVVerts()))//mod->TVMaps.v.Count()))
							{
								if (!ld->GetTVVertSelected(index))//mod->vsel[index])
								{
									SketchIndexData skData;
									skData.mIndex = index;
									skData.mLD = ld;
									indexList.Append(1,&skData);//indexList.Append(1,&index);
									ld->SetTVVertSelected(index,TRUE);//mod->vsel.Set(index);
								}
							}

						}
					}
					pointCount++;
					mod->InvalidateView();
				}
				else if (pointCount==1)
				{
					//             SetCursor(LoadCursor(NULL, IDC_ARROW));
					SetCursor(mod->sketchCur);

					mod->sketchSelMode = SKETCH_DRAWMODE;
					pointCount++;

					if (mod->sketchType == SKETCH_LINE)
						mod->ip->ReplacePrompt( GetString(IDS_PW_SKETCHPROMPT_LINE));
					else if (mod->sketchType == SKETCH_FREEFORM)
					{
						mod->ip->ReplacePrompt( GetString(IDS_PW_SKETCHPROMPT_FREEFORM));
					}
					else if (mod->sketchType == SKETCH_BOX)
					{
						mod->ip->ReplacePrompt( GetString(IDS_PW_SKETCHPROMPT_BOX));
					}
					else if (mod->sketchType == SKETCH_CIRCLE)
					{
						mod->ip->ReplacePrompt( GetString(IDS_PW_SKETCHPROMPT_CIRCLE));
					}
				}
			}








			break;
		}

	case MOUSE_FREEMOVE: {
		Tab<TVHitData> hits;
		Rect rect;
		rect.left = m.x-2;
		rect.right = m.x+2;
		rect.top = m.y-2;
		rect.bottom = m.y+2;
		if (mod->sketchSelMode != SKETCH_DRAWMODE)
		{
			if ((mod->sketchSelMode == SKETCH_SELDRAG)  && (pointCount ==0))
			{
				SetCursor(mod->circleCur[mod->sketchCursorSize]);
			}
			else if (mod->HitTest(rect,hits,FALSE) && (mod->sketchSelMode != SKETCH_SELCURRENT)) 
			{
				SetCursor(mod->sketchPickHitCur);
			}
			else
			{
				if (mod->sketchSelMode == SKETCH_SELPICK)
					SetCursor(mod->sketchPickCur);
				else SetCursor(mod->sketchCur);
				//             SetCursor(LoadCursor(NULL, IDC_ARROW));
			}
		}
		else if (mod->sketchSelMode == SKETCH_DRAWMODE)
		{
			SetCursor(mod->sketchCur);

			if ((mod->sketchType == SKETCH_FREEFORM) && (drawPointCount > 0) && (flags & MOUSE_ALT))
			{

				int ct = ipointList.Count()-1;

				IPoint2 points[2];
				if (ct > 0)
				{
					points[0] = ipointList[ct-1];
					points[1] = om;
					XORDottedPolyline(hWnd, 2, points);

					points[0] = ipointList[ct-1];
					points[1] = m;
					XORDottedPolyline(hWnd, 2, points);
				}
				om = m;

				if (mod->sketchInteractiveMode)
					theHold.Restore();

				ct = ipointList.Count()-1;

				ipointList[ct] = m;

				if ((mod->sketchInteractiveMode) && (drawPointCount == 1))
					Apply(hWnd,m);

				mod->InvalidateView();

			}

		}              
		break;

						 }

	case MOUSE_MOVE: 
		{
			if ((mod->sketchSelMode == SKETCH_DRAWMODE) )
			{

				SetCursor(mod->sketchCur);
				BOOL restore = FALSE;
				if (mod->sketchType == SKETCH_LINE)
				{
					if (drawPointCount == 1)
					{
						//draw a line from our pev point to our current point


						IPoint2 points[2];

						if (flags & MOUSE_SHIFT)
						{
							float xdist = abs(prevPoint.x - m.x);
							float ydist = abs(prevPoint.y - m.y);
							if (xdist < ydist)
								m.x = prevPoint.x;
							else m.y = prevPoint.y;
						}

						points[0] = prevPoint;
						points[1] = om;
						XORDottedPolyline(hWnd, 2, points);
						points[0] = prevPoint;
						points[1] = m;
						XORDottedPolyline(hWnd, 2, points);
						om = m;
						if (mod->sketchInteractiveMode)
							mod->InvalidateView();
						if (mod->sketchInteractiveMode)
							restore = TRUE;
					}

				}
				else if (mod->sketchType == SKETCH_BOX)
				{
					if (drawPointCount == 1)
					{
						//draw a line from our pev point to our current point


						IPoint2 points[2];
						points[0] = prevPoint;
						points[1] = om;
						XORDottedRect(hWnd, points[0],points[1]);
						points[0] = prevPoint;
						points[1] = m;
						XORDottedRect(hWnd,   points[0],points[1]);
						om = m;
						if (mod->sketchInteractiveMode)
							mod->InvalidateView();
						if (mod->sketchInteractiveMode)
							restore = TRUE;
					}

				}
				else if (mod->sketchType == SKETCH_CIRCLE)
				{
					if (drawPointCount == 1)
					{
						//draw a line from our pev point to our current point


						IPoint2 points[2];
						points[0] = prevPoint;
						points[1] = om;
						XORDottedCircle(hWnd, points[0],points[1]);
						points[0] = prevPoint;
						points[1] = m;
						XORDottedCircle(hWnd,   points[0],points[1]);
						om = m;
						if (mod->sketchInteractiveMode)
							mod->InvalidateView();
						if (mod->sketchInteractiveMode)
							restore = TRUE;
							
					}

				}

				else if (mod->sketchType == SKETCH_FREEFORM)
				{
					if ( (drawPointCount == 1) && (!(flags & MOUSE_ALT)))
					{


						//draw a line from our pev point to our current point
						int ct = ipointList.Count()-1;
						if ((m.x != ipointList[ct].x) && (m.y != ipointList[ct].y))
						{
							ipointList.Append(1,&m);
							XORDottedPolyline(hWnd, ipointList.Count(), ipointList.Addr(0));
							mod->InvalidateView();
						}
						om = m;
						inPickLineMode = FALSE;
						if (mod->sketchInteractiveMode)
							restore = TRUE;
					}

				}
				if ((mod->sketchInteractiveMode) && (drawPointCount == 1))
				{
					Apply(hWnd,m);
					UpdateWindow(hWnd);
				}
				if (restore)
					theHold.Restore();

			}
			else
			{
				if ((mod->sketchSelMode == SKETCH_SELDRAG)  && (pointCount ==1))//we are in a pick mode
				{
					// Hit test
					Tab<TVHitData> hits;
					Rect rect;

					rect.right = m.x+mod->sketchCursorSize;
					rect.left = m.x-mod->sketchCursorSize;;
					rect.bottom = m.y+mod->sketchCursorSize;
					rect.top = m.y-mod->sketchCursorSize;;

					/*             rect.left = m.x-2;
					rect.right = m.x+2;
					rect.top = m.y-2;
					rect.bottom = m.y+2;
					*/
					// First hit test sel only
					BOOL hitVert = FALSE;
					if (mod->HitTest(rect,hits,FALSE,TRUE))
					{
						for (int i =0; i < hits.Count(); i++)
						{
							int index = hits[i].mID;
							MeshTopoData *ld = mod->GetMeshTopoData(hits[i].mLocalDataID);

							if (ld && (index >=0) && (index < ld->GetNumberTVVerts()))//mod->TVMaps.v.Count()))
							{
								if (!ld->GetTVVertSelected(index))//mod->vsel[index])
								{
									SketchIndexData skData;
									skData.mIndex = index;
									skData.mLD = ld;
									indexList.Append(1,&skData);//indexList.Append(1,&index);
									ld->SetTVVertSelected(index,TRUE);//mod->vsel.Set(index);
									hitVert = TRUE;
								}
							}

						}
						if (hitVert)
						{
							mod->RebuildDistCache();
							mod->InvalidateView();
							UpdateWindow(hWnd);

						}


						//                SetCursor(mod->selCur);
					}
					else
					{
						//                SetCursor(LoadCursor(NULL, IDC_ARROW));
					}
					lastPoint = point;
					prevPoint = m;
				}

			}
			break;      
		}



	case MOUSE_ABORT:
		{
			if (mod->sketchSelMode == SKETCH_SELPICK)  //we are in a pick mode
			{
				mod->SetMode(oldMode);
				//          mod->sketchSelMode = oldMode;


			}
			else if (mod->sketchSelMode == SKETCH_SELDRAG) 
			{
				mod->sketchSelMode = oldMode;

				pointCount = 0;
				drawPointCount =0;
				for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
				{
					mod->GetMeshTopoData(ldID)->ClearSelection(TVVERTMODE);//mod->vsel.ClearAll();
				}
				indexList.ZeroCount();
				mod->InvalidateView();
			}
			else if ( (mod->sketchSelMode == SKETCH_DRAWMODE)  || (mod->sketchSelMode == SKETCH_SELCURRENT))//we are in a draw mode
			{
				if ( (mod->sketchType == SKETCH_LINE) || (mod->sketchType == SKETCH_FREEFORM)  || (mod->sketchType == SKETCH_BOX) || (mod->sketchType == SKETCH_CIRCLE) ) 
				{
					//bail back to our old mode
					if (mod->sketchInteractiveMode)
						theHold.Cancel();
					mod->sketchSelMode = oldMode;

					pointCount = 0;
					drawPointCount =0;
					for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
					{
						mod->GetMeshTopoData(ldID)->ClearSelection(TVVERTMODE);//mod->vsel.ClearAll();
					}
					indexList.ZeroCount();
					mod->InvalidateView();

				}

			}

			//convert our sub selection type to current selection
			break;
		}
	}
	return 1;
}


void  UnwrapMod::fnSketch(Tab<int> *indexList,Tab<Point3*> *positionList)
{
	int indexCount = indexList->Count();
	Tab<SketchIndexData> newData;
	newData.SetCount(indexCount);
	for (int i = 0; i < indexCount; i++)
	{
//		(*indexList)[i] -= 1;
		newData[i].mIndex = (*indexList)[i] -1;
		newData[i].mLD = mMeshTopoData[0];
	}
	Sketch(&newData,positionList);
}

void  UnwrapMod::fnSketch(Tab<int> *indexList,Tab<Point3*> *positionList,INode *node)
{
	int indexCount = indexList->Count();
	Tab<SketchIndexData> newData;
	newData.SetCount(indexCount);
	MeshTopoData *ld = GetMeshTopoData(node);
	for (int i = 0; i < indexCount; i++)
	{
//		(*indexList)[i] -= 1;
		newData[i].mIndex = (*indexList)[i] -1;
		newData[i].mLD = ld;
	}
	Sketch(&newData,positionList);
}


void  UnwrapMod::Sketch(Tab<SketchIndexData> *indexList,Tab<Point3*> *positionList, BOOL closed)
{
	//get the length
	InitReverseSoftData();
	Tab<float> edgeDist;
	float totalLength = 0.0f;



	int posCount = positionList->Count();
	int indexCount = indexList->Count();
	if (indexCount < 2 ) return;

	edgeDist.SetCount(posCount);
	for (int i = 0; i < posCount; i++)
	{
		//compute the inc amount
		float dist = 0.0f;
		if (i != 0)
		{
			Point3 p1 = *(*positionList)[i];
			Point3 p2 = *(*positionList)[i-1];

			dist = Length(p2-p1);
		}
		totalLength += dist;
		edgeDist[i] = totalLength;


	}

	float inc;
	if (closed)
		inc = totalLength/(float)(indexCount);
	else inc = totalLength/(float)(indexCount-1);
	float currentGoal = 0.0f;
	int currentIndex = 0;
	TimeValue t = GetCOREInterface()->GetTime();
	for (int i = 0; i < posCount; i++)
	{
		if (edgeDist[i] > currentGoal)
		{
			Point3 p = *(*positionList)[i-1];
			float per = (currentGoal - edgeDist[i-1]);
			float edgeLength =  edgeDist[i] - edgeDist[i-1];
			per = per/edgeLength;
			Point3 p1 = *(*positionList)[i];
			Point3 p2 = *(*positionList)[i-1];
			Point3 vec = (p1-p2)*(per);
			p += vec;
			//assign value
			int id = (*indexList)[currentIndex].mIndex;
			MeshTopoData *ld = (*indexList)[currentIndex].mLD;

			if ( (id >= 0) && (id < ld->GetNumberTVVerts()))//TVMaps.v.Count()) )
			{
				ld->SetTVVert(t,id,p,this);
				//TVMaps.v[id].p = p;
				//if (TVMaps.cont[id]) TVMaps.cont[id]->SetValue(0,&TVMaps.v[id].p);
			}

			currentGoal += inc;
			i--;
			currentIndex++;

			if ((currentIndex == (indexCount-1)) && (!closed))
			{
				int id = (*indexList)[currentIndex].mIndex;
				MeshTopoData *ld = (*indexList)[currentIndex].mLD;

				if ((id >= 0) && (id < ld->GetNumberTVVerts()))//TVMaps.v.Count()))
				{
					Point3 p = *(*positionList)[posCount-1];
					ld->SetTVVert(t,id,p,this);
					//TVMaps.v[id].p = p;
					//if (TVMaps.cont[id]) TVMaps.cont[id]->SetValue(0,&TVMaps.v[id].p);
				}

				ApplyReverseSoftData();
				return;
			}
			if (currentIndex >= indexCount)
			{
				ApplyReverseSoftData();
				return;
			}


		}
	}
}


void  UnwrapMod::fnSketchNoParams()
{
	sketchMode->pointCount = 0;
	sketchMode->drawPointCount = 0;
	restoreSketchSelMode = sketchSelMode;;
	restoreSketchType = sketchType;
	restoreSketchDisplayPoints = sketchDisplayPoints;
	restoreSketchInteractiveMode =  sketchInteractiveMode;

	restoreSketchCursorSize = sketchCursorSize; 
	sketchMode->ipointList.ZeroCount();
	sketchMode->inPickLineMode = FALSE;

	if (mode == ID_SKETCHMODE)
		SetMode(ID_UNWRAP_MOVE);


	if (sketchSelMode == SKETCH_SELCURRENT)  //we are in a drag sel mode
	{
		sketchMode->oldMode = SKETCH_SELCURRENT;
		sketchMode->oldSubObjectMode = fnGetTVSubMode();
		TransferSelectionStart();
		sketchMode->drawPointCount = 0;
		//if not in vertex mode convert to vertex
		sketchMode->GetIndexListFromSelection();
		InvalidateView();
		ip->ReplacePrompt( GetString(IDS_PW_SKETCHPROMPT_FIRST));
	}
	else
	{
		if (sketchSelMode == SKETCH_SELPICK)
			ip->ReplacePrompt( GetString(IDS_PW_SKETCHPROMPT_PICKSEL));
		else if (sketchSelMode == SKETCH_SELDRAG)
			ip->ReplacePrompt( GetString(IDS_PW_SKETCHPROMPT_DRAGSEL));

		InvalidateView();
		UpdateWindow(hView);

	}

	SetMode(ID_SKETCHMODE);
	//hold
	//go into select mode
	//gather point
	//go into draw mode
	//apply
}

void  UnwrapMod::fnSketchReverse()
{
	Tab<SketchIndexData> tempIndex;
	tempIndex.SetCount(sketchMode->indexList.Count());
	for (int i =0; i < tempIndex.Count(); i++)
	{
		tempIndex[i] = sketchMode->indexList[i];
	}
	int ct = tempIndex.Count()-1;
	for (int i =0; i < tempIndex.Count(); i++)
	{
		sketchMode->indexList[i] = tempIndex[ct--]; 
	}

	InvalidateView();

}

void  UnwrapMod::fnSketchSetFirst(int index)
{

	for (int i =0; i < sketchMode->indexList.Count(); i++)
	{
		if (sketchMode->indexList[i].mIndex == index)
		{
			index = i;
			i = sketchMode->indexList.Count(); 
		}
	}

	Tab<SketchIndexData> tempIndex;
	tempIndex.SetCount(sketchMode->indexList.Count());
	for (int i =0; i < tempIndex.Count(); i++)
	{
		tempIndex[i] = sketchMode->indexList[i];
	}
	int start = index;
	for (int i =0; i < tempIndex.Count(); i++)
	{
		sketchMode->indexList[i] = tempIndex[start]; 
		start++;
		if (start >= tempIndex.Count()) start = 0;
	}

	InvalidateView();

}




void  UnwrapMod::fnSketchDialog()
{
	//bring up the dialog
	DialogBoxParam(   hInstance,
		MAKEINTRESOURCE(IDD_SKETCHDIALOG),
		GetCOREInterface()->GetMAXHWnd(),
		//                   hWnd,
		UnwrapSketchFloaterDlgProc,
		(LPARAM)this );


}

void  UnwrapMod::SetSketchDialogPos()
{
	if (sketchWindowPos.length != 0) 
		SetWindowPlacement(sketchHWND,&sketchWindowPos);
}

void  UnwrapMod::SaveSketchDialogPos()
{
	sketchWindowPos.length = sizeof(WINDOWPLACEMENT); 
	GetWindowPlacement(sketchHWND,&sketchWindowPos);
}



INT_PTR CALLBACK UnwrapSketchFloaterDlgProc(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UnwrapMod *mod = DLGetWindowLongPtr<UnwrapMod*>(hWnd);
	//POINTS p = MAKEPOINTS(lParam); commented out by sca 10/7/98 -- causing warning since unused.

	static ISpinnerControl *iCursorSize = NULL;


	switch (msg) {
	  case WM_INITDIALOG:
		  {

			  mod = (UnwrapMod*)lParam;
			  mod->sketchHWND = hWnd;

			  if (mod->mode == ID_SKETCHMODE)
				  mod->SetMode(ID_UNWRAP_MOVE);

			  DLSetWindowLongPtr(hWnd, lParam);
			  ::SetWindowContextHelpId(hWnd, idh_unwrap_sketch);

			  HWND hMethod = GetDlgItem(hWnd,IDC_SELECT_COMBO);
			  SendMessage(hMethod, CB_RESETCONTENT, 0, 0);

			  SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_PICKSELECT));
			  SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_DRAGSELECT));
			  
			  int ct = 0;
			  for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
			  { 
				  ct += mod->GetMeshTopoData(ldID)->GetTVVertSelection().NumberSet();
			  }

			  if (ct/*mod->vsel.NumberSet()*/ != 0)
			  {
				  SendMessage(hMethod, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_CURRENTSELECT));

			  }
			  else
			  {
				  if (mod->sketchSelMode == 3)
					  mod->sketchSelMode = 1;
			  }


			  SendMessage(hMethod, CB_SETCURSEL, (mod->sketchSelMode-1), 0L);



			  HWND hAlign = GetDlgItem(hWnd,IDC_ALIGN_COMBO);
			  SendMessage(hAlign, CB_RESETCONTENT, 0, 0);

			  SendMessage(hAlign, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_SKETCHFREEFORM));
			  SendMessage(hAlign, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_SKETCHLINE));
			  SendMessage(hAlign, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_SKETCHBOX));
			  SendMessage(hAlign, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_SKETCHCIRCLE));

			  SendMessage(hAlign, CB_SETCURSEL, (mod->sketchType-1), 0L);

			  CheckDlgButton(hWnd,IDC_SHOW_VERTORDER,mod->sketchDisplayPoints);
			  CheckDlgButton(hWnd,IDC_INTERACTIVEMODE,mod->sketchInteractiveMode);


			  //create spinners and set value
			  iCursorSize = SetupIntSpinner(
				  hWnd,IDC_UNWRAP_DRAGSIZESPIN,IDC_UNWRAP_DRAGSIZE,
				  1,15,mod->sketchCursorSize);

			  break;
		  }

	  case WM_SYSCOMMAND:
		  if ((wParam & 0xfff0) == SC_CONTEXTHELP) 
		  {
			  DoHelp(HELP_CONTEXT, idh_unwrap_sketch); 
		  }
		  return FALSE;
		  break;
	  case WM_COMMAND:
		  switch (LOWORD(wParam)) {
	  case IDC_OK:
		  {
			  mod->SaveSketchDialogPos();


			  int tempSel,tempAlign;

			  tempSel = mod->sketchSelMode;
			  tempAlign = mod->sketchType;


			  mod->restoreSketchSettings = TRUE;
			  mod->restoreSketchSelMode = mod->sketchSelMode;;
			  mod->restoreSketchType = mod->sketchType;
			  mod->restoreSketchDisplayPoints = mod->sketchDisplayPoints;
			  mod->restoreSketchInteractiveMode =  mod->sketchInteractiveMode;

			  mod->restoreSketchCursorSize = mod->sketchCursorSize; 

			  HWND hMethod = GetDlgItem(hWnd,IDC_SELECT_COMBO);
			  mod->sketchSelMode = SendMessage(hMethod, CB_GETCURSEL, 0L, 0)+1;
			  HWND hAlign = GetDlgItem(hWnd,IDC_ALIGN_COMBO);
			  mod->sketchType = SendMessage(hAlign, CB_GETCURSEL, 0L, 0)+1;

			  mod->sketchDisplayPoints = IsDlgButtonChecked(hWnd,IDC_SHOW_VERTORDER);
			  mod->sketchInteractiveMode = IsDlgButtonChecked(hWnd,IDC_INTERACTIVEMODE);


			  mod->sketchCursorSize = iCursorSize->GetIVal();

			  mod->fnSketchNoParams();


			  ReleaseISpinner(iCursorSize);
			  iCursorSize = NULL;

			  EndDialog(hWnd,1);

			  break;
		  }
	  case IDC_CANCEL:
		  {

			  ReleaseISpinner(iCursorSize);
			  iCursorSize = NULL;

			  mod->SaveSketchDialogPos();

			  EndDialog(hWnd,0);

			  break;
		  }
	  case IDC_DEFAULT:
		  {

			  //get align
			  //             mod->unfoldNormalize = IsDlgButtonChecked(hWnd,IDC_NORMALIZE_CHECK);
			  HWND hMethod = GetDlgItem(hWnd,IDC_SELECT_COMBO);
			  mod->sketchSelMode = SendMessage(hMethod, CB_GETCURSEL, 0L, 0)+1;
			  HWND hAlign = GetDlgItem(hWnd,IDC_ALIGN_COMBO);
			  mod->sketchType = SendMessage(hAlign, CB_GETCURSEL, 0L, 0)+1;


			  mod->sketchDisplayPoints = IsDlgButtonChecked(hWnd,IDC_SHOW_VERTORDER);
			  mod->sketchInteractiveMode = IsDlgButtonChecked(hWnd,IDC_INTERACTIVEMODE);
			  mod->sketchCursorSize = iCursorSize->GetIVal();

			  break;
		  }

		  }
		  break;

	  default:
		  return FALSE;
	}
	return TRUE;
}



