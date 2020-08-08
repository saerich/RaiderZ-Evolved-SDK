/**********************************************************************
 *<
	FILE: PolyPaint.h

	DESCRIPTION:   Paint Soft Selection; Editable Polygon Mesh Object

	CREATED BY: Michaelson Britt

	HISTORY: created May 2004

 *>	Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/

#ifndef __POLYPAINT_H__
#define __POLYPAINT_H__

#include "Max.h"
#include "notify.h"
#include "IPainterInterface.h"
#include "IBrushPresets.h"
#include "resource.h"

class MeshPaintManager;
class MeshPaintHandler;
class MeshPaintHost;
class MeshPaintHostRestore;
typedef Tab<float>	FloatTab;
typedef Tab<Point3>	Point3Tab;

TCHAR *GetString(int id);


//-----------------------------------------------------------------------------
//-- Helper classes

// Note this helper is duplicated in multiple projects.  Any code changes should be applied to all copies.
class ResourceString
{
	public:
		int strID;
		TCHAR strBuf[MAX_PATH];
		ResourceString( int strID ) { this->strID = strID, this->strBuf[0] = 0; }

		TCHAR* GetString()
		{
			if( strBuf[0]==0 ) LoadString(hInstance, strID, strBuf, sizeof(strBuf));
			return strBuf;
		}
};

//-----------------------------------------------------------------------------
//--- Data types

enum PaintModeID
{
	PAINTMODE_UNDEF    = -1,
	PAINTMODE_OFF	   = 0 ,
	PAINTMODE_PAINTSEL     , 
	PAINTMODE_ERASESEL     , 
	PAINTMODE_BLURSEL      ,
	PAINTMODE_PAINTPUSHPULL, 
	PAINTMODE_PAINTRELAX   , 
	PAINTMODE_ERASEDEFORM  ,
	//Paint mode categories, used with Activate/DeactivatePaintData()
	PAINTMODE_SEL,    //paint and erase selection modes
	PAINTMODE_DEFORM, //push-pull, relax, and erase deform modes
};

inline BOOL IsPaintSelMode( PaintModeID paintMode ) { // TRUE if the mode is a "Paint Selection" mode, FALSE otherwise
	return ( ((paintMode==PAINTMODE_SEL) || (paintMode==PAINTMODE_PAINTSEL) ||
			  (paintMode==PAINTMODE_ERASESEL) || (paintMode==PAINTMODE_BLURSEL) ) ? TRUE : FALSE );
}
inline BOOL IsPaintSelMode( int paintMode ) {return IsPaintSelMode((PaintModeID)paintMode);}

inline BOOL IsPaintDeformMode( PaintModeID paintMode ) { // TRUE if the mode is a "Paint Deform" mode, FALSE otherwise
	return ( ((paintMode==PAINTMODE_DEFORM) || (paintMode==PAINTMODE_PAINTPUSHPULL) ||
			  (paintMode==PAINTMODE_PAINTRELAX) || (paintMode==PAINTMODE_ERASEDEFORM)) ? TRUE : FALSE );
}
inline BOOL IsPaintDeformMode( int paintMode ) {return IsPaintDeformMode((PaintModeID)paintMode);}

enum PaintAxisID
{
	PAINTAXIS_NONE   =0,
	PAINTAXIS_ORIGNORMS,
	PAINTAXIS_DEFORMEDNORMS,
	PAINTAXIS_XFORM_X,
	PAINTAXIS_XFORM_Y,
	PAINTAXIS_XFORM_Z
};

inline BOOL IsPaintXFormAxis( PaintAxisID paintAxis ) { // TRUE if the axis is xform_x, xform_y or xform_z
	return ((paintAxis==PAINTAXIS_XFORM_X) || (paintAxis==PAINTAXIS_XFORM_Y) || (paintAxis==PAINTAXIS_XFORM_Z)?
			TRUE:FALSE);
}

//! Information about one set of paint data (one object) during a paint stroke
struct PaintDataContext 
{
	PaintModeID				paintMode;
	INode*					node;
	MNMesh*					object;

	MeshPaintHandler*		handler;
	MeshPaintHost*			host;
	MeshPaintHostRestore*	restore;	//to undo the changes of this context
	float*					amounts;	//accumulation buffer; used in any paint mode
	BOOL					inverted;	//set to TRUE after values are inverted due to the ALT key

	// Paint Selection data
	int						paintSelCount;
	float*					origSel;
	float*					goalSel;
	float*					inputSel;
	float*					outputSel;
	// Paint Deformation data
	int						paintDeformCount;
	Point3*					origPositions;
	Point3*					goalPositions;
	Point3*					inputPositions;
	Point3*					outputDeform;
	BitArray				selMask;
	FloatTab				softSelMask;

	BitArray				mChangedPoints;	//watje this is used to track only the points that changed to make operations faster
};

struct PaintBrushContext
{
	BOOL					shiftKey, ctrlKey, altKey;
	float					radius, strength, pressure;
};

// Information about one increment along a paint stroke;
// superset of PaintDataContext
struct PaintStrokeContext
{
	PaintDataContext*		data;
	PaintBrushContext		brush;
	int						pointGatherCount;
	float*					pointGather;
	ICurve*					curve;
};

//-----------------------------------------------------------------------------
//--- PainterSettings: save and restore settings of the painter interface

class PainterSettings {
	public:
		BOOL				drawTrace, drawRing, drawNormal, pressureEnable;
		int					pressureAffects;
		float				brushMinSize, brushMaxSize, brushMinStrength, brushMaxStrength;

		void				Init();
		void				Get( IPainterInterface_V5* painter );
		void				Set( IPainterInterface_V5* painter );
};


//-----------------------------------------------------------------------------
//--- MeshPaintHandler: to be implemented by any Modifier (or object) which supports Paint Mesh
//    see also MeshPaintHost
class __declspec(dllexport) MeshPaintHandler {
	public:
		//-- Implemented by the client
		virtual void		PrePaintStroke (PaintModeID paintMode)=0;	// prepare to receive paint stroke.

		virtual void		GetPaintHosts( Tab<MeshPaintHost*>& hosts, Tab<INode*>& nodes ) = 0;
		virtual float		GetPaintValue( PaintModeID paintMode ) = 0;
		virtual void		SetPaintValue( PaintModeID paintMode, float value ) = 0;
		virtual PaintAxisID GetPaintAxis( PaintModeID paintMode ) = 0;
		virtual void		SetPaintAxis( PaintModeID paintMode, PaintAxisID axis ) = 0;
		// per-host data...
		virtual MNMesh*		GetPaintObject( MeshPaintHost* host ) = 0; //FIXME: switch to ObjectWrapper
		virtual void		GetPaintInputSel( MeshPaintHost* host, FloatTab& selValues ) = 0;
		virtual void		GetPaintInputVertPos( MeshPaintHost* host, Point3Tab& vertPos ) = 0;
		virtual void		GetPaintMask( MeshPaintHost* host, BitArray& sel, FloatTab& softSel ) = 0;
		// paint actions
		virtual void		ApplyPaintSel( MeshPaintHost* host ) = 0;
		virtual void		ApplyPaintDeform( MeshPaintHost* host, BitArray *partialRevert) = 0;
		virtual void		RevertPaintDeform( MeshPaintHost* host, BitArray *partialRevert) = 0;
		// notifications
		virtual void		OnPaintModeChanged( PaintModeID prevMode, PaintModeID curMode ) = 0; //when the mode changes
		virtual void		OnPaintDataActivate( PaintModeID paintMode, BOOL onOff ) = 0; //when the data becomes active/unactive
		virtual void		OnPaintDataRedraw( PaintModeID prevMode ) = 0; //to redraw the screen; called after OnPaintDataChanged()
		virtual void		OnPaintBrushChanged() = 0; //when the brush changes

		// Names for restore objects - undo "Paint Soft Selection", "Paint Deformation", etc - should be localized.
		virtual TCHAR *GetPaintUndoString (PaintModeID mode) = 0;


		//-- Implemented by the system
		MeshPaintHandler();
		virtual ~MeshPaintHandler();
		IOResult			Load(ILoad *iload);
		IOResult			Save(ISave *isave);
		void				BeginEditParams();
		void				EndEditParams();

		void				BeginPaintMode( PaintModeID paintMode );
		void				EndPaintMode();
		PaintModeID		GetPaintMode();
		BOOL				InPaintMode();

		float			GetPaintBrushSize();
		void				SetPaintBrushSize( float size );
		float			GetPaintBrushStrength();
		void				SetPaintBrushStrength( float strength );

		void				UpdatePaintBrush(); //call when brush params change
		void				UpdatePaintObject( MeshPaintHost* host ); //call when the object topology changes

		// Activate the paint data before painting, to allocated the buffers
		void				ActivatePaintData( PaintModeID paintMode );
		void				DeactivatePaintData( PaintModeID paintMode, BOOL accepted=TRUE );
		BOOL				IsPaintDataActive( PaintModeID paintMode );

	protected:
		friend class MeshPaintHandlerRestore;
		friend class MeshPaintMgr;
		PainterSettings*	GetPainterSettings() {return &painterSettings;}
		void				SetPainterSettings( PainterSettings& painterSettings ) {this->painterSettings=painterSettings;}

		// load/save chunk enums
		enum
		{	
			SEL_ACTIVE_CHUNK,
			DEFORM_ACTIVE_CHUNK
		};

		BOOL				paintSelActive, paintDeformActive;
		PainterSettings		painterSettings;
};

//-----------------------------------------------------------------------------
//--- MeshPaintHost: to be implemented by any ModData (or object) which supports Paint Mesh
//    see also MeshPaintHandler
template class __declspec(dllexport) Tab<float>; // This declaration prevents warning C4251
template class __declspec(dllexport) Tab<Point3>; // This declaration prevents warning C4251

class __declspec(dllexport) MeshPaintHost {
	public:
		//-- Implemented by the system
		MeshPaintHost();
		~MeshPaintHost();
		IOResult			Load(ILoad *iload);
		IOResult			Save(ISave *isave);

		float*			GetPaintSelValues();
		int				GetPaintSelCount();
		void			SetPaintSelCount( int count );

		Point3*			GetPaintDeformOffsets();
		int				GetPaintDeformCount();
		void			SetPaintDeformCount( int count );

		void			UpdatePaintObject( MNMesh* object );

	protected:
		friend class MeshPaintMgr;
		friend class MeshPaintHandler;
		friend class MeshPaintHostRestore;

		// used internally
		MeshPaintHostRestore* CreatePaintRestore( PaintModeID mode, INode* node, MeshPaintHandler* handler, int type );
		void				DeletePaintRestore( MeshPaintHostRestore* restore );
		void				PutPaintRestore( PaintModeID mode, INode* node, MeshPaintHandler* handler, int type );


		enum {PAINT_SEL_BUF_CHUNK, PAINT_SEL_DEFORM_CHUNK}; // load/save chunk enums
		// buffer may hold more values than are actively used; store a separate usage count
		int				paintSelCount, paintDeformCount;
		Tab<float>			paintSelBuf; //paint selection values
		Tab<Point3>			paintDeformBuf; //paint deformation offset values
};

//-----------------------------------------------------------------------------
//--- MeshPaintHandlerRestore: Undo object for MeshPaintHandler
class MeshPaintHandlerRestore : public RestoreObj {
	public:	
		MeshPaintHandlerRestore( MeshPaintHandler* handler );
		void				Restore(int isUndo);
		void				Redo();
		int					Size();
	protected:
		MeshPaintHandler*	handler;
		BOOL				paintSelActive, paintDeformActive;
};

//-----------------------------------------------------------------------------
//--- MeshPaintHostRestore: Undo object for MeshPaintHost

class MeshPaintHostRestore : public RestoreObj {
	public:	
		enum {typeActivate, typePaint, typeCommit, typeCancel };

		MeshPaintHostRestore( PaintModeID paintMode, INode* node, MeshPaintHandler* handler, MeshPaintHost* host, int type );
		~MeshPaintHostRestore();
		void				Restore(int isUndo);
		void				Redo();
		int					Size();

		TCHAR*				GetName(); //GetString(IDS_RESTOREOBJ_MESHPAINTSEL)
		FloatTab&			GetPaintSel() {return paintSelValues;}
		Point3Tab&			GetPaintDeform() {return paintDeformValues;}
	protected:
		INode*				node;
		MeshPaintHost*		host;
		MeshPaintHandler*	handler;
		PaintModeID			paintMode;
		int					paintSelCount, paintDeformCount;
		FloatTab			paintSelValues;
		Point3Tab			paintDeformValues;
		int 				type;
};


//-----------------------------------------------------------------------------
//--- IMeshPaintMgr: Published interface for MeshPaintMgr
#define IMESHPAINTMGR_INTERFACE_ID	Interface_ID(0x529e1a5f, 0x71046e8a)
class IMeshPaintMgr : public FPStaticInterface
{
	public:
		//-- From FPInterface
		DECLARE_DESCRIPTOR( IMeshPaintMgr );
		BaseInterface*		GetInterface(Interface_ID id);
};


//-----------------------------------------------------------------------------
//--- MeshPaintMgr: runs paint sessions for MeshPaint handlers and their hosts
// Implements the Singleton pattern.
class MeshPaintMgr :
	public IMeshPaintMgr,
	public IPainterCanvasInterface_V5,
	public IPainterCanvasInterface_V7,
	public ReferenceTarget // Must derive from ReferenceTarget to be a PainterCanvas
{
	private:
		MeshPaintMgr();
		~MeshPaintMgr();
		static MeshPaintMgr* mInstance;
	public:
		static MeshPaintMgr* GetInstance();
		static void		 DestroyInstance();

		Class_ID		ClassID() { return Class_ID(0x61b34126, 0x47ba33e9); }
		void				DeleteThis();
		void				Init();
		void				Free();

		// Accessor methods
		MeshPaintHandler*	GetHandler();
		void				SetHandler( MeshPaintHandler* handler );

		float				GetPaintValue();
		void				SetPaintValue( float f );
		float				GetBrushSize();
		void				SetBrushSize( float f );
		float				GetBrushStrength();
		void				SetBrushStrength( float f );


		// Mode-related methods
		void				BeginEditParams( MeshPaintHandler* handler );
		void				EndEditParams( MeshPaintHandler* handler );

		PaintModeID			GetMode() {return paintMode;}
		void				BeginSession( PaintModeID paintMode );
		void				EndSession( PaintModeID paintMode = PAINTMODE_UNDEF );
		BOOL				InSession() {return (GetMode()!=PAINTMODE_OFF);}

		Class_ID			GetPresetContext() {return presetContextID;}
		void				SetPresetContext( Class_ID presetContextID );

		// Display Painter Interface options
		void				BringUpOptions() {if(pPainter!=NULL) pPainter->BringUpOptions();}

	//-- From ReferenceTarget
		void*				GetInterface( ULONG id );
		RefResult			NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message)
							{return REF_SUCCEED;}


		//-- Painter Canvas methods
		BOOL				StartStroke();
		BOOL				PaintStroke(BOOL hit, IPoint2 mousePos, 
								Point3 worldPoint, Point3 worldNormal,
								Point3 localPoint, Point3 localNormal,
								Point3 bary,  int index,
								BOOL shift, BOOL ctrl, BOOL alt, 
								float radius, float str,
								float pressure, INode *node,
								BOOL mirrorOn,
								Point3 worldMirrorPoint, Point3 worldMirrorNormal,
								Point3 localMirrorPoint, Point3 localMirrorNormal);
		BOOL				EndStroke();
		BOOL				EndStroke(int ct, BOOL *hit,IPoint2 *mousePos, 
								Point3 *worldPoint, Point3 *worldNormal,
								Point3 *localPoint, Point3 *localNormal,
								Point3 *bary,  int *index,
								BOOL *shift, BOOL *ctrl, BOOL *alt, 
								float *radius, float *str,
								float *pressure, INode **node,
								BOOL mirrorOn,
								Point3 *worldMirrorPoint, Point3 *worldMirrorNormal,
								Point3 *localMirrorPoint, Point3 *localMirrorNormal);
		BOOL				CancelStroke();
		BOOL				SystemEndPaintSession();
		void				PainterDisplay(TimeValue t, ViewExp *vpt, int flags);

		//void				CanvasStartPaint() {}	//FIXME: use suspend/resume
		//void				CanvasEndPaint() {}		//FIXME: use suspend/resume

		void				OnPaintBrushChanged();

	protected:
		friend class MeshPaintHostRestore;

		// Helper methods
		PaintDataContext*	BeginDataContext( PaintModeID paintMode, INode* node, MeshPaintHandler* handler, MeshPaintHost* host );
		void				EndDataContext( PaintDataContext* data, BOOL accepted );

		void				InitPaintSel( PaintDataContext* data );			//init the goal values
		void				InitPaintDeform( PaintDataContext* data );		//init the goal positions
		void				UpdatePaintDeform( PaintDataContext* data );	//update the goal positions, mid-stroke
		void				GetPaintInputVertNorm( PaintDataContext* data, Point3Tab& inputNorms );

		void				UpdatePointGather( INode* node, MeshPaintHost* host );
		void				UpdateBrushTracking();

		BOOL				PaintStroke( INode *node, int index, PaintBrushContext brush );
		void				PaintStroke_Sel( PaintStrokeContext* stroke );
		void				PaintStroke_SelBlur( PaintStrokeContext* stroke );
		void				PaintStroke_Deform( PaintStrokeContext* stroke );
		void				EndStroke( BOOL accepted );

		IPainterInterface_V7 *pPainter;
		IPainterInterface_V7* GetPainter();

		PaintModeID			paintMode;
		float				paintSelValue,paintDeformValue;
		Class_ID			presetContextID;

		// Cached values per paint stroke
		MeshPaintHandler*	handler;
		Tab<INode*>			nodes;
		Tab<MeshPaintHost*>	hosts;
		Tab<ObjectState>	objectStates;
		Tab<PaintDataContext*> dataContexts;
		PainterSettings		painterSettingsPrev;

		//-- Static methods
		static void			OnSystemStartup(void *param, NotifyInfo *info);
};

__declspec(dllexport) MeshPaintMgr* GetMeshPaintMgr();


//-----------------------------------------------------------------------------
// class MeshPaintPresetContext
// Superclass for PaintDeformPresetContext and PaintSoftSelPresetContext

#define PAINTDEFORMPRESETCONTEXT_INTERFACE_ID	Interface_ID(0x36764f94, 0x56397b3e)
#define PAINTDEFORMPRESETCONTEXT_ID				Class_ID(0,0x36764f94)

#define PAINTSOFTSELPRESETCONTEXT_INTERFACE_ID	Interface_ID(0x4b70352b, 0x9454285)
#define PAINTSOFTSELPRESETCONTEXT_ID			Class_ID(0,0x4b70352b)

inline Class_ID GetPresetContextID( PaintModeID mode ) {
	if( IsPaintDeformMode(mode) )	return PAINTDEFORMPRESETCONTEXT_ID;
	if( IsPaintSelMode(mode) )		return PAINTSOFTSELPRESETCONTEXT_ID;
	return Class_ID(0,0);
}


class MeshPaintPresetContext : public IBrushPresetContext, public FPStaticInterface {
	public:
		struct ParamDesc 
		{
			TCHAR			internalName[MAX_PATH];
			ResourceString	displayName;
			ParamType2		type;
		};
		//-- From FPInterface
		DECLARE_DESCRIPTOR( MeshPaintPresetContext );
		BaseInterface*		GetInterface(Interface_ID id);

		void				Init( ParamDesc* paramDescs );
		int					GetNumParams();
		int					GetParamID( int paramindex );
		int					GetParamIndex( int paramID );
		MCHAR*				GetParamName( int paramID );
		ParamType2			GetParamType( int paramID );

		MCHAR*				GetDisplayParamName( int paramID );

		Class_ID			PluginClassID() {return EPOLYOBJ_CLASS_ID;}
		SClass_ID			PluginSuperClassID() {return GEOMOBJECT_CLASS_ID;}

		void				OnMeshPaintUpdated();

	protected:
		int					paramCount;
		virtual ParamDesc*	GetParamDescs() = 0;
};


//-----------------------------------------------------------------------------
// class MeshPaintPresetParams
// Superclass for PaintDeformPresetParams and PaintSoftSelPresetParams

class MeshPaintPresetParams : public IBrushPresetParams  {
	public:
		void				Init( IBrushPresetContext* context );
		Class_ID			ContextID();

		int					GetNumParams();
		int					GetParamID( int paramIndex );
		int					GetParamIndex( int paramID );
		BOOL				GetParamToggle( int paramID );
		void				SetParamToggle( int paramID, BOOL onOff );

		void				SetParent( IBrushPreset* parent );

	protected:
		int					parentID;
		BitArray			toggles;

		IBrushPresetContext* context;
		IBrushPresetContext* GetContext() {return context;}
};


//-----------------------------------------------------------------------------
// class PaintDeformPresetContext

class PaintDeformPresetContext : public MeshPaintPresetContext
{
	public:
		PaintDeformPresetContext();
		Class_ID			ContextID() { return PAINTDEFORMPRESETCONTEXT_ID; }
		TCHAR*				GetContextName();
		IBrushPresetParams* CreateParams();
		void				DeleteParams( IBrushPresetParams* params );

	protected:
		ParamDesc*			GetParamDescs();
};

PaintDeformPresetContext* GetPaintDeformPresetContext();


//-----------------------------------------------------------------------------
// class PaintSoftSelPresetContext

class PaintSoftSelPresetContext : public MeshPaintPresetContext
{
	public:
		PaintSoftSelPresetContext();
		Class_ID			ContextID() { return PAINTSOFTSELPRESETCONTEXT_ID; }
		TCHAR*				GetContextName();
		IBrushPresetParams* CreateParams();
		void				DeleteParams( IBrushPresetParams* params );

	protected:
		ParamDesc*			GetParamDescs();
};

PaintSoftSelPresetContext* GetPaintSoftSelPresetContext();


//-----------------------------------------------------------------------------
// class PaintDeformPresetParams

class PaintDeformPresetParams : public MeshPaintPresetParams  {
	public:
		PaintDeformPresetParams();
		void				ApplyParams();
		void				FetchParams();
		FPValue				GetParamValue( int paramID );
		void				SetParamValue( int paramID, FPValue val );

		FPValue				GetDisplayParamValue( int paramID );

	protected:
		PaintModeID			mode;
		PaintAxisID			axis;
		float				value;
};


//-----------------------------------------------------------------------------
// class PaintSoftSelPresetParams

class PaintSoftSelPresetParams : public MeshPaintPresetParams  {
	public:
		PaintSoftSelPresetParams();
		void				ApplyParams();
		void				FetchParams();
		FPValue				GetParamValue( int paramID );
		void				SetParamValue( int paramID, FPValue val );

		FPValue				GetDisplayParamValue( int paramID );

	protected:
		PaintModeID			mode;
		float				value;
};


#endif //__POLYPAINT_H__
