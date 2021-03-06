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

#include "base/types.hpp"
#if sRENDERER == sRENDER_OGL2

#if sCONFIG_SYSTEM_WINDOWS

#undef new
#include <windows.h>
#define new sDEFINE_NEW

extern HWND sHWND;
static HDC GLDC;
static HGLRC GLRC;

#endif

/*#include "gl.h"
#include "glext.h"
#include "glprocs.h"*/
#include "GL/glew.h"

#include "base/system.hpp"
#include "base/math.hpp"
#include "base/graphics.hpp"
#include "base/windows.hpp"

#include "base/serialize.hpp"

void sInitGfxCommon();
void sExitGfxCommon();

int sEngineHacks;

#if sCONFIG_SYSTEM_LINUX

#include <GL/glx.h>

extern Display *sXDisplay();
extern XVisualInfo *sXVisualInfo;
extern Visual *sXVisual;
extern Drawable sXWnd;
extern int sXScreen;
static GLXContext GLXC;

#endif

void GLError(uint32_t err,const sChar *file,int line,const sChar *system)
{
  sString<256> buffer;

  sSPrintF(buffer,L"%s(%d): %s %08x %d\n",file,line,system,err,err);
  sDPrint(buffer);
  sFatal(buffer);
}
#define GLErr(hr) { sBool err=!(hr); if(err) GLError(0,sTXT(__FILE__),__LINE__,L"opengl"); }
#define GLERR() { int err=glGetError(); if(err) GLError(err,sTXT(__FILE__),__LINE__,L"opengl"); }

/****************************************************************************/
/***                                                                      ***/
/***   Globals                                                            ***/
/***                                                                      ***/
/****************************************************************************/

//static sCBufferBase *CurrentCBs[sCBUFFER_MAXSLOT*sCBUFFER_SHADERTYPES];
static sScreenMode DXScreenMode;

/****************************************************************************/

struct sGeoBuffer
{
  GLuint GLName;                  // gl name
  int GLType;                    // GL_ARRAY_BUFFER_ARB or GL_ELEMENT_ARRAY_BUFFER_ARB
  int GLUsage;                   // GL_STATIC_DRAW, ...

  sGeometryDuration Duration;     // sGD_???
  int Type;                      // 0 = VB, 1 = IB

  int Alloc;                     // total available bytes
  int Used;                      // alreaedy used bytes
  int Freed;                     // bytes freed. when Freed == Used, then the buffer is empty and may be reset
};

#define sMAX_GEOBUFFER 256

sGeoBuffer sGeoBuffers[sMAX_GEOBUFFER];
int sGeoBufferCount;

GLuint sFBO = 0;


/****************************************************************************/
/***                                                                      ***/
/***   Vertex Formats                                                     ***/
/***                                                                      ***/
/****************************************************************************/

void sVertexFormatHandle::Create()
{
  sVertexFormatHandle::OGLDecl decl[40];
  int dindex = 0;

  sClear(decl);

  for(int i=0;i<sVF_STREAMMAX;i++)
    VertexSize[i] = 0;

  for(int stream=0;stream<sVF_STREAMMAX;stream++)
  {
    int firstvert = 1;
    int i = 0;
    while(Data[i])
    {
      if(((Data[i]&sVF_STREAMMASK)>>sVF_STREAMSHIFT)==(uint32_t)stream)
      {
        if(firstvert)
        {
          sVERIFY(dindex<sCOUNTOF(decl));
          firstvert = 0;
          decl[dindex].Mode = 2;
          decl[dindex].Index = stream;
          dindex++;
          Streams = stream+1;
        }

        sVERIFY(dindex<sCOUNTOF(decl));
        
        switch(Data[i]&sVF_USEMASK)
        {
        case sVF_NOP:         decl[dindex].Index = -1;  break;
        case sVF_POSITION:    decl[dindex].Index =  0;  break;
        case sVF_NORMAL:      decl[dindex].Index =  2;  break;
        case sVF_TANGENT:     decl[dindex].Index =  6;  break;
        case sVF_BONEINDEX:   decl[dindex].Index =  7;  break;
        case sVF_BONEWEIGHT:  decl[dindex].Index =  1;  break;
        case sVF_COLOR0:      decl[dindex].Index =  3;  break;
        case sVF_COLOR1:      decl[dindex].Index =  4;  break;
        case sVF_UV0:         decl[dindex].Index =  8;  break;
        case sVF_UV1:         decl[dindex].Index =  9;  break;
        case sVF_UV2:         decl[dindex].Index = 10;  break;
        case sVF_UV3:         decl[dindex].Index = 11;  break;
        case sVF_UV4:         decl[dindex].Index = 12;  break;
        case sVF_UV5:         decl[dindex].Index = 13;  break;
        case sVF_UV6:         decl[dindex].Index = 14;  break;
        case sVF_UV7:         decl[dindex].Index = 15;  break;
        default: sVERIFYFALSE;
        }

        int bytes = 0;
        switch(Data[i]&sVF_TYPEMASK)
        {
        case sVF_F2:  decl[dindex].Type=GL_FLOAT;            decl[dindex].Size=2;   decl[dindex].Normalized=0;   bytes=2*4;   break;
        case sVF_F3:  decl[dindex].Type=GL_FLOAT;            decl[dindex].Size=3;   decl[dindex].Normalized=0;   bytes=3*4;   break;
        case sVF_F4:  decl[dindex].Type=GL_FLOAT;            decl[dindex].Size=4;   decl[dindex].Normalized=0;   bytes=4*4;   break;
        case sVF_I4:  decl[dindex].Type=GL_UNSIGNED_BYTE;    decl[dindex].Size=4;   decl[dindex].Normalized=0;   bytes=1*4;   break;
        case sVF_C4:  decl[dindex].Type=GL_UNSIGNED_BYTE;    decl[dindex].Size=4;   decl[dindex].Normalized=1;   bytes=1*4;   break;
        case sVF_S2:  decl[dindex].Type=GL_UNSIGNED_SHORT;   decl[dindex].Size=2;   decl[dindex].Normalized=1;   bytes=1*4;   break;
        case sVF_S4:  decl[dindex].Type=GL_UNSIGNED_SHORT;   decl[dindex].Size=4;   decl[dindex].Normalized=1;   bytes=2*4;   break;
        case sVF_H2:  decl[dindex].Type=GL_HALF_FLOAT_NV;    decl[dindex].Size=2;   decl[dindex].Normalized=0;   bytes=1*4;   break;
        case sVF_H4:  decl[dindex].Type=GL_HALF_FLOAT_NV;    decl[dindex].Size=4;   decl[dindex].Normalized=0;   bytes=2*4;   break;
        case sVF_F1:  decl[dindex].Type=GL_FLOAT;            decl[dindex].Size=1;   decl[dindex].Normalized=0;   bytes=1*4;   break;
        default: sVERIFYFALSE;
        }

        decl[dindex].Mode = 1;
        decl[dindex].Offset = VertexSize[stream];
        VertexSize[stream] += bytes;
        AvailMask |= 1 << (Data[i]&sVF_USEMASK);
        if(decl[dindex].Index >= 0)
          dindex++;
      }
      i++;
    }
  }

  // write endmarker

  sVERIFY(dindex<sCOUNTOF(decl));
  dindex++;

  // copy to handle

  Decl = new sVertexFormatHandle::OGLDecl[dindex];
  sCopyMem(Decl,decl,dindex*sizeof(sVertexFormatHandle::OGLDecl));
}

