//========================================================================
//    NeHe OpenGL Wizard : NeHeSimple.cpp
//    Wizard Created by: Vic Hollis
//========================================================================
/*
 *		This Code Was Created By Jeff Molofee 2000
 *		A HUGE Thanks To Fredric Echols For Cleaning Up
 *		And Optimizing This Code, Making It More Flexible!
 *		If You've Found This Code Useful, Please Let Me Know.
 *		Visit My Site At nehe.gamedev.net
 */
#include <3ds.h>
#include <gfx_device.h>
#include <stdio.h>
#include <fcntl.h>

#include <iostream>
#include <fstream>

#include <GL/gl.h>

#include <sys/time.h>
static int GetTickCount()
{
    struct timeval tv;
    if(gettimeofday(&tv, NULL) != 0)
        return 0;

    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

#include "../../JGE/include/JGE.h"
#include "../../JGE/include/JTypes.h"
#include "../../JGE/include/JApp.h"
#include "../../JGE/include/JFileSystem.h"
#include "../../JGE/include/JRenderer.h"

#include "../../JGE/include/JGameLauncher.h"

//#include "..\src\GameApp.h"

bool	active=true;			// Window Active Flag Set To TRUE By Default
bool	fullscreen=false;		// Windowed Mode By Default

DWORD	lastTickCount;

BOOL	g_keys[256];

//------------------------------------------------------------------------

JGE* g_engine = NULL;
JApp* g_app = NULL;
JGameLauncher* g_launcher = NULL;

//------------------------------------------------------------------------


static u32 gButtons = 0;
static u32 gOldButtons = 0;

static circlePosition mCirclePos;

u8 JGEGetAnalogX() {
    if (mCirclePos.dx < -128) mCirclePos.dx = -128;
    if (mCirclePos.dx > 127) mCirclePos.dx = 127;
    return (u8)(mCirclePos.dx + 128);
}

u8 JGEGetAnalogY() {
    s16 out = mCirclePos.dy;
    if (out < -127) out = -127;
    if (out > 127) out = 127;
    return (u8)(128 - out);
}

inline u32 TransformButtons(u32 buttons) {
    u32 out = 0;
    out |= (buttons & KEY_START) ? PSP_CTRL_START : 0;
    out |= (buttons & KEY_SELECT) ? PSP_CTRL_SELECT : 0;
    out |= (buttons & KEY_DUP) ? PSP_CTRL_UP : 0;
    out |= (buttons & KEY_DRIGHT) ? PSP_CTRL_RIGHT : 0;
    out |= (buttons & KEY_DDOWN) ? PSP_CTRL_DOWN : 0;
    out |= (buttons & KEY_DLEFT) ? PSP_CTRL_LEFT : 0;
    out |= (buttons & KEY_L) ? PSP_CTRL_LTRIGGER : 0;
    out |= (buttons & KEY_R) ? PSP_CTRL_RTRIGGER : 0;
    out |= (buttons & KEY_X) ? PSP_CTRL_TRIANGLE : 0;
    out |= (buttons & KEY_Y) ? PSP_CTRL_SQUARE : 0;
    out |= (buttons & KEY_A) ? PSP_CTRL_CROSS : 0;
    out |= (buttons & KEY_B) ? PSP_CTRL_CIRCLE : 0;
    return out;
}

void JGEControl()
{
    gOldButtons = gButtons;

    gButtons = TransformButtons(hidKeysHeld());
}


BOOL JGEGetKeyState(int key)
{
    return 0;
}


bool JGEGetButtonState(u32 button)
{
    return (gButtons&button)==button;
}


bool JGEGetButtonClick(u32 button)
{
    return (gButtons&button)==button && (gOldButtons&button)!=button;
}

static void gluPerspective( GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar )
{
    GLdouble xmin, xmax, ymin, ymax;

    ymax = zNear * tan( fovy * M_PI / 360.0 );
    ymin = -ymax;
    xmin = ymin * aspect;
    xmax = ymax * aspect;

    glFrustum( xmin, xmax, ymin, ymax, zNear, zFar );
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
    if (height==0)										// Prevent A Divide By Zero By
    {
        height=1;										// Making Height Equal One
    }

    glViewport(0,0,width,height);						// Reset The Current Viewport

    glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
    glLoadIdentity();									// Reset The Projection Matrix

    // Calculate The Aspect Ratio Of The Window
    gluPerspective(75.0f,(GLfloat)width/(GLfloat)height,0.5f,1000.0f);
    glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
    glLoadIdentity();									// Reset The Modelview Matrix

}

int InitGL(GLvoid)												// All Setup For OpenGL Goes Here
{
    glClearColor (0.0f, 0.0f, 0.0f, 0.0f);						// Black Background
    glClearDepth (1.0f);										// Depth Buffer Setup
    glDepthFunc (GL_LEQUAL);									// The Type Of Depth Testing (Less Or Equal)
    glEnable (GL_DEPTH_TEST);									// Enable Depth Testing
//    glShadeModel (GL_SMOOTH);									// Select Smooth Shading
//    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);			// Set Perspective Calculations To Most Accurate

//    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);						// Set Line Antialiasing
    //glEnable(GL_LINE_SMOOTH);									// Enable it!

    glEnable(GL_CULL_FACE);										// do not calculate inside of poly's
//    glFrontFace(GL_CCW);										// counter clock-wise polygons are out

    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_SCISSOR_TEST);									// Enable Clipping
    //glScissor(20, 20, 320, 240);

    return true;												// Initialization Went OK
}


