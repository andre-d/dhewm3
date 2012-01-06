/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "sys/platform.h"
#include "renderer/VertexCache.h"

#include "renderer/tr_local.h"

shaderProgram_t		interactionShader = { -1 };
shaderProgram_t		ambientInteractionShader = { -1 };
shaderProgram_t		stencilShadowShader = { -1 };

static void GLSL_SelectTextureNoClient(int unit) {
	backEnd.glState.currenttmu = unit;
	qglActiveTextureARB(GL_TEXTURE0_ARB + unit);
}

static void GLSL_DrawInteraction(const drawInteraction_t *din) {
	// load all the shader parameters
	if (din->ambientLight) {
		qglUniform4fvARB(ambientInteractionShader.localLightOrigin, 1, din->localLightOrigin.ToFloatPtr());
		qglUniform4fvARB(ambientInteractionShader.lightProjectionS, 1, din->lightProjection[0].ToFloatPtr());
		qglUniform4fvARB(ambientInteractionShader.lightProjectionT, 1, din->lightProjection[1].ToFloatPtr());
		qglUniform4fvARB(ambientInteractionShader.lightProjectionQ, 1, din->lightProjection[2].ToFloatPtr());
		qglUniform4fvARB(ambientInteractionShader.lightFalloff, 1, din->lightProjection[3].ToFloatPtr());
		qglUniform4fvARB(ambientInteractionShader.bumpMatrixS, 1, din->bumpMatrix[0].ToFloatPtr());
		qglUniform4fvARB(ambientInteractionShader.bumpMatrixT, 1, din->bumpMatrix[1].ToFloatPtr());
		qglUniform4fvARB(ambientInteractionShader.diffuseMatrixS, 1, din->diffuseMatrix[0].ToFloatPtr());
		qglUniform4fvARB(ambientInteractionShader.diffuseMatrixT, 1, din->diffuseMatrix[1].ToFloatPtr());

		static const float zero[4] = { 0, 0, 0, 0 };
		static const float one[4] = { 1, 1, 1, 1 };
		static const float negOne[4] = { -1, -1, -1, -1 };

		switch (din->vertexColor) {
		case SVC_IGNORE:
			qglUniform4fARB(ambientInteractionShader.colorModulate, zero[0], zero[1], zero[2], zero[3]);
			qglUniform4fARB(ambientInteractionShader.colorAdd, one[0], one[1], one[2], one[3]);
			break;
		case SVC_MODULATE:
			qglUniform4fARB(ambientInteractionShader.colorModulate, one[0], one[1], one[2], one[3]);
			qglUniform4fARB(ambientInteractionShader.colorAdd, zero[0], zero[1], zero[2], zero[3]);
			break;
		case SVC_INVERSE_MODULATE:
			qglUniform4fARB(ambientInteractionShader.colorModulate, negOne[0], negOne[1], negOne[2], negOne[3]);
			qglUniform4fARB(ambientInteractionShader.colorAdd, one[0], one[1], one[2], one[3]);
			break;
		}

		// set the constant color
		qglUniform4fvARB(ambientInteractionShader.diffuseColor, 1, din->diffuseColor.ToFloatPtr());
	} else {
		qglUniform4fvARB(interactionShader.localLightOrigin, 1, din->localLightOrigin.ToFloatPtr());
		qglUniform4fvARB(interactionShader.localViewOrigin, 1, din->localViewOrigin.ToFloatPtr());
		qglUniform4fvARB(interactionShader.lightProjectionS, 1, din->lightProjection[0].ToFloatPtr());
		qglUniform4fvARB(interactionShader.lightProjectionT, 1, din->lightProjection[1].ToFloatPtr());
		qglUniform4fvARB(interactionShader.lightProjectionQ, 1, din->lightProjection[2].ToFloatPtr());
		qglUniform4fvARB(interactionShader.lightFalloff, 1, din->lightProjection[3].ToFloatPtr());
		qglUniform4fvARB(interactionShader.bumpMatrixS, 1, din->bumpMatrix[0].ToFloatPtr());
		qglUniform4fvARB(interactionShader.bumpMatrixT, 1, din->bumpMatrix[1].ToFloatPtr());
		qglUniform4fvARB(interactionShader.diffuseMatrixS, 1, din->diffuseMatrix[0].ToFloatPtr());
		qglUniform4fvARB(interactionShader.diffuseMatrixT, 1, din->diffuseMatrix[1].ToFloatPtr());
		qglUniform4fvARB(interactionShader.specularMatrixS, 1, din->specularMatrix[0].ToFloatPtr());
		qglUniform4fvARB(interactionShader.specularMatrixT, 1, din->specularMatrix[1].ToFloatPtr());

		static const float zero[4] = { 0, 0, 0, 0 };
		static const float one[4] = { 1, 1, 1, 1 };
		static const float negOne[4] = { -1, -1, -1, -1 };

		switch (din->vertexColor) {
		case SVC_IGNORE:
			qglUniform4fARB(interactionShader.colorModulate, zero[0], zero[1], zero[2], zero[3]);
			qglUniform4fARB(interactionShader.colorAdd, one[0], one[1], one[2], one[3]);
			break;
		case SVC_MODULATE:
			qglUniform4fARB(interactionShader.colorModulate, one[0], one[1], one[2], one[3]);
			qglUniform4fARB(interactionShader.colorAdd, zero[0], zero[1], zero[2], zero[3]);
			break;
		case SVC_INVERSE_MODULATE:
			qglUniform4fARB(interactionShader.colorModulate, negOne[0], negOne[1], negOne[2], negOne[3]);
			qglUniform4fARB(interactionShader.colorAdd, one[0], one[1], one[2], one[3]);
			break;
		}

		// set the constant colors
		qglUniform4fvARB(interactionShader.diffuseColor, 1, din->diffuseColor.ToFloatPtr());
		qglUniform4fvARB(interactionShader.specularColor, 1, din->specularColor.ToFloatPtr());
	}

	// set the textures

	// texture 0 will be the per-surface bump map
	GLSL_SelectTextureNoClient(0);
	din->bumpImage->Bind();

	// texture 1 will be the light falloff texture
	GLSL_SelectTextureNoClient(1);
	din->lightFalloffImage->Bind();

	// texture 2 will be the light projection texture
	GLSL_SelectTextureNoClient(2);
	din->lightImage->Bind();

	// texture 3 is the per-surface diffuse map
	GLSL_SelectTextureNoClient(3);
	din->diffuseImage->Bind();

	if (!din->ambientLight) {
		// texture 4 is the per-surface specular map
		GLSL_SelectTextureNoClient(4);
		din->specularImage->Bind();
	}

	// draw it
	RB_DrawElementsWithCounters(din->surf->geo);
}

