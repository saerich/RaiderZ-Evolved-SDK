//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
/**********************************************************************
	FILE: SimpleFaceChannel.h

	DESCRIPTION: Header for a templated implementation of IFaceDataChannel
					- class declaration
					- class implementation (because of templating)

	AUTHOR: ktong - created 01.30.2006
***************************************************************************/

#ifndef _SIMPLEFACECHANNEL_H_
#define _SIMPLEFACECHANNEL_H_

#include "SimpleFaceData.h"
#include "IDataChannel.h"
#include "ifacedatamgr.h"

//! \brief SimpleFaceChannel<int>* - Named named channel class.
/*! Implements an IFaceDataChannel interface for controlling a named channel.
The extension to IFaceDataChannel provides functions to access data storage.
*/
//! \see SimpleFaceChannel
template <class T>
class SimpleFaceChannel : public IFaceDataChannel  
{
	enum {
		CHUNKID_SUB_CHANNEL_NAME,
		CHUNKID_SUB_CHANNEL_ID,
		CHUNKID_SUB_CHANNEL_DATA,
	};

	protected:
		//! \brief The channel Storage Container
		Tab<T> mData;

		//! \brief The channel Name
		TSTR mChannelName;

		//! \brief The channel ID
		Class_ID mChannelID;

		//! \brief The channel's data type
		int mChannelType;

		//! \brief Default value for all new faces in the data channel.
		T mDefaultValue;
	public:
		SimpleFaceChannel( int type , ULONG size, T defaultValue); 
		~SimpleFaceChannel( ); 
		
		//@{
		//! \brief From IFaceDataChannel
		//! \see IFaceDataChannel
		BOOL FacesCreated( ULONG at, ULONG num ) ;
		BOOL FacesClonedAndAppended( BitArray& set ) ;
		BOOL FacesDeleted( BitArray& set ) ;
		BOOL FacesDeleted( ULONG from, ULONG num ) ;
		void AllFacesDeleted() ;
		BOOL FaceCopied( ULONG from, ULONG to ) ;
		BOOL FaceInterpolated( ULONG numSrc, ULONG* srcFaces, float* coeff, ULONG targetFace ); 

		IFaceDataChannel* CreateChannel( ) ;
		IFaceDataChannel* CloneChannel( ) ;
		BOOL AppendChannel( const IFaceDataChannel* fromChan ) ;
		//@}

		//@{
		//! \brief From IDataChannel	
		Class_ID DataChannelID() const { return mChannelID; }; 
		ULONG Count() const;
		void DeleteThis();
		//@}

		// File IO
		IOResult	Save(ISave* isave);
		IOResult	Load(ILoad* iload);

		// Data access
		//! \brief GetChannelName - Get the channel name.
		TCHAR* GetChannelName();

		//! \brief SetChannelName - Set the channel name
		void SetChannelName(const TCHAR* pName);

		//! \brief GetChannelID - Get the channel ID.
		Class_ID GetChannelID() const;

		//! \brief SetChannelID - Set the channel ID.
		void SetChannelID(const Class_ID &classID);

		//! \brief GetType - Get the data type of the channel.
		int GetChannelType() const;

		//! \brief GetValue - Direct read access to mData.
		BOOL	GetValue( ULONG at, T &val) const;

		//! \brief SetValue - Direct write access to mData.
		BOOL	SetValue( ULONG at, const T &val);

		//! \brief GetValues - get the array of values for this channel.
		BOOL	GetValues(Tab<T> &val) const;

		//! \brief SetValues - set the channel array data with an array of values. Must be the same size.
		BOOL	SetValues(const Tab<T> &val);
};

template <class T>
//-----------------------------------------------------------------------------
SimpleFaceChannel<T>::SimpleFaceChannel(int type, ULONG size, T defaultValue)
//-----------------------------------------------------------------------------
{
	mChannelName = GetString(IDS_SIMPLEFACEDATAATTRIB_NAME);
	mChannelType = type;
	mDefaultValue = defaultValue;
	FacesCreated(0, size);
}

