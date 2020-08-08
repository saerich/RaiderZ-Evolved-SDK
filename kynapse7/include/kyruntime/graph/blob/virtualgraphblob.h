///*
//* Copyright 2010 Autodesk, Inc.  All rights reserved.
//* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
//* or which otherwise accompanies this software in either electronic or hard copy form.
//*/
//
//	
//
//
//	\file
//	\ingroup runtime_graph_blob */
//
//// primary contact: MAMU - secondary contact: NOBODY
//#ifndef KyRuntime_VirtualGraphBlob_H
//#define KyRuntime_VirtualGraphBlob_H
//
//
//#include "kypathdata/blob/blobarray.h"
//#include "kypathdata/graph/graphtypes.h"
//#include "kyruntime/graph/blob/virtualgraphedgeblob.h"
//#include "kyruntime/graph/blob/virtualgraphvertexblob.h"
//#include "kyruntime/graph/blob/virtualgraphlinkblob.h"
//
//namespace Kaim
//{
//
///*!
//Serializable structure (Blob) containing all VirtualGraph data, namely:
//-Vertices and edges of the virtual graph.
//-Links connecting the VirtualGraph to the static graph.
//*/
//class VirtualGraphBlob
//{
//public:
//	static const char* GetBlobType() { return "VirtualGraphBlob"; }
//	static KyUInt32 GetBlobVersion() { return 0; }
//
//public:
//	VirtualGraphBlob() {}
//	
//public:
//	//GraphGUID m_uid;                                             //!< Unique ID of the VirtualGraph in a database.
//	BlobArray<VirtualGraphVertexBlob> m_vertices;                   //!< VirtualGraph Vertices.
//	BlobArray<VirtualGraphEdgeBlob> m_edges;                        //!< VirtualGraph Edges.
//	BlobArray<VirtualGraphLinkBlob> m_links;                       //!< VirtualGraph Links to the static graph.
//};
//
///*!
//Endianness swap
//*/
//inline void SwapEndianness(Endianness::Target e, VirtualGraphBlob& self)
//{
//	//SwapEndianness(e, self.m_uid);
//	SwapEndianness(e, self.m_vertices);
//	SwapEndianness(e, self.m_edges);
//	SwapEndianness(e, self.m_links);
//}
//
//}
//
//#endif // #ifndef KyRuntime_VirtualGraphBlob_H
