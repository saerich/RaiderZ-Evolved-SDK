/*==============================================================================

  file:	        Enumeration.h

  author:       Daniel Levesque
  
  created:	    18 February 2002
  
  description:
        
          Enumeration classes used by this utility.

  modified:	


© 2002 Autodesk
==============================================================================*/
#ifndef _ENUMERATION_H_
#define _ENUMERATION_H_

#include <max.h>

//////////////////////////////////////////////////////////////////////
// class SceneEnumerator
//
// Enumeration procedure which gets called on every node in the scene.
//
class SceneEnumerator {
public:
    enum Result {
        kContinue, kStop, kAbort
    };

    SceneEnumerator() {
        Interface* ip = GetCOREInterface();
        _restoreXRef = ip->GetIncludeXRefsInHierarchy() != 0;
        ip->SetIncludeXRefsInHierarchy(true);
    }
    ~SceneEnumerator() { GetCOREInterface()->SetIncludeXRefsInHierarchy(_restoreXRef); }

    Result enumerate() { return enumerate(GetCOREInterface()->GetRootNode()); }

protected:
    virtual Result callback(INode* node) = 0;

private:
    Result __fastcall enumerate(INode* node);

    bool        _restoreXRef;
};

//////////////////////////////////////////////////////////////////////
// Scene enumeration proc. Calls EnumRefHierarchy() on every node
// in the scene, using the given RefEnumProc.
//
class SceneRefEnumerator : public SceneEnumerator {
public:
    SceneRefEnumerator(RefEnumProc& refEnumProc) : m_refEnumProc(refEnumProc) {}
protected:
    virtual Result callback(INode* node) {
		node->EnumRefHierarchy(m_refEnumProc);
		return kContinue;
    }
private:
    RefEnumProc& m_refEnumProc;
};

////////////////////////////////////////////////////////////////////////
// Enumeration proc class that calls RenderBegin() on a reference
//
class RenderBeginEnum : public RefEnumProc {
public:
    RenderBeginEnum(TimeValue time, ULONG fl) { t = time; rbflag = fl; }
    int proc(ReferenceMaker *m) { 
        m->RenderBegin(t,rbflag);  
		return DEP_ENUM_CONTINUE;
    }
private:
    TimeValue t;
    ULONG rbflag;
};

////////////////////////////////////////////////////////////////////////
// Enumeration proc class that calls RenderEnd() on a reference
//
class RenderEndEnum : public RefEnumProc {
public:
    RenderEndEnum(TimeValue time) { t = time; }
    int proc(ReferenceMaker *m) { 
        m->RenderEnd(t);  
		return DEP_ENUM_CONTINUE;
    }
private:
    TimeValue t;
};

#endif