int InitGame(GLvoid)
{
    g_engine = JGE::GetInstance();

    //JGameLauncher *launcher = new JGameLauncher();
    g_app = g_launcher->GetGameApp();
    g_app->Create();
    g_engine->SetApp(g_app);

    JRenderer::GetInstance()->Enable2D();

    lastTickCount = GetTickCount();

    //delete launcher;

    return true;
}


void DestroyGame(GLvoid)
{
    //	JParticleSystem::Destroy();
    //	JMotionSystem::Destroy();

    g_engine->SetApp(NULL);
    if (g_app)
    {
        g_app->Destroy();
        delete g_app;
        g_app = NULL;
    }

    JGE::Destroy();

    g_engine = NULL;

}


int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{

    // 	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
    // 	glLoadIdentity ();											// Reset The Modelview Matrix

    //if (g_app)
    //	g_app->Render();
    g_engine->Render();

    //	glFlush ();

    return 0;
}

void Update(int dt)
{
    JGEControl();

    g_engine->SetDelta(dt);

    //if (g_app)
    //	g_app->Update();

    g_engine->Update();

    g_engine->mClicked = false;

}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
    DestroyGame();
    void *GfxHandle = gfxMakeCurrent(0);
    gfxDestroyDevice(GfxHandle);
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/

bool CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
    void *GfxHandle = gfxCreateDevice(240, 400, 0);
    gfxMakeCurrent(GfxHandle);
    ReSizeGLScene(width, height);

    if (!InitGL())
    {
        KillGLWindow();
        return false;
    }

    if (!InitGame())
    {
        KillGLWindow();
        return false;
    }

    return true;
}
//ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));

int main(int argc, char **argv) {
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);
    hidInit();
    romfsInit();
    osSetSpeedupEnable(true);

    chdir("sdmc:/3ds/cspsp/");

    g_launcher = new JGameLauncher();

    u32 flags = g_launcher->GetInitFlags();

    if ((flags&JINIT_FLAG_ENABLE3D)!=0)
        JRenderer::Set3DFlag(true);

    // Create Our OpenGL Window
    if (!CreateGLWindow(g_launcher->GetName(),SCREEN_WIDTH,SCREEN_HEIGHT,32,fullscreen))
    {
        return 0;									// Quit If Window Was Not Created
    }

    DWORD	tickCount;
    int		dt;

    while(aptMainLoop())									// Loop That Runs While done=FALSE
    {
        hidScanInput();
        hidCircleRead(&mCirclePos);
        if (active)
        {
            if (g_engine->IsDone())
            {
                break;
            }
            else
            {
                tickCount = GetTickCount();					// Get The Tick Count
                dt = (tickCount - lastTickCount);
                lastTickCount = tickCount;
                Update(dt);									// Update frame
                
                //Mint2D::BackupKeys();
                
                DrawGLScene();					// Draw The Scene
                gfxFlush(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 240, 400, GX_TRANSFER_FMT_RGB8);
                gfxSwapBuffersGpu();
            }
        }
    }
    
    if (g_launcher)
        delete g_launcher;

    KillGLWindow();
    romfsExit();
    hidExit();
    gfxExit();
    return 0;
}

