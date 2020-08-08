/*==============================================================================
Copyright 2009 Autodesk, Inc.  All rights reserved. 

This computer source code and related instructions and comments are the unpublished
confidential and proprietary information of Autodesk, Inc. and are protected under 
applicable copyright and trade secret law.  They may not be disclosed to, copied
or used by any third party without the prior written consent of Autodesk, Inc.

//**************************************************************************/

#include "IGrip.h"

#ifndef __POLY_GRIPS_H
#define __POLY_GRIPS_H

class EditPolyObject;

//icon defines

#define STR_ICON_DIR					_T("Grips\\")
#define BEVEL_TYPE_PNG					_T("Bevel Type.png")
#define BEVEL_GROUP_PNG					_T("Group.png")
#define BEVEL_NORMAL_PNG				_T("Normal.png")
#define BEVEL_POLYGON_PNG				_T("Polygon.png")
#define BEVEL_HEIGHT_PNG				_T("Height.png")
#define BEVEL_OUTLINE_PNG				_T("Outline Amount.png")
#define EXTRUDE_TYPE_PNG				_T("Extrude Type.png")
#define EXTRUDE_GROUP_PNG				_T("Group.png")
#define EXTRUDE_NORMAL_PNG				_T("Normal.png")
#define EXTRUDE_POLYGON_PNG				_T("Polygon.png")
#define EXTRUDE_HEIGHT_PNG				_T("Height.png")
#define EXTRUDE_WIDTH_PNG				_T("Width.png")
#define OUTLINE_AMOUNT_PNG				_T("Outline Amount.png")
#define INSET_TYPE_PNG                  _T("Inset Type.png")
#define INSET_AMOUNT_PNG				_T("Inset Amount.png")
#define INSET_GROUP_PNG					_T("Group.png")
#define INSET_POLYGON_PNG				_T("Polygon.png")
#define CONNECT_EDGE_SEGMENTS_PNG      _T("segments.png")
#define CONNECT_EDGE_PINCH_PNG            _T("segmentPinch.png")
#define CONNECT_EDGE_SLIDE_PNG             _T("segmentSlide.png")
#define CHAMFER_VERTICES_AMOUNT_PNG    _T("Value.png")
#define CHAMFER_VERTICES_OPEN_PNG        _T("hole.png")
#define CHAMFER_EDGE_AMOUNT_PNG         _T("Value.png")
#define CHAMFER_EDGE_SEGMENT_PNG        _T("segments.png")
#define CHAMFER_EDGE_OPEN_PNG              _T("hole.png")
#define BRIDGE_BORDER_TYPE_PNG                _T("byEdge.png")
#define BRIDGE_POLYGON_TYPE_PNG                _T("byPolygon.png")
#define BRIDGE_EDGE_TYPE_PNG                _T("byEdge.png")
#define BRIDGE_SELECTED_PNG          _T("selected.png")
#define BRIDGE_SPECIFIC_PNG           _T("specific.png")
#define BRIDGE_SEGMENTS_PNG        _T("segments.png")
#define BRIDGE_TAPER_PNG              _T("taper.png")
#define BRIDGE_TWIST1_PNG             _T("twist1.png")
#define BRIDGE_TWIST2_PNG             _T("twist2.png")
#define BRIDGE_BIAS_PNG                _T("bias.png")
#define BRIDGE_SMOOTH_PNG          _T("smoothMesh.png")
#define BRIDGE_ADJACENT_PNG        _T("bridgeAdjacent_16.png")
#define BRIDGE_REVERSE_TRI_PNG    _T("reverseTri.png")
#define BRIDGE_EDGES_PICK1_PNG          _T("BridgeEdgePick1.png")
#define BRIDGE_EDGES_PICK2_PNG           _T("BridgeEdgePick2.png")
#define BRIDGE_POLYGONS_PICK1_PNG		_T("BridgePolygonPick1.png")
#define BRIDGE_POLYGONS_PICK2_PNG		_T("BridgePolygonPick2.png")
#define BRIDGE_BORDERS_PICK1_PNG		_T("BridgeBorderPick1.png")
#define BRIDGE_BORDERS_PICK2_PNG		_T("BridgeBorderPick2.png")
#define RELAX_AMOUNT_PNG       _T("Value.png")
#define RELAX_ITERATIONS_PNG   _T("relaxIteration.png")
#define RELAX_BOUNDARY_PNG             _T("holdBoundary.png")
#define RELAX_OUTER_PNG             _T("holdOuterPoints.png")
#define HINGE_FROM_EDGE_ANGLE_PNG    _T("angle_16.png")
#define HINGE_FROM_EDGE_SEGMENTS_PNG    _T("segments.png")
#define HINGE_PICK_PNG    _T("HingePick.png")
#define EXTRUDE_ALONG_SPLINE_SEGMENTS   _T("segments.png")
#define EXTRUDE_ALONG_SPLINE_TAPER_AMOUNT _T("Value.png")
#define EXTRUDE_ALONG_SPLINE_TAPER_CURVE    _T("taperCurve.png")
#define EXTRUDE_ALONG_SPLINE_TWIST              _T("twist.png")
#define EXTRUDE_ALONG_SPLINE_ROTATION         _T("angle_16.png")
#define EXTRUDE_ALONG_SPLINE_PICK			  _T("ExtrudeAlongSplinePick.png")
#define EXTRUDE_ALONG_SPLINE_TO_FACE_NORMAL_PNG         _T("localNormal.png")
#define MSMOOTH_SMOOTHNESS_PNG                  _T("smoothMesh.png")
#define MSMOOTH_SMOOTHING_GROUPS_PNG                  _T("smoothGroups.png")
#define MSMOOTH_SMOOTH_MATERIALS_PNG                  _T("smoothMaterials.png")
#define TESSELLATE_EDGE_PNG                           _T("byEdge.png")
#define  TESSELLATE_FACE_PNG                          _T("byPolygon.png")
#define  TESSELLATE_TYPE_PNG                          _T("Group.png")
#define  TESSELLATE_TENSION_PNG                     _T("Value.png")
#define WELD_THRESHOLD_PNG                      _T("Value.png")
#define PICK_EMPTY_PNG						_T("PickButton Empty.png")
#define PICK_FULL_PNG						_T("PickButton Full.png")
#define FALLOFF_PNG							_T("falloff.png")
#define BUBBLE_PNG							_T("bubble.png")
#define PINCH_PNG							_T("pinch.png")
#define SETFLOW_PNG							_T("setFlow.png")
#define LOOPSHIFT_PNG							_T("loopshift.png")
#define RINGSHIFT_PNG							_T("ringShift.png")
#define EDGECREASE_PNG                        _T("edgeCrease.png")
#define EDGEWEIGHT_PNG                      _T("EdgeWeight.png")
#define VERTEXWEIGHT_PNG                  _T("vertexWeight.png")