template <class T>
//-----------------------------------------------------------------------------
SimpleFaceChannel<T>::~SimpleFaceChannel()
//-----------------------------------------------------------------------------
{
}

template <class T>
//-----------------------------------------------------------------------------
void SimpleFaceChannel<T>::DeleteThis()
//-----------------------------------------------------------------------------
{
	delete this;
}
template <class T>
//-----------------------------------------------------------------------------
IFaceDataChannel* SimpleFaceChannel<T>::CreateChannel( ) {
//-----------------------------------------------------------------------------
	return new SimpleFaceChannel<T>(mChannelType, 0, mDefaultValue);
}
template <class T>
//-----------------------------------------------------------------------------
IFaceDataChannel* SimpleFaceChannel<T>::CloneChannel( ) {
//-----------------------------------------------------------------------------
	IFaceDataChannel* fdc = CreateChannel();
	if ( fdc != NULL ) {
		static_cast<SimpleFaceChannel<T>*>(fdc)->mChannelID = GetChannelID();
		static_cast<SimpleFaceChannel<T>*>(fdc)->mChannelName = GetChannelName();
		fdc->AppendChannel( this );	// adds our data.
	}
	return fdc;
}
template <class T>
//-----------------------------------------------------------------------------
BOOL SimpleFaceChannel<T>::AppendChannel( const IFaceDataChannel* fromChan )
//-----------------------------------------------------------------------------
{
	if ( fromChan == NULL || (fromChan->DataChannelID() != DataChannelID()) ) {
		return FALSE;
	}
	
	const SimpleFaceChannel<T>* fromFDChan = static_cast<const SimpleFaceChannel<T>*>(fromChan);
	int newnum = fromFDChan->Count();

	if (newnum > 0) {
		mData.Append(newnum, fromFDChan->mData.Addr(0));
	}
	
	return TRUE;
}

