
/*****************************************************************************
 * FLT: An OpenFlight file loader
 *
 * Copyright (C) 2007  Michael M. Morrison   All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *****************************************************************************/

/*
 * OpenFlight Parser
 * 
 * Author: Mike Morrison
 *         morrison@users.sourceforge.net
 */

#include "flt.h"

FltRecordEntryName FltNodeEntryName[] = {
	{	"uint32", "type", offsetof( FltNode *, type ) },
	{	"uint32", "length", offsetof( FltNode *, length ) },
	{	"uint32", "treeDepth", offsetof( FltNode *, treeDepth ) },
	{	"FltNode *", "parent", offsetof( FltNode *, parent  ) },
	{	"FltNode *", "prev", offsetof( FltNode *, prev  ) },
	{	"FltNode *", "next", offsetof( FltNode *, next  ) },
	{	"FltNode **", "child", offsetof( FltNode *, child  ) },
	{	"uint32", "numChildren", offsetof( FltNode *, numChildren ) },
	//{	"uint8 [0]", "data", offsetof( FltNode *, data ) },
	{ 0, 0, 0 }
};

FltRecordEntryName FltLightSourceEntryName[] = {
	{	"FltNode", "node", offsetof( FltLightSource *, node ) },
	{	"uint32", "reserved0", offsetof( FltLightSource *, reserved0 ) },
	{	"uint32", "paletteIndex", offsetof( FltLightSource *, paletteIndex ) },
	{	"uint32", "reserved1", offsetof( FltLightSource *, reserved1 ) },
	{	"uint32", "flags", offsetof( FltLightSource *, flags ) },
	{	"uint32", "reserved2", offsetof( FltLightSource *, reserved2 ) },
	{	"real64", "position[0]", offsetof( FltLightSource *, position[0] ) },
	{	"real64", "position[1]", offsetof( FltLightSource *, position[1] ) },
	{	"real64", "position[2]", offsetof( FltLightSource *, position[2] ) },
	{	"real32", "yaw", offsetof( FltLightSource *, yaw ) },
	{	"real32", "pitch", offsetof( FltLightSource *, pitch ) },
 { 0, 0, 0 }
};

FltRecordEntryName FltLightSourcePaletteEntryName[] = {
	{	"FltNode", "node", offsetof( FltLightSourcePaletteEntry *, node ) },
	{	"uint32", "index", offsetof( FltLightSourcePaletteEntry *, index ) },
	{	"char [20]", "ID", offsetof( FltLightSourcePaletteEntry *, ID ) },
	{	"real32", "ambient[0]", offsetof( FltLightSourcePaletteEntry *, ambient[0] ) },
	{	"real32", "ambient[1]", offsetof( FltLightSourcePaletteEntry *, ambient[1] ) },
	{	"real32", "ambient[2]", offsetof( FltLightSourcePaletteEntry *, ambient[2] ) },
	{	"real32", "ambient[3]", offsetof( FltLightSourcePaletteEntry *, ambient[3] ) },
	{	"real32", "diffuse[0]", offsetof( FltLightSourcePaletteEntry *, diffuse[0] ) },
	{	"real32", "diffuse[1]", offsetof( FltLightSourcePaletteEntry *, diffuse[1] ) },
	{	"real32", "diffuse[2]", offsetof( FltLightSourcePaletteEntry *, diffuse[2] ) },
	{	"real32", "diffuse[3]", offsetof( FltLightSourcePaletteEntry *, diffuse[3] ) },
	{	"real32", "specular[0]", offsetof( FltLightSourcePaletteEntry *, specular[0] ) },
	{	"real32", "specular[1]", offsetof( FltLightSourcePaletteEntry *, specular[1] ) },
	{	"real32", "specular[2]", offsetof( FltLightSourcePaletteEntry *, specular[2] ) },
	{	"real32", "specular[3]", offsetof( FltLightSourcePaletteEntry *, specular[3] ) },
	{	"uint32", "type", offsetof( FltLightSourcePaletteEntry *, type ) },
	{	"real32", "spotExponent", offsetof( FltLightSourcePaletteEntry *, 
																															spotExponent ) },
	{	"real32", "spotCutoff", offsetof( FltLightSourcePaletteEntry *, 
																															spotCutoff ) },
	{	"real32", "yaw", offsetof( FltLightSourcePaletteEntry *, yaw ) },
	{	"real32", "pitch", offsetof( FltLightSourcePaletteEntry *, pitch ) },
	{	"real32", "attenC", offsetof( FltLightSourcePaletteEntry *, attenC ) },
	{	"real32", "attenL", offsetof( FltLightSourcePaletteEntry *, attenL ) },
	{	"real32", "attenQ", offsetof( FltLightSourcePaletteEntry *, attenQ ) },
	{	"uint32", "activeDuringModeling", offsetof( FltLightSourcePaletteEntry *, 
																									activeDuringModeling ) },
 { 0, 0, 0 }
};

FltRecordEntryName FltTextureEntryName[] = {
	{ "FltNode",  "node", offsetof( FltTexture *, node ) },
	{ "char [200]", "ID", offsetof( FltTexture *, ID ) }, 
	{ "uint32", "index", offsetof( FltTexture *, index ) },
	{ "uint32", "xloc", offsetof( FltTexture *, xloc ) },
	{ "uint32", "yloc", offsetof( FltTexture *, yloc ) },
	{ 0, 0, 0 }
};

