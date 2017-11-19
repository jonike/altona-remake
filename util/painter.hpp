/*+**************************************************************************/
/***                                                                      ***/
/***   This file is distributed under a BSD license.                      ***/
/***   See LICENSE.txt for details.                                       ***/
/***                                                                      ***/
/**************************************************************************+*/

/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/

#ifndef HEADER_ALTONA_UTIL_PAINTER
#define HEADER_ALTONA_UTIL_PAINTER

#ifndef __GNUC__
#pragma once
#endif


#include "base/types.hpp"
#include "base/system.hpp"

/****************************************************************************/

class sBasicPainter
{
#if !sCONFIG_QUADRICS
  int Alloc;
  int Used;
  struct sVertexSingle *Vertices;
#endif

  class sGeometry *Geo;
  class sGeometry *GeoX;
  class sGeometry *GeoXStandard;
  class sSimpleMaterial *Mtrl;
  class sCubeMaterial *CubeMtrl;
  class sMaterialEnv *Env;
  class sTexture2D *Tex;
  class sViewport *View;
  sF32 UVOffset;
  sF32 XYOffset;

public:
  // management
  sBasicPainter(int vertexmax = 0);
  ~sBasicPainter();
  void SetTarget();
  void SetTarget(const sRect &target);
  void Begin();
  void End();
  void Clip(const sFRect &r);
  void ClipOff();

  // shapes
  void Box(const sFRect &r,const sU32 col);
  void Box(const sFRect &r,const sU32 *colors);
  void Line(sF32 x0,sF32 y0,sF32 x1,sF32 y1,sU32 c0,sU32 c1,sBool skiplastpixel=0);

  // fonts. select unique fontid as you like
  void RegisterFont(int fontid,const sChar *name,int height,int style);
  void Print(int fontid,sF32 x,sF32 y,sU32 col,const sChar *text,int len=-1,sF32 zoom=1.0f);
  sF32 GetWidth(int fontid,const sChar *text,int len=-1);
  sF32 GetHeight(int fontid);

  // debbuging textures
  void PaintTexture(sTexture2D* tex, sU32 col=0xffffffff, int xs=-1, int ys=-1, int xo=0, int yo=0, sBool noalpha=sTRUE);
  void PaintTexture(class sTextureCube *tex, int xs=-1, int ys=-1, int xo=0, int yo=0, sBool noalpha=sTRUE);
};

/****************************************************************************/

class sPainter : public sBasicPainter
{
  int FontId;
  int Advance;
  sU32 TextColor;
  sU32 AltColor;
  sF32 Zoom;
  sString<1024> classbuf;
  void Print0(sF32 x,sF32 y,const sChar *txt);
public:
  sPainter(int vertexmax = 0);
  void Box(const sFRect &r,sU32 col);
  void Box(sF32 x0,sF32 y0,sF32 x1,sF32 y1,sU32 col);
  void Print(sF32 x,sF32 y,const sChar *txt);
  void Print(int fontid,sF32 x,sF32 y,sU32 col,const sChar *text,int len=-1,sF32 zoom=1.0f) { sBasicPainter::Print(fontid,x,y,col,text,len,zoom); }
  void SetPrint(int fontid,sU32 col,sF32 zoom,sU32 altcol=0xffffffff,int advance = 10);
  sPRINTING2(PrintF,sFormatStringBuffer buf=sFormatStringBase(classbuf,format);buf,Print0(arg1,arg2,classbuf);,sF32,sF32);

	using sBasicPainter::Box;
};

/****************************************************************************/

// HEADER_ALTONA_UTIL_PAINTER
#endif
