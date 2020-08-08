#include "max.h"
#include "point3.h"
#include "strbasic.h"
#include "gfx.h"
#include "hold.h"
#include "CreateFaceMouseProcTemplate.h"
#include "omanapi.h"

CreateFaceMouseProcTemplate::CreateFaceMouseProcTemplate(Interface* in_interface) : m_interface(in_interface),
	m_pointsOnPrimaryNode(0), m_primaryNode(0) {
}

CreateFaceMouseProcTemplate::~CreateFaceMouseProcTemplate() {
}

Interface* CreateFaceMouseProcTemplate::GetInterface() {
	return m_interface;
}

int CreateFaceMouseProcTemplate::proc(HWND in_hwnd, int in_msg, int in_point, int in_flags, IPoint2 in_m ) {

	ViewExp *l_vpt = m_interface->GetViewport(in_hwnd);
	int l_dummyVert(0);
	INode* l_dummyNode = NULL;

	switch (in_msg) {
	case MOUSE_PROPCLICK:
		m_interface->PopCommandMode();
		break;

	case MOUSE_POINT:
		{	// use brace for local variables
			if (in_point==1) break;	// Don't want to react to release from first click.
			m_interface->SetActiveViewport(in_hwnd);

			if (in_point == 0) {
				m_points.removeAll();

				// This holds the creation of the 
				// vertices and the creation of the face.
				DbgAssert(!theHold.Holding());
				if (theHold.Holding()) {
					theHold.Cancel();
				}
				theHold.Begin();
			}

			//MZ fix for 913957.. the logic that was here was wrong based upon what's in the spec. So we've changed.
			//it to match up what's in the spec;
			IOsnapManager7* pOsnapMgr = IOsnapManager7::GetInstance();
			Matrix3 l_constructionPlane;
			l_vpt->GetConstructionTM(l_constructionPlane);
			IPoint2 l_snapScreenPoint;
			int l_vertex(0);
			INode* l_node = NULL;
			//If snaps are on and we are snapping to a point (via the IsHolding call)
			if(m_interface->GetSnapState()==TRUE&&pOsnapMgr&&pOsnapMgr->IsHolding())
			{ //snapping
				//we find the snap point..we force 3d since we did for the preview
				Point3 l_snapPoint = l_vpt->SnapPoint(in_m,l_snapScreenPoint,&l_constructionPlane, SNAP_FORCE_3D_RESULT|SNAP_SEL_SUBOBJ_ONLY);
				// from construction plane -> world coordinates
				l_snapPoint = l_snapPoint * l_constructionPlane;  

				//we hit test the location of the returned snap point to a vertex.. since maybe we snapped to a vertex and so need to use
				//that vertex to create the face
				if (HitTestVertex(l_snapScreenPoint, l_vpt, l_node, l_vertex))
				{
					DbgAssert(l_node != 0);
					//it's possible that the hit point that we are hitting is not 'near' the world space snap point
					Point3 l_vertexPoint = GetVertexWP(l_vertex);
					Point3 l_diff = l_snapPoint - l_vertexPoint;
					if(l_diff.LengthSquared()>1e-5f) //assume it's a different point.. and we need to create the vertex!
					{
						//the vertex and snap points are far away, so make a new vertex at the snap point!
						if (CreateVertex(l_snapPoint,  l_vertex))
						{
							m_points.append(PointData(l_vertex, l_snapPoint, l_snapScreenPoint, true));
							DisplayDataChanged();
							m_interface->RedrawViews(m_interface->GetTime());
						} else {
							DbgAssert(false); // what could be done here? 
						}
						m_mouseLocation = l_snapScreenPoint;
						DrawCreatingFace(in_hwnd,false,true);
					}
					else //the snap point world's location and a vertex's position are 'near' so use the vertex!
					{
						m_pointsOnPrimaryNode++;
						// If this is the first point clicked, establish the primary object. All of the 
						// vertices have to be recreated to ensure that they're on the primary object.
						if (m_pointsOnPrimaryNode == 1) { 
							m_primaryNode = l_node;
							RecreateVertices();
						}

						// Finish if this vertex has already been clicked.
						for (int i = 0; i < m_points.length(); i++) {
							if (m_points[i].m_vertex == l_vertex) {
								Finish(l_vpt);
								return FALSE;
							}
						}

						m_points.append(PointData(l_vertex, l_snapPoint, l_snapScreenPoint, false));
						DisplayDataChanged();
						m_interface->RedrawViews(m_interface->GetTime());
						m_mouseLocation = l_snapScreenPoint;
						DrawCreatingFace(in_hwnd,false,true);
					}
				}
				else //didn't hit a vertex, so obviously use snap point to create a new one
				{ 
					if (CreateVertex(l_snapPoint,  l_vertex))
					{
						m_points.append(PointData(l_vertex, l_snapPoint, l_snapScreenPoint, true));
						DisplayDataChanged();
						m_interface->RedrawViews(m_interface->GetTime());
					} else {
						DbgAssert(false); // what could be done here? 
					}
					m_mouseLocation = l_snapScreenPoint;
					DrawCreatingFace(in_hwnd,false,true);
				}
			}
			else //not snapping or snapping and didn't hit anything. (nothing holding)
			{
				//first see if we intersect with a vertex (NOT at the snap point but at the pick point!!)
				if (HitTestVertex(in_m, l_vpt, l_node, l_vertex))
				{
					m_pointsOnPrimaryNode++;
					// If this is the first point clicked, establish the primary object. All of the 
					// vertices have to be recreated to ensure that they're on the primary object.
					if (m_pointsOnPrimaryNode == 1) { 
						m_primaryNode = l_node;
						RecreateVertices();
					}

					// Finish if this vertex has already been clicked.
					for (int i = 0; i < m_points.length(); i++) {
						if (m_points[i].m_vertex == l_vertex) {
							Finish(l_vpt);
							return FALSE;
						}
					}
					Point3 l_vertexPoint = GetVertexWP(l_vertex);
					m_points.append(PointData(l_vertex, l_vertexPoint, in_m, false));

					DisplayDataChanged();
					m_interface->RedrawViews(m_interface->GetTime());
					m_mouseLocation = in_m;
					DrawCreatingFace(in_hwnd,false,true);
				}
				else
				{
					//create the vertex on the construction grid.
					Point3 l_snapPoint = l_vpt->SnapPoint(in_m,l_snapScreenPoint,&l_constructionPlane, SNAP_FORCE_3D_RESULT|SNAP_SEL_SUBOBJ_ONLY);
					// from construction plane -> world coordinates
					l_snapPoint = l_snapPoint * l_constructionPlane;  
					if (CreateVertex(l_snapPoint,  l_vertex)) {
						m_points.append(PointData(l_vertex, l_snapPoint, l_snapScreenPoint, true));
						DisplayDataChanged();
						m_interface->RedrawViews(m_interface->GetTime());
					} else {
						DbgAssert(false); // what could be done here? 
					}

					m_mouseLocation = l_snapScreenPoint;
					DrawCreatingFace(in_hwnd,false,true);
				}
			}
			if (GetMaxDegree() != UNLIMITED_DEGREE && m_points.length() == GetMaxDegree()) {
				Finish(l_vpt);
				return FALSE;
			}

			break;
		}
	case MOUSE_MOVE:
		l_vpt->SnapPreview(in_m, in_m, NULL, SNAP_FORCE_3D_RESULT);
		m_interface->RedrawViews(m_interface->GetTime());

		if (m_points.length() > 0) {
			DrawCreatingFace(in_hwnd,true,false);
		}
		if (HitTestVertex(in_m,l_vpt, l_dummyNode,l_dummyVert)) {
			SetCursor(m_interface->GetSysCursor(SYSCUR_SELECT));
			ShowExistingVertexCursor();
		} else {
			ShowCreateVertexCursor();
		}
		//! Erase the old line..
		DrawCreatingFace(in_hwnd,true,false);
		m_mouseLocation = in_m;
		DrawCreatingFace(in_hwnd,false,true);
		break;

	case MOUSE_FREEMOVE:
		l_vpt->SnapPreview(in_m, in_m, NULL, SNAP_FORCE_3D_RESULT);
		if (HitTestVertex(in_m,l_vpt, l_dummyNode,l_dummyVert)) {
			ShowExistingVertexCursor();
		} else {
			l_vpt->SnapPreview(in_m, in_m, NULL, SNAP_FORCE_3D_RESULT); // is this necessary??
			ShowCreateVertexCursor();
		}
		break;

	case MOUSE_ABORT:
		// In the case of right-clicking or pressing escape, create the face instead 
		// of aborting the operation. This is for consistency with spline creation.
		Finish(l_vpt);
		return FALSE;
	}

	m_interface->ReleaseViewport(l_vpt);
	return TRUE;

}