FltRecordEntryName FltNonuniformScaleEntryName[] = {
	{ "FltNode",  "node", offsetof( FltNonuniformScale *, node ) },
	{ "uint32", "reserved", offsetof( FltNonuniformScale *, reserved ) },
	{ "real64", "centerX", offsetof( FltNonuniformScale *, centerX ) },
	{ "real64", "centerY", offsetof( FltNonuniformScale *, centerY ) },
	{ "real64", "centerZ", offsetof( FltNonuniformScale *, centerZ ) },
	{ "real32", "scaleX", offsetof( FltNonuniformScale *, scaleX ) },
	{ "real32", "scaleY", offsetof( FltNonuniformScale *, scaleY ) },
	{ "real32", "scaleZ", offsetof( FltNonuniformScale *, scaleZ ) },
	{ 0, 0, 0 }
};

FltRecordEntryName FltTranslateEntryName[] = {
	{ "FltNode",  "node", offsetof( FltTranslate *, node ) },
	{ "uint32", "reserved", offsetof( FltTranslate *, reserved ) },
	{ "real64", "fromX", offsetof( FltTranslate *, fromX ) },
	{ "real64", "fromY", offsetof( FltTranslate *, fromY ) },
	{ "real64", "fromZ", offsetof( FltTranslate *, fromZ ) },
	{ "real64", "deltaX", offsetof( FltTranslate *, deltaX ) },
	{ "real64", "deltaY", offsetof( FltTranslate *, deltaY ) },
	{ "real64", "deltaZ", offsetof( FltTranslate *, deltaZ ) },
	{ 0, 0, 0 }
};

FltRecordEntryName FltReplicateEntryName[] = {
	{ "FltNode",  "node", offsetof( FltReplicate *, node ) },
	{ "uint16", "replications", offsetof( FltReplicate *, replications ) },
	{ "uint16", "reserved", offsetof( FltReplicate *, reserved ) },
	{ 0, 0, 0 }
};

FltRecordEntryName FltLineStyleEntryName[] = {
	{ "FltNode",  "node", offsetof( FltLineStyle *, node ) },
	{ "uint16", "index", offsetof( FltLineStyle *, index ) },
	{ "uint16", "patternMask", offsetof( FltLineStyle *, patternMask ) },
	{ "uint32", "lineWidth", offsetof( FltLineStyle *, lineWidth ) },
	{ 0, 0, 0 }
};

FltRecordEntryName FltShaderEntryName[] = {
	{ "FltNode",  "node", offsetof( FltShader *, node ) },
	{ "char [1024]", "ID", offsetof( FltShader *, ID ) },
	{ "uint32", "index", offsetof( FltShader *, index ) },
	{ "uint32", "type", offsetof( FltShader *, type ) },
	{ 0, 0, 0 }
};

FltRecordEntryName FltLightPointEntryName[] = {
	{ "FltNode",  "node", offsetof( FltLightPoint *, node ) },
	{ "char [8]", "ID", offsetof( FltLightPoint *, ID ) },
	{ 0, 0, 0 }
};

FltRecordEntryName FltLightPointSystemEntryName[] = {
	{ "FltNode",  "node", offsetof( FltLightPointSystem *, node ) },
	{ "char [8]", "ID", offsetof( FltLightPointSystem *, ID ) },
	{ "real32", "intensity", offsetof( FltLightPointSystem *, intensity ) },
	{ "uint32", "animationState", offsetof( FltLightPointSystem *, 
                                                          animationState ) },
	{ "uint32", "flags", offsetof( FltLightPointSystem *, flags ) },
	{ 0, 0, 0 }
};

FltRecordEntryName FltTextureMappingPaletteEntryName[] = {
	{ "FltNode",  "node", offsetof( FltTextureMappingPaletteEntry *, node ) },
	{ "uint32", "reserved", offsetof( FltTextureMappingPaletteEntry *,reserved )},
	{ "uint32", "index", offsetof( FltTextureMappingPaletteEntry *, index ) },
	{ "char [20]", "ID", offsetof( FltTextureMappingPaletteEntry *, ID ) },
	{ "uint32", "type", offsetof( FltTextureMappingPaletteEntry *, type ) },
	{ "uint32", "warped", offsetof( FltTextureMappingPaletteEntry *, warped ) },	
{ 0, 0, 0 }
};

FltRecordEntryName FltBSPEntryName[] = {
	{ "FltNode",  "node", offsetof( FltBSP *, node ) },
	{ "char [8]", "ID", offsetof( FltBSP *, ID ) }, 
	{ "uint32", "reserved0", offsetof( FltBSP *, reserved0 ) },
	{ "real64", "coefA", offsetof( FltBSP *, coefA ) },
	{ "real64", "coefB", offsetof( FltBSP *, coefB ) },
	{ "real64", "coefC", offsetof( FltBSP *, coefC ) },
	{ "real64", "coefD", offsetof( FltBSP *, coefD ) },
	{ 0, 0, 0 }
};

FltRecordEntryName FltInstanceDefinitionEntryName[] = {
	{ "FltNode",  "node", offsetof( FltInstanceDefinition *, node ) },
	{ "uint16", "reserved0", offsetof( FltInstanceDefinition *, reserved0 ) }, 
	{ "uint16", "instance", offsetof( FltInstanceDefinition *, instance ) }, 
	{ 0, 0, 0 }
};

FltRecordEntryName FltInstanceReferenceEntryName[] = {
	{ "FltNode",  "node", offsetof( FltInstanceReference *, node ) },
	{ "uint16", "reserved0", offsetof( FltInstanceDefinition *, reserved0 ) }, 
	{ "uint16", "instance", offsetof( FltInstanceDefinition *, instance ) }, 
	{ 0, 0, 0 }
};

FltRecordEntryName FltCommentEntryName[] = {
	{ "FltNode",  "node", offsetof( FltComment *, node ) },
	{ "char [256]", "text", offsetof( FltComment *, text ) }, 
	{ 0, 0, 0 }
};

FltRecordEntryName FltLongIDEntryName[] = {
	{ "FltNode",  "node", offsetof( FltLongID *, node ) },
	{ "char [64]", "text", offsetof( FltLongID *, text ) }, 
	{ 0, 0, 0 }
};