void sVertexFormatHandle::Destroy()
{
  delete[] Decl;
}

/****************************************************************************/
/***                                                                      ***/
/***   sGeometry                                                          ***/
/***                                                                      ***/
/****************************************************************************/

static sGeoBuffer *sFindGeoBuffer(int bytes,int type,sGeometryDuration duration)
{
  // find available buffer (don't allocate, just find!)

  bytes = sAlign(bytes,128);
  for(int i=0;i<sGeoBufferCount;i++)
  {
    sGeoBuffer *gb = &sGeoBuffers[i];
    if(gb->Duration==duration && gb->Type==type && gb->Used+bytes<=gb->Alloc)
      return gb;
  }

  // allocate new buffer

  sVERIFY(sGeoBufferCount<sMAX_GEOBUFFER);
  sGeoBuffer *gb = &sGeoBuffers[sGeoBufferCount++];
  switch(type)
  {
  case 0:
    gb->Alloc = sMax(bytes,1024*1024);
    gb->GLType = GL_ARRAY_BUFFER_ARB;
    break;
  case 1:
    gb->Alloc = sMax(bytes,128*1024);
    gb->GLType = GL_ELEMENT_ARRAY_BUFFER_ARB;
    break;
  default:
    sVERIFYFALSE;
  }
  gb->Used = 0;
  gb->Freed = 0;
  gb->Duration = duration;
  gb->Type = type;
  gb->GLName = 0;
  sVERIFY(bytes<=gb->Alloc);

  // create GL object for buffer

  switch(gb->Duration)
  {
  case sGD_STREAM:
    gb->GLUsage = GL_STREAM_DRAW; 
    break;
  case sGD_FRAME:
    gb->GLUsage = GL_DYNAMIC_DRAW;
    break;
  case sGD_STATIC:
    gb->GLUsage = GL_STATIC_DRAW;
    break;
  default:
    sVERIFYFALSE;
  }
  glGenBuffersARB(1,&gb->GLName);
  GLERR();
  glBindBufferARB(gb->GLType,gb->GLName);
  glBufferDataARB(gb->GLType,gb->Alloc,0,gb->GLUsage);
  GLERR();

  // done

  return gb;
}

sGeoBufferPart::sGeoBufferPart()
{
  Buffer = 0;
  Start = 0;
  Count = 0;
}

sGeoBufferPart::~sGeoBufferPart()
{
  Clear();
}

void sGeoBufferPart::Clear()
{
  if(Buffer)
  {
    Buffer->Freed += Count;
    if(Buffer->Freed == Buffer->Used && Buffer->Duration==sGD_STATIC)
      Buffer->Used = Buffer->Freed = 0;
    Buffer = 0;
  }
  Start = 0;
  Count = 0;
}

void sGeoBufferPart::Init(int count,int size,sGeometryDuration duration,int buffertype)
{
  Buffer = sFindGeoBuffer(size*count,buffertype,duration);
  Start = Buffer->Used;
  Count = count;
}

void sGeoBufferPart::Lock(void **ptr)
{
  glBindBufferARB(Buffer->GLType,Buffer->GLName);
  uint8_t *data = (uint8_t *)glMapBufferARB(Buffer->GLType,GL_WRITE_ONLY);
  GLERR();
  glBindBufferARB(Buffer->GLType,0);
  data+=Start;
  *ptr = data;
}

void sGeoBufferPart::Unlock(int count,int size)
{
  if(count!=-1)
    Count = count;
  Buffer->Used += sAlign(Count*size,128);
  sVERIFY(Buffer->Used<=Buffer->Alloc);
  glBindBufferARB(Buffer->GLType,Buffer->GLName);
  glUnmapBufferARB(Buffer->GLType);
  GLERR();
  glBindBufferARB(Buffer->GLType,0);
}

/****************************************************************************/

void sGeometry::Draw()
{
  sGeometry::Draw(0, 0, 0, 0);
}