class EditablePolyManipulatorGrip:public IBaseGrip
{
public:
	enum OurGrips
	{
		eFalloff = 0, ePinch , eBubble , eSetFlow , eLoopShift ,
		eRingShift , eEdgeCrease , eEdgeWeight , eVertexWeight 
	};

	EditablePolyManipulatorGrip():mpEditPolyObject(NULL){};
	virtual ~EditablePolyManipulatorGrip(){};
	void Init(EditPolyObject *pEdPolyObj){ mpEditPolyObject = pEdPolyObj ; }
	EditPolyObject*  GetEditPolyObject(){ return mpEditPolyObject ;}
	void SetEditPolyObject(EditPolyObject* pEdPolyObj){ mpEditPolyObject = pEdPolyObj ; }
	void SetUpUI(){ GetIGripManager()->ResetAllUI(); }  // called after the grip is enabled to set up init states that are called via IGripManager
	void SetUpVisibility(BitArray &manipMask); // should be called after it's enabled , we will reset everything

	void SetFalloff(bool val){ mSSFalloff = val ; ResetGrip(); }
	void SetBubble(bool val){ mSSBubble = val ; ResetGrip(); }
	void SetPinch( bool val){ mSSPinch = val ; ResetGrip(); }
	void SetSetFlow( bool val){ mSetFlow = val ; ResetGrip(); }
	void SetLoopShift( bool val){ mLoopShift = val ; ResetGrip(); }
	void SetRingShift( bool val){ mRingShift = val ; ResetGrip(); }
	void SetEdgeCrease( bool val){ mEdgeCrease = val ; ResetGrip(); }
	void SetEdgeWeight( bool val){ mEdgeWeight = val ; ResetGrip(); }
	void SetVertexWeight( bool val){ mVertexWeight = val ; ResetGrip(); }

    void ResetGrip();