FltRecordEntryName FltMatrixEntryName[] = {
	{ "FltNode",  "node", offsetof( FltMatrix *, node ) },
	{ "real32 [16]", "matrix", offsetof( FltMatrix *, matrix ) }, 
	{ 0, 0, 0 }
};

FltRecordEntryName FltGeneralMatrixEntryName[] = {
	{ "FltNode",  "node", offsetof( FltMatrix *, node ) },
	{ "real32 [16]", "matrix", offsetof( FltMatrix *, matrix ) }, 
	{ 0, 0, 0 }
};

FltRecordEntryName FltExternalReferenceEntryName[] = {
	{ "FltNode",  "node", offsetof( FltExternalReference *, node ) },
	{ "char [200]", "path", offsetof( FltExternalReference *, path ) }, 
	{ "uint8", "reserved0", offsetof( FltExternalReference *, reserved0 ) }, 
	{ "uint8", "reserved1", offsetof( FltExternalReference *, reserved1 ) }, 
	{ "uint16", "reserved2", offsetof( FltExternalReference *, reserved2 ) }, 
	{ "uint32", "flags", offsetof( FltExternalReference *, flags ) }, 
	{ "uint16", "reserved3", offsetof( FltExternalReference *, reserved3 ) }, 
	{ "uint16", "reserved4", offsetof( FltExternalReference *, reserved4 ) }, 
	{ 0, 0, 0 }
};

FltRecordEntryName FltColorPaletteEntryName[] = {
	{ "FltNode", "node", offsetof( FltColorPalette *, node ) },
	{ "uint32 [1024]", "color", offsetof( FltColorPalette *, color ) },
	{ 0, 0, 0 }
};

FltRecordEntryName FltMaterialEntryName[] = {
	{ "FltNode", "node", offsetof( FltMaterial *, node ) },
	{ "char [12]", "ID", offsetof( FltMaterial *, ID ) },
	{ "uint32", "index", offsetof( FltMaterial *, index ) },
	{ "uint32", "flags", offsetof( FltMaterial *, flags ) },
	{ "real32", "ambientRed", offsetof( FltMaterial *, ambientRed ) },
	{ "real32", "ambientGreen", offsetof( FltMaterial *, ambientGreen ) },
	{ "real32", "ambientBlue", offsetof( FltMaterial *, ambientBlue ) },
	{ "real32", "diffuseRed", offsetof( FltMaterial *, diffuseRed ) },
	{ "real32", "diffuseGreen", offsetof( FltMaterial *, diffuseGreen ) },
	{ "real32", "diffuseBlue", offsetof( FltMaterial *, diffuseBlue ) },
	{ "real32", "specularRed", offsetof( FltMaterial *, specularRed ) },
	{ "real32", "specularGreen", offsetof( FltMaterial *, specularGreen ) },
	{ "real32", "specularBlue", offsetof( FltMaterial *, specularBlue ) },
	{ "real32", "emissiveRed", offsetof( FltMaterial *, emissiveRed ) },
	{ "real32", "emissiveGreen", offsetof( FltMaterial *, emissiveGreen ) },
	{ "real32", "emissiveBlue", offsetof( FltMaterial *, emissiveBlue ) },
	{ "real32", "shininess", offsetof( FltMaterial *, shininess ) },
	{ "real32", "alpha", offsetof( FltMaterial *, alpha ) },
	{ "uint32", "spare", offsetof( FltMaterial *, spare ) },
	{ 0, 0, 0 }
};

//
// material tables are converted to materials in a palette
//
FltRecordEntryName FltMaterialTableEntryName[] = {
	{ "FltNode", "node", offsetof( FltMaterial *, node ) },
	{ "char [12]", "ID", offsetof( FltMaterial *, ID ) },
	{ "uint32", "index", offsetof( FltMaterial *, index ) },
	{ "uint32", "flags", offsetof( FltMaterial *, flags ) },
	{ "real32", "ambientRed", offsetof( FltMaterial *, ambientRed ) },
	{ "real32", "ambientGreen", offsetof( FltMaterial *, ambientGreen ) },
	{ "real32", "ambientBlue", offsetof( FltMaterial *, ambientBlue ) },
	{ "real32", "diffuseRed", offsetof( FltMaterial *, diffuseRed ) },
	{ "real32", "diffuseGreen", offsetof( FltMaterial *, diffuseGreen ) },
	{ "real32", "diffuseBlue", offsetof( FltMaterial *, diffuseBlue ) },
	{ "real32", "specularRed", offsetof( FltMaterial *, specularRed ) },
	{ "real32", "specularGreen", offsetof( FltMaterial *, specularGreen ) },
	{ "real32", "specularBlue", offsetof( FltMaterial *, specularBlue ) },
	{ "real32", "emissiveRed", offsetof( FltMaterial *, emissiveRed ) },
	{ "real32", "emissiveGreen", offsetof( FltMaterial *, emissiveGreen ) },
	{ "real32", "emissiveBlue", offsetof( FltMaterial *, emissiveBlue ) },
	{ "real32", "shininess", offsetof( FltMaterial *, shininess ) },
	{ "real32", "alpha", offsetof( FltMaterial *, alpha ) },
	{ "uint32", "spare", offsetof( FltMaterial *, spare ) },
	{ 0, 0, 0 }
};

FltRecordEntryName FltMaterialPaletteEntryName[] = {
	{ "FltNode", "node", offsetof( FltMaterialPalette *, node ) },
	{ "FltMaterial **", "material", offsetof( FltMaterialPalette *, material ) },
	{ "uint32", "numMaterials", offsetof( FltMaterialPalette *, numMaterials ) },
	{ 0, 0, 0 }
};

