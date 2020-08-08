/**********************************************************************
 *<
	FILE: testonly.h

	DESCRIPTION: The file contains test only functions. It should be 
				 removed from the shipping version of Max 
	CREATED BY: Ravi Karra

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

//#define INC_TESTONLY /* uncomment this to expose functions in testonly struct */
#ifdef INC_TESTONLY

def_struct_primitive( RenderFrame,				testonly, "RenderFrame");
def_struct_primitive( TestNode, 				testonly, "TestNode");
def_struct_primitive( TestTMAxis, 				testonly, "TestTMAxis");
def_struct_primitive( TSetScale, 				testonly, "TSetScale");
def_struct_primitive( TestMatrix3, 				testonly, "TestMatrix3");
def_struct_primitive( ViewLocalRot, 			testonly, "ViewLocalRot");
def_struct_primitive( ViewWorldRot, 			testonly, "ViewWorldRot");
def_struct_primitive( GetSpaceWarp, 			testonly, "GetSpaceWarp");
def_struct_primitive( NumCycles, 				testonly, "NumCycles");
def_struct_primitive( NewBitmapInfo, 			testonly, "NewBitmapInfo");
def_struct_primitive( UndoRedoMultipleSelect, 	testonly, "UndoRedoMultipleSelect");
def_struct_primitive( TestExpr, 				testonly, "TestExpr");
def_struct_primitive( PutSceneAppData, 			testonly, "PutSceneAppData");
def_struct_primitive( GetSceneAppData, 			testonly, "GetSceneAppData");
def_struct_primitive( GetSubAnims, 				testonly, "GetSubAnims");
def_struct_primitive( BitmapWrite, 				testonly, "BitmapWrite");
def_struct_primitive( Create, 					testonly, "Create");
def_struct_primitive( Load, 					testonly, "Load");
def_struct_primitive( GetRenderMeshBug, 		testonly, "GetRenderMeshBug");
def_struct_primitive( AddNewNamedSelSet, 		testonly, "AddNewNamedSelSet");
def_struct_primitive( SnapAngle,				testonly, "SnapAngle");
def_struct_primitive( GetMatID,					testonly, "GetMatID");
def_struct_primitive( SelectionDistance,		testonly, "SelectionDistance");
def_struct_primitive( GetCommandStackSize,		testonly, "GetCommandStackSize");
def_struct_primitive( CheckForRenderAbort,		testonly, "CheckForRenderAbort");
def_struct_primitive( AddGridToScene,			testonly, "AddGridToScene");
def_struct_primitive( GetImportZoomExtents,		testonly, "GetImportZoomExtents");
def_struct_primitive( GetSavingVersion,			testonly, "GetSavingVersion");
/*def_struct_primitive( IsTrialLicense,			testonly, "IsTrialLicense");
def_struct_primitive( IsEmergencyLicense,		testonly, "IsEmergencyLicense");
def_struct_primitive( IsNetworkLicense,			testonly, "IsNetworkLicense");*/

#endif //INC_TESTONLY