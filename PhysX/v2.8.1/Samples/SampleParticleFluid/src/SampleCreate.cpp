#include "SampleParticleFluid.h"
#include "MyFluid.h"
#include "ParticleFactory.h"

extern NxActor *gGroundPlane;
extern bool gHardwareSimulation;
extern NxScene* gScene;
extern NxArray<MyFluid*> gMyFluids;
extern NxVec3 gCameraPos;
extern NxVec3 gCameraForward;

#define MAX_PARTICLES 10000

void SampleCreate::setup()
{
	SetTitleString("Create Fluid Sample");
	SetHelpString(NULL);

	//Create objects in the scene
	gGroundPlane = CreateGroundPlane();

	//Create a set of initial particles
	ParticleSDK*	initParticles = new ParticleSDK[MAX_PARTICLES];
	unsigned initParticlesNum = 0;

	//Setup structure to pass initial particles.
	NxParticleData initParticleData;
	initParticleData.numParticlesPtr		= &initParticlesNum;
	initParticleData.bufferPos				= &initParticles[0].position.x;
	initParticleData.bufferPosByteStride	= sizeof(ParticleSDK);
	initParticleData.bufferVel				= &initParticles[0].velocity.x;
	initParticleData.bufferVelByteStride	= sizeof(ParticleSDK);

	//Create particle filled sphere in buffer.
	NxVec3 fluidPos(0,2,0);
	float distance = 0.1f;
	unsigned sideNum = 16;
	CreateParticleSphere(initParticleData, MAX_PARTICLES, false, fluidPos, NxVec3(0,0,0), 0.0f, distance, sideNum);

	//Setup fluid descriptor
	NxFluidDesc fluidDesc;
    fluidDesc.maxParticles                  = MAX_PARTICLES;
    fluidDesc.kernelRadiusMultiplier		= 2.0f;
    fluidDesc.restParticlesPerMeter			= 10.0f;
	fluidDesc.motionLimitMultiplier			= 3.0f;
	fluidDesc.packetSizeMultiplier			= 8;
    fluidDesc.collisionDistanceMultiplier   = 0.1;
    fluidDesc.stiffness						= 50.0f;
    fluidDesc.viscosity						= 40.0f;
	fluidDesc.restDensity					= 1000.0f;
    fluidDesc.damping						= 0.0f;
    fluidDesc.restitutionForStaticShapes	= 0.2f;
	fluidDesc.dynamicFrictionForStaticShapes= 0.05f;
	fluidDesc.simulationMethod				= NX_F_SPH;
	if (!gHardwareSimulation)
        fluidDesc.flags &= ~NX_FF_HARDWARE;

	//Add initial particles to fluid creation.
	fluidDesc.initialParticleData			= initParticleData;

	//Create user fluid.
	//- create NxFluid in NxScene
	//- setup the buffers to read from data from the SDK
	//- set NxFluid::userData field to MyFluid instance
	bool trackUserData = false;
	bool provideCollisionNormals = false;
	MyFluid* fluid = new MyFluid(fluidDesc, trackUserData, provideCollisionNormals, NxVec3(0.4f,0.5f,0.9f), 0.03f);
	assert(fluid);
	gMyFluids.pushBack(fluid);
	
	delete[] initParticles;

	gCameraPos.set(1.43f,10.96f,17.9f);
	gCameraForward = fluidPos - gCameraPos;
	gCameraForward.normalize();
}

void SampleCreate::cleanup()
{
	//MyFluid cleaned up by ReleaseNx()

}