FltRecordEntryName FltHeaderEntryName[] = {
	{ "FltNode", "node", offsetof( FltHeader *, node) },
	{ "char [8]",	"ID", offsetof( FltHeader *, ID ) },
	{ "uint32",	"formatRevision", offsetof( FltHeader *, formatRevision ) },
	{ "uint32",	"editRevision", offsetof( FltHeader *, editRevision ) },
	{ "char [32]", "dateTime", offsetof( FltHeader *, dateTime ) },
	{	"uint16",	"nextGroupNodeID", offsetof( FltHeader *, nextGroupNodeID ) },
	{	"uint16",	"nextLODNodeID", offsetof( FltHeader *, nextLODNodeID ) },
 { "uint16",	"nextObjectNodeID", offsetof( FltHeader *, nextObjectNodeID ) },
 { "uint16", "nextFaceNodeID", offsetof( FltHeader *, nextFaceNodeID ) },
 { "uint16",	"unitMultiplier", offsetof( FltHeader *, unitMultiplier ) },
 { "uint8",		"coordUnits", offsetof( FltHeader *, coordUnits ) },
 { "uint8",		"setTexWhiteOnNewFaces", offsetof( FltHeader *, setTexWhiteOnNewFaces ) },
 { "uint32",	"flags", offsetof( FltHeader *, flags ) },
 { "uint32 [6]",	"reserved0", offsetof( FltHeader *, reserved0 ) },
 { "uint32",	"projectionType", offsetof( FltHeader *, projectionType ) },
 { "uint32 [7]",	"reserved1", offsetof( FltHeader *, reserved1 ) },
 { "uint16",	"nextDOFNodeID", offsetof( FltHeader *, nextDOFNodeID ) },
 { "uint16",	"vertexStorageLength", offsetof( FltHeader *, vertexStorageLength ) },
 { "uint32",	"databaseOrigin", offsetof( FltHeader *, databaseOrigin ) },
 { "real64",	"swDBCoordX", offsetof( FltHeader *, swDBCoordX ) },
 { "real64",	"swDBCoordY", offsetof( FltHeader *, swDBCoordY ) },
 { "real64",	"deltaXPlace", offsetof( FltHeader *, deltaXPlace ) },
 { "real64",	"deltaYPlace", offsetof( FltHeader *, deltaYPlace ) },
 { "uint16",	"nextSoundNodeID", offsetof( FltHeader *, nextSoundNodeID ) },
 { "uint16",	"nextPathNodeID", offsetof( FltHeader *, nextPathNodeID ) },
 { "uint32 [2]",	"reserved2", offsetof( FltHeader *, reserved2 ) },
 { "uint16",	"nextClipNodeID", offsetof( FltHeader *, nextClipNodeID ) },
 { "uint16",	"nextTextNodeID", offsetof( FltHeader *, nextTextNodeID ) },
 { "uint16",	"nextBSPNodeID", offsetof( FltHeader *, nextBSPNodeID ) },
 { "uint16",	"nextSwitchNodeID", offsetof( FltHeader *, nextSwitchNodeID ) },
 { "uint32",	"reserved3", offsetof( FltHeader *, reserved3 ) },
 { "real64",	"swDBLatitude", offsetof( FltHeader *, swDBLatitude ) },
 { "real64",	"swDBLongitude", offsetof( FltHeader *, swDBLongitude ) },
 { "real64",	"neDBLatitude", offsetof( FltHeader *, neDBLatitude ) },
 { "real64",	"neDBLongitude", offsetof( FltHeader *, neDBLongitude ) },
 { "real64",	"originDBLatitude", offsetof( FltHeader *, originDBLatitude ) },
 { "real64",	"originDBLongitude", offsetof( FltHeader *, originDBLongitude ) },
 { "real64",	"lambertUpperLatitude", offsetof( FltHeader *, lambertUpperLatitude ) },
 { "real64",	"lambertLowerLatitude", offsetof( FltHeader *, lambertLowerLatitude ) },
 { "uint16",	"nextLightSourceNodeID", offsetof( FltHeader *, nextLightSourceNodeID ) },
 { "uint16",	"nextLightPointNodeID", offsetof( FltHeader *, nextLightPointNodeID ) },
 { "uint16",	"nextRoadNodeID", offsetof( FltHeader *, nextRoadNodeID ) },
 { "uint16",	"nextCATNodeID", offsetof( FltHeader *, nextCATNodeID ) },
 { "uint16",	"reserved4", offsetof( FltHeader *, reserved4 ) },
 { "uint16",	"reserved5", offsetof( FltHeader *, reserved5 ) },
 { "uint16",	"reserved6", offsetof( FltHeader *, reserved6 ) },
 { "uint16",	"reserved7", offsetof( FltHeader *, reserved7 ) },
 { "uint32",	"earthEllipsoidModel", offsetof( FltHeader *, earthEllipsoidModel ) },
 { "uint16",	"nextAdaptiveNodeID", offsetof( FltHeader *, nextAdaptiveNodeID ) },
 { "uint16",	"nextCurveNodeID", offsetof( FltHeader *, nextCurveNodeID ) },
 { "uint16",	"utmZone", offsetof( FltHeader *, utmZone ) },
	/* skip the rest since we dont read them anyway */
 { 0, 0, 0 }
};

