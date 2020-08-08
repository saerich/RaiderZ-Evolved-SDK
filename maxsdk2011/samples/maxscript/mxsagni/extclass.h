/**********************************************************************
 *<
	FILE: ExtClass.h

	DESCRIPTION: MXSAgni extension classes

	CREATED BY: Ravi Karra, 1998

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef _H_EXT_CLASS
#define _H_EXT_CLASS

#include "MXSAgni.h"
#include "MNBigMat.h"
#include "notetrck.h"
#include "shadgen.h"

#ifndef NO_SCENEEVENTMANAGER
#include "ISceneEventManager.h"
#endif //NO_SCENEEVENTMANAGER

class BigMatrixValue;

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

visible_class_debug_ok (BigMatrixRowArray)

class BigMatrixRowArray : public Value
{
public:
	BigMatrixValue		*bmv;
	int					row;
						BigMatrixRowArray(BigMatrixValue* bmv);

						classof_methods (BigMatrixRowArray, Value);
	void				collect() { delete this; }
	void				sprin1(CharStream* s);
	void				gc_trace();
	
#include <maxscript\macros\define_implementations.h>
	def_generic	( get,		"get" );
	def_generic	( put,		"put" );
};


applyable_class_debug_ok (BigMatrixValue)

class BigMatrixValue : public Value
{
public:
	BigMatrix			bm;
	BigMatrixRowArray	*rowArray;

						BigMatrixValue(int mm, int nn);
						BigMatrixValue(const BigMatrix& from);

//	ValueMetaClass*		local_base_class() { return class_tag(BigMatrixValue); }			
						classof_methods (BigMatrixValue, Value);
	
	void				collect() { delete this; }
	void				sprin1(CharStream* s);
	void				gc_trace();
#	define				is_bigmatrix(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(BigMatrixValue))
	Value*				get_property(Value** arg_list, int count);
	Value*				set_property(Value** arg_list, int count);
	BigMatrix&			to_bigmatrix() { return bm; }

	/* operations */

#include <maxscript\macros\define_implementations.h>
#	include <maxscript\protocols\bigmatrix.h>
	
};


applyable_class (MAXNoteTrack)

class MAXNoteTrack : public MAXWrapper
{
	
public:
	DefNoteTrack*		note_track;
	TCHAR*				name;
	MAXNoteTrack(TCHAR* iname);
	MAXNoteTrack(DefNoteTrack* dnt);
	static ScripterExport Value* intern(DefNoteTrack* dnt);
	
						classof_methods (MAXNoteTrack, MAXWrapper);
	void				collect() { delete this; }
	void				sprin1(CharStream* s);
	void				gc_trace();
#	define				is_notetrack(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(MAXNoteTrack))
	Value*				get_property(Value** arg_list, int count);
	Value*				set_property(Value** arg_list, int count);
	DefNoteTrack*		to_notetrack() { return note_track; }
	TCHAR*				class_name() { return _T("NoteTrack"); }
	
	def_property		( keys );
#include <maxscript\macros\define_implementations.h>
#	include	<maxscript\protocols\notekey.h>
	def_generic ( copy,		"copy");
};

visible_class (MAXNoteKeyArray)

class MAXNoteKeyArray : public Value
{
public:
	MAXNoteTrack*	note_track;		/* the note track							*/

	ScripterExport	MAXNoteKeyArray(DefNoteTrack* icont);
	ScripterExport	MAXNoteKeyArray(MAXNoteTrack* icont);

					classof_methods (MAXNoteKeyArray, Value);
	BOOL			_is_collection() { return 1; }
	void			collect() { delete this; }
	void			gc_trace();
	ScripterExport	void sprin1(CharStream* s);

	/* operations */
	
#include <maxscript\macros\define_implementations.h>
#	include <maxscript\protocols\arrays.h>
#	include	<maxscript\protocols\notekey.h>
	ScripterExport Value* map(node_map& m);

	/* built-in property accessors */

	def_property ( count );
};


visible_class (MAXNoteKey)

class MAXNoteKey : public Value
{
public:
	MAXNoteTrack*	note_track;		/* MAX-side note track						*/
	int				key_index;

	ScripterExport	MAXNoteKey (DefNoteTrack* nt, int ikey);
	ScripterExport	MAXNoteKey (MAXNoteTrack* nt, int ikey);

					classof_methods (MAXNoteKey, Value);
	void			collect() { delete this; }
	void			gc_trace();
	ScripterExport	void sprin1(CharStream* s);

	def_generic		( delete, "delete" );

	def_property ( time );
	def_property ( selected );
	def_property ( value );
};

/* ---------------------- MAXFilterKernel ----------------------- */

