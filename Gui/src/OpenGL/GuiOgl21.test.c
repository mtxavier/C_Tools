/*  Fondement Michtam
 *  Copyright (C) 2011 Xavier Lacroix
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 *
 * I'm Learning Open GL so, these are tests.
 *
*/

#include <StackEnv.h>
#include <Classes.h>

#include <stdarg.h>
#include <stdio.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <Tools.h>
#include <Geo.h>

#define TheTextureUnit 0
#define ThePaletteUnit 1
#define TheCoordIdx 0
#define TheTextCoordIdx 1
#define BaseRemoteBuffer ((void *)0)

#define MainBufferSize 0x2000

struct {
	GLuint MainBuffer;
	struct MonoProgram {
		GLuint Prog;
		GLuint IdFg,InvScrDimx2;
	} Mono;
	struct LinColProgram {
		GLuint Prog;
		GLuint InvScrDimx2,InvMapDim,Colors;
	} LinCol;
	struct GreyProgram {
		GLint Prog;
		int InvScrDimx2,IdTexture,IdForeground,IdBackground;
	} Grey256,Grey16,Grey4,Bitmap;
	struct TextProgram {
		GLint Prog;
		int InvScrDimx2,IdTexture,IdPalette,IdPaletteBase,IdColorKey;
	} Pix256,Pix16,Pix4;
	Geo2dPoint Cursor;
	Geo2dRectangle Window;
} GLEnv;

typedef struct {
	GLint x,y;
} TexFlatVertex;
typedef struct {
	TexFlatVertex w,t;
} TexTexVertex;

/*--------------------*/

static void TextRectAttrib(Geo2dRectangle *img,int imgH) {
	TexTexVertex Dst[4];
	Dst[0].w.x = Dst[2].w.x = GLEnv.Cursor.x;
	Dst[1].w.x = Dst[3].w.x = Dst[0].w.x+img->Extent.w;
	Dst[0].w.y = Dst[1].w.y = GLEnv.Cursor.y;
	Dst[2].w.y = Dst[3].w.y = Dst[0].w.y+img->Extent.h;
	Dst[0].t.x = Dst[2].t.x = img->Pos.x;
    Dst[1].t.x = Dst[3].t.x = Dst[0].t.x+img->Extent.w;
	Dst[0].t.y = Dst[1].t.y = imgH-(img->Pos.y+img->Extent.h);
	Dst[2].t.y = Dst[3].t.y = Dst[0].t.y+img->Extent.h;
	glBindBuffer(GL_ARRAY_BUFFER,GLEnv.MainBuffer);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(Dst),Dst);
	glEnableVertexAttribArray(TheCoordIdx);
	glVertexAttribPointer(TheCoordIdx,2,GL_INT,(0!=0),
		TlsAtomSize(TexTexVertex),BaseRemoteBuffer+TlsFieldOffset(TexTexVertex,w));
	glEnableVertexAttribArray(TheTextCoordIdx);
	glVertexAttribPointer(TheTextCoordIdx,2,GL_INT,(0!=0),
		TlsAtomSize(TexTexVertex),BaseRemoteBuffer+TlsFieldOffset(TexTexVertex,t));
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	glDisableVertexAttribArray(TheCoordIdx);
	glDisableVertexAttribArray(TheTextCoordIdx);
}
static void TextRectAttrib1(Geo2dRectangle *img,int imgH) {
	TexFlatVertex Dst[8];
	int texoff;
	Dst[0].x = Dst[2].x = GLEnv.Cursor.x;
	Dst[1].x = Dst[3].x = Dst[0].x+img->Extent.w;
	Dst[0].y = Dst[1].y = GLEnv.Cursor.y;
	Dst[2].y = Dst[3].y = Dst[0].y+img->Extent.h;
	Dst[4+0].x = Dst[4+2].x = img->Pos.x;
    Dst[4+1].x = Dst[4+3].x = Dst[4+0].x+img->Extent.w;
	Dst[4+0].y = Dst[4+1].y = imgH-(img->Pos.y+img->Extent.h);
	Dst[4+2].y = Dst[4+3].y = Dst[4+0].y+img->Extent.h;
	glBindBuffer(GL_ARRAY_BUFFER,GLEnv.MainBuffer);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(Dst),Dst);
	glEnableVertexAttribArray(TheCoordIdx);
	glVertexAttribPointer(TheCoordIdx,2,GL_INT,(0!=0),0,BaseRemoteBuffer+0);
	glEnableVertexAttribArray(TheTextCoordIdx);
	texoff=(((void*)(Dst+4))-((void*)Dst+0));
	glVertexAttribPointer(TheTextCoordIdx,2,GL_INT,(0!=0),0,BaseRemoteBuffer+texoff);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	glDisableVertexAttribArray(TheCoordIdx);
	glDisableVertexAttribArray(TheTextCoordIdx);
}