FltRecordEntryName FltVertexEntryName[] = {
	{	"FltNode", "node", offsetof( FltVertex *, node ) },
	{	"real64", "x", offsetof( FltVertex *, x ) },
	{	"real64", "y", offsetof( FltVertex *, y ) },
	{	"real64", "z", offsetof( FltVertex *, z ) },
	{	"real32", "i", offsetof( FltVertex *, i ) },
	{	"real32", "j", offsetof( FltVertex *, j ) },
	{	"real32", "k", offsetof( FltVertex *, k ) },
	{	"real32", "u", offsetof( FltVertex *, u ) },
	{	"real32", "v", offsetof( FltVertex *, v ) },
	{	"FltVertexFlags", "localFlags", offsetof( FltVertex *, localFlags ) },
	{	"uint16", "colorNameIndex", offsetof( FltVertex *, colorNameIndex ) },
	{	"uint16", "flags", offsetof( FltVertex *, flags ) },
	{	"uint32", "packedColor", offsetof( FltVertex *, packedColor ) },
	{	"uint32", "colorIndex", offsetof( FltVertex *, colorIndex ) },
	{	"uint32", "indx", offsetof( FltVertex *, indx ) },
	{	"uint32", "mtMask", offsetof( FltVertex *, mtMask ) },
	{	"real32", "mtU[0]", offsetof( FltVertex *, mtU[0] ) },
	{	"real32", "mtV[0]", offsetof( FltVertex *, mtV[0] ) },
	{	"real32", "mtU[1]", offsetof( FltVertex *, mtU[1] ) },
	{	"real32", "mtV[1]", offsetof( FltVertex *, mtV[1] ) },
	{	"real32", "mtU[2]", offsetof( FltVertex *, mtU[2] ) },
	{	"real32", "mtV[2]", offsetof( FltVertex *, mtV[2] ) },
	{	"real32", "mtU[3]", offsetof( FltVertex *, mtU[3] ) },
	{	"real32", "mtV[3]", offsetof( FltVertex *, mtV[3] ) },
	{	"real32", "mtU[4]", offsetof( FltVertex *, mtU[4] ) },
	{	"real32", "mtV[4]", offsetof( FltVertex *, mtV[4] ) },
	{	"real32", "mtU[5]", offsetof( FltVertex *, mtU[5] ) },
	{	"real32", "mtV[5]", offsetof( FltVertex *, mtV[5] ) },
	{	"real32", "mtU[6]", offsetof( FltVertex *, mtU[6] ) },
	{	"real32", "mtV[6]", offsetof( FltVertex *, mtV[6] ) },
 { 0, 0, 0 }
};

FltRecordEntryName FltSwitchEntryName[] = {
	{	"FltNode", "node", offsetof( FltSwitch *, node ) },
	{	"char [8]", "ID", offsetof( FltSwitch *, ID ) },
	{	"uint32", "reserved0", offsetof( FltSwitch *, reserved0 ) },
	{	"uint32", "currentMask", offsetof( FltSwitch *, currentMask ) },
	{	"uint32", "numUInt32sPerMask", offsetof( FltSwitch *, numUInt32sPerMask ) },
	{	"uint32", "numMasks", offsetof( FltSwitch *, numMasks ) },
	{	"uint32 *", "masks", offsetof( FltSwitch *, masks  ) },
 { 0, 0, 0 }
};

FltRecordEntryName FltDOFEntryName[] = {
	{	"FltNode", "node", offsetof( FltDOF *, node ) },
	{	"char [8]", "ID", offsetof( FltDOF *, ID ) },
	{	"uint32", "reserved0", offsetof( FltDOF *, reserved0 ) },

	{	"real64", "localOriginX", offsetof( FltDOF *, localOriginX ) },
	{	"real64", "localOriginY", offsetof( FltDOF *, localOriginY ) },
	{	"real64", "localOriginZ", offsetof( FltDOF *, localOriginZ ) },

	{	"real64", "localPointX", offsetof( FltDOF *, localPointX ) },
	{	"real64", "localPointY", offsetof( FltDOF *, localPointY ) },
	{	"real64", "localPointZ", offsetof( FltDOF *, localPointZ ) },

	{	"real64", "localPlanePointX", offsetof( FltDOF *, localPlanePointX ) },
	{	"real64", "localPlanePointY", offsetof( FltDOF *, localPlanePointY ) },
	{	"real64", "localPlanePointZ", offsetof( FltDOF *, localPlanePointZ ) },

	{	"real64", "localMinZ", offsetof( FltDOF *, localMinZ ) },
	{	"real64", "localMaxZ", offsetof( FltDOF *, localMaxZ ) },
	{	"real64", "localCurZ", offsetof( FltDOF *, localCurZ ) },
	{	"real64", "localIncZ", offsetof( FltDOF *, localIncZ ) },

	{	"real64", "localMinY", offsetof( FltDOF *, localMinY ) },
	{	"real64", "localMaxY", offsetof( FltDOF *, localMaxY ) },
	{	"real64", "localCurY", offsetof( FltDOF *, localCurY ) },
	{	"real64", "localIncY", offsetof( FltDOF *, localIncY ) },

	{	"real64", "localMinX", offsetof( FltDOF *, localMinX ) },
	{	"real64", "localMaxX", offsetof( FltDOF *, localMaxX ) },
	{	"real64", "localCurX", offsetof( FltDOF *, localCurX ) },
	{	"real64", "localIncX", offsetof( FltDOF *, localIncX ) },

	{	"real64", "localMinPitch;		// about ", offsetof( FltDOF *, localMinPitch ) },
	{	"real64", "localMaxPitch", offsetof( FltDOF *, localMaxPitch ) },
	{	"real64", "localCurPitch", offsetof( FltDOF *, localCurPitch ) },
	{	"real64", "localIncPitch", offsetof( FltDOF *, localIncPitch ) },

	{	"real64", "localMinRoll;		// about ", offsetof( FltDOF *, localMinRoll ) },
	{	"real64", "localMaxRoll", offsetof( FltDOF *, localMaxRoll ) },
	{	"real64", "localCurRoll", offsetof( FltDOF *, localCurRoll ) },
	{	"real64", "localIncRoll", offsetof( FltDOF *, localIncRoll ) },

	{	"real64", "localMinYaw;		// about ", offsetof( FltDOF *, localMinYaw ) },
	{	"real64", "localMaxYaw", offsetof( FltDOF *, localMaxYaw ) },
	{	"real64", "localCurYaw", offsetof( FltDOF *, localCurYaw ) },
	{	"real64", "localIncYaw", offsetof( FltDOF *, localIncYaw ) },

	{	"real64", "localMinScaleZ", offsetof( FltDOF *, localMinScaleZ ) },
	{	"real64", "localMaxScaleZ", offsetof( FltDOF *, localMaxScaleZ ) },
	{	"real64", "localCurScaleZ", offsetof( FltDOF *, localCurScaleZ ) },
	{	"real64", "localIncScaleZ", offsetof( FltDOF *, localIncScaleZ ) },

	{	"real64", "localMinScaleY", offsetof( FltDOF *, localMinScaleY ) },
	{	"real64", "localMaxScaleY", offsetof( FltDOF *, localMaxScaleY ) },
	{	"real64", "localCurScaleY", offsetof( FltDOF *, localCurScaleY ) },
	{	"real64", "localIncScaleY", offsetof( FltDOF *, localIncScaleY ) },

	{	"real64", "localMinScaleX", offsetof( FltDOF *, localMinScaleX ) },
	{	"real64", "localMaxScaleX", offsetof( FltDOF *, localMaxScaleX ) },
	{	"real64", "localCurScaleX", offsetof( FltDOF *, localCurScaleX ) },
	{	"real64", "localIncScaleX", offsetof( FltDOF *, localIncScaleX ) },

	{	"uint32", "flags", offsetof( FltDOF *, flags ) },
 { 0, 0, 0 }
};