	// from IBaseGrip
	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);
	bool SupportsOkayApplyCancel(){ return false; } // from IBaseGrip, we don't support ok , apply , cancel buttons

    int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which, TSTR &string);
	bool GetResolvedIconName(int which, TSTR &string);
	DWORD GetCustomization( int which );
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string){ return false; }

	bool GetValue(int which, TimeValue t, IBaseGrip::GripValue &value);
	bool SetValue(int which, TimeValue t, IBaseGrip::GripValue &value);

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which,float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t, int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);

private:
	EditPolyObject   *mpEditPolyObject;
	bool mSSFalloff;
	bool mSSBubble;
	bool mSSPinch;
	bool mSetFlow;
	bool mLoopShift;
	bool mRingShift;
	bool mEdgeCrease;
	bool mEdgeWeight;
	bool mVertexWeight;

};

class EditablePolyGrip:public IBaseGrip
{
public:
	EditablePolyGrip():mpEditPoly(NULL),mPolyID(0){};
	virtual ~EditablePolyGrip(){};
	virtual void Init(EditPolyObject *m,int polyID){mpEditPoly = m; mPolyID = polyID;}
	EditPolyObject* GetEditPolyObj() { return mpEditPoly; }
	void SetEditPolyObj( EditPolyObject* pEdPolyObj){ mpEditPoly = pEdPolyObj; }
	int  GetPolyID() { return mPolyID; }
	void SetPolyID( int iPolyID){ mPolyID = iPolyID; }
	virtual void SetUpUI(){GetIGripManager()->ResetAllUI();}  //called after the grip is enabled to set up init states that are called via IGripManager

	bool SupportsOkayApplyCancel(){return true;} //from IBaseGrip, we support ok,apply, cancel buttons
private:
	EditPolyObject						*mpEditPoly;
	int									      mPolyID;
};

class BevelGrip :public EditablePolyGrip
{
public:

	enum OurGrips{eBevelType = 0, eBevelHeight, eBevelOutline};

	BevelGrip(){};
	BevelGrip(EditPolyObject *m, int polyID);
	~BevelGrip();

	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);
	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);
	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);

};

class ExtrudeFaceGrip :public EditablePolyGrip
{
public:

	enum OurGrips{eExtrudeFaceType = 0, eExtrudeFaceHeight};

	ExtrudeFaceGrip(){};
	ExtrudeFaceGrip(EditPolyObject *m, int polyID);
	~ExtrudeFaceGrip();
	
	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);
	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);

	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);

};

class ExtrudeEdgeGrip :public EditablePolyGrip
{
public:

	enum OurGrips{ eExtrudeEdgeHeight =0 , eExtrudeEdgeWidth};

	ExtrudeEdgeGrip(){};
	ExtrudeEdgeGrip(EditPolyObject *m, int polyID);
	~ExtrudeEdgeGrip();
	
	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);
	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);

	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);

};

class ExtrudeVertexGrip :public EditablePolyGrip
{
public:

	enum OurGrips{ eExtrudeVertexHeight =0 , eExtrudeVertexWidth};

	ExtrudeVertexGrip(){};
	ExtrudeVertexGrip(EditPolyObject *m, int polyID);
	~ExtrudeVertexGrip();

	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);
	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);

	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);

};

class OutlineGrip :public EditablePolyGrip
{
public:

	enum OurGrips{eOutlineAmount = 0};

	OutlineGrip(){};
	OutlineGrip(EditPolyObject *m, int polyID);
	~OutlineGrip();
	
	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);
	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);
	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 

	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);
	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);

};

class InsetGrip :public EditablePolyGrip
{
public:

	enum OurGrips{eInsetType = 0, eInsetAmount};

	InsetGrip(){};
	InsetGrip(EditPolyObject *m, int polyID);
	~InsetGrip();

	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);
	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);
	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);

};

class ConnectEdgeGrip :public EditablePolyGrip
{
public:

	enum OurGrips{eConnectEdgeSegments = 0, eConnectEdgePinch, eConnectEdgeSlide};

	ConnectEdgeGrip(){};
	ConnectEdgeGrip(EditPolyObject *m, int polyID);
	~ConnectEdgeGrip();

	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);

	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);
	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);

};

class ChamferVerticesGrip :public EditablePolyGrip
{
public:

	enum OurGrips{eChamferVerticesAmount = 0, eChamferVerticesOpen};

