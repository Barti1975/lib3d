///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  MD FidelityFX SUPER RESOLUTION [FSR 1] ::: SPATIAL SCALING & EXTRAS - v1.20210629
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char *shader_functions_FSR_header=""
#ifdef API3D_METAL
"#define AF1_AU1(x) as_type<float>(x)\n"
"#define AU1_AF1(x) as_type<uint>(x)\n"
"\n"
"#define AF2_AU2(x) as_type<float2>(x)\n"
"#define AU2_AF2(x) as_type<uint2>(x)\n"
"\n"
"#define AF3_AU3(x) as_type<float3>(x)\n"
"#define AU3_AF3(x) as_type<uint3>(x)\n"
"\n"
"#define AF4_AU4(x) as_type<float4>(x)\n"
"#define AU4_AF4(x) as_type<uint4>(x)\n"
"\n"
"#define AF1 float\n"
"#define AU1 uint\n"
"#define AF2 float2\n"
"#define AU2 uint2\n"
"#define AF3 float3\n"
"#define AU3 uint3\n"
"#define AF4 float4\n"
"#define AU4 uint4\n"
"\n"
"#define ASU1 int\n"
"#define ASU2 int2\n"
"#define ASU3 int3\n"
"#define ASU4 int4\n"
#else
#if defined(API3D_OPENGL20)||defined(API3D_DIRECT3D9)
#if defined(API3D_OPENGL20)
"#define AF1_AU1(x) x\n"
"#define AU1_AF1(x) x\n"
"\n"
"#define AF2_AU2(x) x\n"
"#define AU2_AF2(x) x\n"
"\n"
"#define AF3_AU3(x) x\n"
"#define AU3_AF3(x) x\n"
"\n"
"#define AF4_AU4(x) x\n"
"#define AU4_AF4(x) x\n"
"\n"
"#define AF1 float\n"
"#define AU1 float\n"
"#define AF2 vec2\n"
"#define AU2 vec2\n"
"#define AF3 vec3\n"
"#define AU3 vec3\n"
"#define AF4 vec4\n"
"#define AU4 vec4\n"
"\n"
"#define ASU1 int\n"
"#define ASU2 ivec2\n"
"#define ASU3 ivec3\n"
"#define ASU4 ivec4\n"
#else
"#define AF1_AU1(x) x\n"
"#define AU1_AF1(x) x\n"
"\n"
"#define AF2_AU2(x) x\n"
"#define AU2_AF2(x) x\n"
"\n"
"#define AF3_AU3(x) x\n"
"#define AU3_AF3(x) x\n"
"\n"
"#define AF4_AU4(x) x\n"
"#define AU4_AF4(x) x\n"
"\n"
"#define AF1 float\n"
"#define AU1 float\n"
"#define AF2 float2\n"
"#define AU2 float2\n"
"#define AF3 float3\n"
"#define AU3 float3\n"
"#define AF4 float4\n"
"#define AU4 float4\n"
"\n"
"#define ASU1 int\n"
"#define ASU2 int2\n"
"#define ASU3 int3\n"
"#define ASU4 int4\n"
#endif
#else
"#define AF1_AU1(x) asfloat(x)\n"
"#define AU1_AF1(x) asuint(x)\n"
"\n"
"#define AF2_AU2(x) asfloat(x)\n"
"#define AU2_AF2(x) asuint(x)\n"
"\n"
"#define AF3_AU3(x) asfloat(x)\n"
"#define AU3_AF3(x) asuint(x)\n"
"\n"
"#define AF4_AU4(x) asfloat(x)\n"
"#define AU4_AF4(x) asuint(x)\n"
"\n"
"#define AF1 float\n"
"#define AU1 uint\n"
"#define AF2 float2\n"
"#define AU2 uint2\n"
"#define AF3 float3\n"
"#define AU3 uint3\n"
"#define AF4 float4\n"
"#define AU4 uint4\n"
"\n"
"#define ASU1 int\n"
"#define ASU2 int2\n"
"#define ASU3 int3\n"
"#define ASU4 int4\n"
#endif
#endif

