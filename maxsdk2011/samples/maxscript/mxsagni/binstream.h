// ============================================================================
// BinStream.h      By Simon Feltman [discreet]
// ----------------------------------------------------------------------------
#ifndef __BINSTREAM_H__
#define __BINSTREAM_H__
#include "MXSAgni.h"
#include <stdio.h>
//#include "Funcs.h"

visible_class_debug_ok (BinStream)

class BinStream : public Value
{
public:
	TSTR     name;
	TSTR     mode;
	FILE    *desc;

//	static BOOL Init();
	BinStream();

//	ValueMetaClass* local_base_class() { return class_tag(BinStream); } // local base class in this class's plug-in
	classof_methods(BinStream, Value);
	void		collect();
	void		sprin1(CharStream* s);
	void		gc_trace();
#	define   is_binstream(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(BinStream))
};


#endif //__BINSTREAM_H__
