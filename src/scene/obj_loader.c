#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../debug.h"

#include "obj_loader.h"

int num_attributes = 1;

bool texture_indices = false;
bool normal_indices = false;


int num_components_per_vertex = 3;

int CountChars(char *str, char delim){
	int count = 0;
	int i = 0;
	while(str[i] != '\0' && ((delim != ' ') ? (str[i] != ' ') : 1)){
		if(str[i] == delim){
			count++;
		}
		i++;
	}
	return count;
}

char *GetFaceIndex(char *str, char delim, int delim_index){
	static char tmp_str[128];
	memset(tmp_str, 0, 128);
	int index = 0;
	int num_delims_found = 0;
	while(str[index] != '\0'){
		if(str[index] == delim){
			num_delims_found++;
		}
		if(num_delims_found == delim_index){
			break;
		}
		index++;
	}
	int tmp_index = 0;
	if(delim_index != 0){
		index++;
	}
	if(str[index] != delim){
		while(str[index] != '\0' && str[index] != delim){
			tmp_str[tmp_index] = str[index];
			// printf("%c\n", str[index]);
			tmp_index++;
			index++;
		}
	}else{
		tmp_str[0] = '-';
		tmp_str[1] = '1';
		tmp_str[2] = '\0';
	}

	return tmp_str;
}

int GetNumFaceElements(char *str){
	int num_elements = CountChars(str, '/') + 1;
	
	// If there are only 2 elements by default, (ex: 1/1) only texture coordinates are enabled
	if(num_elements == 2){
		texture_indices = true;

	// If we have all 3 elements set them both to true
	}else if(num_elements == 3){
		texture_indices = true;
		normal_indices = true;
	}

	// If there is a blank index between 2 others (ex: 1//1) only normals are enabled
	if(strtol(GetFaceIndex(str, '/', 1), NULL, 10) == -1){
		num_elements = 2;
		normal_indices = true;
	}

	return num_elements;
}