#ifdef API3D_OPENGL20
"#define AP1 bool\n"
"\n"
"#define AF1_(x) x\n"
"#define AU1_(x) x\n"
"#define AF2_(x) vec2(x,x)\n"
"#define AU2_(x) vec2(x,x)\n"
"#define AF3_(x) vec3(x,x,x)\n"
"#define AU3_(x) vec3(x,x,x)\n"
"#define AF4_(x) vec4(x,x,x,x)\n"
"#define AU4_(x) vec4(x,x,x,x)\n"
"\n"
"AF1 ARcpF1(AF1 a) {return 1.0/a;}\n"
#else
"#define AP1 uint\n"
"\n"
"#define AF1_(x) ((AF1)x)\n"
"#define AU1_(x) ((AU1)x)\n"
"#define AF2_(x) ((AF2)x)\n"
"#define AU2_(x) ((AU2)x)\n"
"#define AF3_(x) ((AF3)x)\n"
"#define AU3_(x) ((AU3)x)\n"
"#define AF4_(x) ((AF4)x)\n"
"#define AU4_(x) ((AU4)x)\n"
"\n"
"AF1 ARcpF1(AF1 a) {return 1.0f/a;}\n"
#endif
"\n"
"AF1 AMax3F1(AF1 x,AF1 y,AF1 z){return max(x,max(y,z));}\n"
"AF2 AMax3F2(AF2 x,AF2 y,AF2 z){return max(x,max(y,z));}\n"
"AF3 AMax3F3(AF3 x,AF3 y,AF3 z){return max(x,max(y,z));}\n"
"AF4 AMax3F4(AF4 x,AF4 y,AF4 z){return max(x,max(y,z));}\n"
"\n"
"AF1 AMin3F1(AF1 x,AF1 y,AF1 z){return min(x,min(y,z));}\n"
"AF2 AMin3F2(AF2 x,AF2 y,AF2 z){return min(x,min(y,z));}\n"
"AF3 AMin3F3(AF3 x,AF3 y,AF3 z){return min(x,min(y,z));}\n"
"AF4 AMin3F4(AF4 x,AF4 y,AF4 z){return min(x,min(y,z));}\n"
"\n"
#if defined(API3D_OPENGL20)||defined(API3D_DIRECT3D9)
/*
"AF1 APrxLoSqrtF1(AF1 a){return AF1_AU1((AU1_AF1(a)>>1)+532432441);}\n"
"AF1 APrxLoRcpF1(AF1 a){return AF1_AU1(2129690299-AU1_AF1(a));}\n"
"AF1 APrxMedRcpF1(AF1 a){AF1 b=AF1_AU1(2129764351-AU1_AF1(a));return b*(-b*a+2.0);}\n"
"AF1 APrxLoRsqF1(AF1 a){return AF1_AU1(1597275508-(AU1_AF1(a)>>1));}\n"
/**/
"AF1 APrxLoSqrtF1(AF1 a){return sqrt(a);}\n"
"AF1 APrxLoRcpF1(AF1 a){return (1.0/a);}\n"
"AF1 APrxMedRcpF1(AF1 a){return (1.0/a);}\n"
"AF1 APrxLoRsqF1(AF1 a){return 1.0/sqrt(a);}\n"
#else
"AF1 APrxLoSqrtF1(AF1 a){return AF1_AU1((AU1_AF1(a)>>AU1_(1))+AU1_(0x1fbc4639));}\n"
"AF1 APrxLoRcpF1(AF1 a){return AF1_AU1(AU1_(0x7ef07ebb)-AU1_AF1(a));}\n"
"AF1 APrxMedRcpF1(AF1 a){AF1 b=AF1_AU1(AU1_(0x7ef19fff)-AU1_AF1(a));return b*(-b*a+AF1_(2.0));}\n"
"AF1 APrxLoRsqF1(AF1 a){return AF1_AU1(AU1_(0x5f347d74)-(AU1_AF1(a)>>AU1_(1)));}\n"
#endif
"\n"
"AF1 AMinF1(AF1 a,AF1 b) {return a<b?a:b;}\n"
"AF1 AMaxF1(AF1 a,AF1 b) {return a>b?a:b;}\n"
#ifdef API3D_OPENGL20
"AF1 ASatF1(AF1 a) {return AMinF1(1.0,AMaxF1(0.0,a));}\n"
#else
"AF1 ASatF1(AF1 a) {return AMinF1(1.0f,AMaxF1(0.0f,a));}\n"
#endif
"\n"
#ifdef API3D_METAL
"#define INOUT\n"
"#define IN\n"
"#define OUT\n"
#else
"#define INOUT inout\n"
"#define IN in\n"
"#define OUT out\n"
#endif
"\n";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FSR EASU
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char *shader_functions_FSR_EASU=""
"AF4 FsrEasuRF(AF2 p)\n"
"{\n"
"  return FSRGatherRed(p);\n"
"}\n"
"\n"
"AF4 FsrEasuGF(AF2 p)\n"
"{\n"
"  return FSRGatherGreen(p);\n"
"}\n"
"\n"
"AF4 FsrEasuBF(AF2 p)\n"
"{\n"
"  return FSRGatherBlue(p);\n"
"}\n"
"\n"
#if defined(API3D_METAL)||defined(API3D_OPENGL20)
"AU4 con0,con1,con2,con3;\n"
#else
"static AU4 con0,con1,con2,con3;\n"
#endif
"\n"
"void FsrEasuCon(float inputViewportX,float inputViewportY,float inputSizeX,float inputSizeY,float outputSizeX,float outputSizeY)\n"
"{\n"
" con0.x=AU1_AF1(inputViewportX*ARcpF1(outputSizeX));\n"
" con0.y=AU1_AF1(inputViewportY*ARcpF1(outputSizeY));\n"
" con0.z=AU1_AF1(AF1_(0.5)*inputViewportX*ARcpF1(outputSizeX)-AF1_(0.5));\n"
" con0.w=AU1_AF1(AF1_(0.5)*inputViewportY*ARcpF1(outputSizeY)-AF1_(0.5));\n"
"\n"
" con1.x=AU1_AF1(ARcpF1(inputSizeX));\n"
" con1.y=AU1_AF1(ARcpF1(inputSizeY));\n"
" con1.z=AU1_AF1(AF1_( 1.0)*ARcpF1(inputSizeX));\n"
" con1.w=AU1_AF1(AF1_(-1.0)*ARcpF1(inputSizeY));\n"
"\n"
" con2.x=AU1_AF1(AF1_(-1.0)*ARcpF1(inputSizeX));\n"
" con2.y=AU1_AF1(AF1_( 2.0)*ARcpF1(inputSizeY));\n"
" con2.z=AU1_AF1(AF1_( 1.0)*ARcpF1(inputSizeX));\n"
" con2.w=AU1_AF1(AF1_( 2.0)*ARcpF1(inputSizeY));\n"
"\n"
" con3.x=AU1_AF1(AF1_( 0.0)*ARcpF1(inputSizeX));\n"
" con3.y=AU1_AF1(AF1_( 4.0)*ARcpF1(inputSizeY));\n"
#ifdef API3D_OPENGL20
" con3.z=con3.w=0.0;\n"
#else
#if defined(API3D_DIRECT3D9)
" con3.z=con3.w=0.0f;\n"
#else
" con3.z=con3.w=0;\n"
#endif
#endif
"}\n"
"\n"
"void FsrEasuTapF(INOUT AF3 aC,INOUT AF1 aW,AF2 off,AF2 dir,AF2 len,AF1 lob,AF1 clp,AF3 c)\n"
"{\n"
"  AF2 v;\n"
"  v.x=(off.x*( dir.x))+(off.y*dir.y);\n"
"  v.y=(off.x*(-dir.y))+(off.y*dir.x);\n"
"  v*=len;\n"
"  AF1 d2=v.x*v.x+v.y*v.y;\n"
"  d2=min(d2,clp);\n"
"  AF1 wB=AF1_(2.0/5.0)*d2+AF1_(-1.0);\n"
"  AF1 wA=lob*d2+AF1_(-1.0);\n"
"  wB*=wB;\n"
"  wA*=wA;\n"
"  wB=AF1_(25.0/16.0)*wB+AF1_(-(25.0/16.0-1.0));\n"
"  AF1 w=wB*wA;\n"
"  \n"
"  aC+=c*w;\n"
"  aW+=w;\n"
"}\n"
"\n"
" void FsrEasuSetF(INOUT AF2 dir,INOUT AF1 len, AF2 pp,AP1 biS,AP1 biT,AP1 biU,AP1 biV,AF1 lA,AF1 lB,AF1 lC,AF1 lD,AF1 lE)\n"
" {\n"
"\n"
"  AF1 w = AF1_(0.0);\n"
"  if(biS)w=(AF1_(1.0)-pp.x)*(AF1_(1.0)-pp.y);\n"
"  if(biT)w=           pp.x *(AF1_(1.0)-pp.y);\n"
"  if(biU)w=(AF1_(1.0)-pp.x)*           pp.y ;\n"
"  if(biV)w=           pp.x *           pp.y ;\n"
"\n"
"  AF1 dc=lD-lC;\n"
"  AF1 cb=lC-lB;\n"
"  AF1 lenX=max(abs(dc),abs(cb));\n"
"  lenX=APrxLoRcpF1(lenX);\n"
"  AF1 dirX=lD-lB;\n"
"  dir.x+=dirX*w;\n"
"  lenX=ASatF1(abs(dirX)*lenX);\n"
"  lenX*=lenX;\n"
"  len+=lenX*w;\n"
"  \n"
"  AF1 ec=lE-lC;\n"
"  AF1 ca=lC-lA;\n"
"  AF1 lenY=max(abs(ec),abs(ca));\n"
"  lenY=APrxLoRcpF1(lenY);\n"
"  AF1 dirY=lE-lA;\n"
"  dir.y+=dirY*w;\n"
"  lenY=ASatF1(abs(dirY)*lenY);\n"
"  lenY*=lenY;\n"
"\n"
"  len+=lenY*w;\n"
"}\n"
"\n"
"AF3 FsrEasuF(AU2 ip){\n"
"\n"
"  AF3 pix;\n"
"  AF2 pp=AF2(ip)*AF2_AU2(con0.xy)+AF2_AU2(con0.zw);\n"
"  AF2 fp=floor(pp);\n"
"  pp-=fp;\n"
"\n"
"  AF2 p0=fp*AF2_AU2(con1.xy)+AF2_AU2(con1.zw);\n"
"\n"
"  AF2 p1=p0+AF2_AU2(con2.xy);\n"
"  AF2 p2=p0+AF2_AU2(con2.zw);\n"
"  AF2 p3=p0+AF2_AU2(con3.xy);\n"
"  AF4 bczzR=FsrEasuRF(p0);\n"
"  AF4 bczzG=FsrEasuGF(p0);\n"
"  AF4 bczzB=FsrEasuBF(p0);\n"
"  AF4 ijfeR=FsrEasuRF(p1);\n"
"  AF4 ijfeG=FsrEasuGF(p1);\n"
"  AF4 ijfeB=FsrEasuBF(p1);\n"
"  AF4 klhgR=FsrEasuRF(p2);\n"
"  AF4 klhgG=FsrEasuGF(p2);\n"
"  AF4 klhgB=FsrEasuBF(p2);\n"
"  AF4 zzonR=FsrEasuRF(p3);\n"
"  AF4 zzonG=FsrEasuGF(p3);\n"
"  AF4 zzonB=FsrEasuBF(p3);\n"
"\n"
"  AF4 bczzL=bczzB*AF4_(0.5)+(bczzR*AF4_(0.5)+bczzG);\n"
"  AF4 ijfeL=ijfeB*AF4_(0.5)+(ijfeR*AF4_(0.5)+ijfeG);\n"
"  AF4 klhgL=klhgB*AF4_(0.5)+(klhgR*AF4_(0.5)+klhgG);\n"
"  AF4 zzonL=zzonB*AF4_(0.5)+(zzonR*AF4_(0.5)+zzonG);\n"
"\n"
"  AF1 bL=bczzL.x;\n"
"  AF1 cL=bczzL.y;\n"
"  AF1 iL=ijfeL.x;\n"
"  AF1 jL=ijfeL.y;\n"
"  AF1 fL=ijfeL.z;\n"
"  AF1 eL=ijfeL.w;\n"
"  AF1 kL=klhgL.x;\n"
"  AF1 lL=klhgL.y;\n"
"  AF1 hL=klhgL.z;\n"
"  AF1 gL=klhgL.w;\n"
"  AF1 oL=zzonL.z;\n"
"  AF1 nL=zzonL.w;\n"
"\n"
"  AF2 dir=AF2_(0.0);\n"
"  AF1 len=AF1_(0.0);\n"
"  FsrEasuSetF(dir,len,pp,true, false,false,false,bL,eL,fL,gL,jL);\n"
"  FsrEasuSetF(dir,len,pp,false,true ,false,false,cL,fL,gL,hL,kL);\n"
"  FsrEasuSetF(dir,len,pp,false,false,true ,false,fL,iL,jL,kL,nL);\n"
"  FsrEasuSetF(dir,len,pp,false,false,false,true ,gL,jL,kL,lL,oL);\n"
"\n"
"  AF2 dir2=dir*dir;\n"
"  AF1 dirR=dir2.x+dir2.y;\n"
"  AP1 zro=dirR<AF1_(1.0/32768.0);\n"
"  dirR=APrxLoRsqF1(dirR);\n"
"  dirR=zro?AF1_(1.0):dirR;\n"
"  dir.x=zro?AF1_(1.0):dir.x;\n"
"  dir*=AF2_(dirR);\n"
"\n"
"  len=len*AF1_(0.5);\n"
"  len*=len;\n"
"\n"
"  AF1 stretch=(dir.x*dir.x+dir.y*dir.y)*APrxLoRcpF1(max(abs(dir.x),abs(dir.y)));\n"
"\n"
"  AF2 len2=AF2(AF1_(1.0)+(stretch-AF1_(1.0))*len,AF1_(1.0)+AF1_(-0.5)*len);\n"
"\n"
"  AF1 lob=AF1_(0.5)+AF1_((1.0/4.0-0.04)-0.5)*len;\n"
"\n"
"  AF1 clp=APrxLoRcpF1(lob);\n"
"\n"
"  AF3 min4=min(AMin3F3(AF3(ijfeR.z,ijfeG.z,ijfeB.z),AF3(klhgR.w,klhgG.w,klhgB.w),AF3(ijfeR.y,ijfeG.y,ijfeB.y)),\n"
"               AF3(klhgR.x,klhgG.x,klhgB.x));\n"
"  AF3 max4=max(AMax3F3(AF3(ijfeR.z,ijfeG.z,ijfeB.z),AF3(klhgR.w,klhgG.w,klhgB.w),AF3(ijfeR.y,ijfeG.y,ijfeB.y)),\n"
"               AF3(klhgR.x,klhgG.x,klhgB.x));\n"
"\n"
"  AF3 aC=AF3_(0.0);\n"
"  AF1 aW=AF1_(0.0);\n"
"  FsrEasuTapF(aC,aW,AF2( 0.0,-1.0)-pp,dir,len2,lob,clp,AF3(bczzR.x,bczzG.x,bczzB.x));\n"
"  FsrEasuTapF(aC,aW,AF2( 1.0,-1.0)-pp,dir,len2,lob,clp,AF3(bczzR.y,bczzG.y,bczzB.y));\n"
"  FsrEasuTapF(aC,aW,AF2(-1.0, 1.0)-pp,dir,len2,lob,clp,AF3(ijfeR.x,ijfeG.x,ijfeB.x));\n"
"  FsrEasuTapF(aC,aW,AF2( 0.0, 1.0)-pp,dir,len2,lob,clp,AF3(ijfeR.y,ijfeG.y,ijfeB.y));\n"
"  FsrEasuTapF(aC,aW,AF2( 0.0, 0.0)-pp,dir,len2,lob,clp,AF3(ijfeR.z,ijfeG.z,ijfeB.z));\n"
"  FsrEasuTapF(aC,aW,AF2(-1.0, 0.0)-pp,dir,len2,lob,clp,AF3(ijfeR.w,ijfeG.w,ijfeB.w));\n"
"  FsrEasuTapF(aC,aW,AF2( 1.0, 1.0)-pp,dir,len2,lob,clp,AF3(klhgR.x,klhgG.x,klhgB.x));\n"
"  FsrEasuTapF(aC,aW,AF2( 2.0, 1.0)-pp,dir,len2,lob,clp,AF3(klhgR.y,klhgG.y,klhgB.y));\n"
"  FsrEasuTapF(aC,aW,AF2( 2.0, 0.0)-pp,dir,len2,lob,clp,AF3(klhgR.z,klhgG.z,klhgB.z));\n"
"  FsrEasuTapF(aC,aW,AF2( 1.0, 0.0)-pp,dir,len2,lob,clp,AF3(klhgR.w,klhgG.w,klhgB.w));\n"
"  FsrEasuTapF(aC,aW,AF2( 1.0, 2.0)-pp,dir,len2,lob,clp,AF3(zzonR.z,zzonG.z,zzonB.z));\n"
"  FsrEasuTapF(aC,aW,AF2( 0.0, 2.0)-pp,dir,len2,lob,clp,AF3(zzonR.w,zzonG.w,zzonB.w));\n"
"\n"
"  pix=min(max4,max(min4,aC*AF3_(ARcpF1(aW))));\n"
"  return pix;\n"
"}\n"
"\n";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FSR RCAS
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char *shader_functions_FSR_RCAS=""
"AF3 FsrRcasLoadF(ASU2 p)\n"
"{\n"
"  AF2 map;\n"
#if defined(API3D_OPENGL20)||defined(API3D_METAL)
"  map.xy = (AF2(p.xy)+AF2(0.5,0.5)) * SIZEDEST.zw;\n"
#else
"  map.xy = AF2(p.xy) * SIZEDEST.zw;\n"
#endif
"  AF4 val=FSRSample(map.xy);\n"
"  return val.rgb;\n"
"}\n"

