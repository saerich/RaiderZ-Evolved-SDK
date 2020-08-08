/**********************************************************************
 *<
   FILE:       MXSCurveCtl.cpp

   DESCRIPTION:   MaxScript Curve Control extension DLX

   CREATED BY:    Ravi Karra

   HISTORY:    Created on 5/12/99

 *>   Copyright © 1997, All Rights Reserved.
 **********************************************************************/

#include "MXSCurveCtl.h"
#include "3DMath.h"
#include "ColorVal.h"
#include "Parser.h"
#include "macrorec.h"
#include <AppDataChunk.h>
#include <maxscrpt\strings.h>
#include <maxscript\macros\local_class_instantiations.h>
#  include "ccnames.h"
#  include  "curvepro.h"

#include "3dsmaxport.h"

#ifdef ScripterExport
#undef ScripterExport
#define ScripterExport
#endif
int 
GetID(paramType params[], int count, Value* val, int def_val)
{  
   for(int p = 0; p < count; p++)
      if(val == params[p].val)
         return params[p].id;
   if (def_val != -1 ) return def_val;

   // Throw an error showing valid args incase an invalid arg is passed
   TCHAR buf[2056]; buf[0] = '\0';
   
   _tcscat(buf, GetString(IDS_NEEDS_ARG_OF_TYPE));
   for (int i=0; i < count; i++) 
   {
      _tcscat(buf, params[i].val->to_string());
      if (i < count-1) _tcscat(buf, _T(" | #"));
   }  
   _tcscat(buf, GetString(IDS_GOT));
   throw RuntimeError(buf, val);
   return -1;
}

Value* 
GetName(paramType params[], int count, int id, Value* def_val)
{
   for(int p = 0; p < count; p++)
      if(id == params[p].id) return params[p].val;
   if (def_val) return def_val;
   return NULL;
}


DEFINE_LOCAL_GENERIC_CLASS_DEBUG_OK(CCRootClassValue, CurveCtlGeneric);

local_visible_class_instance (CCRootClassValue, _T("CCRootClass"))

/* ------------------- CurvePointValue class instance ----------------------- */
visible_class_instance (CurvePointValue, "ccPoint")

Value*
CurvePointValueClass::apply(Value** arg_list, int count, CallContext* cc)
{
   //  ccPoint <pt_point2> <in_point2> <out_point2>                     \
   // [bezier:<false>] [corner:<false>] [lock_x:<false>] [lock_y:<false>]     \
   // [select:<false>] [end:<false>] [noXConstraint:<false>]
   check_arg_count_with_keys(curvePoint,3,count);
   three_value_locals(a0, a1, a2);
   CurvePointValue* cp_val;
   CurvePoint cp;
   
   vl.a0 = arg_list[0]->eval();
   vl.a1 = arg_list[1]->eval();
   vl.a2 = arg_list[2]->eval();
   
   cp.p = vl.a0->to_point2();
   cp.in = vl.a1->to_point2();
   cp.out = vl.a2->to_point2();
   
   cp_val = new CurvePointValue (cp);
   cp_val->SetFlag(CURVEP_BEZIER,   key_arg_or_default(bezier, &false_value)->eval());
   cp_val->SetFlag(CURVEP_CORNER,   key_arg_or_default(corner, &false_value)->eval());
   cp_val->SetFlag(CURVEP_LOCKED_Y, key_arg_or_default(lock_x, &false_value)->eval());
   cp_val->SetFlag(CURVEP_LOCKED_Y, key_arg_or_default(lock_y, &false_value)->eval());
   cp_val->SetFlag(CURVEP_SELECTED, key_arg_or_default(select, &false_value)->eval());
   cp_val->SetFlag(CURVEP_ENDPOINT, key_arg_or_default(end, &false_value)->eval());
   cp_val->SetFlag(CURVEP_NO_X_CONSTRAINT, key_arg_or_default(noXConstraint, &false_value)->eval());
   
   pop_value_locals();
   return cp_val; // LAM - was: return_value(cp_val);  //Hey!
}

/* -------------------- CurvePointValue methods ----------------------------- */

CurvePointValue::CurvePointValue(CurvePoint pt)
{
   tag = class_tag(CurvePointValue);
   cp = pt;
   curve = NULL;
   cctl = NULL;
}

CurvePointValue::CurvePointValue(ICurveCtl *icctl, ICurve *crv, int ipt)
{
   tag = class_tag(CurvePointValue);   
   curve = crv;
   cctl = icctl;
   pt_index = ipt;
   cp = curve->GetPoint(MAXScript_time(), pt_index);
   //make_ref(0, curve);
}

Value*
CurvePointValue::intern(ICurveCtl *icctl, ICurve *crv, int ipt)
{
   /*CurvePointValue* mxk;    // *** not derived from MAXWrapper ***
   CurvePointValue** mxkp;
   if (maxwrapper_cache_get(CurvePointValue, (long)nt ^ (long)ikey, mxk, mxkp) &&
       mxk->curve == crv &&
      mxk->pt_index == ipt)
      return mxk;
   else
      return (*mxkp = new CurvePointValue (crv, ipt));*/
   return new CurvePointValue (icctl, crv, ipt);
}

void
CurvePointValue::sprin1(CharStream* s)
{
   /*if (ref_deleted)
   {
      s->printf(_T("<Curve Point for deleted curve>"));
      return;
   }*/
   s->printf(_T("#CCPoint(@[%g,%g])"), cp.p.x, cp.p.y);
}