FltRecordEntryName FltLODEntryName[] = {
	{	"FltNode", "node", offsetof( FltLOD *, node ) },
	{	"char [8]", "ID", offsetof( FltLOD *, ID ) },
	{	"uint32", "reserved0", offsetof( FltLOD *, reserved0 ) },
	{	"real64", "switchInDistance", offsetof( FltLOD *, switchInDistance ) },
	{	"real64", "switchOutDistance", offsetof( FltLOD *, switchOutDistance ) },
	{	"uint16", "specialEffectID1", offsetof( FltLOD *, specialEffectID1 ) },
	{	"uint16", "specialEffectID2", offsetof( FltLOD *, specialEffectID2 ) },
	{	"uint32", "flags", offsetof( FltLOD *, flags ) },
	{	"real64", "centerX", offsetof( FltLOD *, centerX ) },
	{	"real64", "centerY", offsetof( FltLOD *, centerY ) },
	{	"real64", "centerZ", offsetof( FltLOD *, centerZ ) },
	{	"real64", "transitionRange", offsetof( FltLOD *, transitionRange ) },
 { 0, 0, 0 }
};

FltRecordEntryName FltVertexPaletteEntryName[] = {
	{	"FltNode", "node", offsetof( FltVertexPalette *, node ) },
	{	"uint32", "numVerts", offsetof( FltVertexPalette *, numVerts ) },
	{	"FltVertex **", "verts", offsetof( FltVertexPalette *, verts  ) },
 { 0, 0, 0 }
};

FltRecordEntryName FltVertexListEntryName[] = {
	{	"FltNode", "node", offsetof( FltVertexList *, node ) },
	{	"uint32", "numVerts", offsetof( FltVertexList *, numVerts ) },
	{	"FltVertex **", "list", offsetof( FltVertexList *, list  ) },
	{	"uint32 *", "indexList", offsetof( FltVertexList *, indexList  ) },
 { 0, 0, 0 }
};

FltRecordEntryName FltGroupEntryName[] = {
	{	"FltNode", "node", offsetof( FltGroup *, node ) },
	{	"char [8]", "ID", offsetof( FltGroup *, ID ) },
	{	"uint16", "relativePriority", offsetof( FltGroup *, relativePriority ) },
	{	"uint16", "reserved0", offsetof( FltGroup *, reserved0 ) },
	{	"uint32", "flags", offsetof( FltGroup *, flags ) },
	{	"uint16", "specialEffectID1", offsetof( FltGroup *, specialEffectID1 ) },
	{	"uint16", "specialEffectID2", offsetof( FltGroup *, specialEffectID2 ) },
	{	"uint16", "significance", offsetof( FltGroup *, significance ) },
	{	"uint8",  "layerCode", offsetof( FltGroup *, layerCode ) },
	{	"uint8",  "reserved1", offsetof( FltGroup *, reserved1 ) },
	{	"uint32", "reserved2", offsetof( FltGroup *, reserved2 ) },
 { 0, 0, 0 }
};

FltRecordEntryName FltObjectEntryName[] = {
	{	"FltNode", "node", offsetof( FltObject *, node ) },
	{	"char [8]", "ID", offsetof( FltObject *, ID ) },
	{	"uint32", "flags", offsetof( FltObject *, flags ) },
	{	"uint16", "relativePriority", offsetof( FltObject *, relativePriority ) },
	{	"uint16", "transparency", offsetof( FltObject *, transparency ) },
	{	"uint16", "specialEffectID1", offsetof( FltObject *, specialEffectID1 ) },
	{	"uint16", "specialEffectID2", offsetof( FltObject *, specialEffectID2 ) },
	{	"uint16", "significance", offsetof( FltObject *, significance ) },
	{	"uint16", "reserved0", offsetof( FltObject *, reserved0 ) },
 { 0, 0, 0 }
};

