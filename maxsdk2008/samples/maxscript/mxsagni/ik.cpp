/**********************************************************************
*<
FILE: ik.cpp

DESCRIPTION: 

CREATED BY: Larry Minton

HISTORY: Created 15 April 2007

*>	Copyright (c) 2007, All Rights Reserved.
**********************************************************************/

#include "MAXScrpt.h"
#include "Numbers.h"
#include "Strings.h"
#include "3DMath.h"
#include "MXSAgni.h"

#include "resource.h"

#include "ikctrl.h"

#ifdef ScripterExport
#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

// ============================================================================

#include "defextfn.h"
#	include "namedefs.h"

#include "agnidefs.h"

#include "definsfn.h"
#	include "ik_wraps.h"
// -----------------------------------------------------------------------------------------

Value*
ik_getAxisActive_cf(Value** arg_list, int count)
{
	check_arg_count(getAxisActive, 2, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], getAxisActive);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	if (whichType == 0)
		workingData = &rotData;
	else
		workingData = &posData;

	BitArray res (3);
	for (int i = 0; i <3; i++) res.Set(i, workingData->active[i]);
	return_value_no_pop( new BitArrayValue (res) );
}

Value*
ik_setAxisActive_cf(Value** arg_list, int count)
{
	check_arg_count(setAxisActive, 3, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], setAxisActive);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	if (whichType == 0)
		workingData = &rotData;
	else
		workingData = &posData;

	BitArray res (3);
	for (int i = 0; i <3; i++) res.Set(i, workingData->active[i]);

	BitArrayValue* newVal = (BitArrayValue*)arg_list[2];
	type_check(newVal, BitArrayValue, "setAxisActive")
		for (int i = 0; i <3; i++) 
			workingData->active[i] = newVal->bits[i];

	MAXScript_interface7->SuspendEditing(1<<TASK_MODE_HIERARCHY);
	controller->InitIKJoints(&posData,&rotData);
	controller->InitIKJoints2(&posData,&rotData);
	controller->NodeIKParamsChanged();
	MAXScript_interface7->ResumeEditing(1<<TASK_MODE_HIERARCHY);

	return_value_no_pop( new BitArrayValue (res) );
}


Value*
ik_getAxisLimit_cf(Value** arg_list, int count)
{
	check_arg_count(getAxisLimit, 2, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], getAxisLimit);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	if (whichType == 0)
		workingData = &rotData;
	else
		workingData = &posData;

	BitArray res (3);
	for (int i = 0; i <3; i++) res.Set(i, workingData->limit[i]);
	return_value_no_pop( new BitArrayValue (res) );
}

Value*
ik_setAxisLimit_cf(Value** arg_list, int count)
{
	check_arg_count(setAxisLimit, 3, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], setAxisLimit);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	if (whichType == 0)
		workingData = &rotData;
	else
		workingData = &posData;

	BitArray res (3);
	for (int i = 0; i <3; i++) res.Set(i, workingData->limit[i]);

	BitArrayValue* newVal = (BitArrayValue*)arg_list[2];
	type_check(newVal, BitArrayValue, "setAxisLimit")
		for (int i = 0; i <3; i++) 
			workingData->limit[i] = newVal->bits[i];

	MAXScript_interface7->SuspendEditing(1<<TASK_MODE_HIERARCHY);
	controller->InitIKJoints(&posData,&rotData);
	controller->InitIKJoints2(&posData,&rotData);
	controller->NodeIKParamsChanged();
	MAXScript_interface7->ResumeEditing(1<<TASK_MODE_HIERARCHY);

	return_value_no_pop( new BitArrayValue (res) );
}

Value*
ik_getAxisEase_cf(Value** arg_list, int count)
{
	check_arg_count(getAxisEase, 2, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], getAxisEase);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	if (whichType == 0)
		workingData = &rotData;
	else
		workingData = &posData;

	BitArray res (3);
	for (int i = 0; i <3; i++) res.Set(i, workingData->ease[i]);
	return_value_no_pop( new BitArrayValue (res) );
}