"#define AExp2F1(a) exp2(AF1(a))\n"
#if defined(API3D_METAL)||defined(API3D_OPENGL20)
"AU4 con;\n"
#else
"static AU4 con;\n"
#endif

"void FsrRcasCon(AF1 sharpness)\n"
"{\n"
" AF1 sharp=AExp2F1(-sharpness);\n"
" con.x=AU1_AF1(sharp);\n"
#ifdef API3D_OPENGL20
" con.y=0.0;\n"
" con.z=0.0;\n"
" con.w=0.0;\n"
#else
#if defined(API3D_DIRECT3D9)
" con.y=0.0f;\n"
" con.z=0.0f;\n"
" con.w=0.0f;\n"
#else
" con.y=0;\n"
" con.z=0;\n"
" con.w=0;\n"
#endif
#endif
"}\n"
"\n"
"#define FSR_RCAS_LIMIT (0.25-(1.0/16.0))\n"
"\n"

/*
#ifdef API3D_METAL
"void FsrLfgaF(AF3 c,AF3 t,AF1 a){c+=(t*AF3_(a))*min(AF3_(1.0)-c,c);}\n"
#else
"void FsrLfgaF(INOUT AF3 c,AF3 t,AF1 a){c+=(t*AF3_(a))*min(AF3_(1.0)-c,c);}\n"
#endif

#ifdef API3D_OPENGL20
"float prng(vec2 uv, float time) {\n"
"  return fract(sin(dot(uv + fract(time), vec2(12.9898, 78.233))) * 43758.5453);\n"
"}\n"
#else
"float prng(float2 uv, float time) {\n"
"  return fract(sin(dot(uv + fract(time), float2(12.9898f, 78.233f))) * 43758.5453f);\n"
"}\n"
#endif

#ifdef API3D_OPENGL20
"float pdf(float noise, float shape) {\n"
"  float orig = noise * 2.0 - 1.0;\n"
"  noise = pow(abs(orig), shape);\n"
"  noise *= sign(orig);\n"
"  noise -= sign(orig);\n"
"  return noise * 0.5;\n"
"}\n"
#else
"float pdf(float noise, float shape) {\n"
"  float orig = noise * 2.0f - 1.0f;\n"
"  noise = pow(abs(orig), shape);\n"
"  noise *= sign(orig);\n"
"  noise -= sign(orig);\n"
"  return noise * 0.5f;\n"
"}\n"
#endif
/**/

