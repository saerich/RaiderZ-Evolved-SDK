#include "mods.h"
#include "iparamm.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "linshape.h"
#include "IBrushPresets.h"
#include "modsres.h"

// This uses the linked-list class templates
#include "linklist.h"
#include "bonesdef.h"


int	  BonesDefMod::GetMirrorBone(Point3 center, int axis)
	{
	//get current bone
	INode *node = BoneData[ModeBoneIndex].Node;
	if (node)
		{
	//get tm
		Matrix3 iMirrorTM(1);
		iMirrorTM.SetRow(3,center);
		iMirrorTM = Inverse(iMirrorTM);
		TimeValue t = GetCOREInterface()->GetTime();
		Matrix3 tm = node->GetNodeTM(t);
		Point3 mirrorPoint = Point3(0.0f,0.0f,0.0f) * tm;
		mirrorPoint = mirrorPoint * iMirrorTM;
	//mirror its location
		mirrorPoint[axis] *= -1.0f;




	//loop through all the bones
		int closestBone = -1;
		float closestDist= 0.0f;
		Box3 bounds;
		bounds.Init();
		for (int i = 0; i < BoneData.Count(); i++)
			{
			node = BoneData[i].Node;
	//get there tm
			if ((node) && (i!=ModeBoneIndex))
				{	
	//find closest bone
				tm = node->GetNodeTM(t);
				Point3 bonePoint = Point3(0.0f,0.0f,0.0f) * tm;
				bonePoint = bonePoint * iMirrorTM;
				bounds += bonePoint;
				float dist = Length(bonePoint-mirrorPoint);
				if ( (closestBone == -1) || (dist < closestDist) )
					{
					closestDist = dist;
					closestBone = i;
					}
				}
			}
		float threshold = Length(bounds.pmax-bounds.pmin)/100.0f;
		if (closestDist < threshold)
			return closestBone;

		}
	
	return -1;
	}

void  BonesDefMod::SetMirrorBone()
{
	mirrorIndex = -1;
	if (pPainterInterface->GetMirrorEnable())
		{
		Point3 center = pPainterInterface->GetMirrorPlaneCenter();
		int dir = pPainterInterface->GetMirrorAxis();

		float offset = pPainterInterface->GetMirrorOffset();
		center[dir] += offset;

		mirrorIndex = GetMirrorBone(center, dir);
		}

	if (mirrorIndex == -1)
		mirrorIndex = ModeBoneIndex;

}

void BonesDefMod::ApplyPaintWeights(BOOL alt, INode *incNode)
	{
//5.1.03
	BOOL blendMode = FALSE;
	pblock_param->GetValue(skin_paintblendmode,0,blendMode,FOREVER);

	INode *node = NULL;
	for (int i =0;i < painterData.Count(); i++)
		{
		painterData[i].alt = alt;
		if (incNode == NULL)
			node = painterData[i].node;
		else node = incNode;
		if (node == painterData[i].node)
			{
			BoneModData *bmd = painterData[i].bmd;
			int count;
			float *str = pPainterInterface->RetrievePointGatherStr(node, count);
			int *isMirror = pPainterInterface->RetrievePointGatherIsMirror(node, count);

			BitArray excludedVerts;
			BOOL useExclusion = FALSE;
			if ((ModeBoneIndex >= 0) && (ModeBoneIndex < bmd->exclusionList.Count()))
			{
				if (bmd->exclusionList[ModeBoneIndex])
				{								
					excludedVerts.SetSize(count);
					excludedVerts.ClearAll();
					useExclusion = TRUE;
					for (int j = 0; j < bmd->exclusionList[ModeBoneIndex]->Count(); j++)
					{
						int index = bmd->exclusionList[ModeBoneIndex]->Vertex(j);
						if (index < excludedVerts.GetSize())
							excludedVerts.Set(index,TRUE);
					}
				}
			}

			if ((mirrorIndex >= 0) && (mirrorIndex < bmd->exclusionList.Count()))
			{
				if ((mirrorIndex != -1) && bmd->exclusionList[mirrorIndex])
				{			
					if (excludedVerts.GetSize() != count)
					{
						excludedVerts.SetSize(count);
						excludedVerts.ClearAll();
					}
					useExclusion = TRUE;
					for (int j = 0; j < bmd->exclusionList[mirrorIndex]->Count(); j++)
					{
						int index = bmd->exclusionList[mirrorIndex]->Vertex(j);
						if (index < excludedVerts.GetSize())
							excludedVerts.Set(index,TRUE);
					}
				}
			}
			for (int j =0; j < count; j++)
				{

				BOOL skip = FALSE;
				if (useExclusion)
				{
					if (excludedVerts[j])
						skip = TRUE;
				}
				if ((str[j] != 0.0f) && (!skip))
					{
//get original amount
					float amount = 0.0f;
					int boneID = ModeBoneIndex;
					if (isMirror[j] == MIRRRORED) boneID = mirrorIndex;

					if(boneID != -1)
						{
//add to it str
						for (int k =0; k < bmd->VertexData[j]->WeightCount(); k++)
							{
							if (bmd->VertexData[j]->GetBoneIndex(k) == boneID)
								{
									amount = bmd->VertexData[j]->GetNormalizedWeight(k);			
									k = bmd->VertexData[j]->WeightCount();
								}
							}
//5.1.03
						if (blendMode &&  bmd->blurredWeights.Count() != 0)
							{
								float blurAmount = bmd->blurredWeights[j];
				 				amount += (bmd->blurredWeights[j] - amount) * str[j];
				 		    }
				 		 else
				 			{
							if (alt)
								amount -= str[j];
							else 
								amount += str[j];
							if (amount > 1.0f) amount = 1.0f;
							if (amount < 0.0f) amount = 0.0f;
							}

						
//set it back
						updateDialogs = FALSE;
						SetVertex(bmd, j, boneID, amount);
						updateDialogs = TRUE;
						}
					}
				}

			}
		}
	}