void sGeometry::Draw(sDrawRange *ir,int irc,int instancecount, sVertexOffset *off/*=0*/)
{
  // set vertexformat
  sVERIFY(!off);      // vertex stream offset currently unsupported

  sVertexFormatHandle::OGLDecl *decl = Format->GetDecl();
  int stride = 0;
  int start = 0;
  int disablemask = 0;

  if(DebugBreak) sDEBUGBREAK;
  
  glEnableClientState(GL_VERTEX_ARRAY);

  while(decl->Mode!=0)
  {
    if(decl->Mode==2)
    {
      stride = Format->GetSize(decl->Index);
      start = VertexPart[decl->Index].Start;
      glBindBufferARB(GL_ARRAY_BUFFER_ARB,VertexPart[decl->Index].Buffer->GLName);
    }
    else
    {
      glVertexAttribPointerARB(
        decl->Index,decl->Size,decl->Type,decl->Normalized,
        stride,(const void *)(ptrdiff_t)(decl->Offset+start));
      glEnableVertexAttribArrayARB(decl->Index);
      disablemask |= (1<<decl->Index);
    }
    decl++;
  }

  // figure primitive type

  int primtype = 0;
  switch(Flags & sGF_PRIMMASK)
  {
  case sGF_TRILIST:
    primtype = GL_TRIANGLES;
    break;
  case sGF_TRISTRIP:
    primtype = GL_TRIANGLE_STRIP;
    break;
  case sGF_LINELIST:
    primtype = GL_LINES;
    break;
  case sGF_LINESTRIP:
    primtype = GL_LINE_STRIP;
    break;
  case sGF_QUADLIST:
    primtype = GL_QUADS;
    break;
  case sGF_SPRITELIST:
    primtype = GL_POINT;
    break;
  default:
    sVERIFYFALSE;
  }

  // fake color to white when unused

  if(!(disablemask&(1<<3)))
    glVertexAttrib4fARB(3,1,1,1,1);

  // do drawing

  sVERIFY(ir==0);
  sVERIFY(irc==0);
  sVERIFY(instancecount==0);

  if(IndexPart.Buffer)
  {
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,IndexPart.Buffer->GLName);
    const int type = (Flags & sGF_INDEX32) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
    glDrawRangeElements(primtype,0,VertexPart[0].Count-1,IndexPart.Count, 
      type, (void*) ptrdiff_t(IndexPart.Start));
  }
  else
  {
    glDrawArrays(primtype,0,VertexPart[0].Count);
  }
  GLERR();

  // disable and unbind

  for(int i=0;i<16;i++)
    if(disablemask & (1<<i))
      glDisableVertexAttribArrayARB(i);

  glDisableClientState(GL_VERTEX_ARRAY);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
  glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
}

/****************************************************************************/
/***                                                                      ***/
/***   Viewport                                                           ***/
/***                                                                      ***/
/****************************************************************************/

/****************************************************************************/
/***                                                                      ***/
/***   Texture                                                            ***/
/***                                                                      ***/
/****************************************************************************/

sBool sReadTexture(sReader &s, sTextureBase *&tex)
{
  return sFALSE;
}

uint64_t sGetAvailTextureFormats()
{
  return (1ULL<<sTEX_ARGB8888);
}

void sTexture2D::Create2(int flags)
{
  GLuint name;
  glGenTextures(1,&name);
  GLName = name;
}

void sTexture2D::Destroy2()
{
  GLuint name = GLName;
  glDeleteTextures(1,&name);
  GLName = 0;
}

void sTexture2D::BeginLoad(uint8_t *&data,int &pitch,int mipmap)
{
  sVERIFY(LoadBuffer==0);
  sVERIFY(mipmap>=0 && mipmap<Mipmaps);
  int xs = SizeX>>mipmap;
  int ys = SizeY>>mipmap;

  LoadMipmap = mipmap;
  LoadBuffer = new uint8_t[BitsPerPixel*xs*ys/8];
  data = LoadBuffer;
  pitch = BitsPerPixel*xs/8;
}

void sTexture2D::EndLoad()
{
  sVERIFY(LoadBuffer!=0);
  int format = 0, channels = 0, type = 0;

  int xs = SizeX>>LoadMipmap;
  int ys = SizeY>>LoadMipmap;

  switch(Flags & sTEX_FORMAT)
  {
  case sTEX_ARGB8888:
    format = GL_RGBA8;
    channels = GL_BGRA;
    type = GL_UNSIGNED_BYTE;
    break;
  case sTEX_QWVU8888:
    format = GL_RGBA8I;
    channels = GL_BGRA_INTEGER;
    type = GL_BYTE;
    break;
  case sTEX_ARGB4444:
    format = GL_RGBA4;
    channels = GL_BGRA;
    type = GL_UNSIGNED_SHORT_4_4_4_4_REV;
    break;
  case sTEX_8TOIA:
    format = GL_INTENSITY8;
    channels = GL_INTENSITY8;
    type = GL_UNSIGNED_BYTE;
    break;
  case sTEX_DXT1:
    format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    channels = GL_RGB;
    type = GL_UNSIGNED_BYTE;
    break;
  case sTEX_DXT3:
    format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    channels = GL_RGBA;
    type = GL_UNSIGNED_BYTE;
    break;
  case sTEX_DXT5:
    format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    channels = GL_RGBA;
    type = GL_UNSIGNED_BYTE;
    break;
  case sTEX_I8:
    format = GL_LUMINANCE8;
    channels = GL_LUMINANCE8;
    type = GL_UNSIGNED_BYTE;
    break;
  
  default:
    sFatal(L"unsupported texture format %d\n",Flags);
  }

  glBindTexture(GL_TEXTURE_2D,GLName);
  glTexImage2D(GL_TEXTURE_2D,LoadMipmap,format,xs,ys,0,channels,type,LoadBuffer);
  glBindTexture(GL_TEXTURE_2D,0);

  sDeleteArray(LoadBuffer);
}

