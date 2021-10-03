#ifndef VECTORLIB_H_
#define VECTORLIB_H_

/* VECTOR 2 */
typedef struct {
	union{
		struct {float x, y;};
		float v[2];
	};
}Vector2;

typedef struct {
	union{
		struct {int x, y;};
		int v[2];
	};
}iVector2;

typedef struct {
	union{
		struct {bool x, y;};
		bool v[2];
	};
}bVector2;


/* VECTOR 3 */
typedef struct {
	union{
		struct {float r, g, b;};
		struct {float x, y, z;};
		struct {float s, t, p;};
		float v[3];
	};
}Vector3;

typedef struct {
	union{
		struct {int r, g, b;};
		struct {int x, y, z;};
		struct {int s, t, p;};
		int v[3];
	};
}iVector3;

typedef struct {
	union{
		struct {bool r, g, b;};
		struct {bool x, y, z;};
		struct {bool s, t, p;};
		bool v[3];
	};
}bVector3;


/* VECTOR 4 */
typedef struct {
	union{
		struct {float r, g, b, a;};
		struct {float x, y, z, w;};
		struct {float s, t, p, q;};
		float v[4];
	};
}Vector4;

typedef struct {
	union{
		struct {int r, g, b, a;};
		struct {int x, y, z, w;};
		struct {int s, t, p, q;};
		int v[4];
	};
}iVector4;

typedef struct {
	union{
		struct {bool r, g, b, a;};
		struct {bool x, y, z, w;};
		struct {bool s, t, p, q;};
		bool v[4];
	};
}bVector4;

#endif