static void GLSL_CreateDrawInteractions(const drawSurf_t *surf) {
	if (!surf)
		return;

	// perform setup here that will be constant for all interactions
	GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | backEnd.depthFunc);

	// bind the vertex and fragment program
	if (backEnd.vLight->lightShader->IsAmbientLight()) {
		if (ambientInteractionShader.program == -1)
			qglUseProgramObjectARB(0);
		else
			qglUseProgramObjectARB(ambientInteractionShader.program);
	} else {
		if (interactionShader.program == -1)
			qglUseProgramObjectARB(0);
		else
			qglUseProgramObjectARB(interactionShader.program);
	}

	// enable the vertex arrays
	qglEnableVertexAttribArrayARB(8);
	qglEnableVertexAttribArrayARB(9);
	qglEnableVertexAttribArrayARB(10);
	qglEnableVertexAttribArrayARB(11);
	qglEnableClientState(GL_COLOR_ARRAY);

	for (; surf; surf = surf->nextOnLight) {
		// set the vertex pointers
		idDrawVert	*ac = (idDrawVert *)vertexCache.Position(surf->geo->ambientCache);
		qglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(idDrawVert), ac->color);
		qglVertexAttribPointerARB(11, 3, GL_FLOAT, false, sizeof(idDrawVert), ac->normal.ToFloatPtr());
		qglVertexAttribPointerARB(10, 3, GL_FLOAT, false, sizeof(idDrawVert), ac->tangents[1].ToFloatPtr());
		qglVertexAttribPointerARB(9, 3, GL_FLOAT, false, sizeof(idDrawVert), ac->tangents[0].ToFloatPtr());
		qglVertexAttribPointerARB(8, 2, GL_FLOAT, false, sizeof(idDrawVert), ac->st.ToFloatPtr());
		qglVertexPointer(3, GL_FLOAT, sizeof(idDrawVert), ac->xyz.ToFloatPtr());

		// set model matrix
		if (backEnd.vLight->lightShader->IsAmbientLight())
			qglUniformMatrix4fvARB(ambientInteractionShader.modelMatrix, 1, false, surf->space->modelMatrix);
		else
			qglUniformMatrix4fvARB(interactionShader.modelMatrix, 1, false, surf->space->modelMatrix);

		// this may cause GLSL_DrawInteraction to be executed multiple
		// times with different colors and images if the surface or light have multiple layers
		RB_CreateSingleDrawInteractions(surf, GLSL_DrawInteraction);
	}

	qglDisableVertexAttribArrayARB(8);
	qglDisableVertexAttribArrayARB(9);
	qglDisableVertexAttribArrayARB(10);
	qglDisableVertexAttribArrayARB(11);
	qglDisableClientState(GL_COLOR_ARRAY);

	// disable features
	GLSL_SelectTextureNoClient(4);
	globalImages->BindNull();

	GLSL_SelectTextureNoClient(3);
	globalImages->BindNull();

	GLSL_SelectTextureNoClient(2);
	globalImages->BindNull();

	GLSL_SelectTextureNoClient(1);
	globalImages->BindNull();

	backEnd.glState.currenttmu = -1;
	GL_SelectTexture(0);

	qglUseProgramObjectARB(0);
}

