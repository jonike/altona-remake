/*+**************************************************************************/
/***                                                                      ***/
/***   This file is distributed under a BSD license.                      ***/
/***   See LICENSE.txt for details.                                       ***/
/***                                                                      ***/
/**************************************************************************+*/

#ifndef FILE_EXTRA_IPP_HPP
#define FILE_EXTRA_IPP_HPP

#include "base/types.hpp"
#include "base/graphics.hpp"

/****************************************************************************/


class sRenderTargetManager_
{
  struct Target
  {
    sTextureBase *Texture;
    int SizeX;
    int SizeY;
    int Format;
    int RefCount;
    int Proxy;
  };
  struct Reference
  {
    void *Ref;
    int SizeX;
    int SizeY;
  };
  sArray<Target> Targets;
  sArray<Reference> Refs;

  sTexture2D *ScreenProxy;
  sBool ScreenProxyDirty;

  Target *Find(sTextureBase *);
  const sRect *Window;
  sTexture2D *ToTexture;
  sTexture2D *ToTextureProxy;
  int ScreenX;
  int ScreenY;
  sGeometry *GeoDouble;
public:

  // initialisation

  sRenderTargetManager_();
  ~sRenderTargetManager_();
  void Flush();
  void ResolutionCheck(void *ref,int xs,int ys);    // flush if resolution registered for reference changes

  // primary interface

  sTexture2D *Acquire(int x,int y,int format=sTEX_2D|sTEX_ARGB8888|sTEX_NOMIPMAPS);
  sTextureCube *AcquireCube(int edge,int format=sTEX_CUBE|sTEX_ARGB8888|sTEX_NOMIPMAPS);
  sTextureBase *AcquireProxy(sTexture2D *tex);
  void AddRef(sTextureBase *);
  void Release(sTextureBase *);

  // screen buffer management

  void SetScreen(const sRect *window);
  void SetScreen(const sTargetSpec &spec);
  sTexture2D *ReadScreen();                   // grab screen to texture
  sTexture2D *WriteScreen(sBool finish=0);    // queue a texture for write-to-screen. return 0 for direct screen rendering
  void SetTarget(sTexture2D *tex,int clrflags=0,uint32_t clrcol=0,sTexture2D *dep=0); // set rendertarget. tex might be 0
  void FinishScreen();                                // write back te
};

extern sRenderTargetManager_ *sRTMan;     // automatically created on startup

/****************************************************************************/

#endif // FILE_EXTRA_IPP_HPP