Value*
CurvePointValue::get_property(Value** arg_list, int count)
{  
   Value* prop = arg_list[0];
   if (curve)
   {
      if (pt_index < 0 || pt_index >= curve->GetNumPts())
         throw(GetString(IDS_CURVE_NO_LONGER_EXISTS));
      cp = curve->GetPoint(MAXScript_time(), pt_index);
   }
   
   if (prop == n_value)       return new Point2Value(cp.p);
   else if (prop == n_inTangent) return new Point2Value(cp.in);
   else if (prop == n_outTangent)   return new Point2Value(cp.out);
   else if (prop == n_bezier)    return (cp.flags&CURVEP_BEZIER)    ? &true_value : &false_value;
   else if (prop == n_corner)    return (cp.flags&CURVEP_CORNER)   ? &true_value : &false_value;
   else if (prop == n_lock_x)    return (cp.flags&CURVEP_LOCKED_X) ? &true_value : &false_value;
   else if (prop == n_lock_y)    return (cp.flags&CURVEP_LOCKED_Y) ? &true_value : &false_value;
   else if (prop == n_selected)  return (cp.flags&CURVEP_SELECTED) ? &true_value : &false_value;
   else if (prop == n_end)       return (cp.flags&CURVEP_ENDPOINT) ? &true_value : &false_value;
   else if (prop == n_noXConstraint)return (cp.flags&CURVEP_NO_X_CONSTRAINT) ? &true_value : &false_value;
   else return Value::get_property(arg_list, count);
}

Value*
CurvePointValue::set_property(Value** arg_list, int count)
{
   Value* val = arg_list[0];
   Value* prop = arg_list[1];
   
   if (curve)
   {
      if (pt_index < 0 || pt_index >= curve->GetNumPts())
         throw(GetString(IDS_CURVE_NO_LONGER_EXISTS));
      cp = curve->GetPoint(MAXScript_time(), pt_index);
   }
   if (prop == n_value)       cp.p = val->to_point2();
   else if (prop == n_inTangent) cp.in = val->to_point2();
   else if (prop == n_outTangent)   cp.out = val->to_point2();
   else if (prop == n_bezier)    SetFlag(CURVEP_BEZIER,   val);
   else if (prop == n_corner)    SetFlag(CURVEP_CORNER,   val);
   else if (prop == n_lock_x)    SetFlag(CURVEP_LOCKED_X, val);
   else if (prop == n_lock_y)    SetFlag(CURVEP_LOCKED_Y, val);
   else if (prop == n_selected)  SetFlag(CURVEP_SELECTED, val);
   else if (prop == n_end)
      throw RuntimeError (GetString(IDS_READONLY_PROPERTY), _T("end"));
   else if (prop == n_noXConstraint)SetFlag(CURVEP_NO_X_CONSTRAINT, val);
   else return Value::set_property(arg_list, count);
   
   CurvePoint _cp = cp;
   if (curve) 
   {  curve->SetPoint(MAXScript_time(), pt_index, &_cp);
      curve->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
   }
   if (cctl) cctl->Redraw();
   return val;
}

local_visible_class_instance (CurvePointsArray, _T("CurvePointsArray"))
/* -------------------- CurvePointsArray methods ----------------------------- */

CurvePointsArray::CurvePointsArray(ICurveCtl *icctl, ICurve* crv)
{
   tag = class_tag(CurvePointsArray);
   cctl = icctl;
   curve = crv;
   make_ref(0, curve);  
}

static bool CurvePointsArray_intern_finalCheckProc(MAXWrapper* val, void* icctl)
{  return (((CurvePointsArray*)val)->cctl == icctl);
}

Value*
CurvePointsArray::intern(ICurveCtl *icctl, ICurve* crv)
{
   /**/CurvePointsArray* mxk;
   CurvePointsArray** mxkp;
   if (maxwrapper_cache_get(CurvePointsArray, crv, mxk, mxkp) &&
       mxk->curve == crv &&
      mxk->cctl == icctl)
      return mxk;
   else
   {
      FindMAXWrapperEnum cb(crv,class_tag(CurvePointsArray),&CurvePointsArray_intern_finalCheckProc,icctl);
      crv->DoEnumDependents(&cb);
      if (cb.result) 
      {  *mxkp = (CurvePointsArray*)cb.result;
         return cb.result;
      }
      return (*mxkp = new CurvePointsArray (icctl, crv));/**/
   }
}

void
CurvePointsArray::sprin1(CharStream* s)
{
   /**/if (check_for_deletion_test())
   {
      s->printf(GetString(IDS_CPARRAY_FOR_DELETED_CURVE));
      return;
   }/**/
      
   int   num_pts = curve->GetNumPts();
   int   i;
   s->puts(_T("#points("));
   for (i = 0; i < num_pts; i++)
   {
      CurvePoint cp = curve->GetPoint(MAXScript_time(), i);
      s->printf(_T("[%g,%g]"), cp.p.x, cp.p.y);
      if (i < num_pts - 1)
         s->puts(_T(", "));
   }
   s->puts(_T(")")); 
}

Value*
CurvePointsArray::get_vf(Value** arg_list, int count)
{
   check_for_deletion();
   Value *arg, *result;
   int     index;

   check_arg_count(get, 2, count + 1);
   arg = arg_list[0];
   if (!is_number(arg) || (index = arg->to_int()) < 1)
      throw TypeError (GetString(IDS_ARRAY_INDEX_MUST_BE_VE_NUMBER_GOT), arg);

   if (index > curve->GetNumPts())
      result = &undefined;
   else
      result = CurvePointValue::intern(cctl, curve, index - 1);  // array indexes are 1-based !!!

   return result;
}

Value*
CurvePointsArray::put_vf(Value** arg_list, int count)
{  
   check_for_deletion();
   Value *arg, *val;
   int     index;

   check_arg_count(set, 3, count + 1);
   arg = arg_list[0];
   val = arg_list[1];
   if (!is_number(arg) || (index = arg->to_int()) < 1)
      throw TypeError (GetString(IDS_ARRAY_INDEX_MUST_BE_VE_NUMBER_GOT), arg);
   if (index > curve->GetNumPts())
      throw RuntimeError(GetString(IDS_POINT_INDEX_OUT_OF_RANGE), arg);
   if (!is_curvepoint(val))
      throw RuntimeError(GetString(IDS_INVALID_POINT_ARG), val);
   
   CurvePoint _cp = ((CurvePointValue*)val)->cp;
   curve->SetPoint(MAXScript_time(), index - 1, &_cp);
   return arg_list[1];
}

