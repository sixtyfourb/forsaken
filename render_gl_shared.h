/*
  This file is shared internally by render_* family.
  For public facing header please use render.h
*/
#ifdef GL
#ifndef RENDER_GL_SHARED_INCLUDED
#define RENDER_GL_SHARED_INCLUDED

#include "main.h"
#include "util.h"
#include "render.h"
#include "texture.h"
#include "file.h"
#include <stdio.h>
#include "main_sdl.h"

/*
 * SDL_opengl.h declares the GL 1.1 entry points itself and leaves everything
 * added since to glext.h, which declares nothing unless this is defined first.
 * Without it every modern call in these backends - glBindBuffer, glBufferData,
 * glMapBuffer, the entire shader API - is an implicit declaration, which C
 * takes to return int. glMapBuffer's pointer then comes back truncated to 32
 * bits and the first write through it lands on an address that was never
 * mapped.
 *
 * The original builds of this engine were 32-bit, where truncating a pointer to
 * int costs nothing, so the bug arrived with the move to 64-bit rather than
 * with any of the work here.
 */
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#include "SDL_opengl.h"

extern render_info_t render_info;

extern GLenum render_last_gl_error;

/*
 * gluErrorString is the one GLU call these backends make, and nothing declares
 * it: SDL_opengl.h covers GL, not GLU. Left implicit it returns int, so the
 * string it hands back is a pointer cut down to 32 bits - and it is called from
 * the error path, which is the worst place to keep a crash. Android and macOS
 * have their own answers below; everywhere else has the real header.
 */
#if !defined(__ANDROID__) && !defined(MACOSX)
#include <GL/glu.h>
#endif

// TODO invalid pointer
#if defined(MACOSX) && SDL_VERSION_ATLEAST(2,0,0)
#define gluErrorString(e)\
	(e == 0x0500 ? "invalid enumerant" : \
	(e == 0x0501 ? "invalid value" : \
	(e == 0x0502 ? "invalid operation" : \
	(e == 0x0503 ? "stack overflow" : \
	(e == 0x0504 ? "stack underflow" : \
	(e == 0x0505 ? "out of memory" : \
	(e == 0x0506 ? "invalid framebuffer operation" : \
	(e == 0x8031 ? "table too large" : \
	 "unknown" \
	))))))))
#endif

const char * render_error_description( int e );

#define CHECK_GL_ERRORS \
	do \
	{ \
		GLenum e; \
		while( ( e = glGetError() ) != GL_NO_ERROR ) \
		{ \
			render_last_gl_error = e; \
			DebugPrintf( "GL error: %s (%s:%d)\n", \
				gluErrorString(e),  __FILE__, __LINE__ ); \
		} \
	} while (0)


typedef struct { float anisotropic; } gl_caps_t;
extern gl_caps_t caps;

typedef struct { GLuint id; } texture_t; // Possibly later: GLuint bump_id;

//
// d3d stored the world/view matrixes
// and then multiplied them together before rendering
// in the following order: world * view * projection
// opengl handles only world and projection
// so we must emulate the behavior of world*view
// although we multiply the arguments backwards view*world
//

extern MATRIX proj_matrix;
extern MATRIX view_matrix;
extern MATRIX world_matrix;

#if GL != 1

void mvp_update( GLuint current_program );

extern GLuint vertex_shader;
extern GLuint fragment_shader;
extern GLuint current_program;

LPVERTEXBUFFER _create_buffer( int size, GLenum type, GLenum gettype, GLenum usage );

#define create_buffer( size, type, usage ) \
        _create_buffer( size, type, type ## _BINDING, usage )

#endif // GL != 1

void FSReleaseRenderObject(RENDEROBJECT *renderObject);

#endif // RENDER_GL_SHARED_INCLUDED
#endif // GL ENABLED