Value*
ik_setAxisEase_cf(Value** arg_list, int count)
{
	check_arg_count(setAxisEase, 3, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], setAxisEase);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	if (whichType == 0)
		workingData = &rotData;
	else
		workingData = &posData;

	BitArray res (3);
	for (int i = 0; i <3; i++) res.Set(i, workingData->ease[i]);

	BitArrayValue* newVal = (BitArrayValue*)arg_list[2];
	type_check(newVal, BitArrayValue, "setAxisEase")
		for (int i = 0; i <3; i++) 
			workingData->ease[i] = newVal->bits[i];

	MAXScript_interface7->SuspendEditing(1<<TASK_MODE_HIERARCHY);
	controller->InitIKJoints(&posData,&rotData);
	controller->InitIKJoints2(&posData,&rotData);
	controller->NodeIKParamsChanged();
	MAXScript_interface7->ResumeEditing(1<<TASK_MODE_HIERARCHY);

	return_value_no_pop( new BitArrayValue (res) );
}

Value*
ik_getAxisMin_cf(Value** arg_list, int count)
{
	check_arg_count(getAxisMin, 2, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], getAxisMin);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	Point3 res;
	if (whichType == 0)
	{
		workingData = &rotData;
		for (int i = 0; i <3; i++) res[i]= RadToDeg(workingData->min[i]);
	}
	else
	{
		workingData = &posData;
		res = workingData->min;
	}
	return_value_no_pop( new Point3Value (res) );
}

Value*
ik_setAxisMin_cf(Value** arg_list, int count)
{
	check_arg_count(setAxisMin, 3, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], setAxisMin);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	Point3 newVal = arg_list[2]->to_point3();
	Point3 res;
	if (whichType == 0)
	{
		workingData = &rotData;
		for (int i = 0; i <3; i++) res[i]= RadToDeg(workingData->min[i]);
		for (int i = 0; i <3; i++) workingData->min[i] = DegToRad(newVal[i]);
	}
	else
	{
		workingData = &posData;
		res = workingData->min;
		workingData->min = newVal;
	}

	MAXScript_interface7->SuspendEditing(1<<TASK_MODE_HIERARCHY);
	controller->InitIKJoints(&posData,&rotData);
	controller->InitIKJoints2(&posData,&rotData);
	controller->NodeIKParamsChanged();
	MAXScript_interface7->ResumeEditing(1<<TASK_MODE_HIERARCHY);

	return_value_no_pop( new Point3Value (res) );
}

Value*
ik_getAxisMax_cf(Value** arg_list, int count)
{
	check_arg_count(getAxisMax, 2, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], getAxisMax);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	Point3 res;
	if (whichType == 0)
	{
		workingData = &rotData;
		for (int i = 0; i <3; i++) res[i]= RadToDeg(workingData->max[i]);
	}
	else
	{
		workingData = &posData;
		res = workingData->max;
	}
	return_value_no_pop( new Point3Value (res) );
}

Value*
ik_setAxisMax_cf(Value** arg_list, int count)
{
	check_arg_count(setAxisMax, 3, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], setAxisMax);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	Point3 newVal = arg_list[2]->to_point3();
	Point3 res;
	if (whichType == 0)
	{
		workingData = &rotData;
		for (int i = 0; i <3; i++) res[i]= RadToDeg(workingData->max[i]);
		for (int i = 0; i <3; i++) workingData->max[i] = DegToRad(newVal[i]);
	}
	else
	{
		workingData = &posData;
		res = workingData->max;
		workingData->max = newVal;
	}

	MAXScript_interface7->SuspendEditing(1<<TASK_MODE_HIERARCHY);
	controller->InitIKJoints(&posData,&rotData);
	controller->InitIKJoints2(&posData,&rotData);
	controller->NodeIKParamsChanged();
	MAXScript_interface7->ResumeEditing(1<<TASK_MODE_HIERARCHY);

	return_value_no_pop( new Point3Value (res) );
}

Value*
ik_getAxisDamping_cf(Value** arg_list, int count)
{
	check_arg_count(getAxisDamping, 2, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], getAxisDamping);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	if (whichType == 0)
		workingData = &rotData;
	else
		workingData = &posData;

	return_value_no_pop( new Point3Value (workingData->damping) );
}

Value*
ik_setAxisDamping_cf(Value** arg_list, int count)
{
	check_arg_count(setAxisDamping, 3, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], setAxisDamping);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	if (whichType == 0)
		workingData = &rotData;
	else
		workingData = &posData;

	Point3 oldVal = workingData->damping;
	workingData->damping = arg_list[2]->to_point3();

	MAXScript_interface7->SuspendEditing(1<<TASK_MODE_HIERARCHY);
	controller->InitIKJoints(&posData,&rotData);
	controller->InitIKJoints2(&posData,&rotData);
	controller->NodeIKParamsChanged();
	MAXScript_interface7->ResumeEditing(1<<TASK_MODE_HIERARCHY);

	return_value_no_pop( new Point3Value (oldVal) );
}