static void DrawLinColRectangle(Geo2dPoint *Pos,Geo2dRectangle *Img) {
	GLEnv.Cursor = *Pos;
	glUseProgram(GLEnv.LinCol.Prog);
    TextRectAttrib(Img,512);
}

/*--------------------------------*/

static void setFlatShader(int *Fg){
	float fg[4];
	int i;
    GLenum err;
	for (i=0;i<4;i++) { float c; c=Fg[i]; fg[i] = c/255.; }
    glUseProgram(GLEnv.Mono.Prog);
	err = glGetError();
	if (err!=GL_NO_ERROR) {
		printf("OGL Error: couldn't attach program %d\n",GLEnv.Mono.Prog);
	}
	glUniform4f(GLEnv.Mono.IdFg,fg[0],fg[1],fg[2],fg[3]);
	err = glGetError();
	if (err!=GL_NO_ERROR) {
		printf("OGL Error: couldn't set foreground color in program %d\n",GLEnv.Mono.Prog);
	}
}
static void setDefaultFlatShader(int *Fg) {
	float fg[4];
	int i;
	for (i=0;i<4;i++) { float c; c=Fg[i]; fg[i] = c/255.; }
	glUseProgram(0);
	glColor4f(fg[0],fg[1],fg[2],fg[3]);

}
static void FlatRectAttrib0(Geo2dRectangle *img,int *Fg) {
	TexFlatVertex Dst[4];

	setDefaultFlatShader(Fg);
	Dst[0].x = Dst[2].x = img->Pos.x;
	Dst[1].x = Dst[3].x = Dst[0].x+img->Extent.w;
	Dst[0].y = Dst[1].y = GLEnv.Window.Extent.h-(img->Pos.y+img->Extent.h);
	Dst[2].y = Dst[3].y = Dst[0].y+img->Extent.h;

    glBegin(GL_POLYGON);
	{
		glVertex2i(Dst[0].x,Dst[0].y);
		glVertex2i(Dst[1].x,Dst[1].y);
		glVertex2i(Dst[3].x,Dst[3].y);
		glVertex2i(Dst[2].x,Dst[2].y);
	}
	glEnd();
}
static void FlatRectAttrib1(Geo2dRectangle *img,int *Fg) {
	TexFlatVertex Dst[4];
    setFlatShader(Fg);
	Dst[0].x = Dst[2].x = img->Pos.x;
	Dst[1].x = Dst[3].x = Dst[0].x+img->Extent.w;
	Dst[0].y = Dst[1].y = img->Pos.y;
	Dst[2].y = Dst[3].y = Dst[0].y+img->Extent.h;
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glEnableVertexAttribArray(TheCoordIdx);
	glVertexAttribPointer(TheCoordIdx,2,GL_INT,GL_FALSE,0,Dst);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	glDisableVertexAttribArray(TheCoordIdx);
}
static void FlatRectAttrib2(Geo2dRectangle *img,int *Fg) {
	TexFlatVertex Dst[4];
    setFlatShader(Fg);
	Dst[0].x = Dst[2].x = img->Pos.x;
	Dst[1].x = Dst[3].x = Dst[0].x+img->Extent.w;
	Dst[0].y = Dst[1].y = img->Pos.y;
	Dst[2].y = Dst[3].y = Dst[0].y+img->Extent.h;
	glBindBuffer(GL_ARRAY_BUFFER,GLEnv.MainBuffer);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(Dst),Dst);
	glEnableVertexAttribArray(TheCoordIdx);
	glVertexAttribPointer(TheCoordIdx,2,GL_INT,GL_FALSE,0,BaseRemoteBuffer);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	glDisableVertexAttribArray(TheCoordIdx);
}
static void FlatRectAttrib3(Geo2dRectangle *img,int *Fg) {
	TexFlatVertex Dst[4];

    setDefaultFlatShader(Fg);

	Dst[0].x = Dst[2].x = img->Pos.x;
	Dst[1].x = Dst[3].x = Dst[0].x+img->Extent.w;
	Dst[0].y = Dst[1].y = GLEnv.Window.Extent.h-(img->Pos.y+img->Extent.h);
	Dst[2].y = Dst[3].y = Dst[0].y+img->Extent.h;
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2,GL_INT,0,Dst);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	glDisableClientState(GL_VERTEX_ARRAY);
}
static void FlatRectAttrib4(Geo2dRectangle *img,int *Fg) {
	TexFlatVertex Dst[4];

    setDefaultFlatShader(Fg);

	Dst[0].x = Dst[2].x = img->Pos.x;
	Dst[1].x = Dst[3].x = Dst[0].x+img->Extent.w;
	Dst[0].y = Dst[1].y = GLEnv.Window.Extent.h-(img->Pos.y+img->Extent.h);
	Dst[2].y = Dst[3].y = Dst[0].y+img->Extent.h;
	glBindBuffer(GL_ARRAY_BUFFER,GLEnv.MainBuffer);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(Dst),Dst);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2,GL_INT,0,BaseRemoteBuffer);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER,0);
}
static void DrawMonoRectangle(Geo2dRectangle *rect,int *Color) {
	FlatRectAttrib2(rect,Color);
}