BOOL  BonesDefMod::StartStroke()
	{
	theHold.Begin();
	int vertCount = 0;
	
//5.1.05
	BOOL blendMode = FALSE;
	pblock_param->GetValue(skin_paintblendmode,0,blendMode,FOREVER);


	
//this puts back the original state of the node vc mods and shade state
	for (int  i = 0; i < painterData.Count(); i++)
		{
		BoneModData *bmd = painterData[i].bmd;
		if (bmd)
			{
//5.1.05
			if (blendMode)
				bmd->BuildBlurData(ModeBoneIndex);
			
			theHold.Put(new WeightRestore(this,bmd,FALSE));
			vertCount += bmd->VertexData.Count();
			} 
		}
	lagRate = pPainterInterface->GetLagRate();

	lagHit = 1;
	for (int i =0;i < painterData.Count(); i++)
		{
		painterData[i].node->FlagForeground(GetCOREInterface()->GetTime());
		}

//get mirror nodeindex
	SetMirrorBone();
	return TRUE;
	}
BOOL  BonesDefMod::PaintStroke(
						  BOOL hit,
						  IPoint2 mousePos, 
						  Point3 worldPoint, Point3 worldNormal,
						  Point3 localPoint, Point3 localNormal,
						  Point3 bary,  int index,
						  BOOL shift, BOOL ctrl, BOOL alt, 
						  float radius, float str,
						  float pressure, INode *node,
						  BOOL mirrorOn,
						  Point3 worldMirrorPoint, Point3 worldMirrorNormal,
						  Point3 localMirrorPoint, Point3 localMirrorNormal
						  ) 
	{
	theHold.Restore();

	int selNodeCount = GetCOREInterface()->GetSelNodeCount();  //this in case of instance modifiers
														  //theHold.Restore will wipe both instances
														  //so we need to compute weights for all nodes
														  // not just the one that got hit
	for (int i = 0; i < selNodeCount; i++)
		{
		ApplyPaintWeights(alt, GetCOREInterface()->GetSelNode(i));
		}

	lagHit++;
	if (  (lagRate == 0) ||  ((lagHit%lagRate) == 0)) NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);;
	
	return TRUE;
	}

BOOL  BonesDefMod::EndStroke(int ct, BOOL *hits, IPoint2 *mousePos, 
						  Point3 *worldPoint, Point3 *worldNormal,
						  Point3 *localPoint, Point3 *localNormal,
						  Point3 *bary,  int *index,
						  BOOL *shift, BOOL *ctrl, BOOL *alt, 
						  float *radius, float *str,
						  float *pressure, INode **node,
						  BOOL mirrorOn,
						  Point3 *worldMirrorPoint, Point3 *worldMirrorNormal,
						  Point3 *localMirrorPoint, Point3 *localMirrorNormal	)
	{
	theHold.Restore();
	ApplyPaintWeights(alt[ct-1], NULL);


	theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	RebuildPaintNodes();
	PaintAttribList(TRUE);
	return TRUE;
	}

BOOL  BonesDefMod::EndStroke()
	{
	theHold.Restore();
	ApplyPaintWeights(painterData[0].alt, NULL);
	theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	RebuildPaintNodes();
	PaintAttribList(TRUE);
	return TRUE;
	}