void *sTexture2D::BeginLoadPalette()
{
  sFatal(L"paletted textures not supported");
  return 0;
}

void sTexture2D::EndLoadPalette()
{
}

/****************************************************************************/

void sTextureCube::Create2(int flags)
{
  GLuint name;
  glGenTextures(1, &name);
  GLName = name;
}

void sTextureCube::Destroy2()
{
  GLuint name = GLName;
  glDeleteTextures(1, &name);
  GLName = 0;
}

void sTextureCube::BeginLoad(sTexCubeFace cf, uint8_t*& data, int& pitch, int mipmap)
{
  sVERIFY(LoadBuffer == 0);
  sVERIFY(mipmap >= 0 && mipmap < Mipmaps);
  int xs = SizeX >> mipmap;
  int ys = SizeY >> mipmap;

  LoadFace = cf;
  LoadMipmap = mipmap;
  LoadBuffer = new uint8_t[BitsPerPixel * xs * ys / 8];
  data = LoadBuffer;
  pitch = BitsPerPixel * xs / 8;
}

void sTextureCube::EndLoad()
{
  sVERIFY(LoadBuffer != 0);
  int format = 0, channels = 0, type = 0;

  int xs = SizeX >> LoadMipmap;
  int ys = SizeY >> LoadMipmap;

  switch (Flags & sTEX_FORMAT)
  {
  case sTEX_ARGB8888:
    format = GL_RGBA8;
    channels = GL_BGRA;
    type = GL_UNSIGNED_INT_8_8_8_8_REV;
    break;
  case sTEX_QWVU8888:
    format = GL_RGBA8I;
    channels = GL_BGRA_INTEGER;
    type = GL_BYTE;
    break;
  case sTEX_8TOIA:
    format = GL_INTENSITY8;
    channels = GL_INTENSITY8;
    type = GL_UNSIGNED_BYTE;
    break;
  case sTEX_I8:
    format = GL_LUMINANCE8;
    channels = GL_LUMINANCE8;
    type = GL_UNSIGNED_BYTE;
    break;
  default:
    sFatal(L"unsupported texture format %d\n", Flags);
  }

  glBindTexture(GL_TEXTURE_CUBE_MAP, GLName);
  GLenum face;
  switch(LoadFace) {
    case sTCF_POSX: 
      face = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
      break;
    case sTCF_NEGX:
      face = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
      break;
    case sTCF_POSY:
      face = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
      break;
    case sTCF_NEGY:
      face = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
      break;
    case sTCF_POSZ:
      face = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
      break;
    case sTCF_NEGZ:
      face = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
      break;
  }
  glTexImage2D(face, LoadMipmap, format, xs, ys, 0, channels, type, LoadBuffer);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

  sDeleteArray(LoadBuffer);
}

/****************************************************************************/

void sPackDXT(uint8_t *d,uint32_t *bmp,int xs,int ys,int format,sBool dither)
{
  sVERIFY("sPackDXT not supported with opengl")
}

/****************************************************************************/
/***                                                                      ***/
/***   Material                                                           ***/
/***                                                                      ***/
/****************************************************************************/

void sMaterial::Create2()
{
}

void sMaterial::Destroy2()
{
}

void sMaterial::Prepare(sVertexFormatHandle *format)
{
  sShader *oldvs = VertexShader;   VertexShader = 0;
  sShader *oldps = PixelShader;    PixelShader = 0;

  SelectShaders(format);

  oldvs->Release();
  oldps->Release();
}

void sMaterial::InitVariants(int variants)
{
}

void sMaterial::DiscardVariants()
{
}

void sMaterial::SetVariant(int variants)
{
}

void sMaterial::Set(sCBufferBase **cbuffers,int cbcount,sBool additive)
{
  SetStates();

  if(VertexShader)
  {
    glEnable(GL_VERTEX_PROGRAM_ARB);
    glBindProgramARB(GL_VERTEX_PROGRAM_ARB,VertexShader->GLName);
  }
  else
  {
    glBindProgramARB(GL_VERTEX_PROGRAM_ARB,0);
    glDisable(GL_VERTEX_PROGRAM_ARB);
  }
  if(PixelShader)
  {
    glEnable(GL_FRAGMENT_PROGRAM_ARB);
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,PixelShader->GLName);
  }
  else
  {
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,0);
  }

  // set constant buffers

  /*for(int i=0;i<cbcount;i++)
  {
    sCBufferBase *cb = cbuffers[i];
    sVERIFY(cb->Slot<sCBUFFER_MAXSLOT*sCBUFFER_SHADERTYPES);
    if(cb != CurrentCBs[cb->Slot])
    {
      cb->SetRegs();
      CurrentCBs[cb->Slot] = cb;
    }
  }*/

  sSetCBuffers(cbuffers,cbcount);
}

static void makeblend(int blend,int &s,int &d,int &f)
{
  static const int arg[16] = 
  {
    0,
    GL_ZERO,
    GL_ONE,
    GL_SRC_COLOR,
    GL_ONE_MINUS_SRC_COLOR,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_COLOR,
    GL_ONE_MINUS_DST_COLOR,
    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA,
    GL_SRC_ALPHA_SATURATE,
    0,
    0,
    GL_CONSTANT_COLOR,
    GL_ONE_MINUS_CONSTANT_COLOR,
  };
  static const int func[16] =
  {
    0,
    GL_FUNC_ADD,
    GL_FUNC_SUBTRACT,
    GL_FUNC_REVERSE_SUBTRACT,
    GL_MIN,
    GL_MAX,
  };

  s = arg[blend&15];
  f = func[(blend>>8)&15];
  d = arg[(blend>>16)&15];
}