/*__________________________________________
 |
 |
 |__________________________________________
*/


static GLchar *FileRead(char *name) {
	GLchar *r;
	FILE *f;
	int l;
	r = "";
	f = fopen(name,"r");
	if(f==NULL) {
		printf("Error: Couldn't open file %s\n",name);
	} else {
	    fseek(f,0,SEEK_END);
        l = ftell(f);
	    rnPush(r,TlsRoundUp(int,l+1));
	    fseek(f,0,SEEK_SET);
	    fread(r,1,l,f);
	    r[l]=0;
	    fclose(f);
	}
	return r;
}

typedef struct {
	char *Name;
	GLuint Desc;
} ShaderDesc;

static void newShader(ShaderDesc *r,GLuint stage,char *codefile) {
	GLchar *code[1];
	r->Name = codefile;
	rOpen {
		GLint resul;
		code[0] = FileRead(codefile);
	    r->Desc = glCreateShader(stage);
	    glShaderSource(r->Desc,1,(const GLchar **)code,NULL);
	    glCompileShader(r->Desc);
		glGetShaderiv(r->Desc,GL_COMPILE_STATUS,&resul);
		if (resul==GL_FALSE) {
			printf("Compilation error(s) in shader %s\n",codefile);
			glGetShaderiv(r->Desc,GL_INFO_LOG_LENGTH,&resul);
			rOpen {
			    GLchar *log;
			    GLsizei lng;
				rnPush(log,resul+1);
			    glGetShaderInfoLog(r->Desc,resul,&lng,log);
				log[lng]=0;
				printf(log); printf("\n");
			} rClose
		}
	} rClose
}

