/**********************************************************************
 *<
	FILE: PolyPaintsel.cpp

	DESCRIPTION:   Paint Soft Selection; Editable Polygon Mesh Object

	CREATED BY: Michaelson Britt

	HISTORY: created May 2004

 *>	Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/

//#include "EPoly.h"
#include "..\PolyPaint\PolyPaint.h"


//-----------------------------------------------------------------------------
//--- General utilities

#define MIN(a,b) ((a)<(b)? (a):(b))

//helper macro to iterate through hosts
#define FOR_EACH_HOST													\
	Tab<MeshPaintHost*> _hosts;											\
	Tab<INode*> _nodes;													\
	MeshPaintHost* host = NULL;												\
	INode* node = NULL;														\
	GetPaintHosts( _hosts, _nodes );									\
	for( int _i=0; _i<_hosts.Count(); _i++ )							\
		if( ((host=_hosts[_i])!=NULL) && ((node=_nodes[_i])!=NULL) )

float Interp( float a, float b, float w ) {
	return (w*b) + ((1.0f-w)*a);
}

Point3 Interp( Point3& a, Point3& b, float w ) {
	return Point3(
		(w*b.x) + ((1.0f-w)*a.x),
		(w*b.y) + ((1.0f-w)*a.y),
		(w*b.z) + ((1.0f-w)*a.z)
	);
}

void memswp( void* a, void* b, int size ) {
	char* temp = new char[size];
	memcpy( temp, a, size );
	memcpy( a, b, size );
	memcpy( b, temp, size );
	delete[] temp;
}

//from iparamb2.h
#define root_type(t)	((ParamType2)((t) & ~(TYPE_TAB | TYPE_BY_VAL | TYPE_BY_REF | TYPE_BY_PTR)))	// get base type ignoring all flags

ParamType2 EssentialType( ParamType2 type ) {
	ParamType2 root = root_type(type);
	switch( root ) {
		case TYPE_WORLD:
		case TYPE_ANGLE:
			return (ParamType2)TYPE_FLOAT;
		case TYPE_bool:
		case TYPE_BOOL:
		case TYPE_TIMEVALUE:
		case TYPE_INDEX:
		case TYPE_ENUM:
			return (ParamType2)TYPE_INT;
		case TYPE_RGBA:
		case TYPE_HSV:
			return (ParamType2)TYPE_POINT3;
		case TYPE_FRGBA:
			return TYPE_POINT4;
	}
	return type;
}

FPValue EssentialValue( FPValue val ) {
	FPValue retval;
	switch( val.type ) {
		case TYPE_FLOAT_BR:
		case TYPE_WORLD_BR:
		case TYPE_ANGLE_BR:
		case TYPE_FLOAT_BP:
		case TYPE_WORLD_BP:
		case TYPE_ANGLE_BP:
			retval.f = *(val.fptr); break;
		case TYPE_bool:
			retval.i = (val.b? TRUE : FALSE); break;
		case TYPE_bool_BR:
		case TYPE_bool_BP:
			retval.i = (*(val.bptr)? TRUE : FALSE); break;
		case TYPE_INT_BR:
		case TYPE_BOOL_BR:
		case TYPE_TIMEVALUE_BR:
		case TYPE_INDEX_BR:
		case TYPE_ENUM_BR:
		case TYPE_INT_BP:
		case TYPE_BOOL_BP:
		case TYPE_TIMEVALUE_BP:
		case TYPE_INDEX_BP:
		case TYPE_ENUM_BP :
			retval.i = *(val.iptr); break;
		case TYPE_DWORD_BR:
		case TYPE_DWORD_BP:
			retval.d = *(val.dptr); break;
		default:
			retval = val; break;
	}
	retval.type = EssentialType( val.type );
	return retval;
}

//-----------------------------------------------------------------------------
//--- PainterSettings: save and restore settings of the painter interface

void PainterSettings::Init() {
	drawTrace = FALSE;
	drawRing = drawNormal = TRUE;
	pressureEnable = TRUE;
	pressureAffects = PRESSURE_AFFECTS_STR;
	brushMinSize = 0.0f,		brushMaxSize = 20.0f;
	brushMinStrength = 0.0f,	brushMaxStrength = 1.0f;
}

void PainterSettings::Get( IPainterInterface_V5* painter ) {
	drawTrace = painter->GetDrawTrace();
	drawRing = painter->GetDrawRing();
	drawNormal = painter->GetDrawNormal();
	pressureEnable = painter->GetPressureEnable();
	pressureAffects = painter->GetPressureAffects();
	brushMinSize = painter->GetMinSize(),		brushMaxSize = painter->GetMaxSize();
	brushMinStrength = painter->GetMinStr(),	brushMaxStrength = painter->GetMaxStr();
}

void PainterSettings::Set( IPainterInterface_V5* painter ) {
	painter->SetDrawTrace( drawTrace );
	painter->SetDrawRing( drawRing );
	painter->SetDrawNormal( drawNormal );
	painter->SetPressureEnable( pressureEnable );
	painter->SetPressureAffects( pressureAffects );
	painter->SetMinSize(brushMinSize),			painter->SetMaxSize(brushMaxSize);
	painter->SetMinStr(brushMinStrength),		painter->SetMaxStr(brushMaxStrength);
}


//-----------------------------------------------------------------------------
//--- MeshPaintMgr


inline MeshPaintMgr* GetMeshPaintMgr() {
	return MeshPaintMgr::GetInstance();
}

//-----------------------------------------------------------------------------
//--- IMeshPaintMgr: Published interface for MeshPaintMgr

BaseInterface* IMeshPaintMgr::GetInterface(Interface_ID id)
{	
	if( id == GetID() )
	{
		return this;
	}
	return NULL;
}


//-----------------------------------------------------------------------------
//--- MeshPaintMgr
// statics
MeshPaintMgr* MeshPaintMgr::mInstance = NULL;

// constructor
MeshPaintMgr::MeshPaintMgr()
	: IMeshPaintMgr (
		IMESHPAINTMGR_INTERFACE_ID, _T("MeshPaintMgr"), 0, NULL, FP_CORE | FP_TEST_INTERFACE,
		end
	)
{
	paintMode = PAINTMODE_OFF;
	presetContextID = Class_ID(0,0);
	paintSelValue = 0.0;
	paintDeformValue = 0.0;
	pPainter = NULL;
	handler = NULL;
	RegisterNotification( OnSystemStartup, NULL, NOTIFY_SYSTEM_STARTUP );
}

// destructor
MeshPaintMgr::~MeshPaintMgr()
{
	Free();
}

// Singleton
MeshPaintMgr* MeshPaintMgr::GetInstance()
{
	MeshPaintMgr* theMeshPaintMgr = (MeshPaintMgr*)GetCOREInterface( IMESHPAINTMGR_INTERFACE_ID );
	if (theMeshPaintMgr) 
	{
		return theMeshPaintMgr;
	}
	if (NULL == mInstance && NULL == theMeshPaintMgr)
	{
		mInstance = new MeshPaintMgr();
		mInstance->Init();
	}
	return mInstance;
}

void MeshPaintMgr::DestroyInstance()
{
	if (mInstance != NULL)
	{
		delete mInstance;
		mInstance = NULL;
	}
}

void MeshPaintMgr::DeleteThis() 
{
	MeshPaintMgr::DestroyInstance();
}

void MeshPaintMgr::Init() {
	if( pPainter==NULL )
		pPainter = GetPainter(); //create a painter instance
}

void MeshPaintMgr::Free() {
	if( paintMode != PAINTMODE_OFF ) {
		EndSession();
	}
	//clean up the ObjectStates
	for( int i=0; i<objectStates.Count(); i++ )
	{
		delete ((PolyObject*)(objectStates[i].obj));
	}
	hosts.Resize(0);
	nodes.Resize(0);
	objectStates.Resize(0);
	dataContexts.Resize(0);
}

// Accessor methods
MeshPaintHandler* MeshPaintMgr::GetHandler() {
	return handler;
}

void MeshPaintMgr::SetHandler( MeshPaintHandler* handler ) {
	if( handler==this->handler )	return;

	Free();
	this->handler = handler;
}

float MeshPaintMgr::GetPaintValue( ) {
	if( IsPaintSelMode(paintMode) )		return paintSelValue;
	if( IsPaintDeformMode(paintMode) )	return paintDeformValue;
	return 0;
}
void  MeshPaintMgr::SetPaintValue( float f ) {
	if( IsPaintSelMode(paintMode) )		this->paintSelValue=f;
	if( IsPaintDeformMode(paintMode) )	this->paintDeformValue=f;
}

float MeshPaintMgr::GetBrushSize()
	{ return pPainter->GetMaxSize(); }
void  MeshPaintMgr::SetBrushSize( float f )
	{ pPainter->SetMaxSize(f); }

float MeshPaintMgr::GetBrushStrength() {
	float s = pPainter->GetMaxStr();
	return (s<0? 0:s);
}
void  MeshPaintMgr::SetBrushStrength( float f )
	{ pPainter->SetMaxStr(f); }


// Mode-related methods
void MeshPaintMgr::BeginEditParams( MeshPaintHandler* handler ) {
	//configure the painter
	painterSettingsPrev.Get( pPainter );
	PainterSettings* painterSettings = handler->GetPainterSettings();
	if( painterSettings!=NULL ) painterSettings->Set( pPainter );
	handler->OnPaintBrushChanged(); //refresh client to display the new settings in the painter
}

void MeshPaintMgr::EndEditParams( MeshPaintHandler* handler ) {
	SetPresetContext( Class_ID(0,0) );

	PainterSettings painterSettings;
	painterSettings.Get( pPainter );
	handler->SetPainterSettings( painterSettings );
	painterSettingsPrev.Set( pPainter );

	SetHandler(NULL);
}

class RightClickToExitPaint : public IPainterRightClickHandler
{
private:
	MeshPaintHandler *mpPaintHandler;
public:
	RightClickToExitPaint() : mpPaintHandler(NULL) { }
	void SetHandler (MeshPaintHandler *pHandler) { mpPaintHandler = pHandler; }
	void RightClick () { if (mpPaintHandler) mpPaintHandler->EndPaintMode(); }
};
static RightClickToExitPaint theRightClicker;

void MeshPaintMgr::BeginSession( PaintModeID paintMode ) {
	if( (!pPainter) || (this->paintMode==paintMode) ) return;
	if( this->paintMode!=PAINTMODE_OFF ) EndSession();

	pPainter->InitializeCallback((ReferenceTarget *) this);
	pPainter->SetEnablePointGather(TRUE);

	Free();
	handler->GetPaintHosts( hosts, nodes );

	Tab<MNMesh*> objects;
	for( int i=0; i<nodes.Count(); i++ ) {
		handler->UpdatePaintObject( hosts[i] );
		MNMesh* object = handler->GetPaintObject( hosts[i] );
		DbgAssert( object!=NULL );

		PaintDataContext* nullDataContext = NULL;
		dataContexts.Append( 1, &nullDataContext ); //contexts are initially null

		// Get the mesh and create an ObjectState (yes, a hack)
		PolyObject* polyObj = new PolyObject;
		polyObj->mm.ShallowCopy( object, TOPO_CHANNEL + GEOM_CHANNEL );
		polyObj->SetChannelLocks( ALL_CHANNELS );
		ObjectState objectState(polyObj);
		objectStates.Append( 1, &objectState );

		objects.Append( 1, &object );
	}

	// Initialize nodes before updating point gather for each node
	pPainter->InitializeNodesByObjState( 0, nodes, objectStates );
	for( int i=0; i<nodes.Count(); i++ ) UpdatePointGather( nodes[i], hosts[i] );

	this->paintMode = paintMode;
	theRightClicker.SetHandler (handler);
	pPainter->StartPaintSession(&theRightClicker);

	handler->UpdatePaintBrush();

	Class_ID presetContext = GetPresetContextID(paintMode);
	if( GetPresetContext() != presetContext )
		SetPresetContext( presetContext );
}

void MeshPaintMgr::EndSession( PaintModeID paintMode ) {
	if( paintMode==PAINTMODE_UNDEF ) paintMode=this->paintMode;

	if( (this->paintMode!=paintMode) || (pPainter==NULL) ) return;
	if( pPainter->InPaintMode() ) pPainter->EndPaintSession();
	this->paintMode = PAINTMODE_OFF;

	Free();
}

void MeshPaintMgr::SetPresetContext( Class_ID presetContextID ) {
	Class_ID prevContextID = this->presetContextID;

	if( prevContextID==presetContextID ) return;
	IBrushPresetMgr* mgr = GetIBrushPresetMgr();
	if( prevContextID!=Class_ID(0,0) )		mgr->EndContext( prevContextID );
	if( presetContextID!=Class_ID(0,0) )	mgr->BeginContext( presetContextID );

	this->presetContextID = presetContextID;
}

//Helper for MeshPaintMgr::BeginDataContext
BOOL IsAnythingSelected( BitArray& selMask, Tab<float>& softSelMask ) {
	if( !selMask.IsEmpty() ) return TRUE;
	int count = softSelMask.Count();
	for( int i=0; i<count; i++ )
		if( softSelMask[i]>0 ) return TRUE;
	return FALSE;
}

PaintDataContext* MeshPaintMgr::BeginDataContext( PaintModeID paintMode, INode* node, MeshPaintHandler* handler, MeshPaintHost* host ) {

	PaintDataContext* data = new PaintDataContext;	// Create a paint context 
	memset( data, 0, sizeof(PaintDataContext) );	// Initialize to zero

	data->paintMode = paintMode;
	data->node = node;

	data->handler = handler;
	data->host = host;
	MeshPaintHostRestore* restore = host->CreatePaintRestore(paintMode,node,handler, MeshPaintHostRestore::typePaint);
	data->restore = restore;
	data->inverted = FALSE;
	int count = 0;

	if( IsPaintSelMode(paintMode) ) {
		data->paintSelCount = count = host->GetPaintSelCount();
		FloatTab origSel;
		handler->GetPaintInputSel( host, origSel );
		if( origSel.Count()>0 ) {
			data->origSel = new float[origSel.Count()];
			memcpy( data->origSel, origSel.Addr(0), origSel.Count() * sizeof(float) );
		}
		data->outputSel = host->GetPaintSelValues();
		if( count>0 ) {
			data->goalSel = new float[count];
			data->inputSel = new float[count];
			memcpy( data->inputSel, data->outputSel, count * sizeof(float) );
		}
	}
	if( IsPaintDeformMode(paintMode) ) {
		data->paintDeformCount = count = host->GetPaintDeformCount();
		Point3* deform = host->GetPaintDeformOffsets(); 
		data->outputDeform = deform;

		//allocate original & goal positions; and init the original positions
		if( count>0 ) {
			data->origPositions = new Point3[count];
			data->goalPositions = new Point3[count];
			data->inputPositions = new Point3[count];

			Point3Tab inputPositions;
			data->handler->GetPaintInputVertPos( data->host, inputPositions );
			for( int i=0; i<count; i++ ) {
				data->origPositions[i] = inputPositions[i];
				data->inputPositions[i] = inputPositions[i] + deform[i];
			}
		}

		handler->GetPaintMask( host, data->selMask, data->softSelMask );
		if( !IsAnythingSelected(data->selMask, data->softSelMask ) )	//If nothing is selected...
			data->selMask.SetAll(), data->softSelMask.SetCount(0);		//...ignore the selection, treat everything as selected
	}
	if( count>0 ) {
		data->amounts = new float[count];
		memset( data->amounts, 0, count * sizeof(float) );
	}

	return data;
}

void MeshPaintMgr::EndDataContext( PaintDataContext* data, BOOL accepted ) {
	MeshPaintHost* host = data->host;
	MeshPaintHostRestore* restore = data->restore;

	if( accepted ) {
		theHold.Begin();
		theHold.Put( restore );
		theHold.Accept( restore->GetName() );
	}
	else {
		restore->Restore( FALSE );
		host->DeletePaintRestore( restore );
	}

	if( data->origSel!=NULL )			delete[] data->origSel;
	if( data->goalSel!=NULL )			delete[] data->goalSel;
	if( data->inputSel!=NULL )			delete[] data->inputSel;
	if( data->origPositions!=NULL )		delete[] data->origPositions;
	if( data->goalPositions!=NULL )		delete[] data->goalPositions;
	if( data->inputPositions!=NULL )	delete[] data->inputPositions;
	if( data->amounts!=NULL )			delete[] data->amounts;
	delete data;
}

void MeshPaintMgr::InitPaintSel( PaintDataContext* data ) {
	if( data->paintMode != PAINTMODE_BLURSEL ) {
		float paintValue = GetPaintValue();
		for( int i=0; i<data->paintSelCount; i++ ) data->goalSel[i] = paintValue;
	}
	else {
		float *inputSel = data->inputSel;
		MNMesh* object = data->object;
		int numFaces = object->FNum(), numVerts = object->VNum();

		float* blurFaceColors = new float[ numFaces+1 ]; //gracefully handle numFaces==0
		memset( blurFaceColors, 0xFFFFFFFF, numFaces* sizeof(float) ); //fill initial values

		for( int i=0; i<data->paintSelCount; i++ ) {
			float vertSum=0;

			// Visit each face of this vert
			IntTab& facesByVert = object->vfac[i];
			int vertDeg = facesByVert.Count(); //degree of the vertex
			for( int j=0; j<vertDeg; j++ ) {
				int fi = facesByVert[j];

				// Calculate the blurred selection value for this face, if we haven't already
				if( ((DWORD*)blurFaceColors)[fi] == 0xFFFFFFFF ) {
					MNFace* face = object->F(fi);
					float faceSum=0;
					for( int k=0; k<face->deg; k++ ) faceSum += inputSel[ face->vtx[k] ];
					blurFaceColors[fi] = (face->deg==0? 0 : (faceSum / face->deg));
				}

				// Add to the face's average to the vertex sum
				vertSum += blurFaceColors[fi];
			}
			// Take the average of the face averages
			data->goalSel[i] = (vertDeg==0? 0 : (vertSum / vertDeg));
		}
		delete[] blurFaceColors;
	}
}

void MeshPaintMgr::InitPaintDeform( PaintDataContext* data ) {
	float paintValue = GetPaintValue();
	MNMesh* object = data->object;
	int numFaces = object->FNum(), numVerts = object->VNum();

	Point3 *goalPositions = data->goalPositions;
	Point3* oldDeformPositions = data->inputPositions;

	if( data->paintMode == PAINTMODE_PAINTRELAX ) {
		int* counts = new int[numVerts];
		memset( counts, 0, numVerts * sizeof(int) );

		Point3 zero(0,0,0);
		for( int i=0; i<numVerts; i++ ) goalPositions[i]=zero;

		for( int f=0; f<numFaces; f++ ) {
			MNFace* face = &(object->f[f]);
			Point3* curPoint = &(oldDeformPositions[ face->vtx[0] ]);
			Point3* prevPoint = &(oldDeformPositions[ face->vtx[face->deg-1] ]);

			for( int fvi=0; fvi<face->deg; fvi++ ) {
				int curVertIndex = face->vtx[fvi];
				int nextVertIndex = face->vtx[ (fvi+1) % face->deg ];
				Point3* nextPoint = &(oldDeformPositions[ nextVertIndex ]);

				goalPositions[ curVertIndex ] += ((*prevPoint) + (*nextPoint));
				counts[ curVertIndex ] += 2;

				prevPoint = curPoint;
				curPoint = nextPoint;
			}
		}

		// divide out to get the averages; these are the final goal positions
		for( int i=0; i<numVerts; i++ ) goalPositions[i] *= (1.0f/counts[i]);
	}

	if( (data->paintMode == PAINTMODE_ERASEDEFORM) )
	{
		for( int f=0; f<numFaces; f++ ) {
			MNFace* face = &(object->f[f]);
			for( int fvi=0; fvi<face->deg; fvi++ ) {
				int curVertIndex = face->vtx[fvi];
				Point3& goalPosition = goalPositions[ curVertIndex ];
				// When erasing, the goal position is just the current position minus any deformation offset
				goalPosition = data->origPositions[ curVertIndex ];
			}
		}
	}

	if( data->paintMode == PAINTMODE_PAINTPUSHPULL )
	{
		Point3Tab inputNorms;
		GetPaintInputVertNorm( data, inputNorms );

		for( int v=0; v<numVerts; v++ ) {
			Point3& goalPosition = goalPositions[ v ];
			goalPosition = oldDeformPositions[ v ];
			goalPosition += (paintValue * inputNorms[v]);
		}
	}

		//-- Paint all verts inside the brush, based on their paint weights
	if (data->mChangedPoints.GetSize() != numVerts)
	{
		data->mChangedPoints.SetSize(numVerts);	
	}
	data->mChangedPoints.ClearAll();
}

void MeshPaintMgr::UpdatePaintDeform( PaintDataContext* data ) {
	if( data->paintMode == PAINTMODE_PAINTPUSHPULL )
	{
		int count = data->paintDeformCount;
		float paintValue = GetPaintValue();
		int numVerts = data->object->VNum();

		Point3 *origPositions	= data->origPositions;
		Point3 *goalPositions	= data->goalPositions;
		Point3 *inputPositions	= data->inputPositions;
		Point3 *outputDeform	= data->outputDeform;

		Point3Tab inputNorms;
		GetPaintInputVertNorm( data, inputNorms );

		// Handling for the ALT key; reverse the paint value if necessary
		float s = (data->inverted? -paintValue : paintValue);
		for( int v=0; v<numVerts; v++ ) {
			inputPositions[v] = origPositions[v]  +  outputDeform[v];
			goalPositions[v]  = inputPositions[v] + (s * inputNorms[v]);
		}

		if( count>0 ) memset( data->amounts, 0, count * sizeof(float) );
	}
}


// helper function
Point3 GetXFormBasis( PaintAxisID axis ) {
	switch( axis ) {
	case PAINTAXIS_XFORM_X: return Point3(1,0,0);
	case PAINTAXIS_XFORM_Y: return Point3(0,1,0);
	case PAINTAXIS_XFORM_Z: return Point3(0,0,1);
	}
	return Point3(0,0,0);
}

void MeshPaintMgr::GetPaintInputVertNorm( PaintDataContext* data, Point3Tab& inputNorms ) {
	//cached values, for convenience
	int count = data->paintDeformCount;
	MNMesh* object = data->object;
	PaintAxisID axis = data->handler->GetPaintAxis( data->paintMode );

	inputNorms.SetCount(count);

	if( IsPaintXFormAxis(axis) ) { //Axis is xform X, Y or Z
		Matrix3 xform;
		Point3 xformBasis = GetXFormBasis(axis);
		Interface* ip = GetCOREInterface();

		if( ip->GetRefCoordSys()==COORDS_LOCAL ) {
			// Special case for local space xform mode
			for( int i=0; i<object->VNum(); i++ ) {
				object->GetVertexSpace( i, xform );
				inputNorms[i] = xform.VectorTransform( xformBasis ).Normalize();
			}
		}
		else {
			TimeValue t= ip->GetTime();
			Matrix3 worldToObj = Inverse( data->node->GetObjectTM(t) );
			xform = ip->GetTransformAxis( data->node, 0 );
			Point3 norm = xform.VectorTransform( xformBasis );
			norm = worldToObj.VectorTransform( norm ).Normalize();
			for( int i=0; i<count; i++ ) inputNorms[i] = norm;
		}
	}
	else { // Axis is Original Normals, or Deformed Normals
		Point3 *origPositions = data->origPositions, *inputPositions = data->inputPositions;
		Point3 v0,v1,v2,norm;
		int vi0, vi1, vi2;
		IntTab triangles;

		//initialize the normals to zero
		Point3 zero(0,0,0);
		for( int i=0; i<count; i++ ) inputNorms[i]=zero;

		for( int fi=0; fi<object->FNum(); fi++ ) {
			MNFace& face = object->f[fi];
			face.GetTriangles( triangles );

			int i=0;
			while( i<triangles.Count() ) {
				vi0 = face.vtx[triangles[i++]], vi1 = face.vtx[triangles[i++]], vi2 = face.vtx[triangles[i++]];
				if( axis == PAINTAXIS_DEFORMEDNORMS )
					v0 = inputPositions[vi0], v1 = inputPositions[vi1], v2 = inputPositions[vi2];
				else
					v0 = origPositions[vi0], v1 = origPositions[vi1], v2 = origPositions[vi2];
				norm = Normalize( (v1 - v0) ^ (v2 - v1) );

				inputNorms[vi0] += norm, inputNorms[vi1] += norm, inputNorms[vi2] += norm;
			}
		}

		for( int i=0; i<count; i++ ) inputNorms[i] = inputNorms[i].Normalize();
	}
}

void MeshPaintMgr::UpdatePointGather( INode* node, MeshPaintHost* host ) {
	Point3Tab inputPositions;
	handler->GetPaintInputVertPos( host, inputPositions );
	int numVerts = inputPositions.Count();

	Point3* outputDeform = host->GetPaintDeformOffsets();

	// Get the world-space verts to paint on
	Point3* worldVerts = new Point3[ numVerts ];
	TimeValue t = GetCOREInterface()->GetTime();
	Matrix3 trans = node->GetObjectTM( t );
	for( int v=0; v<numVerts; v++ ) {
		Point3& pos = inputPositions[v];
		if( outputDeform!=NULL )	worldVerts[v]  =  (pos + outputDeform[v]) * trans;
		else						worldVerts[v]  =  pos * trans;
	}
	pPainter->LoadCustomPointGather( numVerts, worldVerts, node );
	delete[] worldVerts;
}


void MeshPaintMgr::UpdateBrushTracking() {
	pPainter->UpdateMeshesByObjState(TRUE, objectStates);
}

BOOL MeshPaintMgr::PaintStroke( INode *node, int index, PaintBrushContext brush )
{
	int objectIndex;

	//look for the given node in our list
	for( objectIndex=0; objectIndex<nodes.Count(); objectIndex++ )
		if( nodes[objectIndex]==node ) break;
	if( objectIndex>=nodes.Count() ) return TRUE; //can't find this node in our list

	PaintStrokeContext stroke;
	PaintDataContext* data = dataContexts[objectIndex];
	if( data==NULL ) {
		data = dataContexts[objectIndex] = BeginDataContext( paintMode, nodes[objectIndex], handler, hosts[objectIndex] );
		data->object = handler->GetPaintObject(data->host);
		if( IsPaintSelMode(paintMode) )		InitPaintSel( data );
		if( IsPaintDeformMode(paintMode) )	InitPaintDeform( data );
	}
	stroke.data = data;

	stroke.pointGather = pPainter->RetrievePointGatherWeights( node, stroke.pointGatherCount );
	stroke.curve = pPainter->GetFalloffGraph();
	stroke.brush = brush;
	int pressureAffects = pPainter->GetPressureAffects();
	if( (pressureAffects!=PRESSURE_AFFECTS_STR) && (pressureAffects!=PRESSURE_AFFECTS_BOTH) )
		stroke.brush.pressure = stroke.brush.strength = GetBrushStrength();
	// Hack... when the state of the ALT key changes, reverse the goal positions
	if( IsPaintDeformMode(paintMode) && (stroke.brush.altKey!=data->inverted) ) {
		data->inverted = stroke.brush.altKey;
		UpdatePaintDeform( data );
	}


	if( IsPaintSelMode(paintMode) )
		// Call the "Paint Selection" function
		PaintStroke_Sel( &stroke );

	if( IsPaintDeformMode(paintMode) )
		// Call the "Paint Deformation" function
		PaintStroke_Deform( &stroke );

	handler->OnPaintDataRedraw(paintMode);
	return TRUE;
}

void MeshPaintMgr::PaintStroke_Sel( PaintStrokeContext* stroke ) {
	float value, pointAmount; //temp values
	float* pointGather = stroke->pointGather;
	float* amounts = stroke->data->amounts;
	float *origSel = stroke->data->origSel, *goalSel = stroke->data->goalSel;
	float *inputSel = stroke->data->inputSel, *outputSel = stroke->data->outputSel;

	BOOL erase = (paintMode==PAINTMODE_ERASESEL? TRUE:FALSE);
	if( stroke->brush.ctrlKey ) erase = (erase? FALSE:TRUE); //toggle between paint and erase

	// - Paint all verts inside the brush, based on their paint weights
	for( int i=0; i<stroke->pointGatherCount; i++, pointGather++ ) {
		if( *pointGather<=0.0f ) continue;

		pointAmount = stroke->curve->GetValue(0, 1.0f - *pointGather); //FIXME: maybe use lookup table param
		pointAmount = pointAmount * stroke->brush.strength;
		float& amount = amounts[i];

		//color will build up as the user scrubs the brush, but slowly; 2% per brush tick is arbitrary
		if( pointAmount<amount ) amount += 0.02f * pointAmount;
		else amount = pointAmount;
		amount = MIN( amount, 1.0f );

		//Special handling for amount values greater than 99% improves the user friendliness;
		//otherwise the brush falloff makes it difficult to paint on a color completely
		if( erase )
			 value = (amount>0.99f? origSel[i]	: Interp( inputSel[i], origSel[i], amount ));
		else value = (amount>0.99f? goalSel[i]	: Interp( inputSel[i], goalSel[i], amount ));
		outputSel[i] = value;

		*pointGather = 0; //reset the amount; allows the accumulation to work properly
	}

	handler->ApplyPaintSel( stroke->data->host );
}

void MeshPaintMgr::PaintStroke_Deform( PaintStrokeContext* stroke ) {
	//-- temp values
	float pointAmount;
	Point3 oldDeformPos, newDeformPos;

	//-- Data initialization
	float* pointGather = stroke->pointGather;
	float* amounts = stroke->data->amounts;
	Point3 *origPos = stroke->data->origPositions, *goalPos = stroke->data->goalPositions;
	Point3 *inputPos = stroke->data->inputPositions;
	Point3 *outputDeform = stroke->data->outputDeform; //*inputDeform = stroke->data->inputDeform, 
	BitArray& selMask = stroke->data->selMask;
	FloatTab& softSelMask = stroke->data->softSelMask;
	//toggle from paint to erase if revert toggle is true; unless we were already erasing
	BOOL erase = ((stroke->brush.ctrlKey && (paintMode!=PAINTMODE_ERASEDEFORM)) ? TRUE:FALSE );	
	BOOL useSoftSel = (softSelMask.Count()>0? TRUE:FALSE);






	Tab<Point3> delta;
	Tab<int> deltaIndex;

	int extraAlloc = stroke->pointGatherCount * 0.2f;
	

	for( int i=0; i<stroke->pointGatherCount; i++, pointGather++ ) {
		if( *pointGather<=0.0f ) continue;
		float selAmount = (selMask[i]? 1.0f : (useSoftSel? softSelMask[i] : 0.0f));
		if( selAmount<=0 ) continue;

		//calculate the painted amount for this brush tick
		pointAmount = stroke->curve->GetValue(0, 1.0f - *pointGather); //FIXME: maybe use lookup table param
		pointAmount = pointAmount * stroke->brush.strength * selAmount;
		//calculate the new, summed amount for the vertex
		float& amount = amounts[i];
		//color will build up as the user scrubs the brush, but slowly; 2% per brush tick is arbitrary
		if( pointAmount<amount ) amount += 0.02f * pointAmount;
		else amount = pointAmount;
		amount = MIN( amount, 1.0f );

		Point3& oldDeformPos = inputPos[i];

		//calculate the new position
		if( erase ) { //move towards the origPos
			if( amount>0.99f )	newDeformPos = origPos[i];
			else				newDeformPos = Interp( oldDeformPos, origPos[i], amount );
		}
		else { //move towards the goalPos
			if( amount>0.99f )	newDeformPos = goalPos[i];
			else				newDeformPos = Interp( oldDeformPos, goalPos[i], amount );
		}

 		Point3 def = newDeformPos - origPos[i];		
		if (def != outputDeform[i])
		{
			stroke->data->mChangedPoints.Set(i,1);
			delta.Append(1,&def,extraAlloc);
			deltaIndex.Append(1,&i,extraAlloc);
		}


		*pointGather = 0; //reset the amount; allows the accumulation to work properly
	}

	//-- Revert the previous deformation before calculating the new one
	handler->RevertPaintDeform( stroke->data->host , &stroke->data->mChangedPoints);

	for (int i = 0; i < delta.Count(); i++)
	{
		int index = deltaIndex[i];
		outputDeform[index] = delta[i];
	}

	//-- Apply the nre deformation
	handler->ApplyPaintDeform( stroke->data->host, &stroke->data->mChangedPoints );
}

void MeshPaintMgr::EndStroke( BOOL accepted ) {
	int count = dataContexts.Count();
	// For paint deformation, call UpdatePointGather() and UpdateBrushTracking()
	// so the gizmo tracks the deformed mesh properly on the next brush stroke

	for( int i=0; i<count; i++ ) {
		if( dataContexts[i]==NULL ) continue;
		if( IsPaintDeformMode(paintMode) )
			UpdatePointGather( dataContexts[i]->node, dataContexts[i]->host );

		if (objectStates[i].obj) {
			PolyObject *pobj = (PolyObject *)objectStates[i].obj;
			MeshPaintHost *host = dataContexts[i]->host;
			MNMesh *latestMesh = handler->GetPaintObject (dataContexts[i]->host);
			pobj->GetMesh().ShallowCopy (latestMesh, PART_GEOM|PART_TOPO);
			pobj->SetChannelLocks( ALL_CHANNELS );
		}

		MeshPaintHandler* handler = dataContexts[i]->handler;
		EndDataContext( dataContexts[i], accepted );
		dataContexts[i] = NULL;
		handler->OnPaintDataRedraw(paintMode);
	}

	if( IsPaintDeformMode(paintMode) )
		UpdateBrushTracking();
}

void* MeshPaintMgr::GetInterface( ULONG id ) {
	if( id==PAINTERCANVASINTERFACE_V5 )		return (IPainterCanvasInterface_V5*)this;
	//if( id==PAINTERCANVASINTERFACE_V5_1 )	return (IPainterCanvasInterface_V5_1*)this;
	if( id==PAINTERCANVASINTERFACE_V7 )		return (IPainterCanvasInterface_V7*)this;
	return ReferenceTarget::GetInterface(id);
}

IPainterInterface_V7 *MeshPaintMgr::GetPainter()
{
	if( this->pPainter!=NULL ) return this->pPainter;

	//else initialize the painter
	ReferenceTarget *painterRef = (ReferenceTarget*) GetCOREInterface()->CreateInstance(REF_TARGET_CLASS_ID,PAINTERINTERFACE_CLASS_ID);	
	IPainterInterface_V7 *pPainter = NULL;
	//set it to the correct verion
	if (painterRef) pPainter = (IPainterInterface_V7*) painterRef->GetInterface( PAINTERINTERFACE_V7 );
	return pPainter;
}

void MeshPaintMgr::OnSystemStartup(void *param, NotifyInfo *info)
{
	IBrushPresetMgr* mgr = GetIBrushPresetMgr();
	if(mgr != NULL)
	{
		PaintDeformPresetContext*	paintDeformContext = GetPaintDeformPresetContext();
		PaintSoftSelPresetContext*	paintSoftSelContext = GetPaintSoftSelPresetContext();

		mgr->RegisterContext( paintDeformContext );
		mgr->RegisterContext( paintSoftSelContext );
	}
	UnRegisterNotification( OnSystemStartup, NULL, NOTIFY_SYSTEM_STARTUP );
}

//-----------------------------------------------------------------------------
//-- Painter Canvas methods
BOOL MeshPaintMgr::StartStroke()
{
	handler->PrePaintStroke(paintMode);
	return TRUE;
}

BOOL MeshPaintMgr::PaintStroke(BOOL hit, IPoint2 mousePos, 
				Point3 worldPoint, Point3 worldNormal,
				Point3 localPoint, Point3 localNormal,
				Point3 bary,  int index,
				BOOL shift, BOOL ctrl, BOOL alt, 
				float radius, float str,
				float pressure, INode *node,
				BOOL mirrorOn,
				Point3 worldMirrorPoint, Point3 worldMirrorNormal,
				Point3 localMirrorPoint, Point3 localMirrorNormal)
{
	PaintBrushContext brush;
	brush.radius = radius, brush.strength = str, brush.pressure = pressure;
	brush.shiftKey = shift, brush.ctrlKey = ctrl, brush.altKey = alt;

	return PaintStroke( node, index, brush );
}

BOOL MeshPaintMgr::EndStroke() {
	EndStroke( TRUE );
	return TRUE;
}

BOOL MeshPaintMgr::EndStroke(int ct, BOOL *hit,IPoint2 *mousePos, 
				Point3 *worldPoint, Point3 *worldNormal,
				Point3 *localPoint, Point3 *localNormal,
				Point3 *bary,  int *index,
				BOOL *shift, BOOL *ctrl, BOOL *alt, 
				float *radius, float *str,
				float *pressure, INode **node,
				BOOL mirrorOn,
				Point3 *worldMirrorPoint, Point3 *worldMirrorNormal,
				Point3 *localMirrorPoint, Point3 *localMirrorNormal)
{
	//FIXME: this doesn't work does it?
	// the loop will probably go one iteration per brush tick, not once per node
	PaintBrushContext brush;

	//calculate average strength for the stroke
	double avgStr=0;
	for( int i=0; i<ct; i++ ) avgStr += str[i];
	avgStr = (ct>0? avgStr/ct : 1.0);

	for( int i=0; i<ct; i++ ) {
		brush.radius = radius[i], brush.strength = (float)avgStr, brush.pressure = pressure[i];
		brush.shiftKey = shift[i], brush.ctrlKey = ctrl[i], brush.altKey = alt[i];

		PaintStroke( node[i], index[i], brush );
	}

	EndStroke( TRUE );
	return TRUE;
}

BOOL MeshPaintMgr::CancelStroke() {
	EndStroke( FALSE );
	return TRUE;
}

BOOL MeshPaintMgr::SystemEndPaintSession() {
	handler->EndPaintMode();
	return TRUE;
}

void MeshPaintMgr::PainterDisplay(TimeValue t, ViewExp *vpt, int flags) {}

void MeshPaintMgr::OnPaintBrushChanged() {
	if( handler!=NULL ) handler->OnPaintBrushChanged();
}


//-----------------------------------------------------------------------------
//--- MeshPaintHandler: to be implemented by any Modifier which supported Paint Mesh
//    see also MeshPaintHost
MeshPaintHandler::MeshPaintHandler() {
	paintSelActive=FALSE;
	paintDeformActive=FALSE;
	painterSettings.Init();
}

MeshPaintHandler::~MeshPaintHandler()
{
	
}

IOResult MeshPaintHandler::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID()) {
		case SEL_ACTIVE_CHUNK:
			res=iload->Read( &paintSelActive, sizeof(BOOL), &nb );
			break;
		case DEFORM_ACTIVE_CHUNK:
			res=iload->Read( &paintDeformActive, sizeof(BOOL), &nb );
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	return IO_OK;
}

IOResult MeshPaintHandler::Save(ISave *isave) {
	IOResult res;
	ULONG nb;
	isave->BeginChunk(SEL_ACTIVE_CHUNK);
	res = isave->Write( &paintSelActive, sizeof(BOOL), &nb);
	isave->EndChunk();

	isave->BeginChunk(DEFORM_ACTIVE_CHUNK);
	res = isave->Write( &paintDeformActive, sizeof(BOOL), &nb);
	isave->EndChunk();
	return IO_OK;
}

void MeshPaintHandler::BeginEditParams() {
	GetMeshPaintMgr()->BeginEditParams(this);
}
void MeshPaintHandler::EndEditParams() {
	if( InPaintMode() ) EndPaintMode();
	GetMeshPaintMgr()->EndEditParams(this);
}

void MeshPaintHandler::ActivatePaintData( PaintModeID paintMode ) {
	if( IsPaintSelMode(paintMode) && (!paintSelActive)) {
		// Activate the "Paint Selection" data
		theHold.Begin();
		FloatTab inputSel;

		FOR_EACH_HOST {
			host->PutPaintRestore(paintMode,node,this, MeshPaintHostRestore::typeActivate);

			GetPaintInputSel( host, inputSel );
			int count = inputSel.Count();

			host->SetPaintSelCount( count );
			float* outputSel = host->GetPaintSelValues();
			if (count && outputSel) memcpy( outputSel, inputSel.Addr(0), count * sizeof(float) );
		}
		// create undo entry for paintSelActive, after undo entries for the hosts
		theHold.Put( new MeshPaintHandlerRestore(this) );
		paintSelActive = TRUE;

		theHold.Accept (GetPaintUndoString (paintMode));
		OnPaintDataActivate(paintMode,TRUE);
	}

	if( IsPaintDeformMode(paintMode) && (!paintDeformActive) ) {
		// Activate the "Paint Deformation" data
		theHold.Begin();

		FOR_EACH_HOST {
			host->PutPaintRestore (paintMode,node,this, MeshPaintHostRestore::typeActivate);

			MNMesh* object = GetPaintObject( host );
			int count = object->VNum();
			host->SetPaintDeformCount( count );
			Point3* outputDeform = host->GetPaintDeformOffsets();

			//set all deformations to Point3(0,0,0)
			Point3 zero(0,0,0);
			for( int i=0; i<count; i++ ) outputDeform[i] = zero;
		}
		// create undo entry for paintSelActive, after undo entries for the hosts
		theHold.Put( new MeshPaintHandlerRestore(this) );
		paintDeformActive = TRUE;

		theHold.Accept (GetPaintUndoString (paintMode));
		OnPaintDataActivate(paintMode,TRUE);
	}
}

void MeshPaintHandler::DeactivatePaintData( PaintModeID paintMode, BOOL accepted ) {
	MeshPaintMgr* mgr = GetMeshPaintMgr();

	if( IsPaintSelMode(paintMode) && (paintSelActive)) {
	//FIXME: no handling for 'accepted' flag
		// De-activate the "Paint Selection" data
		theHold.Begin();
		FOR_EACH_HOST {
			host->PutPaintRestore(paintMode,node,this, MeshPaintHostRestore::typeCommit);
			host->SetPaintSelCount( 0 );
			//FIXME: the paint selection buffer is not de-allocated
		}
		// create undo entry for paintSelActive, after undo entries for the hosts
		theHold.Put( new MeshPaintHandlerRestore(this) );
		paintSelActive = FALSE;
		theHold.Accept( GetPaintUndoString (paintMode));
		OnPaintDataActivate(paintMode,FALSE);
		if( mgr->GetPresetContext() == GetPresetContextID(paintMode) )
			mgr->SetPresetContext( Class_ID(0,0) );
	}

	if( IsPaintDeformMode(paintMode) && (paintDeformActive) ) {
		// De-activate the "Paint Deform" data
		theHold.Begin();
		FOR_EACH_HOST {
			if( accepted )
				host->PutPaintRestore(paintMode,node,this, MeshPaintHostRestore::typeCommit);
			else {
				host->PutPaintRestore(paintMode,node,this, MeshPaintHostRestore::typeCancel);
				RevertPaintDeform( host, NULL );
			}
			host->SetPaintDeformCount( 0 );
			//FIXME: the paint deform buffer is not de-allocated
		}
		// create undo entry for paintSelActive, after undo entries for the hosts
		theHold.Put( new MeshPaintHandlerRestore(this) );
		paintDeformActive = FALSE;
		theHold.Accept( GetPaintUndoString (paintMode));
		if( !accepted ) OnPaintDataRedraw(paintMode);
		OnPaintDataActivate(paintMode,FALSE);
		if( mgr->GetPresetContext() == GetPresetContextID(paintMode) )
			mgr->SetPresetContext( Class_ID(0,0) );
	}
}

BOOL MeshPaintHandler::IsPaintDataActive( PaintModeID paintMode ) {
	if( IsPaintSelMode(paintMode) ) return paintSelActive;
	if( IsPaintDeformMode(paintMode) ) return paintDeformActive;
	return FALSE;
}


void MeshPaintHandler::BeginPaintMode( PaintModeID paintMode ) {
	MeshPaintMgr* mgr = GetMeshPaintMgr();

	PaintModeID prevMode = mgr->GetMode();
	if( paintMode==prevMode )
		mgr->EndSession(paintMode);
	else { //Begin a painting session
		mgr->SetHandler( this );
		mgr->BeginSession( paintMode );
	}
	OnPaintModeChanged( prevMode, paintMode );
}

void MeshPaintHandler::EndPaintMode() {
	MeshPaintMgr* mgr = GetMeshPaintMgr();

	PaintModeID prevMode = mgr->GetMode();
	if( prevMode!=PAINTMODE_OFF ) {
		mgr->EndSession();
		OnPaintModeChanged( prevMode, PAINTMODE_OFF );
	}
}

PaintModeID MeshPaintHandler::GetPaintMode() {return GetMeshPaintMgr()->GetMode();}
BOOL MeshPaintHandler::InPaintMode() {return (GetPaintMode()==PAINTMODE_OFF? FALSE:TRUE);}

float MeshPaintHandler::GetPaintBrushSize()
	{ return GetMeshPaintMgr()->GetBrushSize(); }
void MeshPaintHandler::SetPaintBrushSize( float size )
	{ GetMeshPaintMgr()->SetBrushSize(size); }

float MeshPaintHandler::GetPaintBrushStrength()
	{ return GetMeshPaintMgr()->GetBrushStrength(); }
void MeshPaintHandler::SetPaintBrushStrength( float strength )
	{ GetMeshPaintMgr()->SetBrushStrength(strength); }

//called when brush params change
void MeshPaintHandler::UpdatePaintBrush() {
	PaintModeID paintMode = GetPaintMode();
	GetMeshPaintMgr()->SetPaintValue( GetPaintValue(paintMode) );

	GetPaintDeformPresetContext()->OnMeshPaintUpdated();
	GetPaintSoftSelPresetContext()->OnMeshPaintUpdated();
}

void MeshPaintHandler::UpdatePaintObject( MeshPaintHost* host ) {
	//Resize the selValues of the host, and exit any paint mode,
	//when the host's mesh increase/decreses in size
	MNMesh* object = GetPaintObject( host );
	if (!object) return;
	int numVerts = object->VNum();

	BOOL staleSel		= (IsPaintDataActive(PAINTMODE_SEL)		&& (host->GetPaintSelCount()!=numVerts));
	BOOL staleDeform	= (IsPaintDataActive(PAINTMODE_DEFORM)	&& (host->GetPaintDeformCount()!=numVerts));
	if( InPaintMode() && (staleSel || staleDeform) )
		EndPaintMode();

	if( staleSel )		host->SetPaintSelCount( numVerts );
	if( staleDeform )	host->SetPaintDeformCount( numVerts );
}

//-----------------------------------------------------------------------------
//--- MeshPaintHost
//    see also MeshPaintHandler

MeshPaintHost::MeshPaintHost() {
	paintSelCount = 0;
	paintDeformCount = 0;
}

MeshPaintHost::~MeshPaintHost() {
	// Steve Anderson note - This is not necessary to prevent memory leaks.  Remove this destructor? - 8/29/04
	paintSelBuf.SetCount(0);
	paintDeformBuf.SetCount(0);
}

float* MeshPaintHost::GetPaintSelValues() {return paintSelBuf.Count()<=0? NULL:paintSelBuf.Addr(0);}
int MeshPaintHost::GetPaintSelCount() {return paintSelCount;}

void MeshPaintHost::SetPaintSelCount( int count ) {
	int oldCount = paintSelBuf.Count();
	if( oldCount < count ) {
		paintSelBuf.SetCount(count); //increase count... then set new entries to zero
		for( int i=oldCount; i<count; i++ ) paintSelBuf[i] = 0;
	}
	// Note: we do NOT downsize the buffer when the mesh is downsized.
	// If the user returns the mesh to its original size, we dont' want any values to be lost.
	// Might be better to handle that using undo? ... or maybe not
	paintSelCount = count;
}

Point3* MeshPaintHost::GetPaintDeformOffsets() {return paintDeformBuf.Count()<=0? NULL:paintDeformBuf.Addr(0);}

int MeshPaintHost::GetPaintDeformCount() {return paintDeformCount;}

void MeshPaintHost::SetPaintDeformCount( int count ) {
	static Point3 zero(0,0,0);
	int oldCount = paintDeformBuf.Count();
	if( oldCount < count ) {
		paintDeformBuf.SetCount(count); //increase count... then set new entries to zero
		for( int i=oldCount; i<count; i++ ) paintDeformBuf[i] = zero; //set to Point3(0,0,0);
	}
	// Note: we do NOT downsize the buffer when the mesh is downsized.
	// If the user returns the mesh to its original size, we dont' want any values to be lost.
	// Might be better to handle that using undo? ... or maybe not
	paintDeformCount = count;
}

void MeshPaintHost::UpdatePaintObject( MNMesh* object ) {
	//Currently the only purpose of this function is to resize the selValues of the host,
	//when the host's mesh increase/decreses in size
	if (!object) return;
	int numVerts = object->VNum();

	//FIXME: need a better way to track if paint data is active
	BOOL isPaintSelActive		= (paintSelCount>0);
	BOOL isPaintDeformActive	= (paintDeformCount>0);

	if( isPaintSelActive )		SetPaintSelCount( numVerts );
	if( isPaintDeformActive )	SetPaintDeformCount( numVerts );
}

IOResult MeshPaintHost::Load(ILoad *iload) {
	IOResult res;
	ULONG nb;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID()) {
		case PAINT_SEL_BUF_CHUNK:
			paintSelCount = iload->CurChunkLength() / sizeof(float);
			paintSelBuf.SetCount( paintSelCount );
			res=iload->Read( paintSelBuf.Addr(0), paintSelCount * sizeof(float), &nb );
			break;
		case PAINT_SEL_DEFORM_CHUNK:
			paintDeformCount = iload->CurChunkLength() / sizeof(Point3);
			paintDeformBuf.SetCount( paintDeformCount );
			res=iload->Read( paintDeformBuf.Addr(0), paintDeformCount * sizeof(Point3), &nb );
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	return IO_OK;
}

IOResult MeshPaintHost::Save(ISave *isave) {
	IOResult res;
	ULONG nb;

	if( (paintSelCount>0) ) {
		isave->BeginChunk(PAINT_SEL_BUF_CHUNK);
		res = isave->Write( paintSelBuf.Addr(0), paintSelCount * sizeof(float), &nb);
		isave->EndChunk();
	}

	if( (paintDeformCount>0) ) {
		isave->BeginChunk(PAINT_SEL_DEFORM_CHUNK);
		res = isave->Write( paintDeformBuf.Addr(0), paintDeformCount * sizeof(Point3), &nb);
		isave->EndChunk();
	}
	return IO_OK;
}

MeshPaintHostRestore* MeshPaintHost::CreatePaintRestore( PaintModeID paintMode, INode* node, MeshPaintHandler* handler, int type ) {
	return new MeshPaintHostRestore(paintMode, node, handler, this, type);
}

void MeshPaintHost::DeletePaintRestore( MeshPaintHostRestore* restore ) {
	delete restore;
}

void MeshPaintHost::PutPaintRestore( PaintModeID paintMode, INode* node, MeshPaintHandler* handler, int type ) {
	if( theHold.IsSuspended() ) return;
	MeshPaintHostRestore* restore = CreatePaintRestore(paintMode, node, handler, type);
	theHold.Begin();
	theHold.Put( restore );
	theHold.Accept( restore->GetName() );
}


//-----------------------------------------------------------------------------
//--- MeshPaintHandlerRestore: Undo object for MeshPaintHandler
MeshPaintHandlerRestore::MeshPaintHandlerRestore( MeshPaintHandler* handler ) {
	this->handler = handler;
	this->paintSelActive = handler->paintSelActive;
	this->paintDeformActive = handler->paintDeformActive;
}

void MeshPaintHandlerRestore::Restore(int isUndo) {
	if( paintSelActive != handler->paintSelActive ) {
		if( handler->InPaintMode() ) handler->EndPaintMode();
		memswp( &paintSelActive, &(handler->paintSelActive), sizeof(paintSelActive) );
		handler->OnPaintDataActivate( PAINTMODE_SEL, handler->paintSelActive );
	}
	if( paintDeformActive!=handler->paintDeformActive ) {
		if( handler->InPaintMode() ) handler->EndPaintMode();
		memswp( &paintDeformActive, &(handler->paintDeformActive), sizeof(paintDeformActive) );
		handler->OnPaintDataActivate( PAINTMODE_DEFORM, handler->paintDeformActive );
	}
}

void MeshPaintHandlerRestore::Redo() {
	// Both undo and redo have the same effect; they swap the active values with the restore values
	Restore( FALSE );
}
int  MeshPaintHandlerRestore::Size()
	{return sizeof(MeshPaintHandlerRestore);}

//-----------------------------------------------------------------------------
//--- MeshPaintHostRestore

// NOTE: changes to the sel value count are NOT supported in this undo object

MeshPaintHostRestore::MeshPaintHostRestore( PaintModeID paintMode, INode* node, MeshPaintHandler* handler, MeshPaintHost* host, int type ) {
	this->node = node;
	this->handler = handler;
	this->host = host;
	this->paintMode = paintMode;
	this->type = type;
	paintDeformCount = paintSelCount = 0;

	if( IsPaintSelMode(paintMode) ) {
		paintSelCount = host->GetPaintSelCount();
		int bufCount = host->paintSelBuf.Count();
		paintSelValues.SetCount( bufCount );
		if( bufCount>0 )
			memcpy( paintSelValues.Addr(0), host->paintSelBuf.Addr(0), bufCount * sizeof(float) );
	}
	if( IsPaintDeformMode(paintMode) ) {
		paintDeformCount = host->GetPaintDeformCount();
		int bufCount = host->paintDeformBuf.Count();
		paintDeformValues.SetCount( bufCount );
		if( bufCount>0 )
			memcpy( paintDeformValues.Addr(0), host->paintDeformBuf.Addr(0), bufCount * sizeof(Point3) );
	}
}

MeshPaintHostRestore::~MeshPaintHostRestore() {
	host = NULL;
	paintMode = PAINTMODE_OFF;
	paintSelValues.SetCount(0);
	paintDeformValues.SetCount(0);
}

void MeshPaintHostRestore::Restore(int isUndo) {
	if( host==NULL ) return;
	BOOL shouldRevert	= ( (type==typePaint)  ||  ((!isUndo)&&(type==typeCancel))  ||   ( isUndo &&(type==typeActivate)) );
	BOOL shouldApply	= ( (type==typePaint)  ||   ( isUndo &&(type==typeCancel))  ||  ((!isUndo)&&(type==typeActivate)) ) ;
	BOOL shouldZero		= ( ((isUndo) &&  (type==typeActivate)) ||
						   ((!isUndo) && ((type==typeCancel) || (type==typeCommit))) );

	if( IsPaintSelMode(paintMode) ) {
		// hack... just do a shallow swap from one tab into the other (memswp() because we don't have direct access to the 'TabHd' pointer)
		memswp( &paintSelValues, &(host->paintSelBuf), sizeof( paintSelValues ) );

		// Update the host's count; see bugs 595036 and 604499
		int count = host->paintSelCount;
		if( count>host->paintSelBuf.Count() )		host->SetPaintSelCount(count);

		handler->ApplyPaintSel( host );
	}
	if( IsPaintDeformMode(paintMode) ) {
		if( shouldRevert ) handler->RevertPaintDeform( host, NULL ); // Revert the previous deformations

		// Swap the old and new defomation offset buffers
		// hack... just do a shallow swap from one tab into the other (memswp() because we don't have direct access to the 'TabHd' pointer)
		memswp( &paintDeformValues, &(host->paintDeformBuf), sizeof( paintDeformValues ) );

		// Update the host's count & apply deformations as needed; see bugs 595036 and 604499
		if( shouldZero ) host->SetPaintDeformCount(0);
		else {
			MNMesh* object = handler->GetPaintObject( host );
			host->SetPaintDeformCount( object==NULL? 0 : object->VNum() );
		}

		if( shouldApply ) handler->ApplyPaintDeform( host, NULL ); // Apply the new deformations

		MeshPaintMgr* mgr = GetMeshPaintMgr();
		if( IsPaintDeformMode( mgr->GetMode() ) ) {
			mgr->UpdatePointGather(node,host);
			mgr->UpdateBrushTracking();
		}
	}
	handler->OnPaintDataRedraw( paintMode );
}

void MeshPaintHostRestore::Redo() {
	// Both undo and redo have the same effect; they swap the active values with the restore values
	Restore( FALSE );
}

int MeshPaintHostRestore::Size() {
	return 1; //fake value, since the system would tend to reject the restore objects if it knew the real size
	int size = 0;
	size += (paintSelValues.Count() * sizeof(float));
	size += (paintDeformValues.Count() * sizeof(Point3));
	return size;
}

TCHAR* MeshPaintHostRestore::GetName() {
	return handler->GetPaintUndoString (paintMode);
}

//-----------------------------------------------------------------------------
// class MeshPaintPresetParams

void MeshPaintPresetParams::Init( IBrushPresetContext* context ) {
	this->context = context;
	int numParams = context->GetNumParams();
	toggles.SetSize( numParams );
	toggles.SetAll();
}

Class_ID MeshPaintPresetParams::ContextID()				{return GetContext()->ContextID();}
int MeshPaintPresetParams::GetNumParams()				{return GetContext()->GetNumParams();}
int MeshPaintPresetParams::GetParamID( int paramIndex )	{return GetContext()->GetParamID( paramIndex );}
int MeshPaintPresetParams::GetParamIndex( int paramID )	{return GetContext()->GetParamIndex( paramID );}

BOOL MeshPaintPresetParams::GetParamToggle( int paramID ) {
	int paramIndex = GetParamIndex( paramID );
	return toggles[ paramIndex ];
}
void MeshPaintPresetParams::SetParamToggle( int paramID, BOOL onOff ) {
	int paramIndex = GetParamIndex( paramID );
	toggles.Set( paramIndex, onOff );
}

void MeshPaintPresetParams::SetParent( IBrushPreset* parent ) {
	if( parent!=NULL ) parentID = parent->PresetID();
}


//-----------------------------------------------------------------------------
// class MeshPaintPresetContext

BaseInterface* MeshPaintPresetContext::GetInterface(Interface_ID id) {
	if( id==GetID() )
		return this;
	return NULL;
}

void MeshPaintPresetContext::Init( ParamDesc* paramDescs ) {
	this->paramDescs = paramDescs;
	//Count the parameters
	paramCount = 0;
	ParamDesc* desc = paramDescs;
	while( desc->type!=TYPE_VOID )
	{
		paramCount++;
		desc++;
	}
}

int MeshPaintPresetContext::GetNumParams() {
	return paramCount;
}

int MeshPaintPresetContext::GetParamID( int paramIndex ) {
	// ID numbers can be arbitrary but nonzero; this code uses the range [1..N]
	if( paramIndex<0 || paramIndex>GetNumParams() ) return 0;
	return (paramIndex + 1); //add one to the index to get the ID
}

int MeshPaintPresetContext::GetParamIndex( int paramID ) {
	// ID numbers can be arbitrary but nonzero; this code uses the range [1..N]
	if( paramID<1 || paramID>(GetNumParams()+1) ) return -1;
	return (paramID - 1);  //subtract one from the ID to get the index
}

TCHAR* MeshPaintPresetContext::GetParamName( int paramID ) {
	int index = GetParamIndex(paramID);
	if( index<0 ) return NULL;
	return GetParamDescs()[ index ].name; //subtract one from the ID to get the index
}

ParamType2 MeshPaintPresetContext::GetParamType( int paramID ) {
	int index = GetParamIndex(paramID);
	if( index<0 ) return TYPE_VOID;
	return GetParamDescs()[ index ].type; //subtract one from the ID to get the index
}

void MeshPaintPresetContext::OnMeshPaintUpdated() {
	IBrushPresetMgr* mgr = GetIBrushPresetMgr();
	if( (mgr!=NULL) && (mgr->IsContextActive( ContextID() )) )
		mgr->OnContextUpdated( ContextID() );
}


//-----------------------------------------------------------------------------
// class PaintDeformPresetContext

// Static instance of PaintDeformPresetContext
static PaintDeformPresetContext sPaintDeformPresetContextInstance;

PaintDeformPresetContext::ParamDesc PaintDeformPresetContext::paramDescs[] =
{
	{ _T(""),	(ParamType2)TYPE_INT },
	{ _T(""),	(ParamType2)TYPE_INT },
	{ _T(""),	(ParamType2)TYPE_WORLD },
	{ _T(""),				TYPE_VOID}
};

void PaintDeformPresetContext::LoadParamDescs()
{
	if (strlen(paramDescs[0].name) > 0) return;
	TCHAR buf[32];
	LoadString(hInstance, IDS_MODE, buf, sizeof(buf));
	_tcscpy(paramDescs[0].name, buf);
	LoadString(hInstance, IDS_AXIS, buf, sizeof(buf));
	_tcscpy(paramDescs[1].name, buf);
	LoadString(hInstance, IDS_PUSH_PULL, buf, sizeof(buf));
	_tcscpy(paramDescs[2].name, buf);
}

PaintDeformPresetContext* GetPaintDeformPresetContext() {
	return &sPaintDeformPresetContextInstance;
}

PaintDeformPresetContext::PaintDeformPresetContext()
	: MeshPaintPresetContext (
		PAINTDEFORMPRESETCONTEXT_INTERFACE_ID, _T("PaintDeformPresetContext"), 0, NULL, FP_CORE | FP_TEST_INTERFACE,
		end
	)
{
	LoadParamDescs();
	Init( paramDescs );
}

TCHAR* PaintDeformPresetContext::GetContextName() {
	return _T("PaintDeform"); //Internal use; do not localize
}

IBrushPresetParams* PaintDeformPresetContext::CreateParams() {
	PaintDeformPresetParams* retval = new PaintDeformPresetParams;
	retval->Init( this );
	return retval;
}

void PaintDeformPresetContext::DeleteParams( IBrushPresetParams* params ) {
	if( (params==NULL) || (params->ContextID()!=ContextID()) )
		return; //invalid params
	delete (PaintDeformPresetParams*)params;
}


//-----------------------------------------------------------------------------
// class PaintSoftSelPresetContext

// Static instance of PaintSoftSelPresetContext
static PaintSoftSelPresetContext sPaintSoftSelPresetContextInstance;

PaintSoftSelPresetContext::ParamDesc PaintSoftSelPresetContext::paramDescs[] =
{
	{ _T(""),	(ParamType2)TYPE_INT },
	{ _T(""),	(ParamType2)TYPE_FLOAT },
	{_T(""),				TYPE_VOID}
};


void PaintSoftSelPresetContext::LoadParamDescs()
{
	if (strlen(paramDescs[0].name) > 0) return;
	TCHAR buf[32];
	LoadString(hInstance, IDS_MODE, buf, sizeof(buf));
	_tcscpy(paramDescs[0].name, buf);
	LoadString(hInstance, IDS_SELECTION_VALUE, buf, sizeof(buf));
	_tcscpy(paramDescs[1].name, buf);
}

PaintSoftSelPresetContext* GetPaintSoftSelPresetContext() {
	return &sPaintSoftSelPresetContextInstance;
}

PaintSoftSelPresetContext::PaintSoftSelPresetContext()
	: MeshPaintPresetContext(
		PAINTSOFTSELPRESETCONTEXT_INTERFACE_ID, _T("PaintSoftSelPresetContext"), 0, NULL, FP_CORE,
		end
	)
{
	LoadParamDescs();
	Init( paramDescs );
}

TCHAR* PaintSoftSelPresetContext::GetContextName() {
	return _T("PaintSoftSel"); //Internal use; do not localize
}

IBrushPresetParams* PaintSoftSelPresetContext::CreateParams() {
	PaintSoftSelPresetParams* retval = new PaintSoftSelPresetParams;
	retval->Init( this );
	return retval;
}

void PaintSoftSelPresetContext::DeleteParams( IBrushPresetParams* params ) {
	if( (params==NULL) || (params->ContextID()!=ContextID()) )
		return; //invalid params
	delete (PaintSoftSelPresetParams*)params;
}


//-----------------------------------------------------------------------------
// class PaintDeformPresetParams

PaintDeformPresetParams::PaintDeformPresetParams() {
	context = NULL;
	mode = PAINTMODE_OFF;
	axis = PAINTAXIS_NONE;
	value = 0.0f;
}

void PaintDeformPresetParams::ApplyParams() {
	MeshPaintMgr* mgr = GetMeshPaintMgr();
	if( (mgr==NULL)  ||  (!IsPaintDeformMode( mode )) )
		return;

	//FIXME: not respecting the parameter toggles
	MeshPaintHandler* handler = mgr->GetHandler();	
	if( mode!=mgr->GetMode() )
		handler->BeginPaintMode( mode );
	if( mode==PAINTMODE_PAINTPUSHPULL ) {
		handler->SetPaintAxis( mode, axis );
		handler->SetPaintValue( mode, value );
		mgr->SetPaintValue( value );
	}
}

void PaintDeformPresetParams::FetchParams() {
	MeshPaintMgr* mgr = GetMeshPaintMgr();
	if( (mgr==NULL)  ||  (!IsPaintDeformMode( mgr->GetMode() )) )
		return; // Must be in deform mode to fetch deform params

	MeshPaintHandler* handler = mgr->GetHandler();	
	mode = mgr->GetMode();
	if( mode==PAINTMODE_PAINTPUSHPULL ) {
		axis = handler->GetPaintAxis( mode );	
		value = handler->GetPaintValue( mode );	
	}
}

FPValue PaintDeformPresetParams::GetParamValue( int paramID ) {
	FPValue val;
	val.type = GetContext()->GetParamType(paramID);

	switch( paramID ) {
	case 1:		val.i = (int)mode;				break;
	case 2:		val.i = (int)axis;				break;
	case 3:		val.f = value;					break;
	}
	return val;
}

void PaintDeformPresetParams::SetParamValue( int paramID, FPValue val ) {
	FPValue rootVal = EssentialValue(val);
	if( rootVal.type != EssentialType(GetContext()->GetParamType(paramID)) )
		return;
	switch( paramID ) {
	case 1:		mode = (PaintModeID)rootVal.i;		break;
	case 2:		axis = (PaintAxisID)rootVal.i;		break;
	case 3:		value = rootVal.f;					break;
	}
}

FPValue PaintDeformPresetParams::GetDisplayParamValue( int paramID ) {
	TCHAR buf[256], *text=buf;
	buf[0] = 0;

	switch( paramID ) {
	case 1:
		if( mode==PAINTMODE_PAINTPUSHPULL )		text = GetString( IDS_PAINTDEFORM_PUSHPULL );
		else if( mode==PAINTMODE_PAINTRELAX )	text = GetString( IDS_PAINTDEFORM_RELAX );
		else if( mode==PAINTMODE_ERASEDEFORM )	text = GetString( IDS_PAINTDEFORM_ERASE );
		break;
	case 2:
		if( mode==PAINTMODE_PAINTPUSHPULL ) {
			if( axis==PAINTAXIS_DEFORMEDNORMS )	text = GetString( IDS_PAINTDEFORM_AXIS_DEFORMEDNORMS );
			else if( axis==PAINTAXIS_ORIGNORMS ) text = GetString( IDS_PAINTDEFORM_AXIS_ORIGNORMS );
			else if( axis==PAINTAXIS_XFORM_X )	text = GetString( IDS_PAINTDEFORM_AXIS_XFORM_X );
			else if( axis==PAINTAXIS_XFORM_Y )	text = GetString( IDS_PAINTDEFORM_AXIS_XFORM_Y );
			else if( axis==PAINTAXIS_XFORM_Z )	text = GetString( IDS_PAINTDEFORM_AXIS_XFORM_Z );
		}
		break;
	case 3:
		if( mode==PAINTMODE_PAINTPUSHPULL )		_stprintf( buf, "%s", FormatUniverseValue(value) );
		break;
	}

	FPValue val;
	val.type = TYPE_TSTR_BV;
	val.tstr = new TSTR(text);
	return val;
}


//-----------------------------------------------------------------------------
// class PaintSoftSelPresetParams

PaintSoftSelPresetParams::PaintSoftSelPresetParams() {
	this->context = context;
	mode = PAINTMODE_OFF;
	value = 0.0f;
}

void PaintSoftSelPresetParams::ApplyParams() {
	MeshPaintMgr* mgr = GetMeshPaintMgr();
	if( (mgr==NULL)  ||  (!IsPaintSelMode( mode )) )
		return;

	MeshPaintHandler* handler = mgr->GetHandler();	
	if( mode!=mgr->GetMode() ) handler->BeginPaintMode( mode );
	if( mode==PAINTMODE_PAINTSEL ) {
		mgr->SetPaintValue( value );
		handler->SetPaintValue( mode, value );	
	}
}

void PaintSoftSelPresetParams::FetchParams() {
	MeshPaintMgr* mgr = GetMeshPaintMgr();
	if( (mgr==NULL)  ||  (!IsPaintSelMode( mgr->GetMode() )) )
		return; // Must be in soft sel mode to fetch soft sel params

	MeshPaintHandler* handler = mgr->GetHandler();	
	mode = mgr->GetMode();
	if( mode==PAINTMODE_PAINTSEL )
		value = handler->GetPaintValue( mode );	
}

FPValue PaintSoftSelPresetParams::GetParamValue( int paramID ) {
	FPValue val;
	val.type = GetContext()->GetParamType(paramID);

	switch( paramID ) {
	case 1:		val.i = (int)mode;				break;
	case 2:		val.f = value;					break;
	}
	return val;
}

void PaintSoftSelPresetParams::SetParamValue( int paramID, FPValue val ) {
	if( val.type != GetContext()->GetParamType(paramID) )
		return;
	switch( paramID ) {
	case 1:		mode = (PaintModeID)val.i;		break;
	case 2:		value = val.f;					break;
	}
}

FPValue PaintSoftSelPresetParams::GetDisplayParamValue( int paramID ) {
	TCHAR buf[256], *text=buf;
	buf[0] = 0;

	switch( paramID ) {
	case 1:
		if( mode==PAINTMODE_PAINTSEL )			text = GetString( IDS_PAINTSEL_PAINT );
		else if( mode==PAINTMODE_ERASESEL )		text = GetString( IDS_PAINTSEL_ERASE );
		else if( mode==PAINTMODE_BLURSEL )		text = GetString( IDS_PAINTSEL_BLUR );
		break;
	case 2:
		if( mode==PAINTMODE_PAINTSEL )			_stprintf( buf, "%f", value );
		break;
	}

	FPValue val;
	val.type = TYPE_TSTR_BV;
	val.tstr = new TSTR(text);
	return val;
}