Value*
ik_getAxisPreferredAngle_cf(Value** arg_list, int count)
{
	check_arg_count(getAxisPreferredAngle, 1, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], getAxisPreferredAngle);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	Point3 res;
	workingData = &rotData;
	for (int i = 0; i <3; i++) res[i]= RadToDeg(workingData->preferredAngle[i]);

	return_value_no_pop( new Point3Value (res) );
}

Value*
ik_setAxisPreferredAngle_cf(Value** arg_list, int count)
{
	check_arg_count(setAxisPreferredAngle, 2, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], setAxisPreferredAngle);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	workingData = &rotData;

	Point3 newVal = arg_list[1]->to_point3();
	Point3 res;
	workingData = &rotData;
	for (int i = 0; i <3; i++) res[i]= RadToDeg(workingData->preferredAngle[i]);
	for (int i = 0; i <3; i++) workingData->preferredAngle[i] = DegToRad(newVal[i]);

	MAXScript_interface7->SuspendEditing(1<<TASK_MODE_HIERARCHY);
	controller->InitIKJoints(&posData,&rotData);
	controller->InitIKJoints2(&posData,&rotData);
	controller->NodeIKParamsChanged();
	MAXScript_interface7->ResumeEditing(1<<TASK_MODE_HIERARCHY);

	return_value_no_pop( new Point3Value (res) );
}

Value*
ik_getAxisSpringOn_cf(Value** arg_list, int count)
{
	check_arg_count(getAxisSpringOn, 2, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], getAxisSpringOn);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	if (whichType == 0)
		workingData = &rotData;
	else
		workingData = &posData;

	InitJointData3* data3 = DowncastToJointData3(workingData);
	if (data3)
	{
		BitArray res (3);
		for (int i = 0; i <3; i++) res.Set(i, data3->springOn[i]);
		return_value_no_pop( new BitArrayValue (res) );
	}
	else
		return &undefined;
}

Value*
ik_setAxisSpringOn_cf(Value** arg_list, int count)
{
	check_arg_count(setAxisSpringOn, 3, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], setAxisSpringOn);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	if (whichType == 0)
		workingData = &rotData;
	else
		workingData = &posData;

	InitJointData3* data3 = DowncastToJointData3(workingData);
	if (data3)
	{
		BitArray res (3);
		for (int i = 0; i <3; i++) res.Set(i, data3->springOn[i]);

		BitArrayValue* newVal = (BitArrayValue*)arg_list[2];
		type_check(newVal, BitArrayValue, "setAxisSpringOn")
			for (int i = 0; i <3; i++) 
				data3->springOn[i] = newVal->bits[i] != FALSE;

		MAXScript_interface7->SuspendEditing(1<<TASK_MODE_HIERARCHY);
		controller->InitIKJoints(&posData,&rotData);
		controller->InitIKJoints2(&posData,&rotData);
		controller->NodeIKParamsChanged();
		MAXScript_interface7->ResumeEditing(1<<TASK_MODE_HIERARCHY);

		return_value_no_pop( new BitArrayValue (res) );
	}
	else
		return &undefined;
}

Value*
ik_getAxisSpring_cf(Value** arg_list, int count)
{
	check_arg_count(getAxisSpring, 2, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], getAxisSpring);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	if (whichType == 0)
		workingData = &rotData;
	else
		workingData = &posData;

	InitJointData3* data3 = DowncastToJointData3(workingData);
	if (data3)
		return_value_no_pop( new Point3Value (data3->spring) )
	else
	return &undefined;
}

Value*
ik_setAxisSpring_cf(Value** arg_list, int count)
{
	check_arg_count(setAxisSpring, 3, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], setAxisSpring);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	if (whichType == 0)
		workingData = &rotData;
	else
		workingData = &posData;

	InitJointData3* data3 = DowncastToJointData3(workingData);
	if (data3)
	{
		Point3 oldVal = data3->spring;
		data3->spring = arg_list[2]->to_point3();

		MAXScript_interface7->SuspendEditing(1<<TASK_MODE_HIERARCHY);
		controller->InitIKJoints(&posData,&rotData);
		controller->InitIKJoints2(&posData,&rotData);
		controller->NodeIKParamsChanged();
		MAXScript_interface7->ResumeEditing(1<<TASK_MODE_HIERARCHY);

		return_value_no_pop( new Point3Value (oldVal) );
	}
	else
		return &undefined;
}