Value*
CurvePointsArray::map(node_map& m)
{
   check_for_deletion();
   int      num_keys = curve->GetNumPts();
   int      i;
   two_value_locals(key, result);   
   save_current_frames();
   for (i = 0; i < num_keys; i++)
   {
      push_alloc_frame();
      try
      {
         vl.key = CurvePointValue::intern(cctl, curve, i);

         if (m.vfn_ptr != NULL)
            vl.result = (vl.key->*(m.vfn_ptr))(m.arg_list, m.count);
         else
         {
            // temporarily replace 1st arg with this key
            Value* arg_save = m.arg_list[0];

            m.arg_list[0] = vl.key;
            if (m.flags & NM_MXS_FN)
               vl.result = ((MAXScriptFunction*)m.cfn_ptr)->apply(m.arg_list, m.count);
            else
               vl.result = (*m.cfn_ptr)(m.arg_list, m.count);
            m.arg_list[0] = arg_save;
         }
         if (vl.result == &loopExit)
         {
            pop_alloc_frame();
            break;
         }
         if (m.collection != NULL && vl.result != &dontCollect)
            m.collection->append(vl.result);
         vl.result = NULL; // LAM: 2/12/01 defect 276146 fix
         vl.key = NULL; 
         pop_alloc_frame();
      }
      catch (LoopContinue)
      {
         pop_alloc_frame(); // LAM - 5/18/01 - flipped order since push_alloc_frame was done last
         restore_current_frames();
      }
      catch (...)
      {
         pop_alloc_frame();
         throw;
      }
   }

   pop_value_locals();
   return &ok;
}

Value* CurvePointsArray::join_vf(Value** arg_list, int count) { throw NoMethodError (_T("join"), this); return &undefined; }
Value* CurvePointsArray::sort_vf(Value** arg_list, int count) { throw NoMethodError (_T("sort"), this); return &undefined; }
Value* CurvePointsArray::findItem_vf(Value** arg_list, int count) { throw NoMethodError (_T("sort"), this); return &undefined; }
Value* CurvePointsArray::deleteItem_vf(Value** arg_list, int count) { throw NoMethodError (_T("sort"), this); return &undefined; }
Value* CurvePointsArray::append_vf(Value** arg_list, int count) { throw NoMethodError (_T("sort"), this); return &undefined; }

/* ---------------- CurvePointsArray property access ------------------------- */

Value*
CurvePointsArray::get_count(Value** arg_list, int count)
{
   check_for_deletion();
   return Integer::intern(curve->GetNumPts());
}

Value*
CurvePointsArray::set_count(Value** arg_list, int count)
{
   check_for_deletion();
   throw RuntimeError (GetString(IDS_CANNOT_SET_ARRAY_SIZE));
   return &undefined;
}

#define APPID_CURVENAME 0x75f11d94

local_visible_class_instance (MAXCurve, "ccCurve")
/* -------------------- MAXCurve methods ----------------------------- */

MAXCurve::MAXCurve(ICurve* icurve, ICurveCtl* icctl)
{
   tag = class_tag(MAXCurve);
   curve = icurve;
   make_ref(0, curve);
   cctl = icctl;
   curve->GetPenProperty(color,width,style);
   curve->GetDisabledPenProperty(dcolor,dwidth,dstyle);
}

static bool MAXCurve_intern_finalCheckProc(MAXWrapper* val, void* icctl)
{  return (((MAXCurve*)val)->cctl == icctl);
}

Value*
MAXCurve::intern(ICurve* icurve, ICurveCtl* icctl)
{
   if (icurve == NULL) return &undefined;

   MAXCurve* mxc;
   MAXCurve** mxcp;
   if (maxwrapper_cache_get(MAXCurve, icurve, mxc, mxcp) &&
       mxc->curve == icurve &&
      mxc->cctl == icctl)
      return mxc;
   else
   {
      FindMAXWrapperEnum cb(icurve,class_tag(MAXCurve),&MAXCurve_intern_finalCheckProc,icctl);
      icurve->DoEnumDependents(&cb);
      if (cb.result) 
      {  *mxcp = (MAXCurve*)cb.result;
         return cb.result;
      }
      return (*mxcp = new MAXCurve (icurve, icctl));
   }
}

void
MAXCurve::sprin1(CharStream* s)
{
   AppDataChunk *ad = curve->GetAppDataChunk(CURVE_CONTROL_CLASS_ID, REF_MAKER_CLASS_ID, APPID_CURVENAME);
   TCHAR *name = (ad) ? (TCHAR*)ad->data : _T("");
   s->printf("ccCurve%s%s", ad?_T(" : "):_T(""), name);
}

void
MAXCurve::gc_trace()
{
   MAXWrapper::gc_trace();
}

void MAXCurve::Update()
{
   curve->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
   if (cctl) cctl->Redraw();
}

#define def_penStyle_types()                 \
   paramType pen_styles[] = {             \
      { PS_SOLID,       n_solid        }, \
      { PS_DASH,        n_dash         }, \
      { PS_DOT,         n_dot       }, \
      { PS_DASHDOT,     n_dashDot      }, \
      { PS_DASHDOTDOT,  n_dashDotDot   }, \
      { PS_NULL,        n_null         }, \
      { PS_INSIDEFRAME, n_insideFrame  }  \
   };    

