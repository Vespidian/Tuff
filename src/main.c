#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "debug.h"

#include "gltf.h"

int main(){
	InitDebug();

	GLTF gltf = GLTFOpen("../assets/monkey.gltf");
	GLTF gltf_nonexistant = GLTFOpen("../assets/nonexistant_file.gltf"); // Testing failsafes

	GLTFReload(&gltf);
	GLTFReload(&gltf_nonexistant);

	GLTFFree(&gltf);
	GLTFFree(&gltf_nonexistant);

	QuitDebug();

	getchar();

	return 0;
}
