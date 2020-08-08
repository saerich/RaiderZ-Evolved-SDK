#include "unwrap.h"
#include <new>

#include "3dsmaxport.h"

/*************************************************************



**************************************************************/
class RenderUVDlgProc : public ParamMap2UserDlgProc
{
public:
	UnwrapMod *mod;
	INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { delete this; };
};

INT_PTR RenderUVDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{

	switch (msg) {
		case WM_INITDIALOG:

			mod->renderUVWindow = hWnd;
			break;

		case WM_CLOSE:
//		case WM_DESTROY:
			
			if (mod->renderUVWindow )
			{
				mod->renderUVWindow = NULL;
				DestroyWindow(hWnd);	
			}
			
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_GUESS_BUTTON:
					{
		 				mod->GuessAspectRatio();
						break;
					}
				case IDC_RENDERBUTTON:
					{
 					mod->fnRenderUV();					
					TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.renderUV"));
					macroRecorder->FunctionCall(mstr, 1, 0,mr_string,mod->renderUVbi.Filename());
					macroRecorder->EmitScript();

					break;
					}
					


			}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}

void UnwrapMod::GuessAspectRatio()
{
	float x,y;
 	x = y = 0.0f;
	Point3 finalVec(0.0f,0.0f,0.0f);

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{

		MeshTopoData *ld = mMeshTopoData[ldID];
		Tab<int> hiddenEdgeCount;
		int fCount = ld->GetNumberFaces();
		hiddenEdgeCount.SetCount(fCount);
		for (int i = 0; i < fCount; i++)
			hiddenEdgeCount[i] = 0;

		for (int i = 0; i < ld->GetNumberGeomEdges(); i++) //TVMaps.gePtrList.Count(); i++)
		{
			int faceCount = ld->GetGeomEdgeNumberOfConnectedFaces(i);//TVMaps.gePtrList[i]->faceList.Count();
			BOOL hidden = ld->GetGeomEdgeHidden(i);//TVMaps.gePtrList[i]->flags & FLAG_HIDDENEDGEA;

			if (hidden)
			{
				for (int j = 0; j < faceCount; j++)
				{
					int faceIndex = ld->GetGeomEdgeConnectedFace(i,j);//TVMaps.gePtrList[i]->faceList[j];
					hiddenEdgeCount[faceIndex] += 1;
				}

			}
		}
		for (int i = 0; i < ld->GetNumberTVEdges(); i++)//TVMaps.ePtrList.Count(); i++)
		{
			//line up the face
			int a,b;
			a = ld->GetTVEdgeVert(i,0);//TVMaps.ePtrList[i]->a;
			b = ld->GetTVEdgeVert(i,1);//TVMaps.ePtrList[i]->b;

			int ga,gb;
			ga = ld->GetTVEdgeGeomVert(i,0);//TVMaps.ePtrList[i]->ga;
			gb = ld->GetTVEdgeGeomVert(i,1);//TVMaps.ePtrList[i]->gb;


			if (ld->IsTVVertVisible(a) && ld->IsTVVertVisible(b) && (!(ld->GetTVEdgeHidden(i))))//TVMaps.ePtrList[i]->flags & FLAG_HIDDENEDGEA)))
			{
			

				Point3 pa,pb;
				Point3 pga,pgb;

				pa = ld->GetTVVert(a);//TVMaps.v[a].p;
				pb = ld->GetTVVert(b);//TVMaps.v[b].p;

				pga = ld->GetGeomVert(ga);//TVMaps.geomPoints[ga];
				pgb = ld->GetGeomVert(gb);// TVMaps.geomPoints[gb];

				Point3 uvec,gvec;
				uvec = pb - pa;
				gvec = pgb - pga;

				Point3 nuvec,ngvec;
				nuvec = Normalize(pb - pa);
				ngvec = Normalize(pgb - pga);

				float uang = 3.14f;
				float vang = 3.14f;

				if (uvec.x > 0.0f)
					uang = fabs(ld->AngleFromVectors(nuvec,Point3(1.0f,0.0f,0.0f)));
				else uang = fabs(ld->AngleFromVectors(nuvec,Point3(-1.0f,0.0f,0.0f)));

				if (uvec.y > 0.0f)
					vang = fabs(ld->AngleFromVectors(nuvec,Point3(0.0f,1.0f,0.0f)));
				else vang = fabs(ld->AngleFromVectors(nuvec,Point3(0.0f,-1.0f,0.0f)));
				
				if (uang < (10.0f * 3.14f/180.0f))
				{
					float gwidth = Length(gvec);
					float uwidth = fabs(uvec.x);
				
					if (uwidth != 0.0f) 
					{
						float tx = gwidth / uwidth;
						if (tx > x)
							x = tx;
					}				
				}

				if (vang < (10.0f * 3.14f/180.0f))
				{
					float gheight = Length(gvec);
					float uheight = fabs(uvec.y);
				
					if (uheight != 0.0f) 
					{
						float ty = gheight / uheight;
						if (ty > y)
							y = ty;
					}				
				}

			}
		}
	}






	if ((x != 0.0f) && (y != 0.0f))
	{
		int newHeight = 0;
		int height,width;

	
		pblock->GetValue(unwrap_renderuv_width,0,width,FOREVER);
		pblock->GetValue(unwrap_renderuv_height,0,height,FOREVER);

		newHeight = (int)((float)width * (y/x));
		
		pblock->SetValue(unwrap_renderuv_height,0,newHeight);

	}

}

void UnwrapMod::fnRenderUVDialog()
{
	RenderUVDlgProc* paramMapDlgProc = new RenderUVDlgProc;
	paramMapDlgProc->mod = this;

	if (renderUVWindow == NULL)
	{
		renderUVMap = CreateModelessParamMap2(
			unwrap_renderuv_params, //param map ID constant
			pblock,
			0,
			hInstance,
			MAKEINTRESOURCE(IDD_UNWRAP_RENDERUVS_PARAMS),
			hWnd,
//			GetCOREInterface()->GetMAXHWnd(),
			paramMapDlgProc);
		
	}
}

void UnwrapMod::fnRenderUV()
{

	RenderUV(renderUVbi);
}


void UnwrapMod::fnRenderUV(TCHAR *name)
{

	renderUVbi.SetName(name);
	RenderUV(renderUVbi);
	
}

void UnwrapMod::BXPLine2(long x1,long y1,long x2,long y2,
					   WORD r, WORD g, WORD b, WORD alpha,
					   Bitmap *map, BOOL wrap)