BOOL  BonesDefMod::CancelStroke()
	{
	theHold.Cancel();
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);;
	PaintAttribList(TRUE);
	return TRUE;
	}
BOOL  BonesDefMod::SystemEndPaintSession()
	{
	//Unhook us from the painter interface
	//lets move this to the start session since some on may bump us off by starting another paint session and we wont know about it
	if (pPainterInterface) pPainterInterface->InitializeCallback(NULL);
	if (iPaintButton!=NULL) iPaintButton->SetCheck(FALSE);
	return TRUE;
	}

void BonesDefMod::RebuildPaintNodes()
	{
	//this sends all our dependant nodes to the painter
	MyEnumProc dep;              
	DoEnumDependents(&dep);
	Tab<INode *> nodes;
	for (int i = 0; i < nodes.Count(); i++)
		{

		ObjectState os = nodes[i]->EvalWorldState(GetCOREInterface()->GetTime());
		
		// Defect 724761 we need to check if the object is a patch, nurbs or a spline
		//for loading the custom PointGather.
		if ( (os.obj->NumPoints() != painterData[i].bmd->VertexData.Count()) ||
			 (painterData[i].bmd->isPatch) || (painterData[i].bmd->inputObjectIsNURBS) ||
			  os.obj->IsSubClassOf(splineShapeClassID))
			{
			int ct = painterData[i].bmd->VertexData.Count();
			Tab<Point3> pointList;
			pointList.SetCount(ct);
			Matrix3 tm = nodes[i]->GetObjectTM(GetCOREInterface()->GetTime());
			for (int j =0; j < ct; j++)
				{
				pointList[j] = painterData[i].bmd->VertexData[j]->LocalPosPostDeform*tm;
				}
			pPainterInterface->LoadCustomPointGather(ct, pointList.Addr(0), nodes[i]);
			}
		}
	pPainterInterface->UpdateMeshes(TRUE);
	}

void BonesDefMod::CanvasStartPaint()
{
	if (iPaintButton!=NULL) iPaintButton->SetCheck(TRUE);
	
	pPainterInterface->SetBuildNormalData(FALSE);
	pPainterInterface->SetEnablePointGather(TRUE);

	//this sends all our dependant nodes to the painter
	MyEnumProc dep;              
	DoEnumDependents(&dep);
	Tab<INode *> nodes;
	painterData.ZeroCount();
	for (int  i = 0; i < dep.Nodes.Count(); i++)
		{
		BoneModData *bmd = GetBMD(dep.Nodes[i]);

		if (bmd)
			{
			nodes.Append(1,&dep.Nodes[i]);
			ObjectState os;
			PainterSaveData temp;
			temp.node = dep.Nodes[i];
			temp.bmd = bmd;
			painterData.Append(1,&temp);
			}
		}
	pPainterInterface->InitializeNodes(0, nodes);
	BOOL updateMesh = FALSE;
	for (int i = 0; i < nodes.Count(); i++)
		{

		ObjectState os = nodes[i]->EvalWorldState(GetCOREInterface()->GetTime());
		
		// Defect 724761 we need to check if the sub object is a patch, nurbs or a spline
		//for loading the custom PointGather.
		if ( (os.obj->NumPoints() != painterData[i].bmd->VertexData.Count()) ||
			  (painterData[i].bmd->isPatch) || (painterData[i].bmd->inputObjectIsNURBS) ||
			  os.obj->IsSubClassOf(splineShapeClassID))   
			{
			int ct = painterData[i].bmd->VertexData.Count();
			Tab<Point3> pointList;
			pointList.SetCount(ct);
			Matrix3 tm = nodes[i]->GetObjectTM(GetCOREInterface()->GetTime());
			for (int j =0; j < ct; j++)
				{
				pointList[j] = painterData[i].bmd->VertexData[j]->LocalPosPostDeform*tm;
				}
			pPainterInterface->LoadCustomPointGather(ct, pointList.Addr(0), nodes[i]);
			updateMesh = TRUE;
			}
		}

	if (updateMesh)
		pPainterInterface->UpdateMeshes(TRUE);

//get mirror nodeindex
	SetMirrorBone();

	for (int i = 0; i < nodes.Count(); i++)
		{
	
		painterData[i].node->FlagForeground(GetCOREInterface()->GetTime());
		}
		
//5.1.03
	for (int i = 0; i < painterData.Count(); i++)
		{
		BoneModData *bmd = painterData[i].bmd;
//5.1.05
		bmd->BuildEdgeList();
		}		

}
void BonesDefMod::CanvasEndPaint()
{
//	pPainterInterface->InitializeCallback(NULL);
	if (iPaintButton!=NULL) iPaintButton->SetCheck(FALSE);
	
//5.1.03
	for (int  i = 0; i < painterData.Count(); i++)
		{
		BoneModData *bmd = painterData[i].bmd;
//5.1.05
		if (bmd)
			bmd->FreeEdgeList();
		}	
}