void sMaterial::SetStates(int variant_ignored)
{
  // basic flags
	glDepthMask((Flags & sMTRL_ZWRITE) != 0);

  if(Flags & sMTRL_ZREAD)
  {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
  }
  else
  {
    glDisable(GL_DEPTH_TEST);
  }

  switch(Flags & sMTRL_CULLMASK)
  {
  case sMTRL_CULLOFF:
    glDisable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    break;
  case sMTRL_CULLON:
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    break;
  case sMTRL_CULLINV:
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    break;
  }

  glColorMask(1,1,1,1);

  // stencil (not yet implemented)

  glStencilMask(1);

  // alpha blending

  if(BlendColor==sMB_OFF)
  {
    glDisable(GL_BLEND);
  }
  else
  {
    glEnable(GL_BLEND);
    if(BlendAlpha==sMB_SAMEASCOLOR)
    {
      int s,d,f;
      makeblend(BlendColor,s,d,f);
      glBlendEquation(f);
      glBlendFunc(s,d);
    }
    else
    {
      int sc,dc,fc;
      int sa,da,fa;
      makeblend(BlendColor,sc,dc,fc);
      makeblend(BlendAlpha,sa,da,fa);
      glBlendEquationSeparateEXT(fc,fa);
      glBlendFuncSeparate(sc,dc,sa,da);
    }
  }

  // alpha test
  const uint8_t alphaFunc = FuncFlags[sMFT_ALPHA];

  if (sMFF_ALWAYS == alphaFunc)
  {
    glDisable(GL_ALPHA_TEST);
  }
  else
  {
    glEnable(GL_ALPHA_TEST);
    const GLenum alphaFuncs[] = {GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL};
    sVERIFY(0 <= alphaFunc);
    sVERIFY(alphaFunc < sizeof(alphaFuncs));
    glAlphaFunc(alphaFuncs[alphaFunc], AlphaRef / 255.0f);
  }

  // textures

  for(int i=0;i<sMTRL_MAXTEX;i++)
  {
    sSetTexture(i,Texture[i]);

    if(Texture[i])
    {
      int mipmaps = Texture[i]->Mipmaps>1;
      switch(TFlags[i] & sMTF_LEVELMASK)
      {
      case sMTF_LEVEL0:
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,mipmaps?GL_NEAREST_MIPMAP_NEAREST:GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        break;
      case sMTF_LEVEL1:
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,mipmaps?GL_NEAREST_MIPMAP_LINEAR:GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        break;
      case sMTF_LEVEL2:
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,mipmaps?GL_LINEAR_MIPMAP_LINEAR:GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        break;
      case sMTF_LEVEL3:
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,mipmaps?GL_LINEAR_MIPMAP_LINEAR:GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        break;
      }
      switch(TFlags[i] & sMTF_ADDRMASK)
      {
      case sMTF_TILE:
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        break;
      case sMTF_CLAMP:
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        break;
      case sMTF_MIRROR:
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_MIRRORED_REPEAT);
        break;
      case sMTF_BORDER:
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
        break;
      case sMTF_TILEU_CLAMPV:
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        break;
      }
    }
  }
}

/****************************************************************************/
/***                                                                      ***/
/***   Shaders                                                            ***/
/***                                                                      ***/
/****************************************************************************/

sShaderTypeFlag sGetShaderPlatform()
{
  return sSTF_NVIDIA;
}

int sGetShaderProfile()
{
  return sSTF_NVIDIA;
}

void sSetVSParamImpl(int o, int count, const sVector4* vsf)
{
  while(count>0)
  {
    glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB,o,&vsf->x);
    o++;
    vsf++;
    count--;
  }
}

void sSetVSParam(int o, int count, const sVector4* vsf)
{
  sSetVSParamImpl(o, count, vsf);
}

void sSetPSParamImpl(int o, int count, const sVector4* psf)
{
  while(count>0)
  {
    glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB,o,&psf->x);
    o++;
    psf++;
    count--;
  }
}

void sSetPSParam(int o, int count, const sVector4* psf)
{
  sSetPSParamImpl(o,count,psf);
}

void sSetVSBool(uint32_t bits,uint32_t mask)
{
}

void sSetPSBool(uint32_t bits,uint32_t mask)
{
}

void sCreateShader2(sShader *shader, sShaderBlob *blob)
{
  int type;
  GLuint name;

  switch(shader->Type&sSTF_KIND)
  {
  case sSTF_VERTEX:
    type = GL_VERTEX_PROGRAM_ARB;
    break;
  case sSTF_PIXEL:
    type = GL_FRAGMENT_PROGRAM_ARB;
    break;
  default:
    type = 0;
    sVERIFYFALSE;
  }
  
  glGenProgramsARB(1,&name);
  glBindProgramARB(type,name);
  int size = blob->Size;
  if(size>0 && blob->Data[size-1]==0)     // there might be a trailing zero
    size--;

  glProgramStringARB(type,GL_PROGRAM_FORMAT_ASCII_ARB,size,blob->Data);  
  if(glGetError()!=0)
  {
    const sChar8 *error = (const sChar8 *)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
    sString<256> buffer;
    sCopyString(buffer,error,sCOUNTOF(buffer));
    sFatal(L"shader compiler error %q\n",buffer);
  }

  shader->GLName = name;
}

void sDeleteShader2(sShader *shader)
{
  GLuint name = shader->GLName;
  glDeleteProgramsARB(1,&name);
  shader->GLName = 0;
}

/****************************************************************************/

sCBufferBase::sCBufferBase()
{
  DataPtr = 0;
  DataPersist = 0;
  Slot = 0;
  Flags = 0;
}