{
  long i,px,py,x,y;
  long dx,dy,dxabs,dyabs,sdx,sdy;
  long count;

  BMM_Color_64 bit;

  bit.r = r;
  bit.g = g;
  bit.b = b;
  bit.a = alpha;

 dx = x2-x1;
 dy = y2-y1;

 if (dx >0)
	 sdx = 1;
	else
	 sdx = -1;

 if (dy >0)
	 sdy = 1;
	else
	 sdy = -1;


 dxabs = abs(dx);
 dyabs = abs(dy);

 x=0;
 y=0;
 px=x1;
 py=y1;

 count = 0;

 if (dxabs >= dyabs)
	 for (i=0; i<=dxabs; i++)
		{
		 y +=dyabs;
		 if (y>=dxabs)
			{
			 y-=dxabs;
			 py+=sdy;
			}
		if (wrap)
			{
			 if (px>=map->Width())
				{
				px = 0;
				}
			 if (px<0)
				{
				px = map->Width()-1;
				}
			 if (py>=map->Height())
				{
				py = 0;
				}
			 if (py<0)
				{
				px = map->Height()-1;
				}
			map->PutPixels(px,py,1,&bit);
			}
			else
			{
			 if ( (px>=0) && (px<map->Width()) &&
				  (py>=0) && (py<map->Height())    )
				{
				map->PutPixels(px,py,1,&bit);
				}

			}
		 px+=sdx;
		 }


		 else

	 for (i=0; i<=dyabs; i++)
		{
		 x+=dxabs;
		 if (x>=dyabs)
			{
			 x-=dyabs;
			 px+=sdx;
			}
		if (wrap)
			{
			 if (px>=map->Width())
				{
				px = 0;
				}
			 if (px<0)
				{
				px = map->Width()-1;
				}
			 if (py>=map->Height())
				{
				py = 0;
				}
			 if (py<0)
				{
				px = map->Height()-1;
				}
			map->PutPixels(px,py,1,&bit);
			}
			else
			{
			 if ( (px>=0) && (px<map->Width()) &&
				  (py>=0) && (py<map->Height())    )
				{
				map->PutPixels(px,py,1,&bit);
				}

			}

		 map->PutPixels(px,py,1,&bit);
		 py+=sdy;
		}
(count)--;
}


void UnwrapMod::BXPLineFloat(long x1,long y1,long x2,long y2,
					   WORD r, WORD g, WORD b, WORD alpha,
					   Bitmap *map/*w4, BOOL wrap*/)
{
	
	BMM_Color_64 bit;

	bit.r = r;
	bit.g = g;
	bit.b = b;
	bit.a = alpha;

	float xinc = 0.0f;
 	float yinc = 0.0f;
	
	float xDist = x2-x1;
	float yDist = y2-y1;
 	int width = map->Width();
	int height = map->Height();

	int ix,iy;

 	if (fabs(xDist) > fabs(yDist))
	{
		if (xDist >= 0.0f)
			xinc = 1.0f;
		else xinc = -1.0f;
		yinc = yDist/fabs(xDist);
		float fx,fy;
		
		fx = (int)x1;
		fy = (int)y1;
		int ct = fabs(xDist);
		for (int i = 0; i <= ct; i++)
		{
			
			ix = fx;
			iy = fy;

			if ((fx - floor(fx)) >= 0.5f) ix += 1;
			if ((fy - floor(fy)) >= 0.5f) iy += 1;
			


			if ( (ix >= 0) && (ix < width) && (iy >= 0) && (iy < height) )
				map->PutPixels(ix,iy,1,&bit);
			fx += xinc;
			fy += yinc;


		}

	}
	else
	{
		if(yDist >= 0.0f)
			yinc = 1.0f;
		else yinc = -1.0f;
		xinc = xDist/fabs(yDist);
		float fx,fy;
		
		fx = x1;
		fy = y1;
		int ct = fabs(yDist);
		for (int i = 0; i <= ct; i++)
		{
			
			ix = fx;
			iy = fy;

			if ((fx - floor(fx)) >= 0.5f) ix += 1;
			if ((fy - floor(fy)) >= 0.5f) iy += 1;

			if ( (ix >= 0) && (ix < width) && (iy >= 0) && (iy < height) )
				map->PutPixels(ix,iy,1,&bit);

			fx += xinc;
			fy += yinc;


		}
	}
}
BOOL UnwrapMod::BXPTriangleCheckOverlap( long x1,long y1,
					   long x2,long y2,
					   long x3,long y3,
					   Bitmap *map,/*w4, BOOL wrap,		*/
					   BitArray &processedPixels
)
{
	//sort top to bottom
	int sx[3], sy[3];
    sx[0] = x1;
	sy[0] = y1;

	if (y2 < sy[0])
	{
		sx[0] = x2;
		sy[0] = y2;

		sx[1] = x1;
		sy[1] = y1;
	}
	else 
	{
		sx[1] = x2;
		sy[1] = y2;
	}

	if (y3 < sy[0])
	{
		sx[2] = sx[1];
		sy[2] = sy[1];

		sx[1] = sx[0];
		sy[1] = sy[0];


		sx[0] = x3;
		sy[0] = y3;
	}
	else if (y3 < sy[1])
	{
		sx[2] = sx[1];
		sy[2] = sy[1];

		sx[1] = x3;
		sy[1] = y3;
	}
	else
	{
		sx[2] = x3;
		sy[2] = y3;
	}

	int hit = 0;

	int width = map->Width();
	int height = map->Height();
	//if flat top
	if (sy[0] == sy[1])
	{

		float xInc0 = 0.0f;
		float xInc1 = 0.0f;
		float yDist = DL_abs(sy[0] - sy[2]);
		float xDist0 = sx[2] - sx[0];
		float xDist1 = sx[2] - sx[1];
		xInc0 = xDist0/yDist;		
		xInc1 = xDist1/yDist;

		float cx0 = sx[0];
		float cx1 = sx[1];
		for (int i = sy[0]; i < sy[2]; i++)
		{
			
			int ix0 = cx0;
			int ix1 = cx1;

			if (cx0 - floor(cx0) >= 0.5f) ix0 += 1;
			if (cx1 - floor(cx1) >= 0.5f) ix1 += 1;
			

			//line 
			if (ix0 > ix1)
			{
				int temp = ix0;
				ix0 = ix1;
				ix1 = temp;
			}
			for (int j = ix0; j <= ix1; j++)
			{
					int index = j + i * width;
					if ((j >= 0) && (j <width) && (i >= 0) && (i < height))
					{

						if (processedPixels[index] && (j > (ix0+1)) && (j < (ix1-1)))
						{
							hit++;						
						}					
						processedPixels.Set(index,TRUE);
					}
							
			}
			cx0 += xInc0;
			cx1 += xInc1;
		}

	}
	//it flat bottom
 	else if (sy[1] == sy[2])
	{

		float xInc0 = 0.0f;
		float xInc1 = 0.0f;
		float yDist = DL_abs(sy[0] - sy[2]);
		float xDist0 = sx[1]- sx[0];
		float xDist1 = sx[2] - sx[0];
		xInc0 = xDist0/yDist;		
		xInc1 = xDist1/yDist;

		float cx0 = sx[0];
		float cx1 = sx[0];
		for (int i = sy[0]; i < sy[2]; i++)
		{
			
			int ix0 = cx0;
			int ix1 = cx1;

			if (cx0 - floor(cx0) >= 0.5f) ix0 += 1;
			if (cx1 - floor(cx1) >= 0.5f) ix1 += 1;

			//line 
			if (ix0 > ix1)
			{
				int temp = ix0;
				ix0 = ix1;
				ix1 = temp;
			}
			for (int j = ix0; j <= ix1; j++)
			{

					int index = j + i * width;
					if ((j >= 0) && (j <width) && (i >= 0) && (i < height))
					{
						if (processedPixels[index]&& (j > (ix0+1)) && (j < (ix1-1)))
						{
							hit++;
							
						}
						processedPixels.Set(index,TRUE);
					}


			}
			cx0 += xInc0;
			cx1 += xInc1;
		}		

	}
	else
	{

		
		float xInc1 = 0.0f;
		float xInc2 = 0.0f;
		float xInc3 = 0.0f;
		float yDist0to1 = DL_abs(sy[1] - sy[0]);
		float yDist0to2 = DL_abs(sy[2] - sy[0]);
		float yDist1to2 = DL_abs(sy[2] - sy[1]);

		float xDist1 = sx[1]- sx[0];
		float xDist2 = sx[2] - sx[0];
		float xDist3 = sx[2] - sx[1];
		xInc1 = xDist1/yDist0to1;		
		xInc2 = xDist2/yDist0to2;
		xInc3 = xDist3/yDist1to2;

		float cx0 = sx[0];
		float cx1 = sx[0];
		//go from s[0] to s[1]
		for (int i = sy[0]; i < sy[1]; i++)
		{
			
			int ix0 = cx0;
			int ix1 = cx1;

			if (cx0 - floor(cx0) >= 0.5f) ix0 += 1;
			if (cx1 - floor(cx1) >= 0.5f) ix1 += 1;

			//line 
			if (ix0 > ix1)
			{
				int temp = ix0;
				ix0 = ix1;
				ix1 = temp;
			}
			for (int j = ix0; j <= ix1; j++)
			{

					int index = j + i * width;
					if ((j >= 0) && (j <width) && (i >= 0) && (i < height))
					{
						if (processedPixels[index] &&  (j > (ix0+1)) && (j < (ix1-1)))
						{
							hit++;
							
						}
						processedPixels.Set(index,TRUE);
					}

			}
			cx0 += xInc1;
			cx1 += xInc2;
		}	
		//go from s[1] to s[2]
		for (int i = sy[1]; i < sy[2]; i++)
		{
			
			int ix0 = cx0;
			int ix1 = cx1;

			if (cx0 - floor(cx0) >= 0.5f) ix0 += 1;
			if (cx1 - floor(cx1) >= 0.5f) ix1 += 1;

			//line 
			if (ix0 > ix1)
			{
				int temp = ix0;
				ix0 = ix1;
				ix1 = temp;
			}
			for (int j = ix0; j <= ix1; j++)
			{

					int index = j + i * width;
					if ((j >= 0) && (j <width) && (i >= 0) && (i < height))
					{
						if (processedPixels[index] && (j > (ix0+1)) && (j < (ix1-1)))
						{
							hit++;
						}
						processedPixels.Set(index,TRUE);
					}

			}
			cx0 += xInc3;
			cx1 += xInc2;
		}	

	}
	if (hit > 1) return TRUE;
	else return FALSE;

}