static GLuint newProgram(ShaderDesc *Vertex,ShaderDesc *Fragment,char *In,...) {
	GLuint r;
	GLint err;
	char *in;
	va_list l;
	r = glCreateProgram();
	in = In;
	va_start(l,In);
	while (in!=NULL) {
		int idx;
        idx = va_arg(l,int);
		glBindAttribLocation(r,idx,in);
		in = va_arg(l,char *);
	}
	va_end(l);
	glAttachShader(r,Vertex->Desc);
    glAttachShader(r,Fragment->Desc);
	glLinkProgram(r);
	glGetProgramiv(r,GL_LINK_STATUS,&err);
	if (err==GL_TRUE) {
	    va_start(l,In);
	    while (in!=NULL) {
		    int idx,id0;
            idx = va_arg(l,int);
		    id0 = glGetAttribLocation(r,in);
			if (id0!=idx) {
		        printf("Link Error %s <-> %s: couldn't get %d attribute (got %d)\n",Vertex->Name,Fragment->Name,idx,id0);
			}
		    in = va_arg(l,char *);
	    }
	    va_end(l);
	} else {
		printf("Link Error %s <-> %s\n",Vertex->Name,Fragment->Name);
		glGetProgramiv(r,GL_INFO_LOG_LENGTH,&err);
		rOpen  {
			GLchar *log;
			GLsizei lng;
			rnPush(log,err+1);
			glGetProgramInfoLog(r,err,&lng,log);
			log[lng]=0;
			printf(log); printf("\n");
		} rClose
	}
	glDetachShader(r,Fragment->Desc);
	glDetachShader(r,Vertex->Desc);
	return r;
}
/*
static void newTextProgram(struct TextProgram *r,ShaderDesc *Vertex,char *FragFile) {
	GLuint p;
	ShaderDesc Fragment;
	newShader(&Fragment,GL_FRAGMENT_SHADER,FragFile);
	r->Prog.Engine = eng;
	r->Prog.Prog = p = newProgram(Vertex,&Fragment,"Coord",(int)TheCoordIdx,"TextCoord",(int)TheTextCoordIdx,NULL);
	glDeleteShader(Fragment.Desc);
	glUseProgram(r->Prog.Prog);
	r->IdTexture = glGetUniformLocation(p,"CurrentPTexture");
	r->IdPalette = glGetUniformLocation(p,"CurrentPalette");
	r->IdPaletteBase = glGetUniformLocation(p,"CurrentPaletteBase");
	r->IdColorKey = glGetUniformLocation(p,"ColorKey");
	r->Palette = -1;
	r->Texture = -1;
	glUniform1i(r->IdColorKey,-1);
	r->ColorKey = -1;
	glUniform1i(r->IdPaletteBase,0);
	r->PaletteBase = 0;
} */
static void newLinColProgram(struct LinColProgram *r,ShaderDesc *Vertex,char *FragFile) {
	GLuint p;
	ShaderDesc Fragment;
	newShader(&Fragment,GL_FRAGMENT_SHADER,FragFile);
	r->Prog = p = newProgram(Vertex,&Fragment,"Coord",(int)TheCoordIdx,"TextCoord",(int)TheTextCoordIdx,NULL);
	glDeleteShader(Fragment.Desc);
	glUseProgram(p);
	r->InvScrDimx2 = glGetUniformLocation(p,"InvScrDimx2");
	glUniform2f(r->InvScrDimx2,2./GLEnv.Window.Extent.w,2./GLEnv.Window.Extent.h);
	r->InvMapDim = glGetUniformLocation(p,"InvMapDim");
	glUniform2f(r->InvMapDim,1./512.,1./512.);
	r->Colors = glGetUniformLocation(p,"Colors");
	{
		GLfloat colors[16] = {
		   1.,1.,0.,1.,
		   0.,1.,1.,1.,
		   1.,0.,1.,1.,
		   1.,1.,1.,1.
		};
	    glUniformMatrix4fv(r->Colors,1,GL_FALSE,colors);
	}
}
static void newMonoProgram(struct MonoProgram *r,ShaderDesc *Vertex,char *FlatFile) {
	GLuint p;
	ShaderDesc Fragment;
	newShader(&Fragment,GL_FRAGMENT_SHADER,FlatFile);
	r->Prog = p = newProgram(Vertex,&Fragment,"Coord",(int)TheCoordIdx,NULL);
	glDeleteShader(Fragment.Desc);
	glUseProgram(p);
	r->IdFg = glGetUniformLocation(p,"Foreground");
	glUniform4f(r->IdFg,1.,1.,1.,1.);
	r->InvScrDimx2 = glGetUniformLocation(p,"InvScrDimx2");
	glUniform2f(r->InvScrDimx2,2./GLEnv.Window.Extent.w,2./GLEnv.Window.Extent.h);
}
/* static void newPixProgram(OGLEngine *eng,struct PixProgram *r,ShaderDesc *Vertex,char *FragFile) {
	GLuint p;
	ShaderDesc Fragment;
	newShader(&Fragment,GL_FRAGMENT_SHADER,FragFile);
	r->Prog.Engine = eng;
	r->Prog.Prog = p = newProgram(Vertex,&Fragment,"Coord",(int)TheCoordIdx,"TextCoord",(int)TheTextCoordIdx,NULL);
	glDeleteShader(Fragment.Desc);
	glUseProgram(p);
	r->IdTexture = glGetUniformLocation(p,"CurrentPTexture");
	r->Texture = -1;
}
static void newGreyProgram(OGLEngine *eng,struct GreyProgram *r,ShaderDesc *Vertex,char *FragFile) {
	GLuint p;
	ShaderDesc Fragment;
	newShader(&Fragment,GL_FRAGMENT_SHADER,FragFile);
	r->Prog.Engine = eng;
	r->Prog.Prog = p = newProgram(Vertex,&Fragment,"Coord",(int)TheCoordIdx,"TextCoord",(int)TheTextCoordIdx,NULL);
	glDeleteShader(Fragment.Desc);
	glUseProgram(p);
	r->IdTexture = glGetUniformLocation(p,"CurrentPTexture");
	r->IdForeground = glGetUniformLocation(p,"Foreground");
	r->IdBackground = glGetUniformLocation(p,"Background");
	glUniform4f(r->IdForeground,eng->Fg.v[0],eng->Fg.v[1],eng->Fg.v[2],eng->Fg.v[3]);
	r->sgnFg = eng->Fg.sgn;
	glUniform4f(r->IdBackground,eng->Bg.v[0],eng->Bg.v[1],eng->Bg.v[2],eng->Bg.v[3]);
	r->sgnBg = eng->Bg.sgn;
	glUniform1i(r->IdTexture,TheTextureUnit);
	r->Texture = -1;
}*/

