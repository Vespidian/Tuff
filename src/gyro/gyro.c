#include "../global.h"
#include "../gl_context.h"
#include "../event.h"
#include "serial.h"

uint8_t buffer[14];
bool found_data = false;

uint8_t start_byte;
float gyro_x;
float gyro_y;
float gyro_z;
short checksum;
short calculated_checksum;

Vector3 rot = {0, 0, 0};
double PI = 3.141592653589793;

Vector3 rotation_offset = {0, 0, 0};

void ResetRotation(){
	glm_vec3_zero(rotation_offset.v);
	glm_vec3_copy(rot.v, rotation_offset.v);
}

void InitGyro(){
	// SerialConnect(CBR_115200, 8, ONESTOPBIT, NOPARITY);
	// BindKeyEvent(ResetRotation, 'j', SDL_KEYDOWN);
}

typedef union{
	float f;
	char c[4];
}input_float;

void LoopGyro(){
	// while(buffer[0] != 170){
		// Status = ReadFile(hComm, &buffer[0], 11, &NoBytesRead, NULL);

	// }
	
	// if((unsigned char)buffer[0] == 170){
	// 	// Status = ReadFile(hComm, &buffer[1], 8, &NoBytesRead, NULL);
	// 	found_data = true;
	// }
	// input_float x = {0}	;
	// input_float y = {0}	;
	// input_float z = {0}	;

	// printf("%s", buffer);
	// // if(found_data){
	// start_byte = (unsigned char)buffer[0];
	// x.c[0] = buffer[1];
	// x.c[1] = buffer[2];
	// x.c[2] = buffer[3];
	// x.c[3] = buffer[4];

	// y.c[0] = buffer[5];
	// y.c[1] = buffer[6];
	// y.c[2] = buffer[7];
	// y.c[3] = buffer[8];

	// z.c[0] = buffer[9];
	// z.c[1] = buffer[10];
	// z.c[2] = buffer[11];
	// z.c[3] = buffer[12];
	// gyro_x = buffer[1] + (buffer[2] << 8) + (buffer[3] << 16) + (buffer[4] << 24);
	// gyro_x = buffer[1] + (buffer[2] << 8) + (buffer[3] << 16) + (buffer[4] << 24);
	// gyro_y = buffer[5] | (buffer[6] << 8) | (buffer[7] << 16) | (buffer[8] << 24);
	// gyro_z = buffer[9] | (buffer[10] << 8) | (buffer[11] << 16) | (buffer[12] << 24);
	// gyro_x = buffer[4] | (buffer[3] << 8) | (buffer[2] << 16) | (buffer[1] << 24);
	// gyro_y = buffer[8] | (buffer[7] << 8) | (buffer[6] << 16) | (buffer[5] << 24);
	// gyro_z = buffer[12] | (buffer[11] << 8) | (buffer[10] << 16) | (buffer[9] << 24);
	// checksum = buffer[7] + (buffer[8] << 9);
	// calculated_checksum = ((gyro_x & gyro_y) ^ gyro_z);
	// for(int i = 0; i < 14; i++){
	// 	printf("%d\n", buffer[i]);
// 
	// }
	// printf("x: %f\ny: %f\nz: %f\n", gyro_x, gyro_y, gyro_z);
	// printf("x: %f\ny: %f\nz: %f\n", x.f, y.f, z.f);
	// printf("%f\n", x.f);
	// printf("checksum: %d\n", checksum);
	// printf("start_byte: %d\n", start_byte);
	// printf("calc_checksum: %d\n", calculated_checksum);
	// printf("byte_recieved: %d\n", (char)buffer[7]);
	// printf("byte_calc: %d\n", (char)(((gyro_x & gyro_y) ^ gyro_z) & 255));
	// printf("byte_recieved: %d\n", (char)buffer[8]);
	// printf("byte_calc: %d\n", (char)(((gyro_x & gyro_y) ^ gyro_z) >> 8));
	// if(buffer[7] == (((gyro_x & gyro_y) ^ gyro_z) & 255)){
		// rot.x = ((2 * PI) / (32768 - -32768)) * (gyro_x - -32768);
		// rot.x = gyro_x / ((gyro_x + 32768) / (2.0 * PI));
		// rot.y = gyro_y / ((gyro_y + 32768) / (2.0 * PI));
		// rot.z = gyro_z / ((gyro_z + 32768) / (2.0 * PI));
		// rot.x = glm_rad(x.f);
		// rot.y = glm_rad(z.f);
		// rot.z = glm_rad(y.f);
		// glm_vec3_sub(rot.v, rotation_offset.v, rot.v);
		// printf("update\n");
		// found_data = false;
	// }
	// printf("%f, %f, %f\n", rot.x, rot.y, rot.z);

}