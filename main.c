#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>
#include <pspdebug.h>
#include <math.h>
#include <stdio.h>

PSP_MODULE_INFO("Street Mayhem PSP", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define W 480
#define H 272
#define BUF 512
#define PI 3.14159265f
#define BOTS 8
#define BUILDINGS 20

static unsigned int __attribute__((aligned(16))) list[262144];
static SceCtrlData pad;
static int state=0, ticks=0, score=0;
static float px=0, pz=0, yaw=0, velocity=0;

typedef struct { float x,z,yaw,spd; unsigned int color; int chase; } Bot;
typedef struct { float x,z,w,d,h; unsigned int color; } Building;
static Bot bots[BOTS];
static Building buildings[BUILDINGS];

static void drawCube(float x,float y,float z,float sx,float sy,float sz,unsigned int color) {
    ScePspFVector3 v[8] __attribute__((aligned(16))) = {
      {-sx,-sy,-sz},{sx,-sy,-sz},{sx,sy,-sz},{-sx,sy,-sz},
      {-sx,-sy,sz},{sx,-sy,sz},{sx,sy,sz},{-sx,sy,sz}
    };
    unsigned short idx[36] __attribute__((aligned(16))) = {
      0,1,2,0,2,3, 4,6,5,4,7,6, 0,4,5,0,5,1,
      3,2,6,3,6,7, 1,5,6,1,6,2, 0,3,7,0,7,4
    };
    ScePspFVector3 pos={x,y,z};
    sceGumPushMatrix();
    sceGumTranslate(&pos);
    sceGuColor(color);
    sceGumDrawArray(GU_TRIANGLES, GU_VERTEX_32BITF|GU_INDEX_16BIT|GU_TRANSFORM_3D,
                    36, idx, v);
    sceGumPopMatrix();
}
static void drawCar(float x,float z,float a,unsigned int c) {
    ScePspFVector3 pos={x,.55f,z};
    sceGumPushMatrix(); sceGumTranslate(&pos); sceGumRotateY(a);
    drawCube(0,0,0,.95f,.32f,1.55f,c);
    drawCube(0,.35f,-.1f,.65f,.28f,.72f,0xff34495e);
    drawCube(-.76f,-.12f,.95f,.19f,.22f,.28f,0xff171717);
    drawCube(.76f,-.12f,.95f,.19f,.22f,.28f,0xff171717);
    drawCube(-.76f,-.12f,-.95f,.19f,.22f,.28f,0xff171717);
    drawCube(.76f,-.12f,-.95f,.19f,.22f,.28f,0xff171717);
    sceGumPopMatrix();
}
static void initGame(void) {
    int i;
    for(i=0;i<BUILDINGS;i++) {
      buildings[i].x=((i%5)-2)*19.0f+8;
      buildings[i].z=((i/5)-1)*23.0f-9;
      buildings[i].w=3.8f+(i%3)*1.2f;
      buildings[i].d=4.5f+(i%4)*.8f;
      buildings[i].h=5.0f+(i%5)*2.4f;
      buildings[i].color=(i%2)?0xff596675:0xff68717b;
    }
    for(i=0;i<BOTS;i++) {
      bots[i].x=(i-4)*5.0f; bots[i].z=-25.0f-(i%3)*9.0f;
      bots[i].yaw=(i%2)?PI:0; bots[i].spd=.10f+(i%3)*.025f;
      bots[i].color=(i%3==0)?0xffd45a35:((i%3==1)?0xff39a7b8:0xffd3ad45);
      bots[i].chase=i%2;
    }
    px=0; pz=0; yaw=0; velocity=0; score=0; ticks=0;
}
static void updateGame(void) {
    int i;
    sceCtrlReadBufferPositive(&pad,1);
    if(state==0) {
      if(pad.Buttons&(PSP_CTRL_CROSS|PSP_CTRL_START)) { initGame(); state=1; }
      return;
    }
    if(pad.Buttons&PSP_CTRL_SELECT) { state=2; return; }
    if(state==2) {
      if(pad.Buttons&(PSP_CTRL_CROSS|PSP_CTRL_START)) state=1;
      if(pad.Buttons&PSP_CTRL_CIRCLE) state=0;
      return;
    }
    if(pad.Buttons&PSP_CTRL_UP) velocity+=.012f;
    if(pad.Buttons&PSP_CTRL_DOWN) velocity-=.018f;
    if(velocity>.34f) velocity=.34f;
    if(velocity<-.14f) velocity=-.14f;
    if(!(pad.Buttons&(PSP_CTRL_UP|PSP_CTRL_DOWN))) velocity*=.985f;
    if(pad.Buttons&PSP_CTRL_LEFT) yaw+=.045f;
    if(pad.Buttons&PSP_CTRL_RIGHT) yaw-=.045f;
    px+=sinf(yaw)*velocity*2.0f;
    pz+=cosf(yaw)*velocity*2.0f;
    for(i=0;i<BOTS;i++) {
      Bot *b=&bots[i];
      float dx=px-b->x, dz=pz-b->z, dist=sqrtf(dx*dx+dz*dz);
      if(b->chase && dist<42.0f) {
        float target=atan2f(dx,dz), diff=target-b->yaw;
        while(diff>PI) diff-=2*PI;
        while(diff<-PI) diff+=2*PI;
        if(diff>.025f) diff=.025f; if(diff<-.025f) diff=-.025f;
        b->yaw+=diff;
      } else if((ticks+i*29)%150==0) b->yaw+=(i&1)?.65f:-.65f;
      b->x+=sinf(b->yaw)*b->spd*1.7f;
      b->z+=cosf(b->yaw)*b->spd*1.7f;
      if(fabsf(b->x)>85 || fabsf(b->z)>85) b->yaw+=PI*.75f;
      if(dist<2.4f) { score++; b->yaw+=.2f; }
    }
    ticks++;
}
static void setupGU(void) {
    sceGuInit();
    sceGuStart(GU_DIRECT,list);
    sceGuDrawBuffer(GU_PSM_8888,(void*)0,BUF);
    sceGuDispBuffer(W,H,(void*)0x88000,BUF);
    sceGuDepthBuffer((void*)0x110000,BUF);
    sceGuOffset(2048-(W/2),2048-(H/2));
    sceGuViewport(2048,2048,W,H);
    sceGuDepthRange(65535,0);
    sceGuScissor(0,0,W,H);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuEnable(GU_DEPTH_TEST);
    sceGuDepthFunc(GU_LEQUAL);
    sceGuFrontFace(GU_CW);
    sceGuShadeModel(GU_SMOOTH);
    sceGuDisable(GU_CULL_FACE);
    sceGuFinish();
    sceGuSync(0,0);
    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);
}
static void render(void) {
    int i;
    sceGuStart(GU_DIRECT,list);
    sceGuClearColor(0xff83b5d7);
    sceGuClearDepth(0);
    sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
    sceGumMatrixMode(GU_PROJECTION); sceGumLoadIdentity();
    sceGumPerspective(55.0f,16.0f/9.0f,.5f,220.0f);
    sceGumMatrixMode(GU_VIEW); sceGumLoadIdentity();
    {
      float cx=px-sinf(yaw)*9.0f, cz=pz-cosf(yaw)*9.0f;
      ScePspFVector3 eye={cx,6.5f,cz}, at={px,0,pz}, up={0,1,0};
      sceGumLookAt(&eye,&at,&up);
    }
    sceGumMatrixMode(GU_MODEL); sceGumLoadIdentity();
    drawCube(0,-.18f,0,100,.15f,100,0xff354238);
    for(i=-5;i<=5;i++) {
      drawCube(i*12,.005f,0,.035f,.01f,100,0xffc9bd91);
      drawCube(0,.006f,i*12,100,.01f,.035f,0xffc9bd91);
    }
    for(i=0;i<BUILDINGS;i++) {
      Building *b=&buildings[i];
      drawCube(b->x,b->h*.5f,b->z,b->w,b->h*.5f,b->d,b->color);
      drawCube(b->x,b->h+.1f,b->z,b->w*.82f,.1f,b->d*.82f,0xff303b49);
    }
    for(i=0;i<BOTS;i++) drawCar(bots[i].x,bots[i].z,bots[i].yaw,bots[i].color);
    drawCar(px,pz,yaw,0xff2877d8);
    sceGuFinish();
    sceGuSync(0,0);
    sceDisplayWaitVblankStart();
    sceGuSwapBuffers();
}
int main(void) {
    pspDebugScreenInit();
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    setupGU();
    while(1) {
      updateGame();
      render();
      pspDebugScreenSetXY(0,0);
      pspDebugScreenSetTextColor(0xffffffff);
      if(state==0) {
        pspDebugScreenPrintf("STREET MAYHEM 2.0\n\n");
        pspDebugScreenPrintf("OPEN CITY DRIVING\n\n");
        pspDebugScreenPrintf("X / START  -  START GAME\n");
        pspDebugScreenPrintf("\nD-PAD: DRIVE\n");
      } else if(state==2) {
        pspDebugScreenPrintf("PAUSED\n\nX / START: RESUME\nCIRCLE: TITLE");
      } else {
        pspDebugScreenPrintf("STREET MAYHEM   SCORE: %d\n",score);
        pspDebugScreenSetXY(0,32);
        pspDebugScreenPrintf("UP accelerate  DOWN brake  LEFT/RIGHT steer  SELECT pause");
      }
    }
    sceGuTerm();
    sceKernelExitGame();
    return 0;
}