	ChamferVerticesGrip(){};
	ChamferVerticesGrip(EditPolyObject *m, int polyID);
	~ChamferVerticesGrip();

	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);

	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);
	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);

};

class ChamferEdgeGrip :public EditablePolyGrip
{
public:

	enum OurGrips{eChamferEdgeAmount = 0, eChamferEdgeSegments, eChamferEdgeOpen};

	ChamferEdgeGrip(){};
	ChamferEdgeGrip(EditPolyObject *m, int polyID);
	~ChamferEdgeGrip();

	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);
	
	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);
	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);

};

class BridgePolygonGrip :public EditablePolyGrip
{
public:

	enum OurGrips{
		eBridgePolygonsSegments = 0,
		eBridgePolygonsTaper,
		eBridgePolygonsBias,
		eBridgePolygonsSmooth,
		eBridgePolygonsTwist1,
		eBridgePolygonsTwist2,
		eBridgePolygonsType,
		eBridgePolygonsPick1,
		eBridgePolygonsPick2
	};

	BridgePolygonGrip(){};
	BridgePolygonGrip(EditPolyObject *m, int polyID);
	~BridgePolygonGrip();

	//need to activate grips
	void SetUpUI(); //called to set up init states, also should get called when a paremeter changes to handle undos

	void SetPoly1PickModeStarted();
	void SetPoly1Picked(int num);
	void SetPoly1PickDisabled();
	void SetPoly2PickModeStarted();
	void SetPoly2Picked(int num);
	void SetPoly2PickDisabled();

	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);
	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);
	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);
private:
	MSTR mPoly1Picked;
	MSTR mPoly2Picked;
	MSTR mPick1Icon;
	MSTR mPick2Icon;
};

class BridgeBorderGrip :public EditablePolyGrip
{
public:

	enum OurGrips{
		eBridgeBordersSegments = 0,
		eBridgeBordersTaper,
		eBridgeBordersBias,
		eBridgeBordersSmooth,
		eBridgeBordersTwist1,
		eBridgeBordersTwist2,
		eBridgeBordersType,
		eBridgeBordersPick1,
		eBridgeBordersPick2
	};

	BridgeBorderGrip(){};
	BridgeBorderGrip(EditPolyObject *m, int polyID);
	~BridgeBorderGrip();

	//need to activate grips
	void SetUpUI(); //called to set up init states, also should get called when a paremeter changes to handle undos

	
	void SetEdge1PickModeStarted();
	void SetEdge1Picked(int edge);
	void SetEdge1PickDisabled();
	void SetEdge2PickModeStarted();
	void SetEdge2Picked(int edge);
	void SetEdge2PickDisabled();

	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);
	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);
	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);

private:
	MSTR mEdge1Picked;
	MSTR mEdge2Picked;
	MSTR mPick1Icon;
	MSTR mPick2Icon;

};

class BridgeEdgeGrip :public EditablePolyGrip
{
public:

	enum OurGrips{
		eBridgeEdgesSegments = 0,
		eBridgeEdgesSmooth,
		eBridgeEdgesAdjacent,
		eBridgeEdgesReverseTriangulation,
		eBridgeEdgesType,
		eBridgeEdgesPick1,
		eBridgeEdgesPick2
	};

	BridgeEdgeGrip(){};
	BridgeEdgeGrip(EditPolyObject *m, int polyID);
	~BridgeEdgeGrip();

	//need to activate grips
	void SetUpUI(); //called to set up init states, also should get called when a paremeter changes to handle undos

	void SetEdge1PickModeStarted();
	void SetEdge1Picked(int edge);
	void SetEdge1PickDisabled();
	void SetEdge2PickModeStarted();
	void SetEdge2Picked(int edge);
	void SetEdge2PickDisabled();

	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);
	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);
	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);

private:
	MSTR mEdge1Picked;
	MSTR mEdge2Picked;
	MSTR mPick1Icon;
	MSTR mPick2Icon;


};

class RelaxGrip :public EditablePolyGrip
{
public:

	enum OurGrips{eRelaxAmount = 0, eRelaxIterations, eRelaxBoundaryPoints,eRelaxOuterPoints};

	RelaxGrip(){};
	RelaxGrip(EditPolyObject *m, int polyID);
	~RelaxGrip();

	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);
	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);
	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);

};


class HingeGrip :public EditablePolyGrip
{
public:

	enum OurGrips{eHingeAngle = 0, eHingeSegments, eHingePick};