void UnwrapMod::BXPTriangleFloat( long x1,long y1,
					   long x2,long y2,
					   long x3,long y3,
					   WORD r1, WORD g1, WORD b1, WORD alpha1,
					   Bitmap *map
					 					   )
{
	//sort top to bottom
	int sx[3], sy[3];
    sx[0] = x1;
	sy[0] = y1;

	if (y2 < sy[0])
	{
		sx[0] = x2;
		sy[0] = y2;

		sx[1] = x1;
		sy[1] = y1;
	}
	else 
	{
		sx[1] = x2;
		sy[1] = y2;
	}

	if (y3 < sy[0])
	{
		sx[2] = sx[1];
		sy[2] = sy[1];

		sx[1] = sx[0];
		sy[1] = sy[0];


		sx[0] = x3;
		sy[0] = y3;
	}
	else if (y3 < sy[1])
	{
		sx[2] = sx[1];
		sy[2] = sy[1];

		sx[1] = x3;
		sy[1] = y3;
	}
	else
	{
		sx[2] = x3;
		sy[2] = y3;
	}

	BMM_Color_64 bit;

	bit.r = r1;
	bit.g = g1;
	bit.b = b1;
	bit.a = alpha1;


	//if flat top
	if (sy[0] == sy[1])
	{

		float xInc0 = 0.0f;
		float xInc1 = 0.0f;
		float yDist = DL_abs(sy[0] - sy[2]);
		float xDist0 = sx[2] - sx[0];
		float xDist1 = sx[2] - sx[1];
		xInc0 = xDist0/yDist;		
		xInc1 = xDist1/yDist;

		float cx0 = sx[0];
		float cx1 = sx[1];
		for (int i = sy[0]; i < sy[2]; i++)
		{
			
			int ix0 = cx0;
			int ix1 = cx1;

			if (cx0 - floor(cx0) >= 0.5f) ix0 += 1;
			if (cx1 - floor(cx1) >= 0.5f) ix1 += 1;
			

			//line 
			if (ix0 > ix1)
			{
				int temp = ix0;
				ix0 = ix1;
				ix1 = temp;
			}
			for (int j = ix0; j <= ix1; j++)
			{
					
				map->PutPixels(j,i,1,&bit);
			}
			cx0 += xInc0;
			cx1 += xInc1;
		}

	}
	//it flat bottom
 	else if (sy[1] == sy[2])
	{

		float xInc0 = 0.0f;
		float xInc1 = 0.0f;
		float yDist = DL_abs(sy[0] - sy[2]);
		float xDist0 = sx[1]- sx[0];
		float xDist1 = sx[2] - sx[0];
		xInc0 = xDist0/yDist;		
		xInc1 = xDist1/yDist;

		float cx0 = sx[0];
		float cx1 = sx[0];
		for (int i = sy[0]; i < sy[2]; i++)
		{
			
			int ix0 = cx0;
			int ix1 = cx1;

			if (cx0 - floor(cx0) >= 0.5f) ix0 += 1;
			if (cx1 - floor(cx1) >= 0.5f) ix1 += 1;

			//line 
			if (ix0 > ix1)
			{
				int temp = ix0;
				ix0 = ix1;
				ix1 = temp;
			}
			for (int j = ix0; j <= ix1; j++)
			{
				map->PutPixels(j,i,1,&bit);
			}
			cx0 += xInc0;
			cx1 += xInc1;
		}		

	}
	else
	{

		
		float xInc1 = 0.0f;
		float xInc2 = 0.0f;
		float xInc3 = 0.0f;
		float yDist0to1 = DL_abs(sy[1] - sy[0]);
		float yDist0to2 = DL_abs(sy[2] - sy[0]);
		float yDist1to2 = DL_abs(sy[2] - sy[1]);

		float xDist1 = sx[1]- sx[0];
		float xDist2 = sx[2] - sx[0];
		float xDist3 = sx[2] - sx[1];
		xInc1 = xDist1/yDist0to1;		
		xInc2 = xDist2/yDist0to2;
		xInc3 = xDist3/yDist1to2;

		float cx0 = sx[0];
		float cx1 = sx[0];
		//go from s[0] to s[1]
		for (int i = sy[0]; i < sy[1]; i++)
		{
			
			int ix0 = cx0;
			int ix1 = cx1;

			if (cx0 - floor(cx0) >= 0.5f) ix0 += 1;
			if (cx1 - floor(cx1) >= 0.5f) ix1 += 1;

			//line 
			if (ix0 > ix1)
			{
				int temp = ix0;
				ix0 = ix1;
				ix1 = temp;
			}
			for (int j = ix0; j <= ix1; j++)
			{
				map->PutPixels(j,i,1,&bit);
			}
			cx0 += xInc1;
			cx1 += xInc2;
		}	
		//go from s[1] to s[2]
		for (int i = sy[1]; i < sy[2]; i++)
		{
			
			int ix0 = cx0;
			int ix1 = cx1;

			if (cx0 - floor(cx0) >= 0.5f) ix0 += 1;
			if (cx1 - floor(cx1) >= 0.5f) ix1 += 1;

			//line 
			if (ix0 > ix1)
			{
				int temp = ix0;
				ix0 = ix1;
				ix1 = temp;
			}
			for (int j = ix0; j <= ix1; j++)
			{
				map->PutPixels(j,i,1,&bit);
			}
			cx0 += xInc3;
			cx1 += xInc2;
		}	

	}

}