visible_class (MAXFilter)

class MAXFilter : public MAXWrapper
{
public:
	FilterKernel*	filter;			// the MAX-side FilterKernel

	ScripterExport MAXFilter(FilterKernel* ifilter);
	static ScripterExport Value* intern(FilterKernel* ifilter);

	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(MAXFilter)) ? 1 : MAXWrapper::is_kind_of(c); }
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	TCHAR*		class_name() { return _T("Filter"); }

	def_property	( name );

	FilterKernel*	to_filter() { check_for_deletion(); return filter; }
};

/* ------------------- DataPair class instance ----------------------- */

applyable_class_debug_ok (DataPair)

class DataPair : public Value
{
public:
	Value*		v1;
	Value*		v2;
	Value*		v1_name;
	Value*		v2_name;

	DataPair();
	DataPair(Value* v1, Value* v2, Value* v1_name = NULL, Value* v2_name = NULL);

	classof_methods(DataPair, Value);
	void		collect() { delete this; }
	void		sprin1(CharStream* s);
	void		gc_trace();

	def_generic ( get_props,	"getPropNames");
	def_generic ( copy,			"copy");
	Value*	get_property(Value** arg_list, int count);
	Value*	set_property(Value** arg_list, int count);
};

/* ---------------------- NodeEventCallbackValue ----------------------- */

#ifndef NO_SCENEEVENTMANAGER
applyable_class_debug_ok (NodeEventCallbackValue)

class NodeEventCallbackValue : public Value, public INodeEventCallback
{
public:

	Array*		m_CallbackFunctions;
	DWORD		m_CallbackKey;
	BOOL		enabled, polling, mouseUp, processing;
	int			delayMilliseconds;

	ScripterExport NodeEventCallbackValue();
				~NodeEventCallbackValue();

	using Collectable::operator new;
	using Collectable::operator delete;

	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(NodeEventCallbackValue)) ? 1 : Value::is_kind_of(c); }
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	void		gc_trace();
	TCHAR*		class_name() { return _T("NodeEventCallback"); }
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	Value*		get_props_vf(Value** arg_list, int count);

	int			EventNameToIndex(Value *eventName);
	void		CallHandler(Value *eventName, NodeKeyTab& nodes);

	// ---------- Hierarchy Events ----------
	// Changes to the scene structure of an object
	void Added( NodeKeyTab& nodes );
	void Deleted( NodeKeyTab& nodes );
	void LinkChanged( NodeKeyTab& nodes );
	void LayerChanged( NodeKeyTab& nodes );
	void GroupChanged( NodeKeyTab& nodes );
	void HierarchyOtherEvent( NodeKeyTab& nodes );

	// ---------- Model Events ----------
	// Changes to the geometry or parameters of an object
	void ModelStructured( NodeKeyTab& nodes );
	void GeometryChanged( NodeKeyTab& nodes );
	void TopologyChanged( NodeKeyTab& nodes );
	void MappingChanged( NodeKeyTab& nodes );
	void ExtentionChannelChanged( NodeKeyTab& nodes );
	void ModelOtherEvent( NodeKeyTab& nodes );

	// ---------- Material Events ----------
	// Changes to the material on an object
	void MaterialStructured( NodeKeyTab& nodes );
	void MaterialOtherEvent( NodeKeyTab& nodes );

	// ---------- Controller Events ----------
	// Changes to the controller on an object
	void ControllerStructured( NodeKeyTab& nodes );
	void ControllerOtherEvent( NodeKeyTab& nodes );

	// ---------- Property Events ----------
	// Changes to object properties
	void NameChanged( NodeKeyTab& nodes );
	void WireColorChanged( NodeKeyTab& nodes );
	void RenderPropertiesChanged( NodeKeyTab& nodes );
	void DisplayPropertiesChanged( NodeKeyTab& nodes );
	void UserPropertiesChanged( NodeKeyTab& nodes );
	void PropertiesOtherEvent( NodeKeyTab& nodes );

	// ---------- Display/Interaction Events ----------
	// Changes to the display or interactivity state of an object
	void SubobjectSelectionChanged( NodeKeyTab& nodes );
	void SelectionChanged( NodeKeyTab& nodes );
	void HideChanged( NodeKeyTab& nodes );
	void FreezeChanged( NodeKeyTab& nodes );
	void DisplayOtherEvent( NodeKeyTab& nodes );

	// ----------  ----------
	void CallbackBegin();
	void CallbackEnd();

	BaseInterface* GetInterface(Interface_ID id);

};
#endif //NO_SCENEEVENTMANAGER

#endif //_H_EXT_CLASS