	HingeGrip():mbEdgePicked(false){};
	HingeGrip(EditPolyObject *m, int polyID);
	~HingeGrip();
	
	void SetUpUI();

	void SetEdgePickModeStarted();
	void SetEdgePicked(int edge);
	void SetEdgePickDisabled();

	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);
	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);
	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);

private:
	MSTR mEdgePicked;
	MSTR mPickIcon;
	bool  mbEdgePicked;

};

class ExtrudeAlongSplineGrip :public EditablePolyGrip
{
public:

	enum OurGrips{eExtrudeAlongSplineSegments = 0, eExtrudeAlongSplineTaperAmount, eExtrudeAlongSplineTaperCurve,
	                      eExtrudeAlongSplineTwist, eExtrudeAlongSplineAlignToFaceNormal, eExtrudeAlongSplineRotation,eExtrudeAlongSplinePickSpline};

	ExtrudeAlongSplineGrip(){};
	ExtrudeAlongSplineGrip(EditPolyObject *m, int polyID);
	~ExtrudeAlongSplineGrip();

	void SetSplinePickModeStarted();
	void SetSplinePicked(INode *node);
	void SetSplinePickDisabled();

	//need to activate grips
	void SetUpUI(); //called to set up init states, also should get called when a paremeter changes to handle undos

	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);
	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);
	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);
private:
	MSTR mSplinePicked;
	MSTR mPickIcon;

};

class WeldVerticesGrip :public EditablePolyGrip
{
public:

	enum OurGrips{eWeldVerticesWeldThreshold = 0, eWeldVerticesNum};

	WeldVerticesGrip::WeldVerticesGrip() : mStrBefore(""), mStrAfter(""){}

	WeldVerticesGrip(EditPolyObject *m, int polyID);
	~WeldVerticesGrip();

	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);
	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);
	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);

	//set by UpdatePreviewMesh
	void SetNumVerts(int before,int after);
private:
	MSTR mStrBefore;
	MSTR mStrAfter;

};

class WeldEdgeGrip :public EditablePolyGrip
{
public:

	enum OurGrips{eWeldEdgeWeldThreshold = 0, eWeldEdgeNum};

	WeldEdgeGrip::WeldEdgeGrip() : mStrBefore("      "), mStrAfter("      "){}

	WeldEdgeGrip(EditPolyObject *m, int polyID);
	~WeldEdgeGrip();

	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);
	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);
	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);

	//set by UpdatePreviewMesh
	void SetNumVerts(int before,int after);

private:
	MSTR mStrBefore;
	MSTR mStrAfter;

};

class MSmoothGrip :public EditablePolyGrip
{
public:

	enum OurGrips{eMSmoothSmoothness = 0, eMSmoothSmoothingGroups, eMSmoothMaterials};

	MSmoothGrip(){};
	MSmoothGrip(EditPolyObject *m, int polyID);
	~MSmoothGrip();

	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);
	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);
	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);

};

class TessellateGrip :public EditablePolyGrip
{
public:

	enum OurGrips{eTessellateType = 0, eTessellateTension };

	TessellateGrip(){};
	TessellateGrip(EditPolyObject *m, int polyID);
	~TessellateGrip();

	//need to activate grips
	void SetUpUI(); //called to set up init states, also should get called when a paremeter changes to handle undos

	void Okay(TimeValue t);
	void Cancel();
	void Apply(TimeValue t);
	int GetNumGripItems();
	IBaseGrip::Type GetType(int which);
	void GetGripName(TSTR &string);
	bool GetText(int which,TSTR &string);
	bool GetResolvedIconName(int which,MSTR &string);
	DWORD GetCustomization(int which);
	bool GetComboOptions(int which, Tab<IBaseGrip::ComboLabel*> &radioOptions);
	bool GetCommandIcon(int which, MSTR &string);
	bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value);
	bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value); 

	bool GetAutoScale(int which);
	bool GetScaleInViewSpace(int which, float &depth);
	bool GetScale(int which, IBaseGrip::GripValue &scaleValue);
	bool GetResetValue(int which,IBaseGrip::GripValue &resetValue);
	bool StartSetValue(int which,TimeValue t);
	bool EndSetValue(int which,TimeValue t,bool accepted);
	bool ResetValue(TimeValue t,int which);
	bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange);
	bool ShowKeyBrackets(int which,TimeValue t);

};






#endif
