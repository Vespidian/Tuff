#include <stdio.h>
#include <stdbool.h>

#include "debug.h"
#include "textures.h"

int main(){
	InitDebug();
	InitTextures();

	Texture texture = TextureOpen("../assets/undefined.png", TEXTURE_FILTERING_NEAREST);

	TextureReload(&texture);

	TextureFree(&texture);

	printf("done\n");
	DebugLog(D_ACT, "-- DONE-- ");
	QuitDebug();

	getchar();

	return 0;
}
