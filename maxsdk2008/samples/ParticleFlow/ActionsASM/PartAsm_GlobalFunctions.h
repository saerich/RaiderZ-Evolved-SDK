/**********************************************************************
 *<
	FILE:			PartAsm_GlobalFunctions.h

	DESCRIPTION:	Collection of useful functions (declaration).
											 
	CREATED FOR:	Oleg Bayborodin

	HISTORY:		created 2003-05-17

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PARTASM_GLOBALFUNCTIONS_H_
#define _PARTASM_GLOBALFUNCTIONS_H_

//#include "iparamb2.h"

#include "RandGenerator.h"
#include "PreciseTimeValue.h"

namespace PartASM {

TCHAR* GetString(int id);
BOOL IsControlAnimated(Control* control);
Point3 RandSphereSurface(RandGenerator* randGen);
Point3 RandSphereVolume(RandGenerator* randGen);

//ClassDesc* GetPFOperatorDisplayDesc();
//ClassDesc* GetPFOperatorRenderDesc();
//ClassDesc* GetPFOperatorSimpleBirthDesc();

//ClassDesc* GetPFOperatorSimpleBirthStateDesc();
//ClassDesc* GetPFOperatorSimplePositionStateDesc();
//ClassDesc* GetPFOperatorInstanceShapeStateDesc();

ClassDesc* GetParticleChannelMultDesc();

ClassDesc* GetPAOperatorDisplayDesc();
ClassDesc* GetPAOperatorValueDesc();
ClassDesc* GetPAOperatorCopyDesc();
ClassDesc* GetPAOperatorFunctionDesc();
ClassDesc* GetPAOperatorRandomDesc();
ClassDesc* GetPAOperatorConditionDesc();
ClassDesc* GetPAOperatorObjectDesc();
ClassDesc* GetPATestDesc();


TCHAR* FormatUniverseVectorValue(Point3 p);
TCHAR* FormatFloatValue(float f, int decimalPrecision);
TCHAR* FormatFloatVectorValue(Point3 p, int decimalPrecision);

enum {	kLocationType_pivot = 0,
		kLocationType_vertex,
		kLocationType_edge,
		kLocationType_surface,
		kLocationType_volume,
		kLocationType_selVertex,
		kLocationType_selEdge,
		kLocationType_selFaces,
		kLocationType_undefined = -1
};

struct PFProbabilityNode
{
	int faceIndex;
	float probValue;
	float accumValue;
	bool useEdge[3];
	float edgeLength[3];
};

class PFProbabilityData
{
public:
	PFProbabilityData();
	void Init(Mesh* mesh, Matrix3* tm, int probabilityType, TimeValue time);
	TimeValue GetTime() const { return time(); }
	float GetTotalShare() const;
	bool GetFace(float randValue, int& faceIndex) const;
	bool GetEdge(float randValue, int& faceIndex, int& edgeIndex) const;
	bool GetVertex(float randValue, int& vertexIndex) const;

protected:
	// const access to class members
	int								probabilityType()	const { return m_probabilityType; }
	TimeValue						time()				const { return m_time; }
	float							totalShare()		const { return m_totalShare; }
	const PFProbabilityNode&		probNode(int i)		const { return m_probNodes[i]; }
	const Tab<PFProbabilityNode>&	probNodes()			const { return m_probNodes; }
	const int						selVertex(int i)	const { return m_selVertices[i]; }
	const Tab<int>&					selVertices()		const { return m_selVertices; }

	// access to class members
	int&					_probabilityType()	{ return m_probabilityType; }
	TimeValue&				_time()				{ return m_time; }
	float&					_totalShare()		{ return m_totalShare; }
	PFProbabilityNode&		_probNode(int i)	{ return m_probNodes[i]; }
	Tab<PFProbabilityNode>&	_probNodes()		{ return m_probNodes; }
	int&					_selVertex(int i)	{ return m_selVertices[i]; }
	Tab<int>&				_selVertices()		{ return m_selVertices; }

private:
	int m_probabilityType;
	TimeValue m_time;
	float m_totalShare;
	Tab<PFProbabilityNode> m_probNodes;
	Tab<int> m_selVertices; // indices of selected vertices (for _selVertex type only)
};

class PFNodeGeometry
{
public:
	PFNodeGeometry();
	~PFNodeGeometry();

	void Init(INode* node, int animatedShape, int subframeSampling, int locationType, RandGenerator* randGen);
	float GetProbabilityShare(TimeValue time);
	bool GetLocationPoint(PreciseTimeValue t, Point3& worldLocation, Point3& localLocation, int& pointIndex,
							bool newPoint=false, float surfaceOffset=0.0f, int attemptsNum=1);
	bool GetClosestPoint(PreciseTimeValue t, const Point3& toPoint, Point3& worldLocation, Point3& localLocation, int& faceIndex);
	bool GetClosestVertex(PreciseTimeValue t, const Point3& toPoint, Point3& worldLocation, int& vertexIndex);
	INode* GetNode() const { return node(); } 
	Mesh* GetMesh(TimeValue time);
	Matrix3* GetTM(TimeValue time);
	Point3* GetNormals(TimeValue time);

protected:
	PFProbabilityData* GetProbData(TimeValue time);
	bool EnsureTimeInterval(TimeValue time);
	bool SetStartTime(TimeValue time);

	// const access to class members
	INode* node() const { return m_node; }
	TimeValue timeStart() const { return m_timeStart; }
	int animatedShape() const { return m_animatedShape; }
	int subframeSampling() const { return m_subframeSampling; }
	int locationType() const { return m_locationType; }
	RandGenerator* randGen() const { return m_randGen; }
	Mesh* mesh(int i) const { return m_meshes[i]; }
	const Tab<Mesh*>& meshes() const { return m_meshes; }
	Point3* normals(int i) const { return m_normals[i]; }
	const Tab<Point3*>& normals() const { return m_normals; }
	Matrix3* tm(int i) const { return m_tm[i]; }
	const Tab<Matrix3*>& tm() const { return m_tm; }
	PFProbabilityData* probData(int i) const { return m_probData[i]; }
	const Tab<PFProbabilityData*>& probData() const { return m_probData; }
	int frameSize() const { return m_frameSize; }

	// access to class members
	INode*& _node() { return m_node; }
	TimeValue& _timeStart() { return m_timeStart; }
	int& _animatedShape() { return m_animatedShape; }
	int& _subframeSampling() { return m_subframeSampling; }
	int& _locationType() { return m_locationType; }
	RandGenerator*& _randGen() { return m_randGen; }
	Mesh*& _mesh(int i) { return m_meshes[i]; }
	Tab<Mesh*>& _meshes() { return m_meshes; }
	Point3*& _normals(int i) { return m_normals[i]; }
	Tab<Point3*>& _normals() { return m_normals; }
	Matrix3*& _tm(int i) { return m_tm[i]; }
	Tab<Matrix3*>& _tm() { return m_tm; }
	PFProbabilityData*& _probData(int i) { return m_probData[i]; }
	Tab<PFProbabilityData*>& _probData() { return m_probData; }
	int& _frameSize() { return m_frameSize; }

private:
	INode* m_node;
	TimeValue m_timeStart;
	int m_animatedShape, m_subframeSampling, m_locationType;
	RandGenerator* m_randGen;
	Tab<Mesh*> m_meshes;
	Tab<Point3*> m_normals;
	Tab<Matrix3*> m_tm;
	Tab<PFProbabilityData*> m_probData;
	int m_frameSize;
};

struct LocationPoint
{
	INode* node;
	Point3 location;
	int pointIndex;
	bool init;
};


BOOL IsBadFace(Mesh* mesh, int index);
float FaceArea(Mesh* mesh, int faceIndex);
float FaceVolume(Mesh* mesh, int faceIndex, Point3 origin);
float MeshVolume(Mesh* mesh);

//
// Shade Context: passed into Mtls and Texmaps to calculate various characteristics for a given point in object space
//
class PFObjectMaterialShadeContext : public ShadeContext {
	public:
	PFNodeGeometry* nodeGeom;
	LocationPoint locationPoint;
	Point3 worldLocation;
	Point3 localLocation;
	TimeValue currentTime;
	Point3 viewVec;
	Point3 normal;
	Point3 gNormal;

	PFObjectMaterialShadeContext();
	void Init(PFNodeGeometry* nodeG, LocationPoint& locP, Point3& worldL, TimeValue t);

	BOOL InMtlEditor() { return FALSE; }	// is this rendering the mtl editor sample sphere?
	LightDesc* Light(int n) { return NULL; }	// get the nth light: no lights
	TimeValue CurTime() { return currentTime; }   	// current time value
	INode *Node();
	int FaceNumber();
	Point3 Normal() { return normal; }	 // interpolated surface normal, in cameara coords: affected by SetNormal()
	Point3 GNormal() { return gNormal; } // geometric (face) normal
	Point3 V() { return viewVec; }	// Unit view vector: from camera towards P 
	void SetView(Point3 p) { ; }	// Set the view vector
	Point3 ReflectVector() { return Point3::XAxis; } // reflection vector
	Point3 RefractVector(float ior) { return -Point3::XAxis; }	// refraction vector
	Point3 CamPos() { return Point3(0.0f, -1000000.0f, 0.0f); }	// camera position
	Point3 P() { return worldLocation; } // point to be shaded;
	Point3 DP() { return Point3(1.0f, 1.0f, 1.0f); } // deriv of P, relative to pixel, for AA
	Point3 PObj() { return localLocation; }	// point in obj coords
	Point3 DPObj() { return Point3(1.0f, 1.0f, 1.0f); }	// deriv of PObj, rel to pixel, for AA
	Box3 ObjectBox(); 	  	// Object extents box in obj coords
	Point3 PObjRelBox();	  	// Point rel to obj box [-1 .. +1 ] 
	Point3 DPObjRelBox() { return Point3(1.0f, 1.0f, 1.0f); };	// deriv of Point rel to obj box [-1 .. +1 ] 
	void ScreenUV(Point2& uv, Point2 &duv) {uv = duv = Point2(0.0, 0.0); } // screen relative uv (from lower left)
	IPoint2 ScreenCoord() { return IPoint2(0, 0); } // integer screen coordinate (from upper left)

	Point3 UVW(int channel);  			// return UVW coords for point
	Point3 DUVW(int channel) { return Point3::Origin; }	// return UVW derivs for point
	void DPdUVW(Point3 dP[3],int channel) { dP[0] = dP[1] = dP[2] = Point3::Origin; }; // Bump vectors for UVW (camera space)

	Point3 UVWNormal(int channel) { return Point3(0,0,1); }  
	void GetBGColor(Color &bgcol, Color& transp, BOOL fogBG=TRUE) { ; }   // returns Background color, bg transparency

	// Transform to and from internal space
	Point3 PointTo(const Point3& p, RefFrame ito); 
	Point3 PointFrom(const Point3& p, RefFrame ifrom); 
	Point3 VectorTo(const Point3& p, RefFrame ito); 
	Point3 VectorFrom(const Point3& p, RefFrame ifrom); 
};




} // end of namespace PartASM


#endif // _PARTASM_GLOBALFUNCTIONS_H_