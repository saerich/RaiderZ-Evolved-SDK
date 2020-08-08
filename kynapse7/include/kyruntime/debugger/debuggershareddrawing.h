/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file 
	\ingroup kyruntime_debugger */

// primary contact: ? - secondary contact: LAPA
#ifndef KYDEBUGGERSHAREDDRAWING_H
#define KYDEBUGGERSHAREDDRAWING_H

#include <kypathdata/containers/array.h>

namespace Kaim
{
	namespace Debugger
	{
		namespace Shared
		{
			/*!	This structure stores a list of points and lines, each associated with a rendering color. The 
				RemoteDebugging service uses an instance of this structure to store the list of points and 
				lines to be drawn in the visual %Debugger interface. This list is sent to the remote %Debugger
				interface at each frame.
				\ingroup kyruntime_debugger */
			struct Drawing
			{
				/*!	Represents a color using red, green, blue and alpha values.
					\ingroup kyruntime_debugger */
				struct Color
				{
					KyUInt32 m_red; /*!< \label_redval */
					KyUInt32 m_green; /*!< \label_greenval */
					KyUInt32 m_blue; /*!< \label_blueval */
					KyUInt32 m_alpha; /*!< \label_alphaval */

					/*!	Sets the channel values for the color.
						\param red				\label_redval
						\param green			\label_greenval
						\param blue				\label_blueval
						\param alpha			\label_alphaval */
					void Set(KyUInt32 red, KyUInt32 green, KyUInt32 blue, KyUInt32 alpha)
					{
						m_red = red;
						m_green = green;
						m_blue = blue;
						m_alpha = alpha;
					}
				};

				/*!	Represents a single point, defined by a position in three-dimensional space and a color.
					\ingroup kyruntime_debugger */
				struct Point
				{
					Kaim::Vec3f m_position; /*!< The coordinates of the point in three-dimensional space. */
					Color m_color; /*!< The color associated with the point. */

					/*!	Sets the coordinates and color of the point.
						\param position			The coordinates of the point in three-dimensional space.
						\param color			The color associated with the point.
					*/
					void Set(const Kaim::Vec3f& position, const Color& color)
					{
						m_position = position;
						m_color = color;
					}
				};

				/*!	Represents a single line segment, defined by the positions of its two endpoints in 
					three-dimensional space and a color.
					\ingroup kyruntime_debugger */
				struct Line
				{
					Kaim::Vec3f m_position[2]; /*!< The coordinates of the endpoints of the line segment. */
					Color m_color; /*!< The color associated with the line. */

					/*!	Sets the coordinates and color of the line.
						\param p1				The coordinates of one endpoint of the line in three-dimensional space.
						\param p2				The coordinates of the other endpoint of the line in three-dimensional space.
						\param color			The color associated with the line. */
					void Set(const Kaim::Vec3f& p1, const Kaim::Vec3f& p2, const Color& color)
					{
						m_position[0] = p1;
						m_position[1] = p2;
						m_color = color;
					}
				};

				/*!	Adds the specified point to the list of points maintained in this Drawing instance. */
				void PushBack(const Point& point)
				{
					m_pointArray.PushBack(point);
				}


				/*!	Adds the specified line segment to the list of lines maintained in this Drawing instance. */
				void PushBack(const Line& line)
				{
					m_lineArray.PushBack(line);
				}

				/*!	Clears the lists of points and lines maintained in this Drawing instance. */
				void Clear()
				{
					m_pointArray.Clear();
					m_lineArray.Clear();
				}

				/*!	Initializes the Drawing instance for first use. For internal use. */
				void Initialize()
				{
					// todo : m_pointArray.Reserve(...);
					// todo : m_lineArray.Reserve(...);
					Clear();
				}
				
				/*!	Destroys the Drawing instance when no longer needed. For internal use. */
				void Terminate()
				{
					m_pointArray.ForceClear();
					m_lineArray.ForceClear();
				}

				/*!	Contains the list of points. Do not modify directly. */
				Kaim::Array<Point> m_pointArray;

				/*!	Contains the list of line segments. Do not modify directly. */
				Kaim::Array<Line> m_lineArray;
			};

			inline bool operator < (const Drawing::Point& /*a*/, const Drawing::Point& /*b*/)
			{
				return false;
			}

			inline bool operator < (const Drawing::Line& /*a*/, const Drawing::Line& /*b*/)
			{
				return false;
			}

		}
	}
}

#endif // KYDEBUGGERSHAREDDRAWING_H
