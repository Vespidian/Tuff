## MATERIALS (.mat):
A material bundles together a shader and multiple textures / values to be assigned to uniforms within said shader

- Must have a base shader
- May have 0 or more uniforms

Parameters types include:
- value 	(float, int, bool, path to texture, vector)

Uniform value type (float, int, ..) and ranges (0-255, 0.0-1.0, ..) are specified in shader files


### Notes:
- When specifying 'value' if the uniform's type is float you must explicitly specify this in the number by adding a .0 to any whole numbers (i.e. 50.0 rather than 50)