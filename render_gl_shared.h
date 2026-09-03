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
#ifdef __ANDROID__
/*
 * SDL_opengl.h declares desktop GL. This platform has GLES 2, which is what the
 * GL=2 backend was chosen for, and gl2ext.h brings GL_OES_mapbuffer with it -
 * see FSLockVertexBuffer, which needs a mappable buffer and has no other way.
 */
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
/*
 * Buffer mapping is core in desktop GL and an extension here. The signatures
 * match, so aliasing the three names covers every call site (the vertex, normal
 * and index buffers) without touching the renderer. GLES only ever allows
 * write-only mapping, which is all this engine asks for.
 *
 * Adreno and Mali both carry GL_OES_mapbuffer. A device without it would need a
 * staging buffer and glBufferSubData instead.
 */
#define GL_WRITE_ONLY   GL_WRITE_ONLY_OES
#define glMapBuffer     glMapBufferOES
#define glUnmapBuffer   glUnmapBufferOES

/*
 * Depth range and clear take floats in GLES rather than doubles. The call sites
 * pass literals, so aliasing the names is enough.
 */
#define glClearDepth    glClearDepthf
#define glDepthRange    glDepthRangef

/*
 * Base-vertex drawing is GL 3.2, and in GLES it arrives as an extension. Both
 * target GPUs carry it, as they do GL_OES_mapbuffer above.
 *
 * The extension-free way would be to stop passing a base vertex and instead
 * re-point every vertex attribute by startVert * stride inside the texture-group
 * loop in render_gl2.c. That is the robust fallback if a device turns up without
 * this, and it is more than an alias - the attributes are currently set up once,
 * before the loop.
 */
void fsk_draw_elements_base_vertex( GLenum mode, GLsizei count, GLenum type,
                                   const void *indices, GLint basevertex );
#define glDrawElementsBaseVertex fsk_draw_elements_base_vertex
#else
#include "SDL_opengl.h"
#endif

extern render_info_t render_info;

extern GLenum render_last_gl_error;

// TODO invalid pointer
#if defined(__ANDROID__) || (defined(MACOSX) && SDL_VERSION_ATLEAST(2,0,0))
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