template <class T>
//-----------------------------------------------------------------------------
BOOL SimpleFaceChannel<T>::FacesCreated( ULONG at, ULONG num )
//-----------------------------------------------------------------------------
{
	if (num < 1) {
		return FALSE;
	}
	if (at > (ULONG)Count()) {
		at = Count();
	}
	
	// Rather than calling insert num times,
	// simply set the new count and manually copy the data.

	// Set desired count:
	int oldCount = Count();
	int end = at+num;
	mData.SetCount (oldCount + num);

	// Move existing data if needed to allow the insert to happen
	for (int i = oldCount - 1; i >= at && i >= 0; --i)
		mData[i + num] = mData[i];

	// initialize new data:
	for(int i=at; i<end; i++) {
		SetValue(i, mDefaultValue);
	}

	return TRUE;
}
template <class T>
//-----------------------------------------------------------------------------
BOOL SimpleFaceChannel<T>::FacesClonedAndAppended( BitArray& set )
//-----------------------------------------------------------------------------
{
	int count = static_cast<int>(Count());
	if ( set.GetSize() !=  count) {
		set.SetSize( count, TRUE );
	}
	
	if ( set.NumberSet() == 0 ) {
		return TRUE;	// nothing to do.
	}

	ULONG n = set.GetSize();
	for ( ULONG i = 0; i < n; i++ ) {
		if (set[i])	{
			ULONG appended = Count();
			FacesCreated(appended, 1);
			FaceCopied(i, appended);
		}
	}

	return TRUE;
}
template <class T>
//-----------------------------------------------------------------------------
BOOL SimpleFaceChannel<T>::FacesDeleted( BitArray& set )
//-----------------------------------------------------------------------------
{
	if ( set.NumberSet() == 0 ) {
		return TRUE;	// nothing to free
	}

	ULONG n = set.GetSize();
	
	if (n > Count()) {
		n = Count();
	}

	// delete faces that are flagged in the bitarray
	for ( int i = n-1; i >= 0; i-- ) {
		if (set[i]) {
			mData.Delete(i, 1);
		}
	}

	return TRUE;
}
template <class T>
//-----------------------------------------------------------------------------
BOOL SimpleFaceChannel<T>::FacesDeleted( ULONG from, ULONG num )
//-----------------------------------------------------------------------------
{
	DbgAssert( from >= 0 );
	DbgAssert( ((long)(from + num) - 1) < (long)Count() );
	if ( num > 0 ) {
		mData.Delete(from, num);
	}
	return TRUE;
}
template <class T>
//-----------------------------------------------------------------------------
void SimpleFaceChannel<T>::AllFacesDeleted()
//-----------------------------------------------------------------------------
{
	FacesDeleted(0, Count());
}
template <class T>
//-----------------------------------------------------------------------------
BOOL SimpleFaceChannel<T>::FaceCopied( ULONG from, ULONG to )
//-----------------------------------------------------------------------------
{
	mData[to] = mData[from];
	return TRUE;
}
//-----------------------------------------------------------------------------
BOOL SimpleFaceChannel<int>::FaceInterpolated( ULONG numSrc, ULONG* srcFaces, float* coeff, ULONG targetFace )
// Have to handle <int> seperately, because it needs an explicit float cast to maintain precision.
// Other types are already float-based, so this is not required for them.
// This specialization encompasses TYPE_INT, TYPE_INDEX and TYPE_BOOL channels.
//-----------------------------------------------------------------------------
{
	if ( numSrc < 1 || srcFaces == NULL || numSrc == Count()  || coeff == NULL ) {
		return FALSE;
	}

	// The new face is the weighted sum of the data in specified faces with specified coefficients.
	float result = 0;
	for ( ULONG i = 0; i < numSrc; i++ ) {
		result += (float)(mData[srcFaces[i]]) * coeff[i];
	}
	mData[targetFace] = (int)(result + 0.5f); // round the final value to the nearest integer.

	return TRUE;
}
template <class T>
//-----------------------------------------------------------------------------
BOOL SimpleFaceChannel<T>::FaceInterpolated( ULONG numSrc, ULONG* srcFaces,float* coeff, ULONG targetFace )
//-----------------------------------------------------------------------------
{
	if ( numSrc < 1 || srcFaces == NULL || numSrc == Count()  || coeff == NULL ) {
		return FALSE;
	}

	// The new face is the weighted sum of the data in specified faces with specified coefficients.
	// Since we don't know the data type of T to start at the equivalent of 0,
	// start with the first face and increment the result beyond the first coefficient.
	T result = mData[srcFaces[0]] * coeff[0];
	for ( ULONG i = 1; i < numSrc; i++ ) {
		result += mData[srcFaces[i]]*coeff[i];
	}
	mData[targetFace] = result;

	return TRUE;
}
template <class T>
//-----------------------------------------------------------------------------
ULONG SimpleFaceChannel<T>::Count() const
//-----------------------------------------------------------------------------
{
	return mData.Count();
}
template <class T>
//-----------------------------------------------------------------------------
IOResult	SimpleFaceChannel<T>::Save(ISave* isave)
//-----------------------------------------------------------------------------
{
	IOResult res = IO_OK;
	ULONG nb;
	ULONG num = Count();

	isave->BeginChunk(CHUNKID_SUB_CHANNEL_NAME); // start name
	isave->WriteWString(GetChannelName()); // write name
	isave->EndChunk(); // end name
	
	isave->BeginChunk(CHUNKID_SUB_CHANNEL_ID); // start id
	ULONG bufA = mChannelID.PartA();
	ULONG bufB = mChannelID.PartB();
	isave->Write(&bufA, sizeof(bufA), &nb);
	isave->Write(&bufB, sizeof(bufB), &nb);
	isave->EndChunk(); // end id

	isave->BeginChunk(CHUNKID_SUB_CHANNEL_DATA); // start channel data
	isave->Write(&num, sizeof(num), &nb); // number of channel data
	if (num > 0) {
		isave->Write(mData.Addr(0), sizeof(T) * num, &nb); // the actual channel data
	}
	isave->EndChunk(); // end data

	return res;
}
template <class T>
//-----------------------------------------------------------------------------
IOResult	SimpleFaceChannel<T>::Load(ILoad* iload)
//-----------------------------------------------------------------------------
{
	IOResult res;
	IOResult error = IO_OK;
	ULONG nb;

	while (IO_OK==(res=iload->OpenChunk())) 
	{ 
		switch(iload->CurChunkID()) 
		{
		case CHUNKID_SUB_CHANNEL_NAME:
			{
				// get channel name
				TCHAR* pName;
				res = iload->ReadWStringChunk(&pName);
				if (res == IO_OK) {
					mChannelName = pName;
				}
			}
		break;
		case CHUNKID_SUB_CHANNEL_ID:
			{
				// get channel ID
				ULONG bufA = 0;
				ULONG bufB = 0;
				res = iload->Read(&bufA, sizeof(bufA), &nb);
				res = iload->Read(&bufB, sizeof(bufB), &nb);
				mChannelID = Class_ID(bufA, bufB);
			}
		case CHUNKID_SUB_CHANNEL_DATA:
			{
				// get channel data
				ULONG num = 0;
				res = iload->Read(&num, sizeof(num), &nb);
				if (num > 0) {
					mData.SetCount(num);
					res = iload->Read(mData.Addr(0), num*sizeof(T), &nb);
				}
			}
		break;
		}
		iload->CloseChunk(); 
		if (res!=IO_OK) {
			error = res;
			break;
		}
	}
	if (error != IO_OK) {

	}
	return error;
}

