/*-------------------------------------------------------------------*//*!
 *
 * Umbra Occlusion Booster
 * -----------------------------------------
 *
 * (C) 2009-2010 Umbra Software Ltd. 
 * All Rights Reserved.
 *
 *
 * This file consists of unpublished, proprietary source code of
 * Umbra Software, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Umbra Software and legal action against the party in breach.
 *
 * \file
 * \brief    Platform independent commander sample
 * 
 *//*-------------------------------------------------------------------*/

// Platfromlib includes.
#include "plObject.hpp"
#include "platform.hpp"

// Application includes.
#include "commander.hpp"

// System includes.
#include <assert.h>

using namespace Platform;
using namespace BasicSample;

// Initialize the debug members
bool    SampleCommander::m_wireframe             = false;

/*-------------------------------------------------------------------*//*!
 *
 * \brief   SampleCommander constructor.
 * 
 * \param   serv    Pointer to a application specific implementation of
 *                  Umbra::LibraryDefs::GraphicsServices. This pointer
 *                  is passed on to Umbra::Commander constructor.
 *
 *//*-------------------------------------------------------------------*/

SampleCommander::SampleCommander(SampleGraphicsServices* serv)
:	Commander(serv),
    m_lineCount         (0)
{
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   SampleCommander destructor.
 *
 *//*-------------------------------------------------------------------*/

SampleCommander::~SampleCommander()
{
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Callback interface for Umbra Occlusion Booster.
 *
 *          After Umbra::Camera::processVisibility() is called,
 *          Umbra Occlusion Booster starts issuing commands, which 
 *          the application processes here.
 *
 *          See the implementation for more details on how each command
 *          has been implemented in this sample.
 *
 *          See Umbra::Commander::Command for more details on different 
 *          types of commands.
 *
 *//*-------------------------------------------------------------------*/

void SampleCommander::command(Command command)
{
	switch(command)
	{

	//--------------------------------------------------------------------
    // Called at the beginning of a frame's visibility resolve.
    // Per frame initialization is done here.
    //--------------------------------------------------------------------

	case RESOLVE_BEGIN:
		{
            m_lineCount = 0;
		}
		break;

	//--------------------------------------------------------------------
    // Called at the end of a frame's visibility resolve.
	//
	// This implementation renders all visible objects here. Objects are 
	// buffered in vector m_visibleObjects when processing command 
	// OBJECTS_VISIBLE. 
    //--------------------------------------------------------------------

	case RESOLVE_END:
		{
			// If wireframe mode enabled
            if (m_wireframe)
            {
				// Clear depth buffer
                Platform::clear(0.25f, 0.25f, 0.25f, 0.f, 1.0f, 0.f);
				// Prepare for wirerame rendering
                Platform::setState(Platform::WIREFRAME_DRAW_STATE);
            }
            else
            {
				// Otherwise prepare for normal color rendering
                Platform::setState(Platform::COLOR_DRAW_STATE);
            }

			if (m_visibleObjects.size() > 0)
			{
				// Render the buffered visible objects
                Platform::draw(&m_visibleObjects[0], (unsigned int)m_visibleObjects.size());
				m_visibleObjects.clear();
			}
		}
		break;

	

	//--------------------------------------------------------------------
    // View parameters changed. Occlusion Booster informs the
	// application on current camera parameters using this command.
	// Reflect these changes to the platform.
	//
	// It's not strictly necessary to use the matrices provided here,
	// but it's recommended. This ensures that Occlusion Booster 
	// internal representation is in sync with the rendering engine.
	//
	// In case there are multiple views and cameras, you can trust the
	// current viewing parameters to be delivered correctly here.
    //--------------------------------------------------------------------
    
	case VIEW_PARAMETERS_CHANGED:
		{
			const Viewer* viewer = getViewer();
			assert(viewer);

			Matrix4x4 projection;
			Matrix4x4 cellToCamera;
	        
			// Get the projection matrix used by Umbra OB.
			viewer->getProjectionMatrix((Umbra::Matrix4x4&)projection[0][0], Viewer::LEFT_HANDED_D3D);

			// Set the same projection matrix for the application.
			Platform::setProjectionMatrix(projection); 

			// Get Umbra OB's view matrix.
			viewer->getCellToCameraMatrix((Umbra::Matrix4x4&)cellToCamera[0][0]);

			// Set the same view matrix for the application.
			Platform::setViewMatrix(cellToCamera); 
		}
		break;
		
	//--------------------------------------------------------------------
	// Occlusion Booster reports visible objects using this command.
	//
    // Visible objects can either be buffered here or rendered 
	// immediately. Buffering can be beneficial to avoid state changes.
	//
	// Object details are accessed using ObjectData. If object is an
	// occluder (ObjectData::isOccluder), the object is to be rendered 
	// in the depth buffer either immediately or in FLUSH_OCCLUDERS. 
	// 
    //--------------------------------------------------------------------

    case OBJECTS_VISIBLE:
		{
            int count = getDataCount();

            for (int i = 0; i < count; i++)
            {
				// Get ObjectData for this object
				const ObjectData* objectData = getObjectData(i);
				assert(objectData);

				// Get our Platform::Object from the object's user pointer
                Platform::Object* userObject = (Platform::Object*)objectData->getUserPointer();
				assert(userObject);

				// Buffer the object

				// Add all occluders in m_depthObjects. 
				// Objects buffered in m_depthObjects are rendered
				// during FLUSH_OCCLUDERS.
                if (objectData->isOccluder())
					m_depthObjects.push_back(userObject);

				// Buffer all objects in m_visibleObjects. Objects
				// buffered here are rendered during RESOLVE_END.
				m_visibleObjects.push_back(userObject);
            }
		}
		break;

	//--------------------------------------------------------------------
    // Flushes all occluder objects reported visible in OBJECTS_VISIBLE.
	// Occluders are to be rendered only in depth buffer.
	//
	// Buffering the calls is especially useful when not using 
	// separate z-pass to allow batching of objects to minimize state 
	// changes.
    //--------------------------------------------------------------------

	case FLUSH_OCCLUDERS:
		{
            // depth prepass

			if (m_depthObjects.size())
			{
				// Enable depth draw state: color writing disabled,
				// depth testing and writing enabled.
    		    Platform::setState(Platform::DEPTH_DRAW_STATE); 
				
				// Draw all objects stored in m_depthObjects.
                Platform::draw(&m_depthObjects[0], (unsigned int)m_depthObjects.size());

				// Clear the depth object array.
				m_depthObjects.clear();
			}

			Platform::flush();
		}
		break;

	//--------------------------------------------------------------------
    // Render occlusion query test shapes into the Z-buffer. 
	// 
	// Occlusion queries are issued here. Occlusion Booster supplies a 
	// query object, created in GraphicsServices, that is to be used. A
	// test shape is also supplied, which is a mesh to be rendered 
	// during the Occlusion Query. This test shape is always bounding
	// box of a tested object.
	//
	// The results are read during GET_OCCLUSION_QUERY_RESULTS.
    //--------------------------------------------------------------------

    case ISSUE_OCCLUSION_QUERIES:
        {
            Platform::setState(Platform::DEPTH_TEST_STATE);

			// Prepare for rendering queries.
            Platform::setupQueryDraw();

			// Get query count.
            int dataCount = getDataCount();

            for (int i = 0; i < dataCount; i++)
            {
				// Get query data for the current query.
	            const OcclusionQueryData* qd = getOcclusionQueryData(i);
				assert(qd);

				// Issue occlusion query begin.
				// Note that the applications specific query objects 
				// accessed here are created in the GraphicsServices 
				// class (see SampleGraphicsServices).
				Platform::beginQuery( (Platform::Query*)qd->getQueryObject() );
				
				// Render query test shape.
				Platform::drawIndexedUP( (const Vector3*)qd->getVertices(), qd->getVertexCount(), (const Vector3i*)qd->getTriangles(), qd->getTriangleCount()); 

				// Issue occlusion query end.
                Platform::endQuery  ( (Platform::Query*)qd->getQueryObject() );
            }

            // Flush queries to ensure we can get the results back as soon as possible.
            Platform::flush();
        }
        break;

    //--------------------------------------------------------------------
    // Read back occlusion query results.
	//
	// Occlusion Booster reads back Occlusion Query results here. The
	// queries were issued earlier during ISSUE_OCCLUSION_QUERIES 
	// commands.
	//
	// The queries are referenced by QueryObject void pointer. These are
	// application specific query objects, that are created in 
	// GraphicsServices.
    //--------------------------------------------------------------------

	case GET_OCCLUSION_QUERY_RESULTS:
	    {
			// Number of query results to read.
            int count = getDataCount();

            for (int i = 0; i < count; i++)
            {
				// Get OcclusionQueryResultData for this occlusion query.
				OcclusionQueryResultData* queryData = getOcclusionQueryResultData(i);
				assert(queryData);

	            int result = 0;

				// Use platform to read the query result.
				// Variable "result" receives the result (number of unoccluded pixels),
				// queryData->getWaitForResult() indicates whether we're supposed to block waiting
				// for the results to become available.
                bool availability = Platform::getQueryResult(result, (Platform::Query*)queryData->getQueryObject(), queryData->getWaitForResult());
                if (!availability)
                    break;

                queryData->setResult(result);
            }
	    }
	    break;
    
    //--------------------------------------------------------------------
	// DRAW_LINE_2D: is a debug function for drawing screen space lines.
	// 
	// The lines can be batched for better performance.
	//--------------------------------------------------------------------

	case DRAW_LINE_2D:
		{
			Vector3 line[2];
			Vector4 color;
			getLine2D((Umbra::Vector2&)line[0].x, (Umbra::Vector2&)line[1].x, (Umbra::Vector4&)color.x);
            line[0].z = 0.f;
            line[1].z = 0.f;

			Platform::setState(Platform::DEBUGLINE_DRAW_STATE);
			Platform::drawLinesScreenSpace(line, 1, color); 
		}
		break;

	//--------------------------------------------------------------------
	// DRAW_LINE_3D is debug functionality for drawing e.g. bounding
	// boxes. See Library::setFlags() for enabling debug lines.
	// 
	// The lines can be batched for better performance.
	//--------------------------------------------------------------------

	case DRAW_LINE_3D:
		{
			Vector3 line[2];
			Vector4 color;
			getLine3D((Umbra::Vector3&)line[0].x, (Umbra::Vector3&)line[1].x, (Umbra::Vector4&)color.x);

            DebugLine dl;
            dl.a = line[0];
            dl.b = line[1];
            dl.c = color;

            m_lineCount++;

			Platform::setState(Platform::DEBUGLINE_DRAW_STATE);
			Platform::drawLines(line, 1, color); 
		}
		break;

    default:
        break;
	}
}

//------------------------------------------------------------------------