void CreateFaceMouseProcTemplate::Finish(ViewExp* in_vpt) {
	// We're done collecting verts - build a face
	int l_facePoints = (int)m_points.length();
	MaxSDK::Array<int> l_vertIndices;
	for (int i = 0; i < m_points.length(); i++) l_vertIndices.append(m_points[i].m_vertex);
	m_points.removeAll();

	// Try to create the face. If there are enough points (3) and the 
	// face creation is successful, then accept the hold. Otherwise
	// cancel the hold to delete new vertices.

	if (l_facePoints >= 3 && CreateFace(l_vertIndices)) {
		theHold.Accept(GetFaceCreationUndoMessage());
	} else {
		theHold.Cancel(); // this deletes all of the new vertices
		DisplayDataChanged();
		InvalidateRect(in_vpt->getGW()->getHWnd(),NULL,FALSE);
		if (l_facePoints >= 3) {
			MessageBox(m_interface->GetMAXHWnd(),GetFailedFaceCreationMessage(),GetFailedFaceCreationTitle(),MB_OK|MB_ICONINFORMATION);						
		}

	}
	m_primaryNode = 0;
	m_pointsOnPrimaryNode = 0;
	m_interface->RedrawViews(m_interface->GetTime());
	m_interface->ReleaseViewport(in_vpt);
}