//"AF3 FsrRcasF(AU2 ip,AF3 filmgrain)\n"
"AF3 FsrRcasF(AU2 ip)\n"
"{\n"
"  AF3 pix = AF3(0,0,0);"
"  ASU2 sp=ASU2(ip);\n"
"  AF3 b=FsrRcasLoadF(sp+ASU2( 0,-1));\n"
"  AF3 d=FsrRcasLoadF(sp+ASU2(-1, 0));\n"
"  AF3 e=FsrRcasLoadF(sp);\n"
"  AF3 f=FsrRcasLoadF(sp+ASU2( 1, 0));\n"
"  AF3 h=FsrRcasLoadF(sp+ASU2( 0, 1));\n"

"  AF1 bR=b.r;\n"
"  AF1 bG=b.g;\n"
"  AF1 bB=b.b;\n"
"  AF1 dR=d.r;\n"
"  AF1 dG=d.g;\n"
"  AF1 dB=d.b;\n"
"  AF1 eR=e.r;\n"
"  AF1 eG=e.g;\n"
"  AF1 eB=e.b;\n"
"  AF1 fR=f.r;\n"
"  AF1 fG=f.g;\n"
"  AF1 fB=f.b;\n"
"  AF1 hR=h.r;\n"
"  AF1 hG=h.g;\n"
"  AF1 hB=h.b;\n"
"  \n"
"  AF1 bL=bB*AF1_(0.5)+(bR*AF1_(0.5)+bG);\n"
"  AF1 dL=dB*AF1_(0.5)+(dR*AF1_(0.5)+dG);\n"
"  AF1 eL=eB*AF1_(0.5)+(eR*AF1_(0.5)+eG);\n"
"  AF1 fL=fB*AF1_(0.5)+(fR*AF1_(0.5)+fG);\n"
"  AF1 hL=hB*AF1_(0.5)+(hR*AF1_(0.5)+hG);\n"
"  \n"
"  AF1 nz=AF1_(0.25)*bL+AF1_(0.25)*dL+AF1_(0.25)*fL+AF1_(0.25)*hL-eL;\n"
"  nz=ASatF1(abs(nz)*APrxMedRcpF1(AMax3F1(AMax3F1(bL,dL,eL),fL,hL)-AMin3F1(AMin3F1(bL,dL,eL),fL,hL)));\n"
"  nz=AF1_(-0.5)*nz+AF1_(1.0);\n"
"  \n"
"  AF1 mn4R=min(AMin3F1(bR,dR,fR),hR);\n"
"  AF1 mn4G=min(AMin3F1(bG,dG,fG),hG);\n"
"  AF1 mn4B=min(AMin3F1(bB,dB,fB),hB);\n"
"  AF1 mx4R=max(AMax3F1(bR,dR,fR),hR);\n"
"  AF1 mx4G=max(AMax3F1(bG,dG,fG),hG);\n"
"  AF1 mx4B=max(AMax3F1(bB,dB,fB),hB);\n"
"  \n"
"  AF2 peakC=AF2(1.0,-1.0*4.0);\n"
"  \n"
"  AF1 hitMinR=min(mn4R,eR)*ARcpF1(AF1_(4.0)*mx4R);\n"
"  AF1 hitMinG=min(mn4G,eG)*ARcpF1(AF1_(4.0)*mx4G);\n"
"  AF1 hitMinB=min(mn4B,eB)*ARcpF1(AF1_(4.0)*mx4B);\n"
"  AF1 hitMaxR=(peakC.x-max(mx4R,eR))*ARcpF1(AF1_(4.0)*mn4R+peakC.y);\n"
"  AF1 hitMaxG=(peakC.x-max(mx4G,eG))*ARcpF1(AF1_(4.0)*mn4G+peakC.y);\n"
"  AF1 hitMaxB=(peakC.x-max(mx4B,eB))*ARcpF1(AF1_(4.0)*mn4B+peakC.y);\n"
"  AF1 lobeR=max(-hitMinR,hitMaxR);\n"
"  AF1 lobeG=max(-hitMinG,hitMaxG);\n"
"  AF1 lobeB=max(-hitMinB,hitMaxB);\n"
"  AF1 lobe=max(AF1_(-FSR_RCAS_LIMIT),min(AMax3F1(lobeR,lobeG,lobeB),AF1_(0.0)))*AF1_AU1(con.x);\n"
//"  lobe*=nz;\n"
"  \n"
"  AF1 rcpL=APrxMedRcpF1(AF1_(4.0)*lobe+AF1_(1.0));\n"
"  pix.r=(lobe*bR+lobe*dR+lobe*hR+lobe*fR+eR)*rcpL;\n"
"  pix.g=(lobe*bG+lobe*dG+lobe*hG+lobe*fG+eG)*rcpL;\n"
"  pix.b=(lobe*bB+lobe*dB+lobe*hB+lobe*fB+eB)*rcpL;\n"

