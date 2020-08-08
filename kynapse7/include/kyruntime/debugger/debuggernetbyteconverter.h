/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: ? - secondary contact: NOBODY
#ifndef KYDEBUGGERNETBYTECONVERTER_H
#define KYDEBUGGERNETBYTECONVERTER_H

#include <kyruntime/debugger/debuggertypes.h>
#include <kypathdata/blob/endianness.h>
#include <kypathdata/generic/algorithm.h>


namespace Kaim
{
	namespace Debugger
	{
		namespace Net
		{
			namespace details
			{
				template <int size>
				struct endianness;
			}

			namespace endianness
			{
				template<typename T>
				T host_to_network(T value)
				{
					return details::endianness<sizeof(T)>::host_to_network(value);
				}

				template <typename T>
				T network_to_host(T value)
				{
					return details::endianness<sizeof(T)>::network_to_host(value);
				}
			}

			namespace details
			{

				template<int size>
				struct endianness
				{
					template <typename T>
					static T host_to_network(T value)
					{
						return value;
					}

					template <typename T>
					static T network_to_host(T value)
					{
						return value;
					}
				};

	#ifdef KYLITTLEENDIAN

				template<>
				struct endianness<2>
				{
					template <typename T>
					static T host_to_network(T value)
					{
						T temp = value;
						Endianness::Swap16(&temp);
						return temp;
					}

					template <typename T>
					static T network_to_host(T value)
					{
						return host_to_network(value);
					}
				};

				template<>
				struct endianness<4>
				{
					template <typename T>
					static T host_to_network(T value)
					{
						T temp = value;
						Endianness::Swap32(&temp);
						return temp;
					}

					template <typename T>
					static T network_to_host(T value)
					{
						return host_to_network(value);
					}
				};

		#ifdef KYINT64_DEFINED

				void Swap64(KyUInt64* mem);

				template<>
				struct endianness<8>
				{
					template <typename T>
					static T host_to_network(T value)
					{	
						T _value = value;
						Swap64((KyUInt64*)(void*)&_value);
						return _value;
					}

					template <typename T>
					static T network_to_host(T value)
					{
						return host_to_network(value);
					}
				};
		#endif // KYINT64_DEFINED

	#endif // KYLITTLEENDIAN

			}

			template <typename T>
			union net_byte_converter
			{
			protected:
				T m_value;

			public:
				ByteType bytes[sizeof(T)];

			public:
				net_byte_converter()
				{
				}

				net_byte_converter(const  T& value)
				{
					Value(value);
				}

				net_byte_converter(ByteType* buffer)
				{
					Kaim::Copy(buffer, buffer + sizeof(T), Begin());
				}

				ByteType* Begin()
				{
					return bytes;
				}

				ByteType* End()
				{
					return bytes + sizeof(T);
				}

				void Value(const T& _value)
				{
					m_value = endianness::host_to_network<T>(_value);
				}

				T Value() const
				{
					return endianness::network_to_host<T>(m_value);
				}
			};
		}
	}
}

#endif // KYDEBUGGERNETBYTECONVERTER_H