void BonesDefMod::StartPaintMode()
	{
	//hook us up to the painter interface
	//lets move this to the start session since some on may bump us off by starting another paint session and we wont know about it
	if (pPainterInterface)
		{
		pPainterInterface->InitializeCallback((ReferenceTarget *) this);
		pPainterInterface->StartPaintSession();  //all we need to do is call startpaintsession
												//with the 5.1 interface the painterinterface will
												//then call either CanvaseStartPaint or CanvaseEndPaint
												//this lets the painter turn off and on the paint mode when it command mode gets popped
		if (pPainterInterface->InPaintMode())
		{
			BOOL mirror;
			pblock_mirror->GetValue(skin_mirrorenabled,0,mirror,FOREVER);
			if (mirror)
				pblock_mirror->SetValue(skin_mirrorenabled,0,FALSE);

		}
		// Brush presets
		BeginBrushPresetContext( this );
		}
	}
void BonesDefMod::PaintOptions()
	{
	if (pPainterInterface) pPainterInterface->BringUpOptions();
	}



//-----------------------------------------------------------------------------
// class BonesDefBrushPresetContext

#define BONESDEFPAINT_CONTEXT_ID Class_ID(0,0x5ecd700d)
class BonesDefBrushPresetContext : public IBrushPresetContext {
	public:
		BonesDefBrushPresetContext();

		Class_ID			ContextID() { return BONESDEFPAINT_CONTEXT_ID; }
		TCHAR*				GetContextName();

		IBrushPresetParams*	CreateParams();
		void				DeleteParams( IBrushPresetParams* params );

		int					GetNumParams()					{return 1;}
		int					GetParamID( int paramIndex )	{return 1;}
		int					GetParamIndex( int paramID )	{return 0;}
		TCHAR*				GetParamName( int paramID );
		ParamType2			GetParamType( int paramID );

		Class_ID			PluginClassID() {return BONESDEFMOD_CLASS_ID;}
		SClass_ID			PluginSuperClassID() {return BONESDEFMOD_SUPERCLASS_ID;}

		static void			OnSystemStartup(void *param, NotifyInfo *info);

		void				SetParent( BonesDefMod* parent );
		static BonesDefMod*	parent;
};

BonesDefBrushPresetContext theBonesDefBrushPresetContext;
BonesDefBrushPresetContext* GetBonesDefBrushPresetContext() {return &theBonesDefBrushPresetContext;}


//-----------------------------------------------------------------------------
// class BonesDefBrushPresetParams

class BonesDefBrushPresetParams : public IBrushPresetParams {
	public:
		BonesDefBrushPresetParams();
		Class_ID			ContextID() { return BONESDEFPAINT_CONTEXT_ID; }

		// The presets param object is expected to have access to its associated context.
		// These methods apply the params into the context, or fetch the param values from the context
		void				ApplyParams();
		void				FetchParams();

		// Use methods in the BrushPresetContext for metadata about the parameters.
		// Parameter values are stored here.
		int					GetNumParams() {return 1;}
		int					GetParamID( int paramIndex ) {return 1;}
		int					GetParamIndex( int paramID ) {return 0;}
		FPValue				GetParamValue( int paramID );
		void				SetParamValue( int paramID, FPValue val );
		FPValue				GetDisplayParamValue( int paramID );

		void				SetParent( IBrushPreset* parent ) {}
		static void			SetParent( BonesDefMod* parent );

	protected:
		enum				{modeNone, modePaint, modeBlend};
		int					mode;
		static BonesDefMod*	parent;
};


//-----------------------------------------------------------------------------
// class BonesDefBrushPresetContext

BonesDefBrushPresetContext::BonesDefBrushPresetContext() {
	RegisterNotification( OnSystemStartup, NULL, NOTIFY_SYSTEM_STARTUP );
}

TCHAR* BonesDefBrushPresetContext::GetContextName()
	{
	return GetString(IDS_BONESDEFPAINT);
	} 

IBrushPresetParams*	BonesDefBrushPresetContext::CreateParams()
	{
	return new BonesDefBrushPresetParams;
	}

