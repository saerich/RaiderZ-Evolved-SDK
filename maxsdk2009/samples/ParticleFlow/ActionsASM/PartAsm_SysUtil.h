/**********************************************************************
 *<
	FILE:			PartASM_SysUtil.h

	DESCRIPTION:	Collection of useful functions and globals unified 
					by SysUtil class (declaration).
											 
	CREATED FOR:		Oleg Bayborodin

	HISTORY:		05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef _PARTASM_SYSUTIL_H_
#define _PARTASM_SYSUTIL_H_


namespace PartASM {

	class WhiteMtl: public Material {
	public:
		WhiteMtl();
	};

	class SysUtil {
	public:
		// debug/memory_management utilities
		static void NeedToImplementLater();
		static void UnderConstructionMessage(const char* functionName);
		static Material* GetWhiteMtl() { return &m_whiteMtl; }
		static Material* GetParticleMtl() { return &m_particleMtl; }

	private:
		static int m_needToImplementLater;
		static WhiteMtl m_whiteMtl;
		static Material m_particleMtl;

	};





} // end of namespace PartASM

#endif // _PARTASM_SYSUTIL_H_