/*
#ifdef API3D_OPENGL20
"  float noise = pdf(prng(0.1*AF2_AU2(ip.xy), filmgrain.x * 0.11), filmgrain.y);\n"
"  FsrLfgaF(pix, AF3(noise), filmgrain.z);\n"
#else
"  float noise = pdf(prng(0.1f*AF2_AU2(ip.xy), filmgrain.x * 0.11f), filmgrain.y);\n"
"  FsrLfgaF(pix, AF3(noise), filmgrain.z);\n"
#endif
/**/
"  return pix;\n"
"}\n"
"\n";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FSR RCAS Sample 0 & 1
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *shader_functions_FSR_header_01=""
"#define AF1_AU1(x) x\n"
"#define AU1_AF1(x) x\n"
"#define AF2_AU2(x) x\n"
"#define AU2_AF2(x) x\n"
"#define AF3_AU3(x) x\n"
"#define AU3_AF3(x) x\n"
"#define AF4_AU4(x) x\n"
"#define AU4_AF4(x) x\n"

#if defined(API3D_OPENGL20)
"#define AF1 float\n"
"#define AU1 float\n"
"#define AF2 vec2\n"
"#define AU2 vec2\n"
"#define AF3 vec3\n"
"#define AU3 vec3\n"
"#define AF4 vec4\n"
"#define AU4 vec4\n"

"#define ASU1 float\n"
"#define ASU2 vec2\n"
"#define ASU3 vec3\n"
"#define ASU4 vec4\n"
#else
"#define AF1 float\n"
"#define AU1 float\n"
"#define AF2 float2\n"
"#define AU2 float2\n"
"#define AF3 float3\n"
"#define AU3 float3\n"
"#define AF4 float4\n"
"#define AU4 float4\n"

"#define ASU1 float\n"
"#define ASU2 float2\n"
"#define ASU3 float3\n"
"#define ASU4 float4\n"
#endif
"#define AP1 uint\n"

"#define AF1_(x) ((AF1)x)\n"
"#define AU1_(x) ((AU1)x)\n"
"#define AF2_(x) ((AF2)x)\n"
"#define AU2_(x) ((AU2)x)\n"
"#define AF3_(x) ((AF3)x)\n"
"#define AU3_(x) ((AU3)x)\n"
"#define AF4_(x) ((AF4)x)\n"
"#define AU4_(x) ((AU4)x)\n"

#if defined(API3D_OPENGL20)
"AF1 ARcpF1(AF1 a) {return 1.0/a;}\n"
#else
"AF1 ARcpF1(AF1 a) {return 1.0f/a;}\n"
#endif