FltRecordEntryName FltFaceEntryName[] = {
	{	"FltNode", "node", offsetof( FltFace *, node ) },
	{	"char [8]", "ID", offsetof( FltFace *, ID ) },
	{	"uint32", "irColorCode", offsetof( FltFace *, irColorCode ) },
	{	"uint16", "relativePriority", offsetof( FltFace *, relativePriority ) },
	{	"uint8",  "drawType", offsetof( FltFace *, drawType ) },
	{	"uint8",  "textureWhite", offsetof( FltFace *, textureWhite ) },
	{	"uint16", "colorNameIndex", offsetof( FltFace *, colorNameIndex ) },
	{	"uint16", "alternateColorNameIndex", offsetof( FltFace *, alternateColorNameIndex ) },
	{	"uint8",  "reserved0", offsetof( FltFace *, reserved0 ) },
	{	"uint8",  "billboardFlags", offsetof( FltFace *, billboardFlags ) },
	{	"int16",  "detailTexturePatternIndex", offsetof( FltFace *, detailTexturePatternIndex ) },
	{	"int16",  "texturePatternIndex", offsetof( FltFace *, texturePatternIndex)},
	{	"int16",  "materialIndex", offsetof( FltFace *, materialIndex ) },
	{	"uint16", "surfaceMaterialCode", offsetof( FltFace *, surfaceMaterialCode)},
	{	"uint16", "featureID", offsetof( FltFace *, featureID ) },
	{	"uint32", "irMaterialCode", offsetof( FltFace *, irMaterialCode ) },
	{	"uint16", "transparency", offsetof( FltFace *, transparency ) },
	{	"uint8",  "LODGenerationControl", offsetof( FltFace *, LODGenerationControl ) },
	{	"uint8",  "lineStyleIndex", offsetof( FltFace *, lineStyleIndex ) },
	{	"uint32", "miscFlags", offsetof( FltFace *, miscFlags ) },
	{	"uint8",  "lightMode", offsetof( FltFace *, lightMode ) },
	{	"uint8",  "reserved1", offsetof( FltFace *, reserved1 ) },
	{	"uint16", "reserved2", offsetof( FltFace *, reserved2 ) },
	{	"uint32", "reserved3", offsetof( FltFace *, reserved3 ) },
	{	"uint32", "packedColorPrimary", offsetof( FltFace *, packedColorPrimary ) },
	{	"uint32", "packedColorAlternate", offsetof( FltFace *, packedColorAlternate ) },
	{	"uint16", "textureMappingIndex", offsetof( FltFace *, textureMappingIndex ) },
	{	"uint16", "reserved4", offsetof( FltFace *, reserved4 ) },
	{	"uint32", "primaryColorIndex", offsetof( FltFace *, primaryColorIndex ) },
	{	"uint32", "alternateColorIndex", offsetof( FltFace *, alternateColorIndex ) },
	{	"uint16", "reserved5", offsetof( FltFace *, reserved5 ) },
	{	"uint16", "shaderIndex", offsetof( FltFace *, shaderIndex ) },
 { 0, 0, 0 }
};

FltRecordEntryName FltMeshEntryName[] = {
	{	"FltNode", "node", offsetof( FltMesh *, node ) },
	{	"char [8]", "ID", offsetof( FltMesh *, ID ) },
	{	"uint32", "irColorCode", offsetof( FltMesh *, irColorCode ) },
	{	"uint16", "relativePriority", offsetof( FltMesh *, relativePriority ) },
	{	"uint8",  "drawType", offsetof( FltMesh *, drawType ) },
	{	"uint8",  "textureWhite", offsetof( FltMesh *, textureWhite ) },
	{	"uint16", "colorNameIndex", offsetof( FltMesh *, colorNameIndex ) },
	{	"uint16", "alternateColorNameIndex", offsetof( FltMesh *, alternateColorNameIndex ) },
	{	"uint8",  "reserved0", offsetof( FltMesh *, reserved0 ) },
	{	"uint8",  "billboardFlags", offsetof( FltMesh *, billboardFlags ) },
	{	"int16",  "detailTexturePatternIndex", offsetof( FltMesh *, detailTexturePatternIndex ) },
	{	"int16",  "texturePatternIndex", offsetof( FltMesh *, texturePatternIndex)},
	{	"int16",  "materialIndex", offsetof( FltMesh *, materialIndex ) },
	{	"uint16", "surfaceMaterialCode", offsetof( FltMesh *, surfaceMaterialCode)},
	{	"uint16", "featureID", offsetof( FltMesh *, featureID ) },
	{	"uint32", "irMaterialCode", offsetof( FltMesh *, irMaterialCode ) },
	{	"uint16", "transparency", offsetof( FltMesh *, transparency ) },
	{	"uint8",  "LODGenerationControl", offsetof( FltMesh *, LODGenerationControl ) },
	{	"uint8",  "lineStyleIndex", offsetof( FltMesh *, lineStyleIndex ) },
	{	"uint32", "miscFlags", offsetof( FltMesh *, miscFlags ) },
	{	"uint8",  "lightMode", offsetof( FltMesh *, lightMode ) },
	{	"uint8",  "reserved1", offsetof( FltMesh *, reserved1 ) },
	{	"uint16", "reserved2", offsetof( FltMesh *, reserved2 ) },
	{	"uint32", "reserved3", offsetof( FltMesh *, reserved3 ) },
	{	"uint32", "packedColorPrimary", offsetof( FltMesh *, packedColorPrimary ) },
	{	"uint32", "packedColorAlternate", offsetof( FltMesh *, packedColorAlternate ) },
	{	"uint16", "textureMappingIndex", offsetof( FltMesh *, textureMappingIndex ) },
	{	"uint16", "reserved4", offsetof( FltMesh *, reserved4 ) },
	{	"uint32", "primaryColorIndex", offsetof( FltMesh *, primaryColorIndex ) },
	{	"uint32", "alternateColorIndex", offsetof( FltMesh *, alternateColorIndex ) },
	{	"uint16", "reserved5", offsetof( FltMesh *, reserved5 ) },
	{	"uint16", "shaderIndex", offsetof( FltMesh *, shaderIndex ) },
 { 0, 0, 0 }
};