void UnwrapMod::BXPTriangleFloat( 
					   BXPInterpData c0, BXPInterpData c1, BXPInterpData c2,
					   WORD alpha,
					   Bitmap *map, 
					   Tab<Point3> &norms,
					   Tab<Point3> &pos
					   )
{
	//sort top to bottom
   	BXPInterpData c[3];

	c[0] = c0;
	

	if (c1.y < c[0].y)
	{
		c[0] = c1;
		c[1] = c0;
	}
	else 
	{
		c[1] =  c1;
	}

	if (c2.y < c[0].y)
	{
		c[2] = c[1];
		c[1] = c[0];
		c[0] = c2;
	}
	else if (c2.y < c[1].y)
	{
		c[2] = c[1];
		c[1] = c2;
	}
	else
	{
		c[2] = c2;
	}

	BMM_Color_64 bit;


	bit.a = alpha;
	int width = map->Width();
	int height = map->Height();

	//if flat top
	if (c[0].IntY() == c[1].IntY())
	{

		BXPInterpData xInc0;
		BXPInterpData xInc1;
		
		float yDist = fabs(c[0].y - c[2].y);


		xInc0 = c[2] - c[0];
		xInc1 = c[2] - c[1];

		xInc0 = xInc0/yDist;
		xInc1 = xInc1/yDist;
		

		BXPInterpData cx0,cx1;
		cx0 = c[0];
		cx1 = c[1];


		int sy,ey;
		sy = c[0].IntY();
		ey = c[2].IntY();
		for (int i = sy; i < ey; i++)
		{
			
			BXPInterpData ix0 = cx0;
			BXPInterpData ix1 = cx1;
			
			//line 
			if (ix0.IntX() > ix1.IntX())
			{
				BXPInterpData temp = ix0;
				ix0 = ix1;
				ix1 = temp;
			}
			int sx,ex;
			sx = ix0.IntX();
			ex = ix1.IntX();

			BXPInterpData subInc = ix1 -  ix0;
			subInc = subInc/(ix1.IntX()-ix0.IntX());
			BXPInterpData newc = ix0;

			
			for (int j = sx; j < ex; j++)
			{	
				
				bit.r = (int)newc.color.x;
				bit.g = (int)newc.color.y;
				bit.b = (int)newc.color.z;


				if ((j >= 0) && (j < width) && (i >= 0) && (i < height))
				{
					int index = i*width+j;
					if (norms[index] == Point3(0.0f,0.0f,0.0f))
					{
						map->PutPixels(j,i,1,&bit);
						norms[index] = newc.normal;
						pos[index] = newc.pos;
					}
					else
					{
						Point3 currentP = pos[index];
						Point3 newP = newc.pos;
						Point3 currentNorm = norms[index];
						Point3 newNorm = newc.normal;

						Point3 averageNormal = (currentNorm + newNorm) *0.5f;
						Matrix3 tm(1);
						MatrixFromNormal(averageNormal, tm);
						tm = Inverse(tm);
						currentP = currentP * tm;
						newP = newP * tm;
						if (newP.z > currentP.z)
						{
							map->PutPixels(j,i,1,&bit);
							norms[index] = newc.normal;
							pos[index] = newc.pos;
						}					
					}
				}

				newc = newc + subInc;
			}
			cx0 = cx0 + xInc0;
			cx1 = cx1 + xInc1;


		}

	}

	//it flat bottom
 	else if (c[1].IntY() == c[2].IntY())
	{

		BXPInterpData xInc0;
		BXPInterpData xInc1;

		float yDist = fabs(c[0].y - c[2].y);
		
		xInc0 = c[1] - c[0];
		xInc1 = c[2] - c[0];
		xInc0 = xInc0 / yDist;
		xInc1 = xInc1 / yDist;


		BXPInterpData cx0,cx1;
		cx0 = c[0];
		cx1 = c[0];


		int sy,ey;
		sy = c[0].IntY();
		ey = c[2].IntY();

		for (int i = sy; i < ey; i++)
		{
			
			BXPInterpData ix0 = cx0;
			BXPInterpData ix1 = cx1;


			//line 
			if (ix0.IntX() > ix1.IntX())
			{
				BXPInterpData temp = ix0;
				ix0 = ix1;
				ix1 = temp;

			}

			BXPInterpData subInc = (ix1-ix0)/(float)(ix1.IntX()-ix0.IntX());
			BXPInterpData newc = ix0;


			int sx,ex;
			sx = ix0.IntX();
			ex = ix1.IntX();
			for (int j = sx; j < ex; j++)
			{

				bit.r = (int)newc.color.x;
				bit.g = (int)newc.color.y;
				bit.b = (int)newc.color.z;

				if ((j >= 0) && (j < width) && (i >= 0) && (i < height))
				{
					int index = i*width+j;
					if (norms[index] == Point3(0.0f,0.0f,0.0f))
					{
						map->PutPixels(j,i,1,&bit);
						norms[index] = newc.normal;
						pos[index] = newc.pos;
					}
					else
					{
						Point3 currentP = pos[index];
						Point3 newP = newc.pos;
						Point3 currentNorm = norms[index];
						Point3 newNorm = newc.normal;

						Point3 averageNormal = (currentNorm + newNorm) *0.5f;
						Matrix3 tm(1);
						MatrixFromNormal(averageNormal, tm);
						tm = Inverse(tm);
						currentP = currentP * tm;
						newP = newP * tm;
						if (newP.z > currentP.z)
						{
							map->PutPixels(j,i,1,&bit);
							norms[index] = newc.normal;
							pos[index] = newc.pos;
						}					
					}
				}

				newc = newc + subInc;
			}
			cx0 = cx0 + xInc0;
			cx1 = cx1 + xInc1;

		}		

	}

	else
	{

		
 		BXPInterpData xInc1;
		BXPInterpData xInc2;
 		BXPInterpData xInc3;
		float yDist0to1 = fabs(c[1].y - c[0].y);
		float yDist0to2 = fabs(c[2].y - c[0].y);
		float yDist1to2 = fabs(c[2].y - c[1].y);

		xInc1 = (c[1]- c[0])/yDist0to1;		
		xInc2 = (c[2] - c[0])/yDist0to2;
		xInc3 = (c[2] - c[1])/yDist1to2;


		BXPInterpData cx0 = c[0];
		BXPInterpData cx1 = c[0];


		//go from s[0] to s[1]
		int sy,ey;
		sy = c[0].IntY();
		ey = c[1].IntY();
		for (int i = sy; i < ey; i++)
		{
			
			BXPInterpData ix0 = cx0;
			BXPInterpData ix1 = cx1;


			//line 
			if (ix0.IntX() > ix1.IntX())
			{
				BXPInterpData temp = ix0;
				ix0 = ix1;
				ix1 = temp;

			}

			BXPInterpData subInc = (ix1-ix0)/(float)(ix1.IntX()-ix0.IntX());
			BXPInterpData newc = ix0;


			int sx,ex;
			sx = ix0.IntX();
			ex = ix1.IntX();
			
			for (int j = sx; j <= ex; j++)
			{
				bit.r = (int)newc.color.x;
				bit.g = (int)newc.color.y;
				bit.b = (int)newc.color.z;

				if ((j >= 0) && (j < width) && (i >= 0) && (i < height))
				{				
					int index = i*width+j;
					if (norms[index] == Point3(0.0f,0.0f,0.0f))
					{
						map->PutPixels(j,i,1,&bit);
						norms[index] = newc.normal;
						pos[index] = newc.pos;
					}
					else
					{
						Point3 currentP = pos[index];
						Point3 newP = newc.pos;
						Point3 currentNorm = norms[index];
						Point3 newNorm = newc.normal;

						Point3 averageNormal = (currentNorm + newNorm) *0.5f;
						Matrix3 tm(1);
						MatrixFromNormal(averageNormal, tm);
						tm = Inverse(tm);
						currentP = currentP * tm;
						newP = newP * tm;
						if (newP.z > currentP.z)
						{
							map->PutPixels(j,i,1,&bit);
							norms[index] = newc.normal;
							pos[index] = newc.pos;
						}					
					}
				}

				newc = newc + subInc;
			}
			cx0 = cx0 + xInc1;
			cx1 = cx1 + xInc2;

		}	
		//go from s[1] to s[2]
		sy = c[1].IntY();
		ey = c[2].IntY();
		for (int i = sy; i < ey; i++)
		{
			
			BXPInterpData ix0 = cx0;
			BXPInterpData ix1 = cx1;


			//line 
			if (ix0.IntX() > ix1.IntX())
			{
				BXPInterpData temp = ix0;
				ix0 = ix1;
				ix1 = temp;

			}

			BXPInterpData subInc = (ix1-ix0)/(float)(ix1.IntX()-ix0.IntX());
			BXPInterpData newc = ix0;


			int sx,ex;
			sx = ix0.IntX();
			ex = ix1.IntX();

			for (int j = sx; j <= ex; j++)
			{

				bit.r = (int)newc.color.x;
				bit.g = (int)newc.color.y;
				bit.b = (int)newc.color.z;
				if ((j >= 0) && (j < width) && (i >= 0) && (i < height))
				{
					int index = i*width+j;
					if (norms[index] == Point3(0.0f,0.0f,0.0f))
					{
						map->PutPixels(j,i,1,&bit);
						norms[index] = newc.normal;
						pos[index] = newc.pos;
					}
					else
					{
						Point3 currentP = pos[index];
						Point3 newP = newc.pos;
						Point3 currentNorm = norms[index];
						Point3 newNorm = newc.normal;

						Point3 averageNormal = (currentNorm + newNorm) *0.5f;
						Matrix3 tm(1);
						MatrixFromNormal(averageNormal, tm);
						tm = Inverse(tm);
						currentP = currentP * tm;
						newP = newP * tm;
						if (newP.z > currentP.z)
						{
							map->PutPixels(j,i,1,&bit);
							norms[index] = newc.normal;
							pos[index] = newc.pos;
						}					
					}
				}

				newc = newc + subInc;
			}
			cx0 = cx0 + xInc3;
			cx1 = cx1 + xInc2;

		}	
	}

}