"AF1 AMax3F1(AF1 x,AF1 y,AF1 z){return max(x,max(y,z));}\n"
"AF2 AMax3F2(AF2 x,AF2 y,AF2 z){return max(x,max(y,z));}\n"
"AF3 AMax3F3(AF3 x,AF3 y,AF3 z){return max(x,max(y,z));}\n"
"AF4 AMax3F4(AF4 x,AF4 y,AF4 z){return max(x,max(y,z));}\n"

"AF1 AMin3F1(AF1 x,AF1 y,AF1 z){return min(x,min(y,z));}\n"
"AF2 AMin3F2(AF2 x,AF2 y,AF2 z){return min(x,min(y,z));}\n"
"AF3 AMin3F3(AF3 x,AF3 y,AF3 z){return min(x,min(y,z));}\n"
"AF4 AMin3F4(AF4 x,AF4 y,AF4 z){return min(x,min(y,z));}\n"

"AF1 APrxLoSqrtF1(AF1 a){return sqrt(a);}\n"

"AF1 AMinF1(AF1 a,AF1 b) {return a<b?a:b;}\n"
"AF1 AMaxF1(AF1 a,AF1 b) {return a>b?a:b;}\n"
#ifdef API3D_OPENGL20
"AF1 APrxLoRcpF1(AF1 a){return (1.0/a);}\n"
"AF1 APrxMedRcpF1(AF1 a){return (1.0/a);}\n"
"AF1 APrxLoRsqF1(AF1 a){return 1.0/sqrt(a);}\n"
"AF1 ASatF1(AF1 a) {return AMinF1(1.0,AMaxF1(0.0,a));}\n"
#else
"AF1 APrxLoRcpF1(AF1 a){return (1.0f/a);}\n"
"AF1 APrxMedRcpF1(AF1 a){return (1.0f/a);}\n"
"AF1 APrxLoRsqF1(AF1 a){return 1.0f/sqrt(a);}\n"
"AF1 ASatF1(AF1 a) {return AMinF1(1.0f,AMaxF1(0.0f,a));}\n"
#endif
#ifdef API3D_METAL
"#define INOUT\n"
"#define IN\n"
"#define OUT\n"
#else
"#define INOUT inout\n"
"#define IN in\n"
"#define OUT out\n"
#endif
"\n";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FSR RCAS Sample 0
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *shader_functions_FSR_RCAS_0=""
"AF4 FsrRcasLoadF0(ASU2 p)\n"
"{\n"
"  AF2 map;\n"
#if defined(API3D_OPENGL20)||defined(API3D_METAL)
"  map.xy = (AF2(p.xy)+AF2(0.5,0.5)) * SIZEDEST0.zw;\n"
#else
"  map.xy = AF2(p.xy) * SIZEDEST0.zw;\n"
#endif
"  AF4 val=FSRSample0(map.xy);\n"
"  return val.rgba;\n"
"}\n"

"#define AExp2F1(a) exp2(AF1(a))\n"
"#define FSR_RCAS_LIMIT (0.25-(1.0/16.0))\n"
"\n"

"AF4 FsrRcasF0(AU2 ip,AF1 sharpness)\n"
"{\n"
"  AU4 con;\n"
"  AF1 sharp=AExp2F1(-sharpness);\n"
"  con.x=AU1_AF1(sharp);\n"
"  AF4 pix = AF4(0,0,0,0);"
"  ASU2 sp=ASU2(ip);\n"
"  AF4 b=FsrRcasLoadF0(sp+ASU2( 0,-1));\n"
"  AF4 d=FsrRcasLoadF0(sp+ASU2(-1, 0));\n"
"  AF4 e=FsrRcasLoadF0(sp);\n"
"  AF4 f=FsrRcasLoadF0(sp+ASU2( 1, 0));\n"
"  AF4 h=FsrRcasLoadF0(sp+ASU2( 0, 1));\n"

"  AF1 bR=b.r;\n"
"  AF1 bG=b.g;\n"
"  AF1 bB=b.b;\n"
"  AF1 bA=b.a;\n"
"  AF1 dR=d.r;\n"
"  AF1 dG=d.g;\n"
"  AF1 dB=d.b;\n"
"  AF1 dA=d.a;\n"
"  AF1 eR=e.r;\n"
"  AF1 eG=e.g;\n"
"  AF1 eB=e.b;\n"
"  AF1 eA=e.a;\n"
"  AF1 fR=f.r;\n"
"  AF1 fG=f.g;\n"
"  AF1 fB=f.b;\n"
"  AF1 fA=f.a;\n"
"  AF1 hR=h.r;\n"
"  AF1 hG=h.g;\n"
"  AF1 hB=h.b;\n"
"  AF1 hA=h.a;\n"

#ifdef API3D_OPENGL20
"  AF1 bL=bB*0.5 + bR*0.5 + bG;\n"
"  AF1 dL=dB*0.5 + dR*0.5 + dG;\n"
"  AF1 eL=eB*0.5 + eR*0.5 + eG;\n"
"  AF1 fL=fB*0.5 + fR*0.5 + fG;\n"
"  AF1 hL=hB*0.5 + hR*0.5 + hG;\n"

"  AF1 nz=0.25*bL + 0.25*dL + 0.25*fL + 0.25*hL - eL;\n"
"  nz=ASatF1(abs(nz)*APrxMedRcpF1(AMax3F1(AMax3F1(bL,dL,eL),fL,hL)-AMin3F1(AMin3F1(bL,dL,eL),fL,hL)));\n"
"  nz=-0.5*nz + 1.0;\n"
#else
"  AF1 bL=bB*0.5f + bR*0.5f + bG;\n"
"  AF1 dL=dB*0.5f + dR*0.5f + dG;\n"
"  AF1 eL=eB*0.5f + eR*0.5f + eG;\n"
"  AF1 fL=fB*0.5f + fR*0.5f + fG;\n"
"  AF1 hL=hB*0.5f + hR*0.5f + hG;\n"

"  AF1 nz=0.25f*bL + 0.25f*dL + 0.25f*fL + 0.25f*hL - eL;\n"
"  nz=ASatF1(abs(nz)*APrxMedRcpF1(AMax3F1(AMax3F1(bL,dL,eL),fL,hL)-AMin3F1(AMin3F1(bL,dL,eL),fL,hL)));\n"
"  nz=-0.5f*nz + 1.0f;\n"
#endif