Value*
ik_getAxisSpringTension_cf(Value** arg_list, int count)
{
	check_arg_count(getAxisSpringTension, 2, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], getAxisSpringTension);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	if (whichType == 0)
		workingData = &rotData;
	else
		workingData = &posData;

	InitJointData3* data3 = DowncastToJointData3(workingData);
	if (data3)
		return_value_no_pop( new Point3Value (data3->springTension) )
	else
	return &undefined;
}

Value*
ik_setAxisSpringTension_cf(Value** arg_list, int count)
{
	check_arg_count(setAxisSpring, 3, count);
	Control *controller = NULL;
	if (is_node(arg_list[0]))
	{
		INode* node = get_valid_node((MAXNode*)arg_list[0], setAxisSpringTension);
		controller = node->GetTMController();
	}
	else
		arg_list[0]->to_controller();

	def_IK_joint_types();
	int whichType = GetID(IKJointTypes, elements(IKJointTypes), arg_list[1]);

	InitJointData3 posData, rotData;
	BOOL res1 = controller->GetIKJoints(&posData,&rotData);
	BOOL res2 = controller->GetIKJoints2(&posData,&rotData);
	if (!res1 && !res2)
		return &undefined;

	InitJointData3 *workingData;
	if (whichType == 0)
		workingData = &rotData;
	else
		workingData = &posData;

	InitJointData3* data3 = DowncastToJointData3(workingData);
	if (data3)
	{
		Point3 oldVal = data3->springTension;
		data3->springTension = arg_list[2]->to_point3();

		MAXScript_interface7->SuspendEditing(1<<TASK_MODE_HIERARCHY);
		controller->InitIKJoints(&posData,&rotData);
		controller->InitIKJoints2(&posData,&rotData);
		controller->NodeIKParamsChanged();
		MAXScript_interface7->ResumeEditing(1<<TASK_MODE_HIERARCHY);

		return_value_no_pop( new Point3Value (oldVal) );
	}
	else
		return &undefined;
}

Value* 
ik_getStartTime_cf (Value** arg_list, int count)
{
	check_arg_count (GetStartTime, 1, count);
	INode* node = arg_list[0]->to_node();

	Control* controller = node->GetTMController();
	// LAM - defect ??? 
	ReferenceTarget* ref = (controller) ? (ReferenceTarget*)controller->GetInterface(I_MASTER) : NULL;
	if (ref && ref->ClassID() == IKMASTER_CLASSID)
	{
		IKMasterControl* themaster = (IKMasterControl*)ref;
		return MSTime::intern(themaster->GetStartTime());
	}
	else
		return &undefined;
}


Value* 
ik_setStartTime_cf (Value** arg_list, int count)
{
	check_arg_count (SetStartTime, 2, count);
	INode* node = arg_list[0]->to_node();

	Control* controller = node->GetTMController();
	// LAM - defect ??? 
	ReferenceTarget* ref = (controller) ? (ReferenceTarget*)controller->GetInterface(I_MASTER) : NULL;
	if (ref && ref->ClassID() == IKMASTER_CLASSID)
	{
		IKMasterControl* themaster = (IKMasterControl*)ref;
		themaster->SetStartTime(arg_list[1]->to_timevalue());
		needs_redraw_set();
		return (arg_list[1]);
	}
	else
		return &undefined;
}


Value* 
ik_getEndTime_cf (Value** arg_list, int count)
{
	check_arg_count (GetEndTime, 1, count);
	INode* node = arg_list[0]->to_node();

	Control* controller = node->GetTMController();
	// LAM - defect ??? 
	ReferenceTarget* ref = (controller) ? (ReferenceTarget*)controller->GetInterface(I_MASTER) : NULL;
	if (ref && ref->ClassID() == IKMASTER_CLASSID)
	{
		IKMasterControl* themaster = (IKMasterControl*)ref;
		return MSTime::intern(themaster->GetEndTime());
	}
	else
		return &undefined;

}


