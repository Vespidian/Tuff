#ifndef PEBBLE_H_
#define PEBBLE_H_

#include <tcclib.h>

// To use structs on both compiled and jitted sides, simply declare them before any code that needs them
typedef struct{
	union{
		struct {float x, y;};
		float v[2];
	};
}Vector2;
typedef struct{
	union{
		struct {float x, y, z;};
		float v[3];
	};
}Vector3;

typedef struct{
	char *name;

}UIElement;

struct Object{
	int d1;
	float f1;
};

extern void LoadScene(char *path);

// extern Vector3 position;

#ifdef _WIN32 /* dynamically linked data needs 'dllimport' */
 __attribute__((dllimport))
#endif

// extern Vector3 color;
// extern struct Object parent;


// extern int add(int a, int b);
// extern int add_vecs(Vector2 a, Vector2 b);


#endif