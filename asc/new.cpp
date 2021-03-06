/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#include "main.hpp"

#include "shadercomp/shadercomp.hpp"

#include <string.h>
#include <stdio.h>



/****************************************************************************/

NewCode::NewCode()
{
  DX9Code = 0;
  DX9Size = 0;
  DX11Code = 0;
  DX11Size = 0;
  GLSLCode = 0;
  GLSLSize = 0;
}

NewCode::~NewCode()
{
  delete[] DX9Code;
  delete[] DX11Code;
  delete[] GLSLCode;
}

/****************************************************************************/

NewShader::NewShader()
{
  ByteCode = 0;
  PermuteCount = 0;
  Permutes = 0;
  CompileFlags = 0;
}

NewShader::~NewShader()
{
  delete ByteCode;
  for(int i=0;i<PermuteCount;i++)
    delete Permutes[i];
  delete[] Permutes;
}

/****************************************************************************/

NewMaterial::NewMaterial()
{
  sClear(Shaders);
}

NewMaterial::~NewMaterial()
{
  for(int i=0;i<sCOUNTOF(Shaders);i++)
    delete Shaders[i];
}

/****************************************************************************/
/****************************************************************************/

sBool Compiler::Process(NewCode *code,NewShader *ns,sTextBuffer *errors)
{
  // general stuff

  sBool ok = 1;
  code->ShaderType = ns->ShaderType;

  // asc -> HLSL23 (dx9)

  if(ok && sCONFIG_SDK_DX9 && !Level11)
  {
    ASC->Output(ns->ShaderType | sSTF_HLSL23,sRENDER_DX9);

    const sChar *profile = ns->Profile;
    if(sCmpStringI(profile,L"ps_4_0")==0 || sCmpStringI(profile,L"ps_5_0")==0) profile = L"ps_3_0";
    if(sCmpStringI(profile,L"vs_4_0")==0 || sCmpStringI(profile,L"vs_5_0")==0) profile = L"vs_3_0";
    ok &= sShaderCompileDX(ASC->Out.Get(),profile,L"main",code->DX9Code,code->DX9Size,ns->CompileFlags,errors);
  }

  // asc -> HLSL45

  if(ok && sCONFIG_SDK_DX11)
  {
    if(ns->AscSource.IsEmpty())
    {
      const sChar *profile = ns->Profile;
      ok &= sShaderCompileDX(ns->HlslSource.Get(),profile,L"main",code->DX11Code,code->DX11Size,ns->CompileFlags,errors);
    }
    else
    {
      ASC->Output(ns->ShaderType | sSTF_HLSL45,sRENDER_DX11);
      const sChar *profile = ns->Profile;
      if(sCmpStringI(profile,L"ps_2_0")==0 || sCmpStringI(profile,L"ps_3_0")==0) profile = L"ps_4_0";
      if(sCmpStringI(profile,L"vs_2_0")==0 || sCmpStringI(profile,L"vs_3_0")==0) profile = L"vs_4_0";
      ok &= sShaderCompileDX(ASC->Out.Get(),profile,L"main",code->DX11Code,code->DX11Size,ns->CompileFlags,errors);
    }
  }

  // asc -> OGL

 /* if (ok && sRENDERER == sRENDER_OGL2)
  {*/
    /*if (ns->AscSource.IsEmpty())
    {
      const sChar *profile = ns->Profile;
      ok &= sShaderCompileDX(ns->HlslSource.Get(), profile, L"main", code->DX11Code, code->DX11Size, ns->CompileFlags, errors);
    }
    else
    {*/
    /*  ASC->Output(ns->ShaderType | sSTF_HLSL45, sRENDER_DX11);
      const sChar *profile = ns->Profile;
      if (sCmpStringI(profile, L"ps_2_0") == 0 || sCmpStringI(profile, L"ps_3_0") == 0)
        profile = L"ps_4_0";
      if (sCmpStringI(profile, L"vs_2_0") == 0 || sCmpStringI(profile, L"vs_3_0") == 0)
        profile = L"vs_4_0";
      ok &= sShaderCompileDX(ASC->Out.Get(), profile, L"main", code->DX11Code, code->DX11Size, ns->CompileFlags, errors);
    //}*/
  //}
  /*if(!Level11 && !PcOnly)
  {*/
  /*if (!ns->AscSource.IsEmpty())
  {
    ASC->Output(ns->ShaderType | sSTF_HLSL23, sRENDER_DX9);
    const sChar *profile = ns->Profile;
    EShLanguage lang = EShLangCount;
    if(sFindStringI(profile, L"ps") == 0) {
      lang = EShLangFragment;
    }
    else if (sFindStringI(profile, L"vs") == 0) {
      lang = EShLangVertex;
    } else {
      sFatal(L"GLSL: shader type is neither ps nor vs");
    }
      /*ok &= sShaderCompileDX(ns->HlslSource.Get(), profile, L"main", code->DX11Code, code->DX11Size, ns->CompileFlags, errors);*/
    /*ShHandle compiler = Hlsl2Glsl_ConstructCompiler(lang);

    //printf("%s", sLinuxFromWide(ASC->Out.Get()));
    if(!Hlsl2Glsl_Parse(compiler, sLinuxFromWide(ASC->Out.Get()), ETargetGLSL_120, NULL, ETranslateOpNone)) {
      sPrintF(L"GLSL: parse error:");
      const char *log = Hlsl2Glsl_GetInfoLog(compiler);
      sFatal(sLinuxToWide(log, strlen(log)));
    }
    if (!Hlsl2Glsl_Translate(compiler, "main", ETargetGLSL_120, ETranslateOpNone))
    {
      sPrintF(L"GLSL: translation error:");
      const char *log = Hlsl2Glsl_GetInfoLog(compiler);
      sFatal(sLinuxToWide(log, strlen(log)));
    }
    sChar8 *GLSLCode = (sChar8 *) Hlsl2Glsl_GetShader(compiler);
    code->GLSLSize = strlen((char *) GLSLCode) + 1;
    code->GLSLCode = new uint8_t[code->GLSLSize];
    sCopyString((sChar8 *) code->GLSLCode, GLSLCode, code->GLSLSize);
    Hlsl2Glsl_DestructCompiler(compiler);
  }
  else if (!ns->GlslSource.IsEmpty())
  {
    code->GLSLSize = sGetStringLen(ns->GlslSource) + 1;
    code->GLSLCode = new uint8_t[code->GLSLSize];
    sCopyString((sChar8 *)code->GLSLCode, ns->GlslSource, code->GLSLSize);
  }*/
  if(!ns->GlslSource.IsEmpty())
    {
      code->GLSLSize = sGetStringLen(ns->GlslSource)+1;
      code->GLSLCode = new uint8_t[code->GLSLSize];
      sCopyString((sChar8 *)code->GLSLCode,ns->GlslSource,code->GLSLSize);
    }
    else if(ok && sCONFIG_SDK_CG && (sPLATFORM != sPLAT_WINDOWS || sGetShellSwitch(L"-gen_cg")))
    {
#if sCONFIG_SDK_CG
      ASC->Output(ns->ShaderType | sSTF_CG,sRENDER_OGL2);
      const sChar *prof = ((ns->ShaderType & sSTF_KIND)==sSTF_PIXEL) ? L"arbfp1" : L"arbvp1";
      ok &= sShaderCompileCG(ASC->Out.Get(),prof,L"main",code->GLSLCode,code->GLSLSize,ns->CompileFlags,errors);
#endif
  }
  //}

  // done

  if(!ok)
    Scan.Errors++;
  return ok;
}