void CreateFaceMouseProcTemplate::DrawCreatingFace(HWND in_hwnd, bool bErase, bool bPresent) {
	if (m_points.length() < 1) return;

	Tab<IPoint2> points;
	points.Append(1,&m_points[0].m_screenLocation,5);
	points.Append(1,&m_mouseLocation,5);

	if (points.Count() > 1) 
		points.Append(1,&m_points.last().m_screenLocation,1);
		
	XORDottedPolyline(in_hwnd, points.Count(), points.Addr(0),0,bErase,bPresent);
	
}

void CreateFaceMouseProcTemplate::DrawEstablishedFace(GraphicsWindow* in_gw) {
	if (m_points.length() < 2) return;

	in_gw->setColor(LINE_COLOR,GetUIColor(COLOR_SUBSELECTION));

	// see documentation for GraphicsWindow::polyline ... it requires the array of vertices to have
	// room for 1 additional vertex
	Point3* l_locations = new Point3[m_points.length()+1];

	for (int i = 0; i < m_points.length(); i++) {
		l_locations[i] = m_points[i].m_worldLocation;
	}

	// set the graphics window matrix to I to ensure that world coordinates
	// are valid
	Matrix3 l_backup = in_gw->getTransform();
	const Matrix3 IDENTITY(true);
	in_gw->setTransform(IDENTITY);
	in_gw->polyline ((int)m_points.length(), l_locations, NULL, NULL, FALSE, NULL);
	in_gw->setTransform(l_backup);
	
	delete [] l_locations;
}

void CreateFaceMouseProcTemplate::Notify()
{
	Backspace();
}

void CreateFaceMouseProcTemplate::Backspace() 
{
	if (m_points.length() <= 1) return;

	// If the point wasn't newly created, then it was created on the primary object. 
	// Hence there is one less point on the primary object.
	if (!m_points.last().m_new) {
		m_pointsOnPrimaryNode--;
		if (m_pointsOnPrimaryNode == 0) {
			m_primaryNode = 0;
		}
	}
	m_points.removeLast();

	RecreateVertices();

	m_interface->RedrawViews(m_interface->GetTime());
}

bool CreateFaceMouseProcTemplate::RecreateVertices()
{
	// TODO (PSF, Nov 14, 2006): figure out why theHold.Restore() doesn't work.
	// I ought to be able to use theHold.Restore() instead of Cancel() and Begin(). 
	// For some reason, theHold.Restore() doesn't work in this situation:
	//     Create a face. Without exiting creation mode, start creating another face. 
	//     With at least two vertices created on the new face, start pressing backspace.
	//     After two backspaces, Max crashes. 
	// LocalCreateData::RemoveLastVertex is called too many times. Check this out!
	theHold.Cancel();
	theHold.Begin();

	bool l_result = true;

	for (int i = 0; i < m_points.length(); i++) {
		if (m_points[i].m_new) {
			l_result &= CreateVertex(m_points[i].m_worldLocation, m_points[i].m_vertex);
			DbgAssert(l_result);
		}
	}
	return l_result;
}

bool CreateFaceMouseProcTemplate::HasPrimaryNode() {
	// m_primaryNode had better not be NULL if true is going to be returned
	DbgAssert(m_pointsOnPrimaryNode <= 0 || m_primaryNode != 0);
	return m_pointsOnPrimaryNode > 0;
}

INode* CreateFaceMouseProcTemplate::GetPrimaryNode() {
	// Assert the precondition 
	DbgAssert(HasPrimaryNode());
	return m_primaryNode;
}