Value*
MAXCurve::get_property(Value** arg_list, int count)
{
   Value* prop = arg_list[0]; 
   if (prop == n_name)
   {
      AppDataChunk *ad = curve->GetAppDataChunk(CURVE_CONTROL_CLASS_ID, REF_MAKER_CLASS_ID, APPID_CURVENAME);
      TCHAR *name = (ad) ? (TCHAR*)ad->data : _T("");
      return new String (name);
   }
   else if (prop == n_animatable)      return curve->GetCanBeAnimated() ? &true_value : &false_value;
   else if (prop == n_color)        return new ColorValue(color);
   else if (prop == n_disabledColor)   return new ColorValue(dcolor);
   else if (prop == n_width)        return Integer::intern(width);
   else if (prop == n_disabledWidth)   return Integer::intern(dwidth);
   else if (prop == n_disabledStyle)   
   {
      def_penStyle_types()
      return GetName(pen_styles, elements(pen_styles), dstyle);
   }
   else if (prop == n_lookupTableSize) return Integer::intern(curve->GetLookupTableSize());
   else if (prop == n_style)        
   {
      def_penStyle_types()
      return GetName(pen_styles, elements(pen_styles), style);
   }
   else if (prop == n_numPoints)    return Integer::intern(curve->GetNumPts());
   else if (prop == n_points)
   {
      return CurvePointsArray::intern(cctl, curve);
      /*one_typed_value_local(Array* result); 
      int n = curve->GetNumPts();
      vl.result = new Array(n);
      for(int i = 0; i < n; i++)
         vl.result->append(new CurvePointValue(curve->GetPoint(MAXScript_time(), i))); 
      return_value(vl.result);*/
   }
   else return Value::get_property(arg_list, count);
}

Value*
MAXCurve::set_property(Value** arg_list, int count)
{  
   Value* val = arg_list[0];
   Value* prop = arg_list[1];

   if (prop == n_name)
   {
      TCHAR* data = save_string(val->to_string());
      curve->RemoveAppDataChunk(CURVE_CONTROL_CLASS_ID, REF_MAKER_CLASS_ID, APPID_CURVENAME);
      curve->AddAppDataChunk(CURVE_CONTROL_CLASS_ID, REF_MAKER_CLASS_ID, 
            APPID_CURVENAME, static_cast<int>(_tcslen(data) + 1), data);         // SR DCAST64: Downcast to 2G limit.
   }

   else if (prop == n_animatable) curve->SetCanBeAnimated(val->to_bool());
   else if (prop == n_color)
   {
      color = val->to_colorref();
      curve->SetPenProperty(color,width,style);
   }
   else if (prop == n_disabledColor)
   {
      dcolor = val->to_colorref();
      curve->SetDisabledPenProperty(dcolor,dwidth,dstyle);
   }
   else if (prop == n_width)
   {
      width = val->to_int();
      curve->SetPenProperty(color,width,style);
   }
   else if (prop == n_disabledWidth)
   {
      dwidth = val->to_int();
      curve->SetDisabledPenProperty(dcolor,dwidth,dstyle);
   }
   else if (prop == n_style)
   {
      def_penStyle_types()
      style = GetID(pen_styles, elements(pen_styles), val);
      curve->SetPenProperty(color,width,style);
   }
   else if (prop == n_disabledStyle)
   {
      def_penStyle_types()
      dstyle = GetID(pen_styles, elements(pen_styles), val);
      curve->SetDisabledPenProperty(dcolor,dwidth,dstyle);
   }
   else if (prop == n_lookupTableSize)
   {
      curve->SetLookupTableSize(val->to_int());
   }
   else if (prop == n_numPoints)
   {
      int pn = curve->GetNumPts();
      if (pn==2) pn=0; // By default the numPoints is 2, so avoid that case
      int cn = val->to_int();
      curve->SetNumPts(cn);
      for (int i=pn; i<cn; i++)
      {
         CurvePoint cp = curve->GetPoint(MAXScript_time(), i);
         //cp.p = Point2(0.0,0.0);
         //cp.in = Point2(0.2,-0.2);
         //cp.out = Point2(0.2,0.2);
         cp.flags |= (CURVEP_CORNER|CURVEP_BEZIER);
         if (i!=0 && i != cn-1)
         {
            cp.flags |= CURVEP_NO_X_CONSTRAINT;
         }
         curve->SetPoint(MAXScript_time(), i, &cp);
      }
   }  
   else if (prop == n_points) 
      throw RuntimeError (GetString(IDS_READONLY_PROPERTY), _T("points"));
   else return Value::set_property(arg_list, count);
   Update();
   return val;
}

Value*
MAXCurve::getValue_vf(Value** arg_list, int count)
{
   // getValue <ccCurve> <time_value> <float_x> [lookup:<false>]
   check_gen_arg_count_with_keys(getValue, 3, count+1);
   BOOL lookup = key_arg_or_default(bezier, &false_value)->to_bool();
   return Float::intern(curve->GetValue(arg_list[0]->to_timevalue(),
      arg_list[1]->to_float(), FOREVER, lookup));
}

Value*
MAXCurve::isAnimated_vf(Value** arg_list, int count)
{
   // isAnimated <ccCurve> <index>
   check_gen_arg_count(isAnimated, 2, count);
   int ind = arg_list[0]->to_int();
   if (ind < 1 || ind > curve->GetNumPts())
      throw RuntimeError(GetString(IDS_POINT_INDEX_OUT_OF_RANGE));
   return curve->IsAnimated(ind-1) ? &true_value : &false_value;
}

Value*
MAXCurve::getSelectedPts_vf(Value** arg_list, int count)
{
   // getSelectedPts <ccCurve>
   check_gen_arg_count(getSelectedPts, 1, count);
   return new BitArrayValue(curve->GetSelectedPts());
}

Value*
MAXCurve::setSelectedPts_vf(Value** arg_list, int count)
{
   // setSelectedPts <ccCurve> <bitarray> [#select] [#deSelect] [#clear]
   if (count < 1) check_gen_arg_count(setSelectedPts, 2, count);
   int flags = 0;
   for(int i=1; i < count; i++)
   {
      Value* val = arg_list[i];
      if (val == n_select)
         flags |= SELPTS_SELECT;
      else if (val == n_deSelect) 
         flags |= SELPTS_DESELECT;
      else if (val == n_clear) 
         flags |= SELPTS_CLEARPTS;
      else throw RuntimeError(GetString(IDS_INVALID_ARG), val);
   }
   BitArray sel = arg_list[0]->to_bitarray();
   sel.SetSize(curve->GetSelectedPts().GetSize(),TRUE);
   curve->SetSelectedPts(sel, flags);
   Update();
   return &ok;
}