/****************************************************************************/

sBool Compiler::Process(NewShader *ns)
{
  sTextBuffer errors;

  sBool ok = 1;

  ASC->Void();
  if(!ns->AscSource.IsEmpty())
  {
    ok &= ASC->Parse(ns->AscSource);
  }
  if(ok)
  {
    if(!ns->AscSource.IsEmpty() && ASC->UsePermute)
    {
      if(!ns->GlslSource.IsEmpty() || !ns->HlslSource.IsEmpty() || !ns->CgSource.IsEmpty())
        Scan.Error(L"can not permutate glsl, hlsl or cg");

      ns->PermuteCount = 1<<ASC->UsePermute->MaxShift;
      ns->Permutes = new NewCode*[ns->PermuteCount];
      
      for(int i=0;i<ns->PermuteCount;i++)
      {
        ns->Permutes[i] = 0;
        if(ASC->IsValidPermutation(ASC->UsePermute,i))
        {
          ns->Permutes[i] = new NewCode;
          ok &= Process(ns->Permutes[i],ns,&errors);
        }
      }
    }
    else
    {
      ns->ByteCode = new NewCode;
      ok &= Process(ns->ByteCode,ns,&errors);
    }
  }

  if(!ok)
    sPrint(errors.Get());

  return ok;
}

/****************************************************************************/
/****************************************************************************/