void UnwrapMod::RenderUV(BitmapInfo bi)
{

	//get the width/height
	int width, height;
	pblock->GetValue(unwrap_renderuv_width,0,width,FOREVER);
	pblock->GetValue(unwrap_renderuv_height,0,height,FOREVER);
	if (height <= 0) width = 512;
	if (height <= 0) height = 512;

	float fEdgeAlpha;
	pblock->GetValue(unwrap_renderuv_edgealpha,0,fEdgeAlpha,FOREVER);
	if (fEdgeAlpha < 0.0f) fEdgeAlpha = 0.0f;
	if (fEdgeAlpha > 1.0f) fEdgeAlpha = 1.0f;


	Color edgeColor;
	pblock->GetValue(unwrap_renderuv_edgecolor,0,edgeColor,FOREVER);
	edgeColor.ClampMinMax();

	Color seamColor;
	pblock->GetValue(unwrap_renderuv_seamcolor,0,seamColor,FOREVER);
	seamColor.ClampMinMax();

	BOOL drawSeams, drawHiddenEdge, drawEdges;

	pblock->GetValue(unwrap_renderuv_visible,0,drawEdges,FOREVER);
	pblock->GetValue(unwrap_renderuv_invisible,0,drawHiddenEdge,FOREVER);
	pblock->GetValue(unwrap_renderuv_seamedges,0,drawSeams,FOREVER);

	BOOL showFrameBuffer;
	pblock->GetValue(unwrap_renderuv_showframebuffer,0,showFrameBuffer,FOREVER);


	
	BOOL force2Sided;
	pblock->GetValue(unwrap_renderuv_force2sided,0,force2Sided,FOREVER);

	BOOL backFaceCull = !force2Sided;

	int fillMode;
	pblock->GetValue(unwrap_renderuv_fillmode,0,fillMode,FOREVER);
	if (fillMode < 0) fillMode = 0;
	if (fillMode > 3) fillMode = 3;

	Color fillColor;
	pblock->GetValue(unwrap_renderuv_fillcolor,0,fillColor,FOREVER);
	fillColor.ClampMinMax();

	float fFillAlpha;
	pblock->GetValue(unwrap_renderuv_fillalpha,0,fFillAlpha,FOREVER);
	if (fFillAlpha < 0.0f) fFillAlpha = 0.0f;
	if (fFillAlpha > 1.0f) fFillAlpha = 1.0f;


	Color overlapColor;
	pblock->GetValue(unwrap_renderuv_overlapcolor,0,overlapColor,FOREVER);
	overlapColor.ClampMinMax();

	BOOL overlap;
	pblock->GetValue(unwrap_renderuv_overlap,0,overlap,FOREVER);



	int edgeAlpha = int(0xFFFF * fEdgeAlpha);
	int fillAlpha = int(0xFFFF * fFillAlpha);

	bi.SetWidth(width);
	bi.SetFlags(MAP_HAS_ALPHA);
	bi.SetHeight(height);
	bi.SetType(BMM_TRUE_64);

	int edgeColorR,edgeColorG,edgeColorB;
	edgeColorR = int(0xFFFF * edgeColor.r);
	edgeColorG = int(0xFFFF * edgeColor.g);
	edgeColorB = int(0xFFFF * edgeColor.b);

	int seamColorR,seamColorG,seamColorB;
	seamColorR = int(0xFFFF * seamColor.r);
	seamColorG = int(0xFFFF * seamColor.g);
	seamColorB = int(0xFFFF * seamColor.b);

	int fillColorR,fillColorG,fillColorB;
	fillColorR = int(0xFFFF * fillColor.r);
	fillColorG = int(0xFFFF * fillColor.g);
	fillColorB = int(0xFFFF * fillColor.b);

	int overlapColorR,overlapColorG,overlapColorB;
	overlapColorR = int(0xFFFF * overlapColor.r);
	overlapColorG = int(0xFFFF * overlapColor.g);
	overlapColorB = int(0xFFFF * overlapColor.b);

	

	Bitmap *map;
 	map = TheManager->Create(&bi);
	if (map == NULL) 
	{
		if (ip)
			ip->ReplacePrompt(GetString(IDS_RENDERMAP_FAILED) );
		return;
	}
	map->Fill(0,0,0,0);


	//draw our edges

	BitArray processedPixels;
	processedPixels.SetSize(map->Width()*map->Height());
	processedPixels.ClearAll();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		BitArray facingFaces;
		facingFaces.SetSize(ld->GetNumberFaces());//TVMaps.f.Count());

		facingFaces.SetAll();

		Tab<Point3> geomVertexNormals;
		Tab<Point3> geomFaceNormals;
		Tab<Point3> norms;
		Tab<Point3> pos;

		
		geomFaceNormals.SetCount(ld->GetNumberFaces());
		for (int i =0; i < ld->GetNumberFaces(); i++)
		{
			geomFaceNormals[i] = ld->GetGeomFaceNormal(i);
			Point3 uvwNormal = ld->GetUVFaceNormal(i);
			if (uvwNormal.z < 0.0f) 
				facingFaces.Set(i,FALSE);			
		}


		Tab<int> geomVertexNormalsCT;
		geomVertexNormals.SetCount(ld->GetNumberGeomVerts());//TVMaps.geomPoints.Count());
		geomVertexNormalsCT.SetCount(ld->GetNumberGeomVerts());//(TVMaps.geomPoints.Count());

		for (int i =0; i < ld->GetNumberGeomVerts(); i++)
		{
			geomVertexNormals[i] = Point3(0.0f,0.0f,0.0f);
			geomVertexNormalsCT[i] = 0;
		}
		for (int i =0; i < ld->GetNumberFaces(); i++)
		{
			int deg = ld->GetFaceDegree(i);//TVMaps.f[i]->count;
			for (int j = 0; j < deg; j++)
			{
				int index = ld->GetFaceGeomVert(i,j);//TVMaps.f[i]->v[j];
				geomVertexNormalsCT[index] = geomVertexNormalsCT[index] + 1;
				geomVertexNormals[index] += geomFaceNormals[i];
			}
		}

		for (int i =0; i < ld->GetNumberGeomVerts(); i++)
		{
			if (geomVertexNormalsCT[i] != 0)
				geomVertexNormals[i] = geomVertexNormals[i]/(float)geomVertexNormalsCT[i];
			geomVertexNormals[i] = Normalize(geomVertexNormals[i]);
		}

		if (fillMode >= 2)
		{
			try
			{
				norms.SetCount(map->Width()*map->Height());
			}
			catch( std::bad_alloc&)
			{
				if (ip)
					ip->DisplayTempPrompt(GetString(IDS_RENDERMAP_FAILED),20000 );
				map->DeleteThis();
				return;
			};


			for (int i = 0; i < norms.Count(); i++)
				norms[i] = Point3(0.0f,0.0f,0.0f);
			try
			{
				pos = norms;
			}
			catch( std::bad_alloc&)
			{
				if (ip)
					ip->DisplayTempPrompt(GetString(IDS_RENDERMAP_FAILED),20000 );
				map->DeleteThis();
				return;
			};
		}



		if (fillMode > 0)
		{
			Tab<int> minX;
			Tab<int> maxX;


			int width,height;
  			width = map->Width();
			height = map->Height();
			minX.SetCount(height);
			maxX.SetCount(height);
			
			
			Box3 bounds;
			bounds.Init();
			for (int i = 0; i < ld->GetNumberGeomVerts(); i++)
				bounds += ld->GetGeomVert(i);//TVMaps.geomPoints[i];
			
			bounds.Scale(4.0f);

			BitArray reCheckTheseFaces;
			reCheckTheseFaces.SetSize(ld->GetNumberFaces());//TVMaps.f.Count());
			reCheckTheseFaces.ClearAll();

	 		for (int i = 0; i < ld->GetNumberFaces(); i++)
			{
				BOOL drawFace = TRUE;
				if (backFaceCull)
				{
					drawFace = facingFaces[i];
				}
				if (ld->GetFaceDegree(i) < 3)
					drawFace = FALSE;
				if (drawFace  && (!ld->GetFaceHasVectors(i)))//TVMaps.f[i]->vecs == NULL))
				{
					int deg = ld->GetFaceDegree(i);//TVMaps.f[i]->count;
					int numberOfTris = deg-2;
					
					for (int j = 0; j < height; j++)
					{
						minX[j] = width+10;
						maxX[j] = -10;
					}

					
					int x0, y0;
					int id = ld->GetFaceTVVert(i,0);//TVMaps.f[i]->t[0];
					Point3 Vert0 = ld->GetTVVert(id);//TVMaps.v[id].p;
					x0 = (map->Width()*Vert0.x);
					y0 = map->Height()-1-(long) ((map->Height()-1)*Vert0.y);

					for (int j = 0; j < numberOfTris; j++)
					{
						int index[4];
						index[0] = ld->GetFaceTVVert(i,0);//TVMaps.f[i]->t[0];
						index[1] = ld->GetFaceTVVert(i,j+1);//TVMaps.f[i]->t[j + 1];
						index[2] = ld->GetFaceTVVert(i,j+2);//TVMaps.f[i]->t[j + 2]; 
						
		
						if (ld->IsTVVertVisible(index[0]) && ld->IsTVVertVisible(index[1]) && ld->IsTVVertVisible(index[2]))
						{
							int x1,y1,x2,y2;
							int id =index[1];
							Point3 VertA = ld->GetTVVert(id);//TVMaps.v[id].p;
							id = index[2];
							Point3 VertB = ld->GetTVVert(id);//TVMaps.v[id].p;

							x1 = (map->Width()*VertA.x);
							y1 = map->Height()-1-(long) ((map->Height()-1)*VertA.y);

							x2 = (map->Width()*VertB.x);
							y2 = map->Height()-1-(long) ((map->Height()-1)*VertB.y);

							if (fillMode == 1)
								if (overlap)
								{
									BOOL hit = BXPTriangleCheckOverlap(x0,y0,
													x1,y1,
													x2,y2,												
													map, 
													processedPixels
													);
									if (hit)
									{
										BXPTriangleFloat(	x0,y0,
													x1,y1,
													x2,y2,
													overlapColorR,overlapColorG,overlapColorB,fillAlpha,
													map
													);
										reCheckTheseFaces.Set(i,TRUE);
									}
									else BXPTriangleFloat(	x0,y0,
													x1,y1,
													x2,y2,
													fillColorR,fillColorG,fillColorB,fillAlpha,
													map
													);


								}

								else BXPTriangleFloat(	x0,y0,
													x1,y1,
													x2,y2,
													fillColorR,fillColorG,fillColorB,fillAlpha,
													map
													);
							else if (fillMode >= 2)
							{
 								Point3 n1,n2,n3;
								int a = ld->GetFaceGeomVert(i,0);//TVMaps.f[i]->v[0];
								int b = ld->GetFaceGeomVert(i,j+1);//TVMaps.f[i]->v[j + 1];
								int c = ld->GetFaceGeomVert(i,j+2);//TVMaps.f[i]->v[j + 2]; 

								n1 = geomVertexNormals[a] ;
								n2 = geomVertexNormals[b] ;
								n3 = geomVertexNormals[c] ;

								if (!facingFaces[i])							
								{
									n1 *= -1.0f;
									n2 *= -1.0f;
									n3 *= -1.0f;
								}


								BXPInterpData c0,c1,c2;
								c0.normal = n1;
								c1.normal = n2;
								c2.normal = n3;

								c0.pos = ld->GetGeomVert(a);//TVMaps.geomPoints[a];
								c1.pos = ld->GetGeomVert(b);//TVMaps.geomPoints[b];
								c2.pos = ld->GetGeomVert(c);//TVMaps.geomPoints[c];

								c0.x = x0;
								c0.y = y0;

								c1.x = x1;
								c1.y = y1;

								c2.x = x2;
								c2.y = y2;
								BOOL hit = FALSE;
								if (overlap)
								{
									hit = BXPTriangleCheckOverlap(x0,y0,
													x1,y1,
													x2,y2,												
													map, 
													processedPixels
													);
								}
								if (hit)
								{

									BXPTriangleFloat(	x0,y0,
													x1,y1,
													x2,y2,
													overlapColorR,overlapColorG,overlapColorB,fillAlpha,
													map
													);
									reCheckTheseFaces.Set(i,TRUE);
								}
								else
								{

									if (fillMode == 2)
									{
										n1 = n1 + 1.0f;
										n1 *= 0.5f;
										n1 *= 0xfff0;


										n2 = n2 + 1.0f;
										n2 *= 0.5f;
										n2 *= 0xfff0;

										n3 = n3 + 1.0f;
										n3 *= 0.5f;
										n3 *= 0xfff0;

																
										

										c0.color = n1;


										c1.color = n2;
										

										c2.color = n3;
									}
									else if (fillMode == 3)
									{
 										Point3 zPos(0.0f,0.0f,1.0f);
										Point3 zNeg(0.0f,0.0f,-1.0f);

										

 										Point3 colorPos(1.0f,1.0f,1.0f);
										Point3 colorNeg(0.5f,0.5f,1.0f);
										colorPos = fillColor;
										colorNeg = colorPos;
										colorNeg.x *= 0.5f;
										colorNeg.y *= 0.5f;

										zPos = c0.pos - bounds.pmax;
										zPos = Normalize(zPos);

										zNeg = c0.pos - bounds.pmin;
										zNeg = Normalize(zNeg);


										float dot = DotProd(zPos,n1);
										if (dot > 0.0f)
											c0.color += colorPos * dot;
										dot = DotProd(zNeg,n1);
										if (dot > 0.0f)
											c0.color += colorNeg * dot;

										zPos = c1.pos - bounds.pmax;
										zPos = Normalize(zPos);

										zNeg = c1.pos - bounds.pmin;
										zNeg = Normalize(zNeg);


										dot = DotProd(zPos,n2);
										if (dot > 0.0f)
											c1.color += colorPos * dot;
										dot = DotProd(zNeg,n2);
										if (dot > 0.0f)
											c1.color += colorNeg * dot;

										zPos = c2.pos - bounds.pmax;
										zPos = Normalize(zPos);

										zNeg = c2.pos - bounds.pmin;
										zNeg = Normalize(zNeg);


										dot = DotProd(zPos,n3);
										if (dot > 0.0f)
											c2.color += colorPos * dot;
										dot = DotProd(zNeg,n3);
										if (dot > 0.0f)
											c2.color += colorNeg * dot;

										c0.color *= 0xf000;
										c1.color *= 0xf000;
										c2.color *= 0xf000;

									}


									BXPTriangleFloat(	c0,c1,c2,
														fillAlpha,
														map, 
														norms,
														pos);
								}

							}
						}

					}
				}
			}		
 			if (overlap && reCheckTheseFaces.NumberSet() > 0)
			{
				processedPixels.ClearAll();
	 			for (int i = (ld->GetNumberFaces()-1); i >= 0 ; i--)
				{
					BOOL drawFace = TRUE;
					if (backFaceCull)
					{
						drawFace = facingFaces[i];
					}

					if (ld->GetFaceDegree(i) < 3)
						drawFace = FALSE;

					if (drawFace  && (!ld->GetFaceHasVectors(i)))
					{
						int deg = ld->GetFaceDegree(i);//TVMaps.f[i]->count;
						int numberOfTris = deg-2;
						
						for (int j = 0; j < height; j++)
						{
							minX[j] = width+10;
							maxX[j] = -10;
						}

						
						int x0, y0;
						int id = ld->GetFaceTVVert(i,0);//TVMaps.f[i]->t[0];
						Point3 Vert0 = ld->GetTVVert(id);//TVMaps.v[id].p;
						x0 = (map->Width()*Vert0.x);
						y0 = map->Height()-1-(long) ((map->Height()-1)*Vert0.y);

						for (int j = 0; j < numberOfTris; j++)
						{
							int index[4];
							index[0] = ld->GetFaceTVVert(i,0);//TVMaps.f[i]->t[0];
							index[1] = ld->GetFaceTVVert(i,j+1);//TVMaps.f[i]->t[j + 1];
							index[2] = ld->GetFaceTVVert(i,j+2);//TVMaps.f[i]->t[j + 2]; 
							
			
							if (ld->IsTVVertVisible(index[0]) && ld->IsTVVertVisible(index[1]) && ld->IsTVVertVisible(index[2]))
							{
								int x1,y1,x2,y2;
								int id =index[1];
								Point3 VertA = ld->GetTVVert(id);//TVMaps.v[id].p;
								id = index[2];
								Point3 VertB = ld->GetTVVert(id);//TVMaps.v[id].p;

								x1 = (map->Width()*VertA.x);
								y1 = map->Height()-1-(long) ((map->Height()-1)*VertA.y);

								x2 = (map->Width()*VertB.x);
								y2 = map->Height()-1-(long) ((map->Height()-1)*VertB.y);

								if (fillMode == 1)
									if (overlap)
									{
										BOOL hit = BXPTriangleCheckOverlap(x0,y0,
														x1,y1,
														x2,y2,												
														map,
														processedPixels
														);
										if (hit)
										{
											BXPTriangleFloat(	x0,y0,
														x1,y1,
														x2,y2,
														overlapColorR,overlapColorG,overlapColorB,fillAlpha,
														map
														);
											
										}


									}
								else if (fillMode >= 2)
								{

									BOOL hit = FALSE;
									if (overlap)
									{
										hit = BXPTriangleCheckOverlap(x0,y0,
														x1,y1,
														x2,y2,												
														map, 
														processedPixels
														);
									}
									if (hit)
									{

										BXPTriangleFloat(	x0,y0,
														x1,y1,
														x2,y2,
														overlapColorR,overlapColorG,overlapColorB,fillAlpha,
														map
														);
										reCheckTheseFaces.Set(i,TRUE);
									}

								}
							}

						}
					}
				}
			}
		}
	

		for (int i = 0; i < ld->GetNumberTVEdges(); i++)//TVMaps.ePtrList.Count(); i++)
		{
			int a,b;
			a = ld->GetTVEdgeVert(i,0);//TVMaps.ePtrList[i]->a;
			b = ld->GetTVEdgeVert(i,1);//TVMaps.ePtrList[i]->b;
			int veca,vecb;

			veca = ld->GetTVEdgeVec(i,0);//TVMaps.ePtrList[i]->avec;
			vecb = ld->GetTVEdgeVec(i,1);//TVMaps.ePtrList[i]->bvec;
			if (ld->IsTVVertVisible(a) && ld->IsTVVertVisible(b))
			{
				Point3 VertA = ld->GetTVVert(a);//TVMaps.v[a].p;
				Point3 VertB = ld->GetTVVert(b);//TVMaps.v[b].p;


				int cr,cg,cb;
				cr = edgeColorR;
				cg = edgeColorG;
				cb = edgeColorB;

				BOOL edgeHidden = ld->GetTVEdgeHidden(i);//TVMaps.ePtrList[i]->flags & FLAG_HIDDENEDGEA;

				BOOL draw = TRUE;
				if (!drawHiddenEdge)
				{
					if (edgeHidden)
						draw = FALSE;
				}

				if ((backFaceCull) && (draw))
				{
					int ct = ld->GetTVEdgeNumberTVFaces(i);//TVMaps.ePtrList[i]->faceList.Count();
					int facing = 0;
					for (int j = 0; j < ct; j++)
					{
						int faceIndex = ld->GetTVEdgeConnectedTVFace(i,j);//TVMaps.ePtrList[i]->faceList[j];
						if (facingFaces[faceIndex])
							facing++;
					}
					if (facing == 0)
						draw = FALSE;
				}
				if ((draw) && ((veca != -1) && (vecb != -1)))
				{

					if ((ld->GetTVEdgeNumberTVFaces(i)/*TVMaps.ePtrList[i]->faceList.Count()*/ == 1) && drawSeams)
					{
						cr = seamColorR;
						cg = seamColorG;
						cb = seamColorB;

					}

					Point3 pvecA = ld->GetTVVert(veca);//TVMaps.v[veca].p;
					Point3 pvecB = ld->GetTVVert(vecb);//TVMaps.v[vecb].p;
					Point3 pa = ld->GetTVVert(a);//TVMaps.v[a].p;
					Point3 pb = ld->GetTVVert(b);//TVMaps.v[b].p;

 					Point2 fpa,fpb,fpvecA,fpvecB;

					//		pvecA = pva;
					//		pvecB = pvb;

					fpa.x = (float)pa.x;
					fpa.y = (float)pa.y;

					fpb.x = (float)pb.x;
					fpb.y = (float)pb.y;

					fpvecA.x = (float)pvecA.x;
					fpvecA.y = (float)pvecA.y;

					fpvecB.x = (float)pvecB.x;
					fpvecB.y = (float)pvecB.y;

					VertA.x = pa.x;
					VertA.y = pa.y;


	//				MoveToEx(hdc,pa.x, pa.y, NULL);
 					for (int j = 1; j < 7; j++)
					{
						float t = float (j)/7.0f;

						float s = (float)1.0-t;
						float t2 = t*t;
						Point2 p = ( ( s*fpa + (3.0f*t)*fpvecA)*s + (3.0f*t2)* fpvecB)*s + t*t2*fpb;
						
						VertB.x = p.x;
						VertB.y = p.y;
	//					LineTo(hdc,(int)p.x, (int)p.y);
						int x1,y1,x2,y2;
						x1 = (map->Width()*VertA.x);
						y1 = map->Height()-1-(long) ((map->Height()-1)*VertA.y);

						x2 = (map->Width()*VertB.x);
						y2 = map->Height()-1-(long) ((map->Height()-1)*VertB.y);



						BXPLineFloat(x1,y1,x2,y2,									
										cr, cg, cb,edgeAlpha,
										map);
						VertA.x = VertB.x;
						VertA.y = VertB.y;

					}

					int x1,y1,x2,y2;
					x1 = (map->Width()*VertA.x);
					y1 = map->Height()-1-(long) ((map->Height()-1)*VertA.y);

					x2 = (map->Width()*pb.x);
					y2 = map->Height()-1-(long) ((map->Height()-1)*pb.y);


					BXPLineFloat(x1,y1,x2,y2,									
									cr, cg, cb,edgeAlpha,
									map);

	//				LineTo(hdc,pb.x, pb.y);


				}

				else if ((draw) && ((veca == -1) || (vecb == -1)))
				{

					int x1,y1,x2,y2;
					x1 = (map->Width()*VertA.x);
					y1 = map->Height()-1-(long) ((map->Height()-1)*VertA.y);

					x2 = (map->Width()*VertB.x);
					y2 = map->Height()-1-(long) ((map->Height()-1)*VertB.y);

					if ((ld->GetTVEdgeNumberTVFaces(i) == 1) && drawSeams)
					{
						cr = seamColorR;
						cg = seamColorG;
						cb = seamColorB;
						BXPLineFloat(x1,y1,x2,y2,									
									cr, cg, cb,edgeAlpha,
									map);
					}
					else if (drawEdges)
					{
						BXPLineFloat(x1,y1,x2,y2,
								cr, cg, cb,edgeAlpha,
								map);
					}
				}
			}

		}
	}

	

 	map->OpenOutput(&bi);
	map->Write(&bi);
	map->Close(&bi);

	if (showFrameBuffer)
	{
		TSTR title;
		title.printf("%s",GetString(IDS_RENDERMAP));
		map->Display(title,BMM_CN,TRUE);
	}
	else map->DeleteThis();

}