Value*
MAXCurve::setPoint_vf(Value** arg_list, int count)
{
   // setPoint <ccCurve> <index> <ccpoint> [checkConstraints:<true>]
   check_gen_arg_count_with_keys(setPoint, 3, count+1);
   int ind = arg_list[0]->to_int();
   Value* cc_val = arg_list[1];
   
   if (!is_curvepoint(cc_val))
      throw RuntimeError(GetString(IDS_INVALID_POINT_ARG), cc_val);
   if (ind < 1 || ind > curve->GetNumPts())
      throw RuntimeError(GetString(IDS_POINT_INDEX_OUT_OF_RANGE));
   
   CurvePoint cp = ((CurvePointValue*)cc_val)->to_curvepoint();
   curve->SetPoint(MAXScript_time(), ind-1, &cp,
      key_arg_or_default(checkConstraints, &true_value)->to_bool());
   Update();
   return &ok;
}

Value*
MAXCurve::getPoint_vf(Value** arg_list, int count)
{  
   // getPoint <ccCurve> <index> 
   check_gen_arg_count(getPoint, 2, count);
   int ind = arg_list[0]->to_int();
   if (ind < 1 || ind > curve->GetNumPts())
      throw RuntimeError(GetString(IDS_POINT_INDEX_OUT_OF_RANGE));
   return new CurvePointValue(curve->GetPoint(MAXScript_time(), ind-1, FOREVER));
      
}

Value*
MAXCurve::insertPoint_vf(Value** arg_list, int count)
{
   // insertPoint <ccCurve> <index> <ccpoint>
   check_gen_arg_count(insertPoint, 3, count);
   int ind = arg_list[0]->to_int();
   Value* cc_val = arg_list[1];
   
   if (!is_curvepoint(cc_val))
      throw RuntimeError(GetString(IDS_INVALID_POINT_ARG), cc_val);
   if (ind < 1)
      throw RuntimeError(GetString(IDS_POINT_INDEX_OUT_OF_RANGE));
   
   CurvePoint cp = ((CurvePointValue*)cc_val)->to_curvepoint();
   curve->Insert(ind-1, cp);
   Update();
   return &ok;
}

Value*
MAXCurve::deletePoint_vf(Value** arg_list, int count)
{
   // deletePoint <ccCurve> <index>
   check_gen_arg_count(deletePoint, 2, count);
   int ind = arg_list[0]->to_int();
   if (ind < 1)
      throw RuntimeError(GetString(IDS_POINT_INDEX_OUT_OF_RANGE));
   curve->Delete(ind-1);
   Update();
   return &ok;
}

Value*
MAXCurve::delete_vf(Value** arg_list, int count)
{
   // delete <ccCurve>
   check_gen_arg_count(delete, 1, count);
   if (!cctl) return &ok;
   int nCurves = cctl->GetNumCurves();
   for (int i = 0; i < nCurves; i++ )
      if (cctl->GetControlCurve(i) == curve)
      {  cctl->DeleteCurve(i);
         break;
      }
   return &ok;
}

#define def_cmdMode_types()                     \
   paramType cmd_modes[] = {                 \
      { CID_CC_MOVE_XY,       n_move_xy   }, \
      { CID_CC_MOVE_X,        n_move_x }, \
      { CID_CC_MOVE_Y,        n_move_y }, \
      { CID_CC_SCALE,            n_scale     }, \
      { CID_CC_INSERT_CORNER,    n_corner }, \
      { CID_CC_INSERT_BEZIER,    n_bezier }  \
   };    

#define def_uiFlag_types()                      \
   paramType ui_flags[] = {                     \
      { CC_DRAWBG,            n_drawBG    }, \
      { CC_DRAWGRID,          n_drawGrid     }, \
      { CC_DRAWUTOOLBAR,         n_upperToolbar }, \
      { CC_SHOWRESET,            n_showReset    }, \
      { CC_DRAWLTOOLBAR,         n_lowerToolbar }, \
      { CC_DRAWSCROLLBARS,    n_scrollBars   }, \
      { CC_AUTOSCROLL,        n_autoScroll   }, \
      { CC_DRAWRULER,            n_ruler        }, \
      { CC_CONSTRAIN_Y,       n_constrainY   }, \
      { CC_HIDE_DISABLED_CURVES, n_hideDisabled }, \
      { CC_SHOW_CURRENTXVAL,     n_xvalue    }, \
      { CC_SINGLESELECT,         n_singleSelect }, \
      { CC_NOFILTERBUTTONS,      n_noFilterButtons }, \
      { (CC_ALL&(~CC_ALL_RCMENU)),n_all         }  \
   };    


#define def_rcmFlag_types()                     \
   paramType rcm_flags[] = {                 \
      { CC_RCMENU_MOVE_XY,    n_move_xy   }, \
      { CC_RCMENU_MOVE_X,        n_move_x }, \
      { CC_RCMENU_MOVE_Y,        n_move_y }, \
      { CC_RCMENU_SCALE,         n_scale     }, \
      { CC_RCMENU_INSERT_CORNER, n_corner }, \
      { CC_RCMENU_INSERT_BEZIER, n_bezier }, \
      { CC_RCMENU_DELETE,        n_delete }, \
      { CC_ALL_RCMENU,        n_all    }  \
   };    

static int GetCurveIndex(ICurveCtl *cctl, ICurve *crv)
{
   for (int i=0; i < cctl->GetNumCurves(); i++)
      if (cctl->GetControlCurve(i)==crv) 
         return i;
   return -1;
}