// ****************************************************************************
// External interface
// ****************************************************************************
template <class T>
//-----------------------------------------------------------------------------
TCHAR* SimpleFaceChannel<T>::GetChannelName()
//-----------------------------------------------------------------------------
{
	return mChannelName;
}
template <class T>
//-----------------------------------------------------------------------------
void SimpleFaceChannel<T>::SetChannelName(const TCHAR* pName)
//-----------------------------------------------------------------------------
{
	mChannelName = TSTR(pName);
}
template <class T>
//-----------------------------------------------------------------------------
Class_ID SimpleFaceChannel<T>::GetChannelID() const
//-----------------------------------------------------------------------------
{
	return mChannelID;
}
template <class T>
//-----------------------------------------------------------------------------
void SimpleFaceChannel<T>::SetChannelID(const Class_ID &classID)
//-----------------------------------------------------------------------------
{
	mChannelID = classID;
}
template <class T>
//-----------------------------------------------------------------------------
int SimpleFaceChannel<T>::GetChannelType() const
//-----------------------------------------------------------------------------
{
	return mChannelType;
}
template <class T>
//-----------------------------------------------------------------------------
BOOL SimpleFaceChannel<T>::GetValue( ULONG at, T& val ) const
//-----------------------------------------------------------------------------
{
	val = mData[at];
	return TRUE;
}
template <class T>
//-----------------------------------------------------------------------------
BOOL SimpleFaceChannel<T>::SetValue( ULONG at, const T& val)
//-----------------------------------------------------------------------------
{
	mData[at] = val;
	return TRUE;
}

template <class T>
//-----------------------------------------------------------------------------
BOOL SimpleFaceChannel<T>::GetValues( Tab<T>& val ) const 
//-----------------------------------------------------------------------------
{
	// clear out in incoming tab to replace its contents with the channel data.
	val.Delete(0, val.Count());
	val.Append(mData.Count(), mData.Addr(0));
	return TRUE;
}

template <class T>
//-----------------------------------------------------------------------------
BOOL SimpleFaceChannel<T>::SetValues( const Tab<T>& val)
//-----------------------------------------------------------------------------
{
	// If the tabs are the same size, clear out the channel data to replace
	// the channel with the incoming data.
	if (val.Count() == mData.Count()) {
		mData.Delete(0, mData.Count());
		mData.Append(val.Count(), val.Addr(0));
		return TRUE;
	}
	return FALSE;
}

#endif