/*
==================
RB_GLSL_DrawInteractions
==================
*/
void RB_GLSL_DrawInteractions(void) {
	viewLight_t *vLight;

	GL_SelectTexture(0);
	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);

	// for each light, perform adding and shadowing
	for (vLight = backEnd.viewDef->viewLights; vLight ;vLight = vLight->next) {
		backEnd.vLight = vLight;

		// do fogging later
		if (vLight->lightShader->IsFogLight())
			continue;

		if (vLight->lightShader->IsBlendLight())
			continue;

		// if there are no interactions, get out!
		if (!vLight->localInteractions &&
			!vLight->globalInteractions &&
			!vLight->translucentInteractions) {
			continue;
		}

		// clear the stencil buffer if needed
		if (vLight->globalShadows || vLight->localShadows) {
			backEnd.currentScissor = vLight->scissorRect;

			if (r_useScissor.GetBool()) {
				qglScissor(backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1,
					backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
					backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
					backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1);
			}

			qglClear(GL_STENCIL_BUFFER_BIT);
		} else {
			// no shadows, so no need to read or write the stencil buffer
			// we might in theory want to use GL_ALWAYS instead of disabling
			// completely, to satisfy the invarience rules
			qglStencilFunc(GL_ALWAYS, 128, 255);
		}

		if (r_useShadowVertexProgram.GetBool()) {
			qglUseProgramObjectARB(stencilShadowShader.program);

			RB_StencilShadowPass(vLight->globalShadows);
			GLSL_CreateDrawInteractions(vLight->localInteractions);

			qglUseProgramObjectARB(stencilShadowShader.program);
			RB_StencilShadowPass(vLight->localShadows);
			GLSL_CreateDrawInteractions(vLight->globalInteractions);

			// if there weren't any globalInteractions, it would have stayed on
			qglUseProgramObjectARB(0);
		} else {
			RB_StencilShadowPass(vLight->globalShadows);
			GLSL_CreateDrawInteractions(vLight->localInteractions);

			RB_StencilShadowPass(vLight->localShadows);
			GLSL_CreateDrawInteractions(vLight->globalInteractions);
		}

		// translucent surfaces never get stencil shadowed
		if (r_skipTranslucent.GetBool())
			continue;

		qglStencilFunc(GL_ALWAYS, 128, 255);

		backEnd.depthFunc = GLS_DEPTHFUNC_LESS;
		GLSL_CreateDrawInteractions(vLight->translucentInteractions);
		backEnd.depthFunc = GLS_DEPTHFUNC_EQUAL;
	}

	// disable stencil shadow test
	qglStencilFunc(GL_ALWAYS, 128, 255);

	GL_SelectTexture(0);
	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

