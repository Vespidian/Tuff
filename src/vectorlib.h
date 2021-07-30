#ifndef VECTORLIB_H_
#define VECTORLIB_H_

typedef struct Vector2_i{
	int x, y;
	int v[2];
}Vector2_i;

typedef struct {
	union{
		struct {float x, y;};
		float v[2];
	};
}Vector2;

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
}Vector3_i;

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
}Vector4_i;

// typedef union{
// 	Vector4_f f;
// 	Vector4_i i;
// 	Vector4_b b;
// } Vector4;

#endif