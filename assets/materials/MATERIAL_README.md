## MATERIALS (.mat):
A material bundles together a shader and multiple textures / values to be assigned to said shader

- Must have a base shader
- May have 0 or more parameters

Parameters types include:
- texture 	(1D, 2D, 3D)
- value 	(float, int, bool)

When specifying an asset that is loaded in the bundle file (ie textures, shaders, ...) the index must be specified rather than the path. Index means the position of the asset in the bundle.

Assets are indexed starting at 0

Uniform value type (float, int, ..) and ranges (0-255, 0.0-1.0, ..) are specified in shader files