static void GLSL_DeleteShader(GLhandleARB *shader) {
	if (*shader != -1) {
		qglDeleteShader(*shader);
		*shader = -1;
	}
}

static bool GLSL_LoadShader(const char *path, GLhandleARB *shader, GLenum type) {
	void *buf;

	if (!glConfig.isInitialized)
		return false;

	GLSL_DeleteShader(shader);

	// load the program even if we don't support it, so
	// fs_copyfiles can generate cross-platform data dumps
	fileSystem->ReadFile(path, &buf, NULL);
	if (!buf) {
		common->Printf("%s: File not found\n", path);
		return false;
	}

	*shader = qglCreateShaderObjectARB( type );
	qglShaderSourceARB(*shader, 1, (const GLcharARB **)&buf, 0);
	qglCompileShaderARB(*shader);

	fileSystem->FreeFile( buf );

	GLint logLength;
	qglGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 1) {
		GLchar *log = (GLchar *)malloc(logLength);
		qglGetShaderInfoLog(*shader, logLength, &logLength, log);
		common->Printf("glGetShaderInfoLog: %s\n", log);
		free(log);
	}

	GLint status;
	qglGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
	if (status == 0) {
		GLSL_DeleteShader(shader);
		return false;
	}

	return true;
}

static void GLSL_DeleteProgram(shaderProgram_t *shaderProgram) {
	qglDetachObjectARB(shaderProgram->program, shaderProgram->fragmentShader);
	qglDetachObjectARB(shaderProgram->program, shaderProgram->vertexShader);

	GLSL_DeleteShader(&shaderProgram->vertexShader);
	GLSL_DeleteShader(&shaderProgram->fragmentShader);
}

static bool GLSL_LinkProgram(shaderProgram_t *shaderProgram, bool needsAttributes) {
	GLint linked;

	shaderProgram->program = qglCreateProgramObjectARB( );

	qglAttachObjectARB(shaderProgram->program, shaderProgram->vertexShader);
	qglAttachObjectARB(shaderProgram->program, shaderProgram->fragmentShader);

	if (needsAttributes) {
		qglBindAttribLocationARB(shaderProgram->program, 8, "attr_TexCoord");
		qglBindAttribLocationARB(shaderProgram->program, 9, "attr_Tangent");
		qglBindAttribLocationARB(shaderProgram->program, 10, "attr_Bitangent");
		qglBindAttribLocationARB(shaderProgram->program, 11, "attr_Normal");
	}

	qglLinkProgramARB( shaderProgram->program );

	qglGetObjectParameterivARB( shaderProgram->program, GL_OBJECT_LINK_STATUS_ARB, &linked );

	if (!linked) {
		GLSL_DeleteProgram(shaderProgram);
		return false;
	}

	return true;
}

static bool GLSL_ValidateProgram(shaderProgram_t *shaderProgram) {
	GLint validProgram;

	qglValidateProgramARB(shaderProgram->program);
	qglGetObjectParameterivARB(shaderProgram->program, GL_OBJECT_VALIDATE_STATUS_ARB, &validProgram);

	if (!validProgram) {
		GLSL_DeleteProgram(shaderProgram);
		return false;
	}

	return true;
}

static bool GLSL_LoadProgram(const char *basename, shaderProgram_t *shaderProgram, bool needsAttributes) {
	char path[1024];

	common->Printf("%s\n", basename);

	GLSL_DeleteProgram(shaderProgram);

	idStr::snPrintf(path, sizeof(path), "gl2progs/%s.vertex", basename);
	if (!GLSL_LoadShader(path, &shaderProgram->vertexShader, GL_VERTEX_SHADER_ARB)) {
		GLSL_DeleteProgram(shaderProgram);
		return false;
	}

	idStr::snPrintf(path, sizeof(path), "gl2progs/%s.fragment", basename);
	if (!GLSL_LoadShader(path, &shaderProgram->fragmentShader, GL_FRAGMENT_SHADER_ARB)) {
		GLSL_DeleteShader(&shaderProgram->vertexShader);
		return false;
	}

	if (!GLSL_LinkProgram(shaderProgram, needsAttributes)) {
		common->Printf("program failed to linkn\n" );
		return false;
	}

	if (!GLSL_ValidateProgram(shaderProgram)) {
		common->Printf("program is invalid.\n" );
		return false;
	}

	return true;
}