LRESULT CALLBACK CCTL_WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
   MAXCurveCtl *mcc = DLGetWindowLongPtr<MAXCurveCtl*>(hWnd);
   int         count = (message==WM_CC_CHANGE_CURVETANGENT) ? 3 : 2; 
   Value*      cb;
   Value**     arg_list;   
   switch (message) {
      case WM_CC_SEL_CURVEPT:
         cb = n_selChanged;
         break;
      case WM_CC_CHANGE_CURVEPT:
         cb = n_ptChanged;
         break;
      case WM_CC_CHANGE_CURVETANGENT:
         cb = n_tangentChanged;
         break;
      case WM_CC_DEL_CURVEPT:
         cb = n_deleted;
         break;
      case WM_CC_INSERT_CURVEPT:
         cb = n_ptInserted;
         break;
      default:
         return DefWindowProc(hWnd, message, wParam, lParam);
      }
   
   int num = (int)LOWORD(wParam);
   if (message != WM_CC_SEL_CURVEPT) num++; // point indice's are 1-based

   init_thread_locals();
   push_alloc_frame();
   value_local_array(arg_list, count);
   arg_list[0] = Integer::intern(GetCurveIndex(mcc->cctl, (ICurve*)lParam) + 1);
   arg_list[1] = Integer::intern(num);
   if (count==3) 
   {
      int which = (int)HIWORD(wParam);
      if (which==1 || which==3) // in-tangent handler
      {
         arg_list[2] =  n_inTangent;
         mcc->run_event_handler(mcc->parent_rollout, cb, arg_list, count);
      }
      if (which==2 || which==3) // out-tangent handler
      {
         arg_list[2] = n_outTangent;
         mcc->run_event_handler(mcc->parent_rollout, cb, arg_list, count);
      }
   }
   else
      mcc->run_event_handler(mcc->parent_rollout, cb, arg_list, count);
   pop_value_local_array(arg_list);
   pop_alloc_frame();
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void MSResourceMakerCallback::ResetCallback(int curvenum, ICurveCtl *pCCtl)
{
   init_thread_locals();
   push_alloc_frame();
   one_value_local(num);
   vl.num = Integer::intern(curvenum + 1);
   mcc->run_event_handler(mcc->parent_rollout, n_reset, &vl.num, 1);
   pop_value_locals();
   pop_alloc_frame();
}

/* -------------------- MAXCurveCtl  ------------------- */

local_visible_class_instance (MAXCurveCtl, "MAXCurveCtl")