void LoadObj(const char *path, MeshObject *mesh){
	FILE *file = fopen(path, "r");

	if(file == NULL){
		DebugLog(D_ERR, "Could not open file '%s'", path);
	}
	
	char *line_buffer = NULL;
	unsigned int line_length = 0;
	int read;

	char header[8];

	while((read = getline(&line_buffer, &line_length, file)) != -1){
		sscanf(line_buffer, "%s", header);

		switch(header[0]){
			case 'v':
				switch(header[1]){
					case 't':
						// 'vt' consists of 2 floats (Vector2)
						mesh->vt = realloc(mesh->vt, sizeof(float) * (mesh->vt_size + 3));

						// Split the line into seperate floats, starting from the first space (after the header)
						sscanf(strchr(line_buffer, ' '), "%f %f\n", &mesh->vt[mesh->vt_size + 0], &mesh->vt[mesh->vt_size + 1]);
						mesh->vt[mesh->vt_size + 2] = 0.0f;
						
						// Debugging
						// printf("%f, %f, %f\n", mesh->vt[mesh->vt_size].x, mesh->vt[mesh->vt_size].y, mesh->vt[mesh->vt_size].z);
						
						// Increment the number of vertex texture coordinates
						mesh->vt_size += 3;
					break;

					case 'n':
						// 'vn' consists of 3 floats (Vector3)
						mesh->vn = realloc(mesh->vn, sizeof(float) * (mesh->vn_size + 3));

						// Split the line into seperate floats, starting from the first space (after the header)
						sscanf(strchr(line_buffer, ' '), "%f %f %f\n", &mesh->vn[mesh->vn_size + 0], &mesh->vn[mesh->vn_size + 1], &mesh->vn[mesh->vn_size + 2]);
						
						// Debugging
						// printf("%f, %f, %f\n", mesh->vn[mesh->vn_size].x, mesh->vn[mesh->vn_size].y, mesh->vn[mesh->vn_size].z);
						
						// Increment the number of vertex normals
						mesh->vn_size += 3;
					break;

					default:
						// 'v' consists of 3 floats (Vector3)
						mesh->v = realloc(mesh->v, sizeof(float) * (mesh->v_size + 3));

						// Split the line into seperate floats, starting from the first space (after the header)
						sscanf(strchr(line_buffer, ' '), "%f %f %f\n", &mesh->v[mesh->v_size + 0], &mesh->v[mesh->v_size + 1], &mesh->v[mesh->v_size + 2]);

						// Debugging
						// printf("%f, %f, %f\n", v[mesh->v_size].x, v[mesh->v_size].y, v[mesh->v_size].z);
						
						// Increment the number of vertices
						mesh->v_size += 3;
					break;
				}
			break;

			case 'f':
				if(mesh->num_faces == 0){
					num_attributes = GetNumFaceElements(strchr(line_buffer, ' ') + 1);
					printf("num_attribs: %d\n", num_attributes);
					if(num_attributes == 2){
						if(texture_indices){
							num_components_per_vertex = 5;
						}else if(normal_indices){
							num_components_per_vertex = 6;
						}
					}else if(num_attributes == 3){
						// num_components_per_vertex = 8;
						//tmp
						num_components_per_vertex = 9;
					}else{
						num_components_per_vertex = 3;
					}
				}
				// allocate space: for each face there are 3 vertices, each vertex consists of 'num_attributes' attributes, each element is a float
				int num_polygon_verts = CountChars(strchr(line_buffer, ' ') + 1, ' ') + 1;
				if(num_polygon_verts == 4){

				// Disable loading of anything more than quads
				}else if(num_polygon_verts != 3){
					DebugLog(D_WARN, "OBJ face parsing failed '%s'. Cannot parse n-gons with more than 4 vertices", path);
					num_polygon_verts = 3;
				}

				mesh->f = realloc(mesh->f, (mesh->num_vertices + (num_polygon_verts - 2) * 3) * num_attributes * sizeof(float));

				char vertex_string[128];

				// Orders face data {{x, y, z}, {u, v, w}, {Nx, Ny, Nz}}
				for(int attribute = 0; attribute < num_attributes; attribute++){
					strcpy(vertex_string, GetFaceIndex(strchr(line_buffer, ' ') + 1, ' ', attribute));
					// To parse triangles simply retrieve vertices
					if(num_polygon_verts == 3){
						for(int vert = 0; vert < 3; vert++){
							mesh->f[mesh->num_faces * num_attributes * 3 + attribute * 3 + vert] = strtol(GetFaceIndex(vertex_string, '/', vert), NULL, 10);
						}
					
					// To parse quads, first convert to triangles
					}else{
						printf("QUAD!\n");
						// Get quad vertices
						unsigned int v1 = strtol(GetFaceIndex(vertex_string, '/', 0), NULL, 10);
						unsigned int v2 = strtol(GetFaceIndex(vertex_string, '/', 1), NULL, 10);
						unsigned int v3 = strtol(GetFaceIndex(vertex_string, '/', 2), NULL, 10);
						unsigned int v4 = strtol(GetFaceIndex(vertex_string, '/', 3), NULL, 10);
						
						// Triangle 1
						mesh->f[mesh->num_faces * num_attributes * 3 + attribute * 3 + 0] = v1;
						mesh->f[mesh->num_faces * num_attributes * 3 + attribute * 3 + 1] = v2;
						mesh->f[mesh->num_faces * num_attributes * 3 + attribute * 3 + 2] = v3;
						
						// Triangle 2
						mesh->f[(mesh->num_faces + 1) * num_attributes * 3 + attribute * 3 + 0] = v3;
						mesh->f[(mesh->num_faces + 1) * num_attributes * 3 + attribute * 3 + 1] = v4;
						mesh->f[(mesh->num_faces + 1) * num_attributes * 3 + attribute * 3 + 2] = v1;
					}
				}
				mesh->num_vertices += (num_polygon_verts - 2) * 3;
				mesh->num_faces += num_polygon_verts - 2;
			break;

			case 'o':
				mesh->num_meshes++;
				break;
		}
	}
	free(line_buffer);
	fclose(file);
	printf("num meshes: %d\n", mesh->num_meshes);

	// 3 vertices per face
	// (f, (mesh->num_faces + 1) * 3 * num_components_per_vertex * sizeof(float));
	mesh->data = malloc((mesh->num_vertices + 1) * num_components_per_vertex * sizeof(float));

	// printf("num v: %d\nnum vt: %d\nnum vn: %d\nnum f: %d\n", v_size, vt_size, vn_size, mesh->num_faces);

	for(int vert = 0; vert < mesh->num_vertices; vert++){
		// printf("v %d: ", vert);
		for(int attrib = 0; attrib < num_attributes; attrib++){
			for(int i = 0; i < 3; i++){
				if(attrib == 0){// v
					mesh->data[vert * num_components_per_vertex + attrib * 3 + i] =  mesh->v[(mesh->f[vert * num_attributes + attrib] - 1) * 3 + i];
				}else if(attrib == 1){// vt
					mesh->data[vert * num_components_per_vertex + attrib * 3 + i] = mesh->vt[(mesh->f[vert * num_attributes + attrib] - 1) * 3 + i];
				}else if(attrib == 2){// vn
					mesh->data[vert * num_components_per_vertex + attrib * 3 + i] = mesh->vn[(mesh->f[vert * num_attributes + attrib] - 1) * 3 + i];
				}
				// printf("%f /", mesh->data[vert * num_components_per_vertex + attrib * 3 + i]);
				// printf("%f /", mesh->data[vert * num_components_per_vertex + attrib * 3 + 1]);
				// printf("%f /", mesh->data[vert * num_components_per_vertex + attrib * 3 + 2]);
				// printf("%d /", vert * num_attributes + attrib);
			}
			// printf("%d /", f[face].v[vert].v[attrib]);
			// if(attrib == 2){
			// 	printf("\n");
			// printf("%f /", v[f[vert * num_attributes + attrib] - 1].v[i]);
		}
	}
	// mesh->num_vertices = mesh->num_faces * 3;
	mesh->data_size = mesh->num_vertices * num_components_per_vertex;

	// char string[] = "2/14/7 21/13/7 13/17/7 17/18/7 4/19/7 10/20/7 9/21/7";
	// printf("num spaces: %d\n", CountChars(string, ' '));
}

void FreeObj(MeshObject *mesh){
	free(mesh->data);
	free(mesh->v);
	free(mesh->vt);
	free(mesh->vn);
	free(mesh->f);
	mesh->data_size = 0;
	mesh->v_size = 0;
	mesh->vt_size = 0;
	mesh->vn_size = 0;
	mesh->f_size = 0;
	mesh->num_vertices = 0;
	mesh->num_faces = 0;
	mesh->num_meshes = 0;
}

void LoadMtl(const char *path){

}