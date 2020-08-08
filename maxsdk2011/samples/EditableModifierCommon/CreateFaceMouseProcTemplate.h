/**********************************************************************
*<
FILE: CreateFaceMouseProcTemplate.h

DESCRIPTION: Template for face creation procedures used by editable poly, edit
poly modifier, editable mesh, edit mesh modifier, editable patch, and edit
patch modifier. Gamma et al. describe the template design pattern in "Design
Patterns: Elements of Reusable Object-Oriented Software". 

CREATED BY: Peter Feiner

HISTORY: created November 2006

*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/

#ifndef __CreateFaceMouseProcTemplate_h__
#define __CreateFaceMouseProcTemplate_h__

#include "containers/Array.h"
#include "windef.h"
#include "mouseman.h"
#include "strbasic.h"

class ViewExp;
class Interface;
class Point3;
class INode;
class GraphicsWindow;

// A generalization of a mouse procedure for face creation. This class
// implements the template design pattern described by Gamma et al. in "Design
// Patterns: Elements of Reusable Object-Oriented Software". Hence this class
// provides a general implementation of MouseCallBack::proc. proc is
// implemented in terms of the template operations. This class also receives
// EventRouter::Notify messages that trigger the Backspace function.
class CreateFaceMouseProcTemplate : public MouseCallBack, public EventUser {
public:

	CreateFaceMouseProcTemplate(Interface* in_interface);
	virtual ~CreateFaceMouseProcTemplate();

	// This is the template method. It implements MouseCallBack::proc. The
	// mouse procedure works like this: This method adds vertices to the face
	// until GetMaxDegree() is reached. New vertices are created when
	// HitTestVertex() returns false. Existing vertices are used when
	// HitTestVertex() returns true. The first node returned by HitTestVertex()
	// is set as the primary node. As the user moves the mouse around, the
	// cursor is changed to reflect the value of HitTestVertex().
	int proc(HWND in_hwnd, int in_msg, int in_point, int in_flags, IPoint2 in_m );

	// Implements EventUser::Notify, calls Backspace. The user must register
	// instances of this class with an instance of EventRouter for this to
	// work. 
	void Notify();

	// Removes the most recent vertex from the face. If the vertex was newly
	// created, then it's deleted.
	void Backspace();

	// Draws a red polygon connecting the established points. The user has to
	// call this function.
	void DrawEstablishedFace(GraphicsWindow* gw); 

protected:
	// The following pure virtual functions are the template operations.
	// Subclasses provide the implementations. 

	// Returns the message that appears in the Edit > Undo menu after successfully 
	// creating a face.
	virtual TSTR GetFaceCreationUndoMessage() = 0;
	// Returns a message describing a failed face creation.
	virtual TSTR GetFailedFaceCreationMessage() = 0;
	// Returns a title for the failed face creation report (i.e., for a dialog
	// box)
	virtual TSTR GetFailedFaceCreationTitle() = 0;

	static const int UNLIMITED_DEGREE = -1;

	// Returns the maximum degree of the face. After the user clicks
	// GetMaxDegree() points, the face will be created. Return UNLIMITED_DEGREE
	// if there's no maximum degree 
	virtual int GetMaxDegree() = 0;

	// Attempts to create a vertex at the given location. If the operation
	// fails, then false is returned and the value of out_vertex is undefined.
	// If the operation succeeds, then true is returned and the index of the
	// new vertex is written to out_vertex. The index is only used in as an
	// argument to CreateFace. All vertices should be created on the primary
	// node, if HasPrimaryNode() is true; a suitable default node should be 
	// chosen otherwise. 
	virtual bool CreateVertex(const Point3& in_worldLocation, int& out_vertex) = 0;

	// Attempts to create a face comprising the given vertex indices. If the
	// operation fails, then false is returned. If the operation succeeds,
	// then true is returned. The given vertex indices will be those given by
	// CreateVertex and HitTestVertex. The face should be created on the primary
	// node, if HasPrimaryNode() is true; a suitable default node should be 
	// chosen otherwise. 
	virtual bool CreateFace(MaxSDK::Array<int>& in_vertices) = 0;

	// Performs a vertex hit test at the mouse location in the given viewport.
	// If a vertex exists at that location that can be a part of a new face,
	// then true is returned and the vertex's index is written to out_vertex.
	// The INode that the vertex belongs to is written to out_node. If no such
	// vertex exists, then false is returned, out_node is undefined, and
	// out_vertex is undefined. The values of out_vertex and out_node are used 
	// for CreateFace and GetPrimaryNode respectively. 
	virtual bool HitTestVertex(IPoint2 in_mouse, ViewExp* in_viewport, INode*& out_node, int& out_vertex) = 0;

	// Informs the mesh/patch/poly that the display should be updated.
	virtual void DisplayDataChanged() = 0;
	
	// Displays a cursor indicating that a new vertex will be created if the user clicks.
	virtual void ShowCreateVertexCursor() = 0;

	// Displays a cursor indicating that an existing vertex will be used if the user clicks.
	virtual void ShowExistingVertexCursor() = 0;

	//Get the world position of the passed in vertex index. If index is invalid,returns 0,0,0
	virtual Point3 GetVertexWP(int in_vertex) = 0;

	// The following methods are provided by CreateFaceMouseProcTemplate:

	// Returns true iff the primary object has been set. 
	bool HasPrimaryNode();

	// Returns the primary node. Precondition: HasPrimaryObject() is true.
	INode* GetPrimaryNode();

	// Returns the Interface* passed to the constructor. 
	Interface* GetInterface();

private:

	// Data used by Backspace(), DrawEstablishedFace(), and DrawCreatingFace(). 
	struct PointData {
		// index into the Mesh/Patch/PolyObject
		int m_vertex;

		// 3d world coordinate
		Point3 m_worldLocation;

		// coordinate of mouse-click location
		IPoint2 m_screenLocation;

		// true if this is a new vertex; false if already a member of the mesh
		bool m_new;

		PointData(int in_vertex, Point3 in_object, IPoint2 in_screen, bool in_new) :
			m_vertex(in_vertex), m_worldLocation(in_object), m_screenLocation(in_screen), m_new(in_new) {}
	};

	// Deletes all created vertices. Then re-creates all vertices using
	// CreateVertex method. Returns false if any call to CreateVertex fails. 
	bool RecreateVertices();

	// All of the points in the order that they were added. 
	MaxSDK::Array<PointData> m_points;

	// The first hit record that was returned from the first call to
	// HitTestVertex to return true. 
	INode* m_primaryNode;

	// The number of points in m_points that are on the primary node. 
	int m_pointsOnPrimaryNode;

	// The most recent location of the mouse. Written to by proc and used by
	// DrawCreatingFace().
	IPoint2 m_mouseLocation;

	// The interface passed to the constructor.
	Interface* m_interface;

	// Attempts to create a new face with the user-supplied points. If the face
	// can't be created, then the user is prompted with a dialog box. 
	void Finish(ViewExp* in_vpt);

	// Draws dotted lines showing the last two edges of the face. 
	void DrawCreatingFace (HWND in_hwnd, bool bErase, bool bPresent);
};

#endif