void
MAXCurveCtl::add_control(Rollout *ro, HWND parent, HINSTANCE hInst, int& current_y)
{
   // CurveControl <name>  [ <caption> ] [ visible: <boolean> ] [ numCurves: <integer> ]           \
   //                [ x_range: <point2> ] [ y_range: <point2> ] [ zoomValues: <point2> ]    \
   //                [ scrollValues: <point2> ] [ displayModes: <bitarray> ]                 \
   //                [ commandMode: <#move_xy | #move_x | #move_y | #scale | #corner | bezier> ]   \
   //                [ uiFlags: <array_of_ui_flags> ] [xValue:<float>                     \
   //                [ rcmFlags: <array_of_rcm_flags> ]  [ asPopup:<boolean> ]   

   caption = caption->eval();

   HWND  hLabel;
   int      left, top, width, height;
   SIZE  size;
   TCHAR*   label_text = caption->eval()->to_string();
         control_ID = next_id();
   WORD  label_id = next_id();
   DWORD ccflags;  // cc flags
   bool  zoom = true;

   // LAM - 11/11/02 - defect 466888 - cctl->SetActive later on creates a RestoreObj
   HoldSuspend hs (true);
// if(theHold.Holding()) theHold.Cancel();
   /* add 2 controls for a control: the label static & curveControl frame*/
   parent_rollout = ro;
   
   cctl = (ICurveCtl*)CreateInstance(REF_MAKER_CLASS_ID,CURVE_CONTROL_CLASS_ID);
   ccflags = cctl->GetCCFlags();
   uiFlags = ccflags^CC_ALL_RCMENU;
   rcmFlags = ccflags&CC_ALL_RCMENU;
   resource_cb = new MSResourceMakerCallback(this);
   cctl->RegisterResourceMaker(resource_cb);

   // compute bounding box, apply layout params
   layout_data pos;
   setup_layout(ro, &pos, current_y);
   int label_height = (strlen(label_text) != 0) ? ro->text_height + SPACING_BEFORE - 2 : 0;
   Value* height_obj;
   int cc_height = int_control_param(height, height_obj, 200) + 7;
   process_layout_params(ro, &pos, current_y);
   pos.height = label_height + cc_height;
   current_y = pos.top + pos.height;

   // place optional label
   left = pos.left;
   Value*   asPopup = control_param(asPopup);
   if (asPopup != &unsupplied && asPopup->to_bool())
   {
      popup = true;
      cctl->SetTitle(label_text);
   }
   else 
   {
      popup = false;    
      // LAM - defect 298613 - not creating the caption HWND was causing problems (whole screen redrawn
      // when control moved, setting caption text set wrong HWND). Now always create.
//    if (label_height != 0)
//    {
         DLGetTextExtent(ro->rollout_dc, label_text, &size);
         width = min(size.cx, pos.width); height = ro->text_height;
         top = pos.top; 
         hLabel = CreateWindow(_T("STATIC"),
                           label_text,
                           WS_VISIBLE | WS_CHILD | WS_GROUP,
                           left, top, width, height,    
                           parent, (HMENU)label_id, hInst, NULL);
//    }
   }

   // place curve control
   top = pos.top + label_height;
   width = pos.width; height = cc_height; 
   hFrame = CreateWindow(_T("STATIC"),
                     _T(""),
                     WS_VISIBLE | WS_CHILD | WS_GROUP,
                     left, top, width, height,
                     parent, (HMENU)control_ID, hInst, NULL);
   DLSetWindowLongPtr(hFrame, this);
   DLSetWindowLongPtr(hFrame, CCTL_WndProc, GWLP_WNDPROC);
   cctl->SetCustomParentWnd(hFrame);
   cctl->SetMessageSink(hFrame);
    
   SendDlgItemMessage(parent, label_id, WM_SETFONT, (WPARAM)ro->font, 0L);
    SendDlgItemMessage(parent, control_ID, WM_SETFONT, (WPARAM)ro->font, 0L);

   /* setup the curve control */

   Value*   active = control_param(visible);
   Value*   numCurves = control_param(numCurves);
   Value*   x_rangeVal = control_param(x_range);
   Value*   y_rangeVal = control_param(y_range);
   Value*   zoomValues = control_param(zoomValues);
   Value*   scrollValues = control_param(scrollValues);
   Value*   displayModes = control_param(displayModes);
   Value*   commandMode = control_param(commandMode); 
   Value*   uiFlagsVal = control_param(uiFlags);
   Value*   rcmFlagsVal = control_param(rcmFlags); 
   Value*   xValueVal = control_param(xvalue);  
   
   if (ro->init_values)
   {
      def_cmdMode_types()
      int cmd_mode = (commandMode == &unsupplied) ? CID_CC_MOVE_XY :
         GetID(cmd_modes, elements(cmd_modes), commandMode);
      cctl->SetCommandMode(cmd_mode);
      
      // Set various flags
      if (uiFlagsVal != &unsupplied)
      {
         uiFlags = 0;
         if (!is_array(uiFlagsVal))
            throw RuntimeError(GetString(IDS_NEEDS_UIFLAGS_GOT), uiFlagsVal);
         def_uiFlag_types()
         for (int i=0; i < ((Array*)uiFlagsVal)->size; i++)
            uiFlags |= GetID(ui_flags, elements(ui_flags), ((Array*)uiFlagsVal)->data[i]);
      }
      if (rcmFlagsVal != &unsupplied)
      {
         rcmFlags = 0;
         if (!is_array(rcmFlagsVal))
            throw RuntimeError(GetString(IDS_NEEDS_RCMFLAGS_GOT), rcmFlagsVal);
         def_rcmFlag_types()
         for (int i=0; i < ((Array*)rcmFlagsVal)->size; i++)
            rcmFlags |= GetID(rcm_flags, elements(rcm_flags), ((Array*)rcmFlagsVal)->data[i]);
      }
      else
      {
         rcmFlags = (CC_RCMENU_MOVE_XY|CC_RCMENU_MOVE_X|CC_RCMENU_MOVE_Y|CC_RCMENU_SCALE|
            CC_RCMENU_INSERT_CORNER|CC_RCMENU_INSERT_BEZIER|CC_RCMENU_DELETE);
      }           
      ccflags = rcmFlags|uiFlags;
      if (popup) ccflags |= CC_ASPOPUP;
      else ccflags &= ~CC_ASPOPUP;  // by default not a popup window
      cctl->SetCCFlags(ccflags);

      if (xValueVal != &unsupplied) xvalue = xValueVal->to_float();
      cctl->SetCurrentXValue(xvalue);
      
      if (numCurves != &unsupplied) cctl->SetNumCurves(numCurves->to_int());
      if (x_rangeVal != &unsupplied) 
      {
         Point2 x_range = x_rangeVal->to_point2();
         cctl->SetXRange(x_range.x, x_range.y);
      }
      if (y_rangeVal != &unsupplied) 
      {
         Point2 y_range = y_rangeVal->to_point2();
         cctl->SetYRange(y_range.x, y_range.y);
      }
      if (zoomValues != &unsupplied) 
      {
         Point2 p = zoomValues->to_point2();
         cctl->SetZoomValues(p.x, p.y);
         zoom = false;
      }
      if (scrollValues != &unsupplied) 
      {
         Point2 p = scrollValues->to_point2();
         cctl->SetScrollValues((int)p.x, (int)p.y);
         zoom = false;
      }
      if (displayModes != &unsupplied) 
         cctl->SetDisplayMode(displayModes->to_bitarray());
      else
      {
         BitArray ba (cctl->GetNumCurves());
         ba.SetAll();
         cctl->SetDisplayMode(ba);
      }
      if (active != &unsupplied && !cctl->IsActive()) cctl->SetActive(active->to_bool());
      else cctl->SetActive(TRUE);
   }
   if (zoom) cctl->ZoomExtents();
}

MAXCurveCtl::MAXCurveCtl(Value* name, Value* caption, Value** keyparms, int keyparm_count) 
   : RolloutControl(name, caption, keyparms, keyparm_count)
{ 
   tag = class_tag(MAXCurveCtl); 
   cctl= NULL;
   resource_cb = NULL;
   xvalue = 0.0f;
}

MAXCurveCtl::~MAXCurveCtl()
{
    if (cctl) cctl->DeleteThis();
    if (resource_cb) delete resource_cb;
}

void
MAXCurveCtl::gc_trace()
{
   RolloutControl::gc_trace();
}

BOOL
MAXCurveCtl::handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam)
{
   // All messages are handled in the dialog proc
	if (message == WM_CONTEXTMENU)
	{
		call_event_handler(ro, n_rightClick, NULL, 0);
		return TRUE;
	}
   return FALSE;
}

