#include "../global.h"
#include "../debug.h"
#include "renderer.h"
#include "../gl_utils.h"

#include "tilesheet.h"

TilesheetObject undefined_tilesheet;

TilesheetObject *tilesheet_stack;

// Number of tilesheets currently loaded into the 'tilesheet_stack'
unsigned int num_tilesheets = 0;

// ID of next tilesheet to be loaded
static unsigned int nextID = 0;

void InitTilesheets(){
	undefined_tilesheet = (TilesheetObject){-1, undefined_texture, 16, 16};
    DebugLog(D_ACT, "Initialized tilesheet subsystem");
}

TilesheetObject *NewTilesheet(Texture texture, int tile_w, int tile_h){
	// Expand 'tilesheet_stack' for new tilesheet
    tilesheet_stack = realloc(tilesheet_stack, sizeof(TilesheetObject) * (num_tilesheets + 1));

	// Copy data to new tilesheet
    tilesheet_stack[num_tilesheets] = (TilesheetObject){nextID, texture, tile_w, tile_h};
    DebugLog(D_ACT, "Created tilesheet id '%d'", nextID);

	// Increment stack counters
    num_tilesheets++;
    nextID++;
    return &tilesheet_stack[num_tilesheets - 1];
}

TilesheetObject *NewTilesheetFromFile(char *path, int format, int tile_w, int tile_h){
    return NewTilesheet(TextureOpen(path), tile_w, tile_h);
}

TilesheetObject *FindTilesheet(unsigned int id){
    for(int i = 0; i < num_tilesheets; i++){
        if(tilesheet_stack[i].id == id){
            return &tilesheet_stack[i];
        }
    }
    return &undefined_tilesheet;
}