static void DeleteAllGLSLPrograms() {
	qglUseProgramObjectARB(0);
	GLSL_DeleteProgram(&interactionShader);
	GLSL_DeleteProgram(&ambientInteractionShader);
	GLSL_DeleteProgram(&stencilShadowShader);
}

static bool InitGLSLShaders() {
	DeleteAllGLSLPrograms();

	// load interation shaders
	if (!GLSL_LoadProgram("interaction", &interactionShader, true)) {
		common->Printf("GLSL interactionShader unavailable.\n");
		return false;
	}

	// set uniform locations
	interactionShader.u_normalTexture = qglGetUniformLocationARB(interactionShader.program, "u_normalTexture");
	interactionShader.u_lightFalloffTexture = qglGetUniformLocationARB(interactionShader.program, "u_lightFalloffTexture");
	interactionShader.u_lightProjectionTexture = qglGetUniformLocationARB(interactionShader.program, "u_lightProjectionTexture");
	interactionShader.u_diffuseTexture = qglGetUniformLocationARB(interactionShader.program, "u_diffuseTexture");
	interactionShader.u_specularTexture = qglGetUniformLocationARB(interactionShader.program, "u_specularTexture");

	interactionShader.modelMatrix = qglGetUniformLocationARB(interactionShader.program, "u_modelMatrix");

	interactionShader.localLightOrigin = qglGetUniformLocationARB(interactionShader.program, "u_lightOrigin");
	interactionShader.localViewOrigin = qglGetUniformLocationARB(interactionShader.program, "u_viewOrigin");
	interactionShader.lightProjectionS = qglGetUniformLocationARB(interactionShader.program, "u_lightProjectionS");
	interactionShader.lightProjectionT = qglGetUniformLocationARB(interactionShader.program, "u_lightProjectionT");
	interactionShader.lightProjectionQ = qglGetUniformLocationARB(interactionShader.program, "u_lightProjectionQ");
	interactionShader.lightFalloff = qglGetUniformLocationARB(interactionShader.program, "u_lightFalloff");

	interactionShader.bumpMatrixS = qglGetUniformLocationARB(interactionShader.program, "u_bumpMatrixS");
	interactionShader.bumpMatrixT = qglGetUniformLocationARB(interactionShader.program, "u_bumpMatrixT");
	interactionShader.diffuseMatrixS = qglGetUniformLocationARB(interactionShader.program, "u_diffuseMatrixS");
	interactionShader.diffuseMatrixT = qglGetUniformLocationARB(interactionShader.program, "u_diffuseMatrixT");
	interactionShader.specularMatrixS = qglGetUniformLocationARB(interactionShader.program, "u_specularMatrixS");
	interactionShader.specularMatrixT = qglGetUniformLocationARB(interactionShader.program, "u_specularMatrixT");

	interactionShader.colorModulate = qglGetUniformLocationARB(interactionShader.program, "u_colorModulate");
	interactionShader.colorAdd = qglGetUniformLocationARB(interactionShader.program, "u_colorAdd");

	interactionShader.diffuseColor = qglGetUniformLocationARB(interactionShader.program, "u_diffuseColor");
	interactionShader.specularColor = qglGetUniformLocationARB(interactionShader.program, "u_specularColor");

	// set texture locations
	qglUseProgramObjectARB(interactionShader.program);
	qglUniform1iARB(interactionShader.u_normalTexture, 0);
	qglUniform1iARB(interactionShader.u_lightFalloffTexture, 1);
	qglUniform1iARB(interactionShader.u_lightProjectionTexture, 2);
	qglUniform1iARB(interactionShader.u_diffuseTexture, 3);
	qglUniform1iARB(interactionShader.u_specularTexture, 4);
	qglUseProgramObjectARB(0);

	// load ambient interation shaders
	if (!GLSL_LoadProgram("ambientInteraction", &ambientInteractionShader, true)) {
		common->Printf("GLSL ambientInteractionShader unavailable.\n");
		DeleteAllGLSLPrograms();
		return false;
	}

	// set uniform locations
	ambientInteractionShader.u_normalTexture = qglGetUniformLocationARB(ambientInteractionShader.program, "u_normalTexture");
	ambientInteractionShader.u_lightFalloffTexture = qglGetUniformLocationARB(ambientInteractionShader.program, "u_lightFalloffTexture");
	ambientInteractionShader.u_lightProjectionTexture = qglGetUniformLocationARB(ambientInteractionShader.program, "u_lightProjectionTexture");
	ambientInteractionShader.u_diffuseTexture = qglGetUniformLocationARB(ambientInteractionShader.program, "u_diffuseTexture");

	ambientInteractionShader.modelMatrix = qglGetUniformLocationARB(ambientInteractionShader.program, "u_modelMatrix");

	ambientInteractionShader.localLightOrigin = qglGetUniformLocationARB(ambientInteractionShader.program, "u_lightOrigin");
	ambientInteractionShader.lightProjectionS = qglGetUniformLocationARB(ambientInteractionShader.program, "u_lightProjectionS");
	ambientInteractionShader.lightProjectionT = qglGetUniformLocationARB(ambientInteractionShader.program, "u_lightProjectionT");
	ambientInteractionShader.lightProjectionQ = qglGetUniformLocationARB(ambientInteractionShader.program, "u_lightProjectionQ");
	ambientInteractionShader.lightFalloff = qglGetUniformLocationARB(ambientInteractionShader.program, "u_lightFalloff");

	ambientInteractionShader.bumpMatrixS = qglGetUniformLocationARB(ambientInteractionShader.program, "u_bumpMatrixS");
	ambientInteractionShader.bumpMatrixT = qglGetUniformLocationARB(ambientInteractionShader.program, "u_bumpMatrixT");
	ambientInteractionShader.diffuseMatrixS = qglGetUniformLocationARB(ambientInteractionShader.program, "u_diffuseMatrixS");
	ambientInteractionShader.diffuseMatrixT = qglGetUniformLocationARB(ambientInteractionShader.program, "u_diffuseMatrixT");

	ambientInteractionShader.colorModulate = qglGetUniformLocationARB(ambientInteractionShader.program, "u_colorModulate");
	ambientInteractionShader.colorAdd = qglGetUniformLocationARB(ambientInteractionShader.program, "u_colorAdd");

	ambientInteractionShader.diffuseColor = qglGetUniformLocationARB(ambientInteractionShader.program, "u_diffuseColor");

	// set texture locations
	qglUseProgramObjectARB(ambientInteractionShader.program);
	qglUniform1iARB(ambientInteractionShader.u_normalTexture, 0);
	qglUniform1iARB(ambientInteractionShader.u_lightFalloffTexture, 1);
	qglUniform1iARB(ambientInteractionShader.u_lightProjectionTexture, 2);
	qglUniform1iARB(ambientInteractionShader.u_diffuseTexture, 3);
	qglUseProgramObjectARB(0);

	// load stencil shadow extrusion shaders
	if (!GLSL_LoadProgram("stencilshadow", &stencilShadowShader, true)) {
		common->Printf("GLSL stencilShadowShader unavailable\n.");
		DeleteAllGLSLPrograms();
		return false;
	}

	// set uniform locations
	stencilShadowShader.localLightOrigin = qglGetUniformLocationARB(stencilShadowShader.program, "u_lightOrigin");

	return true;
}

/*
==================
R_ReloadGLSLPrograms_f
==================
*/
void R_ReloadGLSLPrograms_f(const idCmdArgs &args) {
	common->Printf("----- R_ReloadGLSLPrograms_f -----\n");

	if (glConfig.GLSLAvailable) {
		glConfig.allowGLSLPath = InitGLSLShaders();

		if (!glConfig.allowGLSLPath)
			common->Printf("GLSL shaders failed to init.\n");
	} else {
		common->Printf("Not available.\n");
	}

	common->Printf("----------------------------------\n");
}

void R_GLSL_Init(void) {
	glConfig.allowGLSLPath = false;

	common->Printf("---------- R_GLSL_Init ----------\n");

	if (glConfig.GLSLAvailable)
		common->Printf("Available.\n");
	else
		common->Printf("Not available.\n");

	common->Printf("---------------------------------\n");

	glConfig.allowGLSLPath = true;
}