Value*
MAXCurveCtl::get_property(Value** arg_list, int count)
{
   Value* prop = arg_list[0];
   
   if (!cctl) return &undefined; // control not yet created 
   if (prop == n_visible)     return cctl->IsActive() ? &true_value : &false_value;
   else if (prop == n_numCurves) return Integer::intern (cctl->GetNumCurves());
   else if (prop == n_x_range)      return new Point2Value (cctl->GetXRange());
   else if (prop == n_y_range)      return new Point2Value (cctl->GetYRange());
   else if (prop == n_displayModes) return new BitArrayValue (cctl->GetDisplayMode());
   else if (prop == n_commandMode) 
   {
      def_cmdMode_types()
      return GetName(cmd_modes, elements(cmd_modes), cctl->GetCommandMode());
   }
   else if (prop == n_zoomValues) 
   {
      Point2 p; 
      cctl->GetZoomValues(&p.x, &p.y);
      return new Point2Value (p);
   }
   else if (prop == n_scrollValues) 
   {
      int x, y; 
      cctl->GetScrollValues(&x, &y);
      return new Point2Value (Point2((float)x,(float)y));
   }  
   else if (prop == n_curves)
   {
      //push_alloc_frame();
      one_typed_value_local(Array* result); 
      int n = cctl->GetNumCurves();
      vl.result = new Array(n);
      for(int i = 0; i < n; i++)
         vl.result->append(MAXCurve::intern(cctl->GetControlCurve(i), cctl));
      return_value(vl.result); //Hey!
   }
   else if (prop == n_uiFlags)
   {
      one_typed_value_local(Array* result);
      def_uiFlag_types()
      int n = elements(ui_flags) - 1; //do not count #all
      vl.result = new Array(n); 
      for(int i = 0; i < n; i++)
         if (uiFlags&ui_flags[i].id)
            vl.result->append(ui_flags[i].val);
      return_value(vl.result); //Hey!
   }
   else if (prop == n_rcmFlags)
   {
      one_typed_value_local(Array* result);
      def_rcmFlag_types()
      int n = elements(rcm_flags) - 1; //do not count #all
      vl.result = new Array(n);
      for(int i = 0; i < n; i++)
         if (rcmFlags&rcm_flags[i].id)
            vl.result->append(rcm_flags[i].val);
      return_value(vl.result); //Hey!
   }
   else if (prop == n_xvalue) return Float::intern(xvalue);
   return RolloutControl::get_property(arg_list, count);
}

Value*
MAXCurveCtl::set_property(Value** arg_list, int count)
{
   Value* val = arg_list[0];
   Value* prop = arg_list[1]; 
   
   if (!cctl) return &undefined; // control not yet created 
   if (prop == n_text || prop == n_caption)
   {
      TCHAR* text = val->to_string();
      caption = val->get_heap_ptr();
      if (parent_rollout != NULL && parent_rollout->page != NULL)
         set_text(text, GetDlgItem(parent_rollout->page, control_ID), n_left);
   }  
   else if(prop == n_commandMode) 
   {
      def_cmdMode_types()  
      cctl->SetCommandMode(GetID(cmd_modes, elements(cmd_modes), val));
   }
   else if (prop == n_visible) 
   {
      type_check(val, Boolean, _T("visible:"));
      bool visible = (val == &true_value);
      cctl->SetActive(visible);
      return RolloutControl::set_property(arg_list, count);
   }
   else if (prop == n_numCurves) cctl->SetNumCurves(val->to_int());
   else if (prop == n_displayModes) cctl->SetDisplayMode(val->to_bitarray());
   else if (prop == n_x_range) 
   {
      Point2 x_range = val->to_point2();
      cctl->SetXRange(x_range.x, x_range.y);
   }
   else if (prop == n_y_range) 
   {
      Point2 y_range = val->to_point2();
      cctl->SetYRange(y_range.x, y_range.y);
   }
   else if (prop == n_zoomValues) 
   {
      Point2 p = val->to_point2();
      cctl->SetZoomValues(p.x, p.y);
   }
   else if (prop == n_scrollValues) 
   {
      Point2 p = val->to_point2();
      cctl->SetScrollValues((int)p.x, (int)p.y);
   }
   else if (prop == n_uiFlags)
   {
      uiFlags = 0;
      if (!is_array(val))
         throw RuntimeError(GetString(IDS_NEEDS_UIFLAGS_GOT), val);
      def_uiFlag_types()
      for (int i=0; i < ((Array*)val)->size; i++)
         uiFlags |= GetID(ui_flags, elements(ui_flags), ((Array*)val)->data[i]);
      cctl->SetCCFlags(rcmFlags|uiFlags);
   }
   else if (prop == n_rcmFlags)
   {
      rcmFlags = 0;
      if (!is_array(val))
         throw RuntimeError(GetString(IDS_NEEDS_RCMFLAGS_GOT), val);
      def_rcmFlag_types()
      for (int i=0; i < ((Array*)val)->size; i++)
         rcmFlags |= GetID(rcm_flags, elements(rcm_flags), ((Array*)val)->data[i]);
      cctl->SetCCFlags(rcmFlags|uiFlags);
   }
   else if (prop == n_xvalue)
   {
      xvalue = val->to_float();
      cctl->SetCurrentXValue(xvalue);
   }
   else if (prop == n_curves) 
      throw RuntimeError (GetString(IDS_READONLY_PROPERTY), _T("curves"));
   else return RolloutControl::set_property(arg_list, count);
   if (cctl) cctl->Redraw();
   return val;
}

void
MAXCurveCtl::set_enable()
{
   if (parent_rollout != NULL && parent_rollout->page != NULL && cctl)
   {
      // set curve control enable
      EnableWindow(cctl->GetHWND(), enabled);
      EnableWindow(hFrame, enabled);
      // set caption enable
      HWND ctrl = GetDlgItem(parent_rollout->page, control_ID);
      if (ctrl)
         EnableWindow(ctrl, enabled);
      InvalidateRect(parent_rollout->page, NULL, TRUE);
   }
}

Value*
MAXCurveCtl::zoom_vf(Value** arg_list, int count)
{
   // zoom <ccCtl> #all
   check_gen_arg_count(zoomExtents, 2, count);
   if (!cctl) return &ok;
   if (arg_list[0] == n_all)
      cctl->ZoomExtents();
   else
   {
      Point2 z = arg_list[0]->to_point2();
      cctl->SetZoomValues(z.x, z.y);
   }
   return &ok;
}

/* --------------------- plug-in init --------------------------------- */
// this is called by the dlx initializer, register the global var here
void cc_init()
{
   install_rollout_control(Name::intern("CurveControl"), MAXCurveCtl::create);
#include <maxscript\macros\local_implementations.h>

#  include "ccnames.h"

}