"  AF1 mn4R=min(AMin3F1(bR,dR,fR),hR);\n"
"  AF1 mn4G=min(AMin3F1(bG,dG,fG),hG);\n"
"  AF1 mn4B=min(AMin3F1(bB,dB,fB),hB);\n"
"  AF1 mn4A=min(AMin3F1(bA,dA,fA),hA);\n"
"  AF1 mx4R=max(AMax3F1(bR,dR,fR),hR);\n"
"  AF1 mx4G=max(AMax3F1(bG,dG,fG),hG);\n"
"  AF1 mx4B=max(AMax3F1(bB,dB,fB),hB);\n"
"  AF1 mx4A=max(AMax3F1(bA,dA,fA),hA);\n"

#ifdef API3D_OPENGL20
"  AF2 peakC=AF2(1.0,-4.0);\n"

"  AF1 hitMinR=min(mn4R,eR)*ARcpF1(4.0*mx4R);\n"
"  AF1 hitMinG=min(mn4G,eG)*ARcpF1(4.0*mx4G);\n"
"  AF1 hitMinB=min(mn4B,eB)*ARcpF1(4.0*mx4B);\n"
"  AF1 hitMinA=min(mn4A,eA)*ARcpF1(4.0*mx4A);\n"
"  AF1 hitMaxR=(peakC.x-max(mx4R,eR))*ARcpF1(4.0*mn4R + peakC.y);\n"
"  AF1 hitMaxG=(peakC.x-max(mx4G,eG))*ARcpF1(4.0*mn4G + peakC.y);\n"
"  AF1 hitMaxB=(peakC.x-max(mx4B,eB))*ARcpF1(4.0*mn4B + peakC.y);\n"
"  AF1 hitMaxA=(peakC.x-max(mx4A,eA))*ARcpF1(4.0*mn4A + peakC.y);\n"
#else
"  AF2 peakC=AF2(1.0f,-4.0f);\n"

"  AF1 hitMinR=min(mn4R,eR)*ARcpF1(4.0f*mx4R);\n"
"  AF1 hitMinG=min(mn4G,eG)*ARcpF1(4.0f*mx4G);\n"
"  AF1 hitMinB=min(mn4B,eB)*ARcpF1(4.0f*mx4B);\n"
"  AF1 hitMinA=min(mn4A,eA)*ARcpF1(4.0f*mx4A);\n"
"  AF1 hitMaxR=(peakC.x-max(mx4R,eR))*ARcpF1(4.0f*mn4R + peakC.y);\n"
"  AF1 hitMaxG=(peakC.x-max(mx4G,eG))*ARcpF1(4.0f*mn4G + peakC.y);\n"
"  AF1 hitMaxB=(peakC.x-max(mx4B,eB))*ARcpF1(4.0f*mn4B + peakC.y);\n"
"  AF1 hitMaxA=(peakC.x-max(mx4A,eA))*ARcpF1(4.0f*mn4A + peakC.y);\n"
#endif

"  AF1 lobeR=max(-hitMinR,hitMaxR);\n"
"  AF1 lobeG=max(-hitMinG,hitMaxG);\n"
"  AF1 lobeB=max(-hitMinB,hitMaxB);\n"
"  AF1 lobeA=max(-hitMinA,hitMaxA);\n"
#ifdef API3D_OPENGL20
"  AF1 lobe=max(-FSR_RCAS_LIMIT,min(AMax3F1(lobeR,lobeG,lobeB),0.0))*con.x;\n"
"  AF1 rcpL=APrxMedRcpF1(4.0*lobe+1.0);\n"
#else
"  AF1 lobe=max(-FSR_RCAS_LIMIT,min(AMax3F1(lobeR,lobeG,lobeB),0.0f))*con.x;\n"
"  AF1 rcpL=APrxMedRcpF1(4.0f*lobe+1.0f);\n"
#endif
"  pix.r=(lobe*bR+lobe*dR+lobe*hR+lobe*fR+eR)*rcpL;\n"
"  pix.g=(lobe*bG+lobe*dG+lobe*hG+lobe*fG+eG)*rcpL;\n"
"  pix.b=(lobe*bB+lobe*dB+lobe*hB+lobe*fB+eB)*rcpL;\n"
"  pix.a=(lobe*bA+lobe*dA+lobe*hA+lobe*fA+eA)*rcpL;\n"

"  return pix;\n"
"}\n"
"\n";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FSR RCAS Sample 0 "samplebump"
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char * shader_functions_FSR_RCAS_0_warp_samplebump=""
"AF4 TiledSample0(AF2 m)\n"
"{\n"
"	AU2 gxy = AU2(m.xy * SIZEDEST0.xy);\n"
"	AF4 col = FsrRcasF0(AF2(gxy),%s);\n"
"	return col;\n"
"}\n";


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FSR RCAS Sample 1
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *shader_functions_FSR_RCAS_1=""
"AF4 FsrRcasLoadF1(ASU2 p)\n"
"{\n"
"  AF2 map;\n"
#if defined(API3D_OPENGL20)||defined(API3D_METAL)
"  map.xy = (AF2(p.xy)+AF2(0.5,0.5)) * SIZEDEST1.zw;\n"
#else
"  map.xy = AF2(p.xy) * SIZEDEST1.zw;\n"
#endif
"  AF4 val=FSRSample1(map.xy);\n"
"  return val.rgba;\n"
"}\n"

"#define AExp2F1(a) exp2(AF1(a))\n"
"#define FSR_RCAS_LIMIT (0.25-(1.0/16.0))\n"
"\n"

"AF4 FsrRcasF1(AU2 ip,AF1 sharpness)\n"
"{\n"
"  AU4 con;\n"
"  AF1 sharp=AExp2F1(-sharpness);\n"
"  con.x=AU1_AF1(sharp);\n"
"  AF4 pix = AF4(0,0,0,0);\n"
"  ASU2 sp=ASU2(ip);\n"
"  AF4 b=FsrRcasLoadF1(sp+ASU2( 0,-1));\n"
"  AF4 d=FsrRcasLoadF1(sp+ASU2(-1, 0));\n"
"  AF4 e=FsrRcasLoadF1(sp);\n"
"  AF4 f=FsrRcasLoadF1(sp+ASU2( 1, 0));\n"
"  AF4 h=FsrRcasLoadF1(sp+ASU2( 0, 1));\n"

