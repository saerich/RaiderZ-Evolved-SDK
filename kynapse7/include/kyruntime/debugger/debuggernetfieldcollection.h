/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERNETFIELDCOLLECTION_H
#define KYDEBUGGERNETFIELDCOLLECTION_H

#include <kyruntime/debugger/debuggernetfield.h>

namespace Kaim
{
	namespace Debugger
	{
		namespace Net
		{
			struct FieldCollection : public Deque<Field>
			{
				// \return total length (header length + sum of field[i].Length).
				Kaim::Debugger::SizeType Length() const;

				/* Push Back value converted as a field in the collection.
				* \return generic error if anything goes wrong
				*/
				template <typename T>
				KyResult operator<<(const T& value)
				{
					KY_FUNCTION("FieldCollection::operator<<(const T& value)");

					KAIM_DEBUGGER_FORWARD_ERROR(PushBack());
					KAIM_DEBUGGER_FORWARD_ERROR(Back() << value);

					return Kaim::Result::Success;
				}

				/* Pop front field from collection and convert it to value.
				* \return generic error if anything goes wrong
				*/
				template <typename T>
				KyResult operator>>(T& value)
				{
					KY_FUNCTION("operator>>(T& value)");

					if (IsEmpty())
						return -1;

					KAIM_DEBUGGER_FORWARD_ERROR(Front() >> value);

					PopFront();

					return Kaim::Result::Success;
				}
			};

			/* Pop field collection from front of buffer.
			* \return generic error if anything goes wrong
			*/
			KyResult operator>>(Kaim::Debugger::Net::NetBuffer& buffer, Kaim::Debugger::Net::FieldCollection& value);

			/* Push Back field collection in buffer.
			* \return generic error if anything goes wrong
			*/
			KyResult operator<<(Kaim::Debugger::Net::NetBuffer& buffer, const Kaim::Debugger::Net::FieldCollection& value);

			/* Push Back field collection in buffer.
			* \return generic error if anything goes wrong
			*/
			KyResult operator<<(Kaim::Debugger::Net::NetArray& buffer, const Kaim::Debugger::Net::FieldCollection& value);
		}
	}
}
#endif // KYDEBUGGERNETFIELDCOLLECTION_H