Value* 
ik_setEndTime_cf (Value** arg_list, int count)
{
	check_arg_count (SetEndTime, 2, count);
	INode* node = arg_list[0]->to_node();

	Control* controller = node->GetTMController();
	// LAM - defect ??? 
	ReferenceTarget* ref = (controller) ? (ReferenceTarget*)controller->GetInterface(I_MASTER) : NULL;
	if (ref && ref->ClassID() == IKMASTER_CLASSID)
	{
		IKMasterControl* themaster = (IKMasterControl*)ref;
		themaster->SetEndTime(arg_list[1]->to_timevalue());
		needs_redraw_set();
		return (arg_list[1]);
	}
	else
		return &undefined;
}


Value* 
ik_getIterations_cf (Value** arg_list, int count)
{
	check_arg_count (GetIterations, 1, count);
	INode* node = arg_list[0]->to_node();

	Control* controller = node->GetTMController();
	// LAM - defect ??? 
	ReferenceTarget* ref = (controller) ? (ReferenceTarget*)controller->GetInterface(I_MASTER) : NULL;
	if (ref && ref->ClassID() == IKMASTER_CLASSID)
	{
		IKMasterControl* themaster = (IKMasterControl*)ref;
		return Integer::intern(themaster->GetIterations());
	}
	else
		return &undefined;
}


Value* 
ik_setIterations_cf (Value** arg_list, int count)
{
	check_arg_count (SetIterations, 2, count);
	INode* node = arg_list[0]->to_node();

	Control* controller = node->GetTMController();
	// LAM - defect ??? 
	ReferenceTarget* ref = (controller) ? (ReferenceTarget*)controller->GetInterface(I_MASTER) : NULL;
	if (ref && ref->ClassID() == IKMASTER_CLASSID)
	{
		IKMasterControl* themaster = (IKMasterControl*)ref;
		themaster->SetIterations (arg_list[1]->to_int());
		needs_redraw_set();
		return (arg_list[1]);
	}
	else
		return &undefined;
}


Value* 
ik_getPosThreshold_cf (Value** arg_list, int count)
{
	check_arg_count (GetPosThreshold, 1, count);
	INode* node = arg_list[0]->to_node();

	Control* controller = node->GetTMController();
	// LAM - defect ??? 
	ReferenceTarget* ref = (controller) ? (ReferenceTarget*)controller->GetInterface(I_MASTER) : NULL;
	if (ref && ref->ClassID() == IKMASTER_CLASSID)
	{
		IKMasterControl* themaster = (IKMasterControl*)ref;
		return Float::intern(themaster->GetPosThresh());
	}
	else
		return &undefined;
}


Value* 
ik_setPosThreshold_cf (Value** arg_list, int count)
{
	check_arg_count (SetPosThreshold, 2, count);
	INode* node = arg_list[0]->to_node();

	Control* controller = node->GetTMController();
	// LAM - defect ??? 
	ReferenceTarget* ref = (controller) ? (ReferenceTarget*)controller->GetInterface(I_MASTER) : NULL;
	if (ref && ref->ClassID() == IKMASTER_CLASSID)
	{
		IKMasterControl* themaster = (IKMasterControl*)ref;
		themaster->SetPosThresh(arg_list[1]->to_float());
		needs_redraw_set();
		return (arg_list[1]);
	}
	else
		return &undefined;

}


Value* 
ik_getRotThreshold_cf (Value** arg_list, int count)
{
	check_arg_count (GetRotThreshold, 1, count);
	INode* node = arg_list[0]->to_node();

	Control* controller = node->GetTMController();
	// LAM - defect ??? 
	ReferenceTarget* ref = (controller) ? (ReferenceTarget*)controller->GetInterface(I_MASTER) : NULL;
	if (ref && ref->ClassID() == IKMASTER_CLASSID)
	{
		IKMasterControl* themaster = (IKMasterControl*)ref;
		return Float::intern(RadToDeg(themaster->GetRotThresh()));
	}
	else
		return &undefined;

}

Value* 
ik_setRotThreshold_cf (Value** arg_list, int count)
{
	check_arg_count (SetRotThreshold, 2, count);
	INode* node = arg_list[0]->to_node();

	Control* controller = node->GetTMController();
	// LAM - defect ??? 
	ReferenceTarget* ref = (controller) ? (ReferenceTarget*)controller->GetInterface(I_MASTER) : NULL;
	if (ref && ref->ClassID() == IKMASTER_CLASSID)
	{
		IKMasterControl* themaster = (IKMasterControl*)ref;
		themaster->SetRotThresh (DegToRad(arg_list[1]->to_float()));
		needs_redraw_set();
		return (arg_list[1]);
	}
	else
		return &undefined;

}