void sCBufferBase::SetPtr(void **dataptr,void *data)
{
  DataPtr = dataptr;
  DataPersist = data;
  if(DataPtr)
    *DataPtr = DataPersist;
}

void sCBufferBase::SetRegs()
{
  switch(Slot & sCBUFFER_SHADERMASK)
  {
  case sCBUFFER_VS:
    sSetVSParamImpl(RegStart,RegCount,(sVector4*)DataPersist);
    break;
  case sCBUFFER_PS:
    sSetPSParamImpl(RegStart,RegCount,(sVector4*)DataPersist);
    break;
  }
  *DataPtr = 0;
}

sCBufferBase::~sCBufferBase()
{
}

/****************************************************************************/
/***                                                                      ***/
/***   Initialisation                                                     ***/
/***                                                                      ***/
/****************************************************************************/

void PreInitGFX(int &flags,int &xs,int &ys)
{
#if sPLATFORM == sPLAT_WINDOWS
  
  if(flags & sISF_FULLSCREEN)
  {
	  DEVMODE dm;
    sClear(dm);
	  dm.dmSize=sizeof(dm);
	  dm.dmPelsWidth	= xs;
	  dm.dmPelsHeight	= ys;
	  dm.dmBitsPerPel	= 32;
	  dm.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

	  if(ChangeDisplaySettings(&dm,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
    {
	    dm.dmPelsWidth	= xs = 800;
	    dm.dmPelsHeight	= ys = 600;
	    if(ChangeDisplaySettings(&dm,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
      {
        sFatal(L"could not set displaymode %dx%d",xs,ys);
      }
    }
  }

#elif sPLATFORM == sPLAT_LINUX
  
  static int attribList[] =
  {
    GLX_RGBA, GLX_DOUBLEBUFFER,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_DEPTH_SIZE, 24,
    GLX_STENCIL_SIZE, 8,
    None
  };
  
  Display *dpy = sXDisplay();
  
  sXVisualInfo = glXChooseVisual(dpy,sXScreen,attribList);
  if(!sXVisualInfo)
    sFatal(L"glXChooseVisual returned 0!\n");
  
#endif
}

void InitGFX(int flags,int xs,int ys)
{
#if sPLATFORM == sPLAT_WINDOWS
  PIXELFORMATDESCRIPTOR pfd;
  int format;

  // get the device context (DC). Note: we don't use the forbidden GetDC(0)!

  GLDC = GetDC(sHWND);
  GLErr(GLDC);
  
  // set the pixel format for the DC

  sClear(pfd);
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  if(flags & sISF_2D)
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
  else
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 24;
  pfd.iLayerType = PFD_MAIN_PLANE;

  GLErr(format = ChoosePixelFormat(GLDC,&pfd));
  GLErr(SetPixelFormat(GLDC,format,&pfd));
  
  // create and enable the render context (RC)

  GLErr(GLRC = wglCreateContext(GLDC));
  GLErr(wglMakeCurrent(GLDC,GLRC));

#elif sPLATFORM == sPLAT_LINUX  
  
  Display *dpy = sXDisplay();
  GLXC = glXCreateContext(dpy,sXVisualInfo,0,True);
  if(!GLXC)
    sFatal(L"glXCreateContext failed!\n");
  
  if(!glXMakeCurrent(dpy,sXWnd,GLXC))
    sFatal(L"glXMakeCurrent failed!\n");
  
#endif

  glewInit();
  
  // query extensions and other strings

  sString<256> Vendor;
  sString<256> Renderer;
  sString<256> Version;
  sString<256> Extension;
  sCopyString(Vendor  ,(const sChar8 *)glGetString(GL_VENDOR)  ,sCOUNTOF(Vendor  ));
  sCopyString(Renderer,(const sChar8 *)glGetString(GL_RENDERER),sCOUNTOF(Renderer));
  sCopyString(Version ,(const sChar8 *)glGetString(GL_VERSION ),sCOUNTOF(Version ));
  const GLubyte *ext = glGetString(GL_EXTENSIONS);

  sLogF(L"gfx",L"GL Vendor  <%s>\n",Vendor);
  sLogF(L"gfx",L"GL Renderer <%s>\n",Renderer);
  sLogF(L"gfx",L"GL Version  <%s>\n",Version);
  sLogF(L"gfx",L"GL Extensions:\n");
  while(*ext!=0)
  {
    int i=0;
    while(*ext==' ') ext++;
    while(*ext!=' ' && *ext!=0)
      Extension[i++] = *ext++;
    while(*ext==' ') ext++;
    Extension[i++] = 0;

    sLogF(L"gfx",L"<%s>\n",Extension);
  }
  sLogF(L"gfx",L".. end of extension list\n");



  // other initializaation

  DXScreenMode.ScreenX = xs;
  DXScreenMode.ScreenY = ys;
  DXScreenMode.Aspect = float(xs)/ys;
  sGFXViewRect.Init(0,0,xs,ys);

  sInitGfxCommon();
}

void ExitGFX()
{
  sExitGfxCommon();

#if sPLATFORM == sPLAT_WINDOWS
  wglMakeCurrent(0,0);
  wglDeleteContext(GLRC);
  ReleaseDC(sHWND,GLDC);
#elif sPLATFORM == sPLAT_LINUX
  Display *dpy = sXDisplay();
  
  glXMakeCurrent(dpy,None,0);
  glXDestroyContext(dpy,GLXC);
#endif
}

void ResizeGFX(int x,int y)  // this is called when the windows size changes
{
  if(x && y && (x!=DXScreenMode.ScreenX || y!=DXScreenMode.ScreenY)) 
  {
    DXScreenMode.ScreenX = x;
    DXScreenMode.ScreenY = y;
  }
}


/****************************************************************************/
/***                                                                      ***/
/***   ScreenModes                                                        ***/
/***                                                                      ***/
/****************************************************************************/

int sGetDisplayCount()
{
  return 1;
}

void sGetScreenInfo(sScreenInfo &si,int flags,int display)
{
  si.Clear();
  si.Resolutions.HintSize(1);
  si.RefreshRates.HintSize(1);
  si.AspectRatios.HintSize(1);
  si.Resolutions.AddMany(1);
  si.RefreshRates.AddMany(1);
  si.AspectRatios.AddMany(1);

  si.Resolutions[0].x = 800;
  si.Resolutions[0].y = 600;
  si.RefreshRates[0] = 60;
  si.AspectRatios[0].x = 4;
  si.AspectRatios[0].y = 3;
  si.CurrentAspect = float(DXScreenMode.ScreenX)/DXScreenMode.ScreenY;
  si.CurrentXSize = DXScreenMode.ScreenX;
  si.CurrentYSize = DXScreenMode.ScreenY;
}

void sGetScreenMode(sScreenMode &sm)
{
  sm = DXScreenMode;
}

sBool sSetScreenMode(const sScreenMode &sm)
{
  DXScreenMode = sm;
  return 1;
}

void sGetScreenSafeArea(float &xs, float &ys)
{
#if sCONFIG_BUILD_DEBUG || sCONFIG_BUILD_DEBUGFAST
  xs = 0.95f;
  ys = 0.9f;
#elif sCONFIG_OPTION_LOCAKIT
  xs = 0.95f;
  ys = 0.95f;
#else
  xs=ys=1;
#endif
}

void sSetScreen(class sTexture2D *tex, sGrabFilterFlags filter, const sRect *dst, const sRect *src)
{
}

/****************************************************************************/
/***                                                                      ***/
/***   Rendering                                                          ***/
/***                                                                      ***/
/****************************************************************************/

sTexture2D *sGetCurrentBackBuffer(void)
{
  return 0;
}

sTexture2D *sGetCurrentBackZBuffer(void)
{
  return 0;
}

/*void sSetRendertarget(const sRect *vrp,int flags,uint32_t clearcolor,sTexture2D **tex,int count)
{
  sVERIFY(0);
  
}

static void sSetRendertargetPrivate(const sRect *vrp,int flags,uint32_t color)
{
}

void sSetRendertarget(const sRect *vrp, int clearflags, uint32_t clearcolor)
{
  sSetRendertarget(vrp,0,clearflags,clearcolor);
}

void sSetRendertarget(const sRect *vrp,sTexture2D *tex, int clearflags, uint32_t clearcolor)
{
  sVERIFY(tex==0);


  // find rect

  sRect r;
  if(vrp)
    r = *vrp;
  else
    r.Init(0,0,DXScreenMode.ScreenX,DXScreenMode.ScreenY);

  // set scissor

  glScissor(r.x0,DXScreenMode.ScreenY-r.y1,r.SizeX(),r.SizeY());
  glEnable(GL_SCISSOR_TEST);
  glViewport(r.x0,DXScreenMode.ScreenY-r.y1,r.SizeX(),r.SizeY());

  // clear

  sVector4 col;
  col.InitColor(clearcolor);
  glDepthMask(1);
  glColorMask(1,1,1,1);
  glStencilMask(1);
  glClearColor(col.x,col.y,col.z,col.w);
  glClearDepth(1.0f);

  int mode = 0;
  if(clearflags & sCLEAR_COLOR) mode |= GL_COLOR_BUFFER_BIT;
  if(clearflags & sCLEAR_ZBUFFER) mode |= GL_DEPTH_BUFFER_BIT;

  glClear(mode);

}

void sSetRendertargetCube(sTextureCube* tex,sTexCubeFace face,int cf, uint32_t cc)
{
  sFatal(L"sSetRendertargetCube not implemented!");
}

void sGrabScreen(class sTexture2D *tex, sGrabFilterFlags filter, const sRect *dst, const sRect *src)
{
  sFatal(L"sGrabScreen not implemented!");
}*/

void sSetTarget(const sTargetPara &para) {
  //sVERIFY(tex == 0);

  // find rect

  /*sRect r;
  if (vrp)
    r = *vrp;
  else
    r.Init(0, 0, DXScreenMode.ScreenX, DXScreenMode.ScreenY);*/
  if(sFBO) {
    glDeleteFramebuffersEXT(1, &sFBO);
  }
  glGenFramebuffersEXT(1, &sFBO);
  if(para.Flags & sST_SCISSOR) {
    glViewport(para.Window.x0, DXScreenMode.ScreenY - para.Window.y1, para.Window.SizeX(), para.Window.SizeY());
    glScissor(para.Window.x0, DXScreenMode.ScreenY - para.Window.y1, para.Window.SizeX(), para.Window.SizeY());
    glEnable(GL_SCISSOR_TEST);
  } else {
    glDisable(GL_SCISSOR_TEST);
    glViewport(0, 0, DXScreenMode.ScreenX, DXScreenMode.ScreenY);
  }

  // clear

  glDepthMask(1);
  glColorMask(1, 1, 1, 1);
  glStencilMask(1);
  glClearColor(para.ClearColor[0].x, para.ClearColor[0].y, para.ClearColor[0].z, para.ClearColor[0].w);
  glClearDepth(1.0f);

  int mode = 0;
  if (para.Flags & sST_CLEARCOLOR)
    mode |= GL_COLOR_BUFFER_BIT;
  if (para.Flags & sST_CLEARDEPTH)
    mode |= GL_DEPTH_BUFFER_BIT;

  glClear(mode);
}

sTexture2D *sGetRTDepthBuffer() {
  //sFatal(L"sGetRTDepthBuffer is not implemented for OpenGL");
  return sNULL;
}

sTexture2D *sGetScreenDepthBuffer(int screen)
{
  //sFatal(L"sGetScreenDepthBuffer is not implemented for OpenGL");
  return sNULL;
}

sTexture2D *sGetScreenColorBuffer(int screen)
{
  //sFatal(L"sGetScreenColorBuffer is not implemented for OpenGL");
  return sNULL;
}

void sCopyTexture(const sCopyTexturePara &para) {

}

void sBeginSaveRT(const uint8_t *&data, int32_t &pitch, enum sTextureFlags &flags)
{
}

void sEndSaveRT()
{
}

void sBeginReadTexture(const uint8_t*& data, int32_t& pitch, enum sTextureFlags& flags,sTexture2D *tex)
{
  sVERIFYFALSE;
}

void sEndReadTexture()
{
}

void sRender3DEnd(sBool flip)
{
  sPreFlipHook->Call();
#if sPLATFORM == sPLAT_WINDOWS
//  SwapBuffers(GLDC);
#elif sPLATFORM == sPLAT_LINUX
  glXSwapBuffers(sXDisplay(),sXWnd);
#endif
  
  sFlipMem();
  sPostFlipHook->Call();
}

void sRender3DFlush()
{
}

sBool sRender3DBegin()
{
  // prepare rendering

  sGFXRendertargetX = DXScreenMode.ScreenX;
  sGFXRendertargetY = DXScreenMode.ScreenY;
  sGFXRendertargetAspect = DXScreenMode.Aspect;
  sGFXViewRect.Init(0,0,DXScreenMode.ScreenX,DXScreenMode.ScreenY);
  // reset dynamic buffers

  for(int i=0;i<sGeoBufferCount;i++)
  {
    sGeoBuffer *geo = &sGeoBuffers[i];
    if(geo->Duration==sGD_STREAM || geo->Duration==sGD_FRAME)
    {
      sVERIFY(geo->Alloc);
      geo->Used = 0;
      geo->Freed = 0;
    }
  }

  // set some common gl-states

//	glEnable(GL_DEPTH_TEST);
//	glDisable(GL_DEPTH_TEST);
//	glDepthFunc(GL_LEQUAL);
  glDisable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
  
  // clear caching

  return 1;
}

void sSetRenderClipping(sRect *r,int count)
{
}


void sGetGraphicsStats(sGraphicsStats &stat)
{
  sLogF(L"gfx",L"sGetGraphicsStats not implemented\n");
}

int sRenderStateTexture(uint32_t* data, int texstage, uint32_t tflags)
{
  sLogF(L"gfx",L"sRenderStateTexture not implemented\n");
  return 0;
}

void sSetRenderStates(const uint32_t* data, int count)
{
  sLogF(L"gfx",L"sSetRenderStates not implemented\n");
}

int sRenderStateTexture(uint32_t* data, int texstage, uint32_t tflags,float lodbias)
{
  sFatal(L"sRenderStateTexture not implemented!");
  return 0;
}

void sSetTexture(int stage,class sTextureBase *tex)
{
  if(tex)
  {
    sVERIFY(tex->Mipmaps>0);
    switch(tex->Flags&sTEX_TYPE_MASK)
    {
      case sTEX_2D:
        glActiveTexture(GL_TEXTURE0+stage);
        glBindTexture(GL_TEXTURE_2D,tex->GLName);
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_TEXTURE_3D);
        glDisable(GL_TEXTURE_CUBE_MAP);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,tex->Mipmaps-1);
        break;
      case sTEX_3D:
        glActiveTexture(GL_TEXTURE0 + stage);
        glBindTexture(GL_TEXTURE_3D, tex->GLName);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_TEXTURE_3D);
        glDisable(GL_TEXTURE_CUBE_MAP);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, tex->Mipmaps - 1);
        break;
      case sTEX_CUBE:
        glActiveTexture(GL_TEXTURE0 + stage);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex->GLName);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_TEXTURE_3D);
        glEnable(GL_TEXTURE_CUBE_MAP);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, tex->Mipmaps - 1);
        break;
    }
  }
  else
  {
//    glActiveTexture(GL_TEXTURE0+stage);

//    glDisable(GL_TEXTURE_2D);
//    glDisable(GL_TEXTURE_CUBE_MAP);
//    glDisable(GL_TEXTURE_3D);

//    glActiveTexture(GL_TEXTURE0);
  }
}