"  AF1 bR=b.r;\n"
"  AF1 bG=b.g;\n"
"  AF1 bB=b.b;\n"
"  AF1 bA=b.a;\n"
"  AF1 dR=d.r;\n"
"  AF1 dG=d.g;\n"
"  AF1 dB=d.b;\n"
"  AF1 dA=d.a;\n"
"  AF1 eR=e.r;\n"
"  AF1 eG=e.g;\n"
"  AF1 eB=e.b;\n"
"  AF1 eA=e.a;\n"
"  AF1 fR=f.r;\n"
"  AF1 fG=f.g;\n"
"  AF1 fB=f.b;\n"
"  AF1 fA=f.a;\n"
"  AF1 hR=h.r;\n"
"  AF1 hG=h.g;\n"
"  AF1 hB=h.b;\n"
"  AF1 hA=h.a;\n"

#ifdef API3D_OPENGL20
"  AF1 bL=bB*0.5 + bR*0.5 + bG;\n"
"  AF1 dL=dB*0.5 + dR*0.5 + dG;\n"
"  AF1 eL=eB*0.5 + eR*0.5 + eG;\n"
"  AF1 fL=fB*0.5 + fR*0.5 + fG;\n"
"  AF1 hL=hB*0.5 + hR*0.5 + hG;\n"

"  AF1 nz=0.25*bL + 0.25*dL + 0.25*fL + 0.25*hL - eL;\n"
"  nz=ASatF1(abs(nz)*APrxMedRcpF1(AMax3F1(AMax3F1(bL,dL,eL),fL,hL)-AMin3F1(AMin3F1(bL,dL,eL),fL,hL)));\n"
"  nz=-0.5*nz + 1.0;\n"
#else
"  AF1 bL=bB*0.5f + bR*0.5f + bG;\n"
"  AF1 dL=dB*0.5f + dR*0.5f + dG;\n"
"  AF1 eL=eB*0.5f + eR*0.5f + eG;\n"
"  AF1 fL=fB*0.5f + fR*0.5f + fG;\n"
"  AF1 hL=hB*0.5f + hR*0.5f + hG;\n"

"  AF1 nz=0.25f*bL + 0.25f*dL + 0.25f*fL + 0.25f*hL - eL;\n"
"  nz=ASatF1(abs(nz)*APrxMedRcpF1(AMax3F1(AMax3F1(bL,dL,eL),fL,hL)-AMin3F1(AMin3F1(bL,dL,eL),fL,hL)));\n"
"  nz=-0.5f*nz + 1.0f;\n"
#endif

"  AF1 mn4R=min(AMin3F1(bR,dR,fR),hR);\n"
"  AF1 mn4G=min(AMin3F1(bG,dG,fG),hG);\n"
"  AF1 mn4B=min(AMin3F1(bB,dB,fB),hB);\n"
"  AF1 mn4A=min(AMin3F1(bA,dA,fA),hA);\n"
"  AF1 mx4R=max(AMax3F1(bR,dR,fR),hR);\n"
"  AF1 mx4G=max(AMax3F1(bG,dG,fG),hG);\n"
"  AF1 mx4B=max(AMax3F1(bB,dB,fB),hB);\n"
"  AF1 mx4A=max(AMax3F1(bA,dA,fA),hA);\n"

#ifdef API3D_OPENGL20
"  AF2 peakC=AF2(1.0,-4.0);\n"

"  AF1 hitMinR=min(mn4R,eR)*ARcpF1(4.0*mx4R);\n"
"  AF1 hitMinG=min(mn4G,eG)*ARcpF1(4.0*mx4G);\n"
"  AF1 hitMinB=min(mn4B,eB)*ARcpF1(4.0*mx4B);\n"
"  AF1 hitMinA=min(mn4A,eA)*ARcpF1(4.0*mx4A);\n"
"  AF1 hitMaxR=(peakC.x-max(mx4R,eR))*ARcpF1(4.0*mn4R + peakC.y);\n"
"  AF1 hitMaxG=(peakC.x-max(mx4G,eG))*ARcpF1(4.0*mn4G + peakC.y);\n"
"  AF1 hitMaxB=(peakC.x-max(mx4B,eB))*ARcpF1(4.0*mn4B + peakC.y);\n"
"  AF1 hitMaxA=(peakC.x-max(mx4A,eA))*ARcpF1(4.0*mn4A + peakC.y);\n"
#else
"  AF2 peakC=AF2(1.0f,-4.0f);\n"

"  AF1 hitMinR=min(mn4R,eR)*ARcpF1(4.0f*mx4R);\n"
"  AF1 hitMinG=min(mn4G,eG)*ARcpF1(4.0f*mx4G);\n"
"  AF1 hitMinB=min(mn4B,eB)*ARcpF1(4.0f*mx4B);\n"
"  AF1 hitMinA=min(mn4A,eA)*ARcpF1(4.0f*mx4A);\n"
"  AF1 hitMaxR=(peakC.x-max(mx4R,eR))*ARcpF1(4.0f*mn4R + peakC.y);\n"
"  AF1 hitMaxG=(peakC.x-max(mx4G,eG))*ARcpF1(4.0f*mn4G + peakC.y);\n"
"  AF1 hitMaxB=(peakC.x-max(mx4B,eB))*ARcpF1(4.0f*mn4B + peakC.y);\n"
"  AF1 hitMaxA=(peakC.x-max(mx4A,eA))*ARcpF1(4.0f*mn4A + peakC.y);\n"
#endif

"  AF1 lobeR=max(-hitMinR,hitMaxR);\n"
"  AF1 lobeG=max(-hitMinG,hitMaxG);\n"
"  AF1 lobeB=max(-hitMinB,hitMaxB);\n"
"  AF1 lobeA=max(-hitMinA,hitMaxA);\n"
#ifdef API3D_OPENGL20
"  AF1 lobe=max(-FSR_RCAS_LIMIT,min(AMax3F1(lobeR,lobeG,lobeB),0.0))*con.x;\n"
"  AF1 rcpL=APrxMedRcpF1(4.0*lobe+1.0);\n"
#else
"  AF1 lobe=max(-FSR_RCAS_LIMIT,min(AMax3F1(lobeR,lobeG,lobeB),0.0f))*con.x;\n"
"  AF1 rcpL=APrxMedRcpF1(4.0f*lobe+1.0f);\n"
#endif
"  pix.r=(lobe*bR+lobe*dR+lobe*hR+lobe*fR+eR)*rcpL;\n"
"  pix.g=(lobe*bG+lobe*dG+lobe*hG+lobe*fG+eG)*rcpL;\n"
"  pix.b=(lobe*bB+lobe*dB+lobe*hB+lobe*fB+eB)*rcpL;\n"
"  pix.a=(lobe*bA+lobe*dA+lobe*hA+lobe*fA+eA)*rcpL;\n"

"  return pix;\n"
"}\n"
"\n";


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
