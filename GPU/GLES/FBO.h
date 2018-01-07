// Copyright (c) 2012- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#pragma once

// Simple wrapper around FBO functionality.
// Very C-ish API because that's what I felt like, and it's cool to completely
// hide the data from callers...

#include "gfx/gl_common.h"

struct FBO;


enum FBOColorDepth {
	FBO_8888,
	FBO_565,
	FBO_4444,
	FBO_5551,
};

enum FBOChannel {
	FB_COLOR_BIT = 1,
	FB_DEPTH_BIT = 2,
	FB_STENCIL_BIT = 4,
};

enum FBBlitFilter {
	FB_BLIT_NEAREST = 0,
	FB_BLIT_LINEAR = 1,
};

// Creates a simple FBO with a RGBA32 color buffer stored in a texture, and
// optionally an accompanying Z/stencil buffer.
// No mipmap support.
// num_color_textures must be 1 for now.
// you lose bound texture state.

// On some hardware, you might get a 24-bit depth buffer even though you only wanted a 16-bit one.
FBO *fbo_create(int width, int height, int num_color_textures, bool z_stencil, FBOColorDepth colorDepth = FBO_8888);

void fbo_copy_image(FBO *src, int level, int x, int y, int z, FBO *dst, int dstLevel, int dstX, int dstY, int dstZ, int width, int height, int depth);

// Create an opaque FBO from a native GL FBO, optionally reusing an existing FBO structure.
// Useful for overriding the backbuffer FBO that is generated outside of this wrapper.
FBO *fbo_create_from_native_fbo(GLuint native_fbo, FBO *fbo = NULL);

void fbo_blit(FBO *src, int srcX1, int srcY1, int srcX2, int srcY2, FBO *dst, int dstX1, int dstY1, int dstX2, int dstY2, int channelBits, FBBlitFilter filter);

int fbo_standard_z_depth();
int fbo_check_framebuffer_status(FBO *fbo);

// These functions should be self explanatory.
void fbo_bind_as_render_target(FBO *fbo);
// color must be 0, for now.
void fbo_bind_color_as_texture(FBO *fbo, int color);
void fbo_bind_for_read(FBO *fbo);
void fbo_unbind();
void fbo_unbind_render_target();
void fbo_destroy(FBO *fbo);
void fbo_get_dimensions(FBO *fbo, int *w, int *h);

void fbo_override_backbuffer(FBO *fbo);  // Makes unbind bind this instead of the real backbuffer.