void BonesDefBrushPresetContext::DeleteParams( IBrushPresetParams* params )
	{
	if( (params==NULL) || (params->ContextID()!=ContextID()) )
		return; //invalid params
	delete (BonesDefBrushPresetParams*)params;
	}

TCHAR* BonesDefBrushPresetContext::GetParamName( int paramID )
	{
	return GetString(IDS_BONESDEFPAINT_MODE);
	}

ParamType2 BonesDefBrushPresetContext::GetParamType( int paramID )
	{
	return (ParamType2)TYPE_INT;
	}

void BonesDefBrushPresetContext::OnSystemStartup(void *param, NotifyInfo *info)
	{
	BonesDefBrushPresetContext* context = GetBonesDefBrushPresetContext();
	IBrushPresetMgr* mgr = GetIBrushPresetMgr();
	if( (context!=NULL) && (mgr!=NULL) )
		mgr->RegisterContext( context );
	}


//-----------------------------------------------------------------------------
// class BonesDefBrushPresetParams

BonesDefMod* BonesDefBrushPresetParams::parent;

BonesDefBrushPresetParams::BonesDefBrushPresetParams()
	{
	mode = modeNone;
	}

// The presets param object is expected to have access to its associated context.
// These methods apply the params into the context, or fetch the param values from the context
void BonesDefBrushPresetParams::ApplyParams()
	{
	BOOL isPaintEnabled = ((parent->iPaintButton!=NULL) && parent->iPaintButton->IsEnabled()?  TRUE:FALSE);

	if (isPaintEnabled && (mode!=modeNone)) //FIXME: hack... this is an easy way to know whether painting is allowed
		{
		BOOL isBlendMode = (mode==modeBlend?  TRUE:FALSE);
		mode = (isBlendMode? modeBlend:modePaint);
		parent->pblock_param->SetValue(skin_paintblendmode, 0, isBlendMode);

		if( !parent->pPainterInterface->InPaintMode() )
			parent->StartPaintMode();
		}
	}

void BonesDefBrushPresetParams::FetchParams()
	{
	BOOL isPaintEnabled = ((parent->iPaintButton!=NULL) && parent->iPaintButton->IsEnabled()?  TRUE:FALSE);
	BOOL isPaintActive = parent->pPainterInterface->InPaintMode();

	if( isPaintEnabled && isPaintActive )
		{
		BOOL isBlendMode = parent->pblock_param->GetInt(skin_paintblendmode);
		mode = (isBlendMode? modeBlend:modePaint);
		}
	}

FPValue BonesDefBrushPresetParams::GetParamValue( int paramID )
	{
	FPValue val;
	val.type = (ParamType2)TYPE_INT;
	val.i = mode;
	return val;
	}

void BonesDefBrushPresetParams::SetParamValue( int paramID, FPValue val )
	{
	if( val.type != (ParamType2)TYPE_INT )
		return;
	mode = val.i;
	}

FPValue BonesDefBrushPresetParams::GetDisplayParamValue( int paramID )
	{
	TCHAR buf[256], *text=buf;
	buf[0] = 0;

	if( mode==modePaint )			text = GetString( IDS_BONESDEFPAINT_PAINT );
	else if( mode==modeBlend )		text = GetString( IDS_BONESDEFPAINT_BLEND );

	FPValue val;
	val.type = TYPE_TSTR_BV;
	val.tstr = new TSTR( text );
	return val;
	}

void BonesDefBrushPresetParams::SetParent( BonesDefMod* parent ) {
	BonesDefBrushPresetParams::parent = parent;
}



// BonesDef hooks to the Brush Preset system

void BonesDefMod::BeginBrushPresetContext( BonesDefMod* mod )
	{
	BonesDefBrushPresetParams::SetParent( mod );
	IBrushPresetMgr* mgr = GetIBrushPresetMgr();
	if( mgr!=NULL )
		mgr->BeginContext( BONESDEFPAINT_CONTEXT_ID );
	}

void BonesDefMod::EndBrushPresetContext( BonesDefMod* mod )
	{
	BonesDefBrushPresetParams::SetParent( mod );
	IBrushPresetMgr* mgr = GetIBrushPresetMgr();
	if( mgr!=NULL )
		mgr->EndContext( BONESDEFPAINT_CONTEXT_ID );
	}

void BonesDefMod::OnBrushPresetContextUpdated()
	{
	IBrushPresetMgr* mgr = GetIBrushPresetMgr();
	if( mgr!=NULL )
		mgr->OnContextUpdated( BONESDEFPAINT_CONTEXT_ID );
	}