static void ManageDisplay(void) {
	static int White[4] = {255,255,255,255};
	static int Red[4] = {255,0,0,255};
	static int Green[4] = {0,255,0,255};
	static int Blue[4] = {0,0,255,255};
	static int Yellow[4] = {255,255,0,255};
    Geo2dRectangle Rect;
	Geo2dPoint Pos;
	glClearColor(0.,0.,0.,1.);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glUseProgram(0);
	glShadeModel(GL_FLAT);
	glViewport(0,0,GLEnv.Window.Extent.w,GLEnv.Window.Extent.h);
	Rect.Extent.w = 300; Rect.Extent.h = 200;
	Rect.Pos.x = 250; Rect.Pos.y = 200;
    DrawMonoRectangle(&Rect,White);
	Rect.Pos.x = 50; Rect.Pos.y = 50;
	DrawMonoRectangle(&Rect,Red);
	Rect.Pos.x = 450; Rect.Pos.y = 50;
	DrawMonoRectangle(&Rect,Green);
	Rect.Pos.x = 50; Rect.Pos.y = 350;
	DrawMonoRectangle(&Rect,Blue);
	Rect.Pos.x = 450; Rect.Pos.y = 350;
	DrawMonoRectangle(&Rect,Yellow);
	Rect.Pos.x = 0; Rect.Pos.y = 0;
	Pos.x = 250; Pos.y = 0;
    DrawLinColRectangle(&Pos,&Rect);
	Rect.Extent.w = Rect.Extent.h = 1;
	Rect.Pos.x = 0; Rect.Pos.y = 0;
	DrawMonoRectangle(&Rect,White);
	Rect.Pos.x = GLEnv.Window.Extent.w-1;
	Rect.Pos.y = GLEnv.Window.Extent.h-1;
	DrawMonoRectangle(&Rect,White);
	glutSwapBuffers();
}
static void ManageKeyboard(unsigned char key,int x,int y) {
	if (key=='q'||key=='Q') {
		glutLeaveMainLoop();
	}
}

main (int Argc,char *Argv[]) {
	int WindowId;
	EnvOpen(4096,4096);
	GLEnv.Cursor.x = 0; GLEnv.Cursor.y = 0;
	GLEnv.Window.Pos.x = GLEnv.Window.Pos.y = 0;
	GLEnv.Window.Extent.w = 800; GLEnv.Window.Extent.h = 600;
	glutInit(&Argc,Argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowSize(800,600);
	glutInitWindowPosition(0,0);
	printf("TextVertSize:%d,CoordOffset:%d,TextOffset:%d\n",
		TlsAtomSize(TexTexVertex), TlsFieldOffset(TexTexVertex,w), TlsFieldOffset(TexTexVertex,t)
	);
	WindowId = glutCreateWindow("glsl test");
	glutDisplayFunc(ManageDisplay);
	glutKeyboardFunc(ManageKeyboard);
	{
		ShaderDesc p;
	    glewInit();
	    newShader(&p,GL_VERTEX_SHADER,"data/OpenGL21/Vert2dFlat.glsl");
	    newMonoProgram(&GLEnv.Mono,&p,"data/OpenGL21/FragMono.glsl");
	    glDeleteShader(p.Desc);
		newShader(&p,GL_VERTEX_SHADER,"data/OpenGL21/Vert2dText.glsl");
        newLinColProgram(&GLEnv.LinCol,&p,"data/OpenGL21/FragLinClr.glsl");
		// newTextProgram(&GLEnv.Grey256,&p,"data/OpenGL21/Frag256Grey.glsl");
		glDeleteShader(p.Desc);
		glGenBuffers(1,&GLEnv.MainBuffer);
		glBindBuffer(GL_ARRAY_BUFFER,GLEnv.MainBuffer);
		glBufferData(GL_ARRAY_BUFFER,MainBufferSize,(void *)0,GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER,0);
	}
	glutMainLoop();
	glDeleteBuffers(1,&GLEnv.MainBuffer);
	glDeleteProgram(GLEnv.Mono.Prog);
	glutDestroyWindow(WindowId);
	EnvClose();
}