void sSetDesiredFrameRate(float rate)
{
}

void sSetScreenGamma(float gamma)
{
}

void sGetGraphicsCaps(sGraphicsCaps &caps)
{
  sClear(caps);
  caps.ShaderProfile = sSTF_NVIDIA;
  caps.UVOffset = 0.0f;
  caps.XYOffset = 0.0f;
}

/****************************************************************************/

/****************************************************************************/
/***                                                                      ***/
/***   Geometry Prim Interface                                            ***/
/***                                                                      ***/
/****************************************************************************/

void sGeometry::BeginQuadrics()
{
  sFatal(L"BeginQuadrics not implemented!");
}

void sGeometry::EndQuadrics()
{
  sFatal(L"EndQuadrics not implemented!");
}

void sGeometry::BeginQuad(void **data,int count)
{
  sFatal(L"BeginQuad not implemented!");
}

void sGeometry::EndQuad()
{
  sFatal(L"EndQuad not implemented!");
}

void sGeometry::BeginGrid(void **data,int xs,int ys)
{
  sFatal(L"BeginGrid not implemented!");
}

void sGeometry::EndGrid()
{
  sFatal(L"EndGrid not implemented!");
}

void sEnableGraphicsStats(sBool enable)
{
}

#endif // sRENDERER == sRENDER_OGL2

