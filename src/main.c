#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "debug.h"

#include "shader.h"
#include "material.h"

void TestShaderLib(){
	// for(int i = 0; i < 20; i++){
		Shader default_shader = ShaderOpen("../assets/default.shader");
		Shader axis_shader = ShaderOpen("../assets/axis.shader");
		Shader grid_shader = ShaderOpen("../assets/grid.shader");
		Shader mask_shader = ShaderOpen("../assets/mask.shader");
		Shader mesh_shader = ShaderOpen("../assets/mesh.shader");
		Shader quad_shader = ShaderOpen("../assets/quad_default.shader");
		Shader ui_shader = ShaderOpen("../assets/ui.shader"); // Purposefully errors (not surrounded by curly braces and other things)

		ShaderReload(&default_shader);
		ShaderReload(&axis_shader);
		ShaderReload(&grid_shader);
		ShaderReload(&mask_shader);
		ShaderReload(&mesh_shader);
		ShaderReload(&quad_shader);
		ShaderReload(&ui_shader);

		ShaderUniformSetFloat(&default_shader, "value", 3.4f); // Not exposed uniform
		ShaderUniformSetFloat(&default_shader, "exposed_uni", 7.15f); // Exposed uniform

		ShaderFree(&default_shader);
		ShaderFree(&axis_shader);
		ShaderFree(&grid_shader);
		ShaderFree(&mask_shader);
		ShaderFree(&mesh_shader);
		ShaderFree(&quad_shader);
		ShaderFree(&ui_shader);
	// }

}

void TestMaterialLib(){
	Shader default_shader;
	default_shader = ShaderOpen("../assets/default.shader");
	Material default_material = MaterialOpen("../assets/default.mat");

	printf("Tint.v[2] = %f\n", MaterialUniformFind(&default_material, "tint_u")->value._vec3.z);

	MaterialShaderSet(&default_material, &default_shader);
	MaterialUniformsValidate(&default_material);
	MaterialShaderPassUniforms(&default_material);

	for(int i = 0; i < 100; i++){
		MaterialReload(&default_material);
	}

	MaterialFree(&default_material);
	ShaderFree(&default_shader);
}

int main(){
	InitDebug();

	TestShaderLib();
	TestMaterialLib();

	DebugLog(D_ACT, "-- DONE-- ");
	QuitDebug();

	// getchar();

	return 0;
}
