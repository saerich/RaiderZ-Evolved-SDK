//#define DEFLECTOBJECT_CLASS_ID		0xe5242
//#define DEFLECTMOD_CLASS_ID			0xe5243
#define DEFLECTOBJECT_CLASSID Class_ID(DEFLECTOBJECT_CLASS_ID,0)
#define PLANARSPAWNDEF_CLASSID Class_ID(0x4e94628d, 0x4e437774)
#define SSPAWNDEFL_CLASSID Class_ID(0x656107ca, 0x1f284a6f)
#define USPAWNDEFL_CLASSID Class_ID(0x19fd4916,0x557f71d9)
#define SPHEREDEF_CLASSID Class_ID(0x6cbd289d, 0x3fef6656)
#define UNIDEF_CLASSID Class_ID(0x28497b68, 0)
#define PFOLLOW_CLASSID Class_ID(0x7ab83ab5, 0x5e1d34bd)

#define DEFLECTMOD_CLASSID Class_ID(DEFLECTMOD_CLASS_ID,0)
#define PLANARSPAWNDEFMOD_CLASSID Class_ID(0x7d257b98, 0x439e09de)
#define SSPAWNDEFLMOD_CLASSID Class_ID(0x72a61178, 0x21b407d9)
#define USPAWNDEFLMOD_CLASSID Class_ID(0x36350a51,0x5073041f)
#define SPHEREDEFMOD_CLASSID Class_ID(0x5cdf4181, 0x4c5b42f9)
#define UNIDEFMOD_CLASSID Class_ID(0x4d456b2d, 0)
#define PFOLLOWMOD_CLASSID Class_ID(0x263e723d, 0x132724e5)

const Point3 Zero=Point3(0.0f,0.0f,0.0f); 

class VNormal;

class ICollisionObject : public CollisionObject {
	public:		
		SimpleWSMObject *obj;
		INode *node;
		BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt, int index, float *ct,BOOL UpdatePastCollide);
		Object *GetSWObject();
	};

class IDeflectMod : public SimpleWSMMod {
	public:				
		ICollisionObject deflect;
	};

// DEFLECTOBJECT_CLASSID
class DeflectorField : public CollisionObject {
	public:		
		SimpleWSMObject *obj;
		INode *node;
		Matrix3 tm, invtm;
		Interval tmValid;		
		BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt, int index, float *ct,BOOL UpdatePastCollide);
		Object *GetSWObject();
	};

// PLANARSPAWNDEF_CLASS_ID
class PSpawnDeflObj : public SimpleWSMObject {	
	public:		
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
					
		int lastrnd;
		TimeValue t;
	};

class PSpawnDeflField : public CollisionObject {
	public:		
		PSpawnDeflObj *obj;
		INode *node;
		Matrix3 tm,invtm,tp;
		Interval tmValid;		
		Point3 Vc,Vcp;
		BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt, int index,float *ct,BOOL UpdatePastCollide);
		Object *GetSWObject();
	};

// SSPAWNDEFL_CLASS_ID
class SSpawnDeflObject : public SimpleWSMObject {	
	public:		
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
					
		int lastrnd;
		TimeValue t;
	};
class SSpawnDeflField : public CollisionObject {
	public:		
		SSpawnDeflObject *obj;
		INode *node;
		Matrix3 tm, invtm,tp;
		Interval tmValid;
		Point3 Vc,Vcp;
		BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt, int index,float *ct,BOOL UpdatePastCollide);
		Object *GetSWObject();
	};

// USPAWNDEFL_CLASS_ID
class USpawnDeflObject : public SimpleWSMObject {	
	public:		
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
		static HWND hParams;
					
		INode *custnode;
		int lastrnd;
		TimeValue t;
		TSTR custname;
		USpawnDeflObject();
		~USpawnDeflObject();
		BOOL SupportsDynamics() {return FALSE;}
		Mesh *dmesh;
		int nv,nf;
		VNormal *vnorms;
		Point3 *fnorms;
		Matrix3 tm,ptm,invtm,tmNoTrans,invtmNoTrans;
		Interval tmValid,mValid;
		Point3 dvel;
	};
class USpawnDeflField : public CollisionObject {
	public:		
		USpawnDeflObject *obj;
		INode *node;
		int badmesh;
		BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt, int index,float *ct,BOOL UpdatePastCollide);
		Object *GetSWObject();
	};

// SPHEREDEF_CLASS_ID
class SphereDefObject : public SimpleWSMObject {	
	public:		
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
		int lastrnd;
		TimeValue t;
	};
class SphereDeflectorField : public CollisionObject {
	public:		
		SphereDefObject *obj;
		INode *node;
		Matrix3 tm, invtm,tp;
		Interval tmValid;
		Point3 Vc,Vcp;
		BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt, int index,float *ct,BOOL UpdatePastCollide);
		Object *GetSWObject();
	};


// UNIDEF_CLASS_ID
class UniDefObject : public SimpleWSMObject {	
	public:		
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
		static HWND hParams;
					
		INode *custnode;
		int lastrnd;
		TimeValue t;
		TSTR custname;
		UniDefObject();
		~UniDefObject();
		Mesh *dmesh;
		int nv,nf;
		VNormal *vnorms;
		Point3 *fnorms;
		Matrix3 tm,ptm,invtm,tmNoTrans,invtmNoTrans;
		Interval tmValid,mValid;
		Point3 dvel;
	};
class UniDeflectorField : public CollisionObject {
	public:		
		UniDefObject *obj;
		INode *node;
		int badmesh;
		BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt, int index,float *ct,BOOL UpdatePastCollide);
		Object *GetSWObject();
	};

/* can't do path follow - requires a particle system
// PFOLLOW_CLASSID
class PFollowField : public CollisionObject {
	public:		
		SimpleWSMObject *obj;
		ParticleObject *partobj;
		INode *node,*pnode;
		Object *pobj;
		Matrix3 tm,ptm;
		Interval tmValid,mValid;
		PFollowData *pd;
		ShapeObject *pathOb;
		int badmesh;
		BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt,int index,float *ct,BOOL UpdatePastCollide);
		Object *GetSWObject();
	};
*/