FltRecordEntryName FltMeshPrimitiveEntryName[] = {
	{	"FltNode", "node", offsetof( FltMeshPrimitive *, node ) },
	{	"uint16", "primitiveType", offsetof( FltMeshPrimitive *, primitiveType ) },
	{	"uint16", "vertIndexLength", offsetof( FltMeshPrimitive *, 
																												vertIndexLength ) },
	{	"uint32", "numVerts", offsetof( FltMeshPrimitive *, numVerts ) },
	{	"uint32 *", "indices", offsetof( FltMeshPrimitive *, indices ) },
 { 0, 0, 0 }
};

FltRecordEntryName FltLocalVertexPoolEntryName[] = {
	{	"FltNode", "node", offsetof( FltLocalVertexPool *, node ) },
	{	"uint32", "numVerts", offsetof( FltLocalVertexPool *, numVerts )},
	{	"uint32", "attrMask", offsetof( FltLocalVertexPool *, attrMask ) },
	{	"FltLVPEntry *", "entries", offsetof( FltLocalVertexPool *, entries ) },
 { 0, 0, 0 }
};

FltRecordEntryName FltMultiTextureEntryName[] = {
	{	"FltNode", "node", offsetof( FltMultiTexture *, node ) },
	{	"uint32", "mask", offsetof( FltMultiTexture *, mask )},
	{	"layer *", "layer", offsetof( FltMultiTexture *, layer ) },
 { 0, 0, 0 }
};

FltRecordEntryName FltUVListEntryName[] = {
	{	"FltNode", "node", offsetof( FltUVList *, node ) },
	{	"uint32", "mask", offsetof( FltUVList *, mask )},
	{	"uint32", "numValues", offsetof( FltUVList *, numValues )},
	{	"real32 * ", "uvValues", offsetof( FltUVList *, uvValues )},
 { 0, 0, 0 }
};

FltRecordEntryName FltRoadPathEntryName[] = {
	{ "FltNode",  "node", offsetof( FltRoadPath *, node ) },
	{ "char [8]", "ID", offsetof( FltRoadPath *, ID ) }, 
	{ "uint32", "reserved0", offsetof( FltRoadPath *, reserved0 ) },
	{ "char [120]", "pathName", offsetof( FltRoadPath *, pathName ) },
	{ "real64", "speedLimit", offsetof( FltRoadPath *, speedLimit ) },
	{ "uint32", "noPassing", offsetof( FltRoadPath *, noPassing ) },
	{ "uint32", "vertexNormalType", offsetof( FltRoadPath *, vertexNormalType ) },
	{ "uint8 [480]", "spare", offsetof( FltRoadPath *, spare ) },
	{ 0, 0, 0 }
};

FltRecordEntryName FltRoadSegmentEntryName[] = {
	{ "FltNode",  "node", offsetof( FltRoadSegment *, node ) },
	{ "char [8]", "ID", offsetof( FltRoadSegment *, ID ) }, 
	{ 0, 0, 0 }
};

FltRecordEntryName FltRoadConstructionEntryName[] = {
	{ "FltNode",  "node", offsetof( FltRoadConstruction *, node ) },
	{ "char [8]", "ID", offsetof( FltRoadConstruction *, ID ) }, 
	{ "uint32", "reserved0", offsetof( FltRoadConstruction *, reserved0 ) },
	{ "uint32", "roadType", offsetof( FltRoadConstruction *, roadType ) },
	{ "uint32", "roadtoolsVersion", offsetof( FltRoadConstruction *, 
																											roadtoolsVersion ) },
	{ "real64", "entryX", offsetof( FltRoadConstruction *, entryX ) },
	{ "real64", "entryY", offsetof( FltRoadConstruction *, entryY ) },
	{ "real64", "entryZ", offsetof( FltRoadConstruction *, entryZ ) },
	{ "real64", "alignmentX", offsetof( FltRoadConstruction *, alignmentX ) },
	{ "real64", "alignmentY", offsetof( FltRoadConstruction *, alignmentY ) },
	{ "real64", "alignmentZ", offsetof( FltRoadConstruction *, alignmentZ ) },
	{ "real64", "exitX", offsetof( FltRoadConstruction *, exitX ) },
	{ "real64", "exitY", offsetof( FltRoadConstruction *, exitY ) },
	{ "real64", "exitZ", offsetof( FltRoadConstruction *, exitZ ) },
	{ "real64", "arcRadius", offsetof( FltRoadConstruction *, arcRadius ) },
	{ "real64", "entrySpiralLength", offsetof( FltRoadConstruction *, 
																											entrySpiralLength ) },
	{ "real64", "exitSpiralLength", offsetof( FltRoadConstruction *, 
																											exitSpiralLength ) },
	{ "real64", "superelevation", offsetof( FltRoadConstruction *, 
																											superelevation ) },
	{ "uint32", "spiralType", offsetof( FltRoadConstruction *, spiralType ) },
	{ "uint32", "verticalParabolaFlag", offsetof( FltRoadConstruction *, 
																											verticalParabolaFlag ) },
	{ "real64", "verticalCurveLength", offsetof( FltRoadConstruction *, 
																											verticalCurveLength ) },
	{ "real64", "minimumCurveLength", offsetof( FltRoadConstruction *, 
																											minimumCurveLength ) },
	{ "real64", "entrySlope", offsetof( FltRoadConstruction *, entrySlope ) },
	{ "real64", "exitSlope", offsetof( FltRoadConstruction *, exitSlope ) },
	{ 0, 0, 0 }
};

