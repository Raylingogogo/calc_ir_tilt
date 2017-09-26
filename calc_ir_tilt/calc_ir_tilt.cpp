// calc_ir_tilt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#define SIZE_640x480 307200
#define SIZE_352x352 123904
#define SIZE_416x416 173056
#define SKIP_LINES	 50

FILE *pFile1, *pFile2; // File1 = front, File2 = IR
long lSize1, lSize2;
char *buffer1, *buffer2;
size_t result;
char *file1_str;
char *file2_str;
int front_width, front_height, front_block_x, front_block_y;
int ir_width, ir_height, ir_block_x, ir_block_y;
unsigned int pixel_value;
int valid_count = 0;
int skip_pixels;
bool front_block_found = 0, ir_block_found = 0;

int main(int argc, char *argv[])
{
	if (argc < 5)
	{
		printf("Please follow format: calc_ir_tilt file1 file2 criteria_x criteria_y\n");
		return 0;
	}
	file1_str = argv[1];
	file2_str = argv[2];
	printf("file1=%s, file2=%s\n", file1_str, file2_str);
	fopen_s(&pFile1, file1_str, "rb");
	if (pFile1 == NULL) { fputs("File error1", stderr); return 1; }
	fopen_s(&pFile2, file2_str, "rb");
	if (pFile2 == NULL) { fputs("File error2", stderr); return 1; }


	// obtain file size:
	fseek(pFile1, 0, SEEK_END);
	lSize1 = ftell(pFile1);
	rewind(pFile1);

	fseek(pFile2, 0, SEEK_END);
	lSize2 = ftell(pFile2);
	rewind(pFile2);

	// Check the file size to decide width and height
	// Front Cam
	if (lSize1 == SIZE_640x480)
	{
		front_width = 640;
		front_height = 480;
	}
	else
	{
		printf("[ERROR] Abnormal Front Cam file size\n");
		return 1;
	}

	if (lSize2 == SIZE_416x416)
	{
		ir_width = 416;
		ir_height = 416;
	}
	else if (lSize2 == SIZE_352x352)
	{
		ir_width = 352;
		ir_height = 352;
	}
	else
	{
		printf("[ERROR] Abnormal IR Cam file size\n");
		return 1;
	}
	// allocate memory to contain the whole file:
	buffer1 = (char*)malloc(sizeof(char)*lSize1);
	if (buffer1 == NULL) { fputs("Memory1 error", stderr); return 1; }
	buffer2 = (char*)malloc(sizeof(char)*lSize2);
	if (buffer2 == NULL) { fputs("Memory2 error", stderr); return 1; }

	// copy the file into the buffer:
	result = fread(buffer1, 1, lSize1, pFile1);
	if (result != lSize1) 
	{ fputs("Reading1 error", stderr); return 1; }
	
	result = fread(buffer2, 1, lSize2, pFile2);
	if (result != lSize2) 
	{ fputs("Reading2 error", stderr); return 1; }

	// While loop to check the black block location. Note: skip 50 lines to prevent corner case.
	// Handle Front Cam First
	skip_pixels = SKIP_LINES * front_width;
	valid_count = 0;
	for (int i= skip_pixels; i<lSize1 - skip_pixels; i++)
	{
		pixel_value = (int) *(buffer1 + i);
		if (pixel_value < 10) // Criteria is below 10
		{
			valid_count++;
			//printf("%x\n", (int) *(buffer1 + i));
		}
		else
		{
			if (valid_count > 0) // Should continue 5 pixel to be highter than criteria
				valid_count--;
		}
		if (valid_count > 5) // The block is found. Break
		{
			front_block_y = i / front_width;
			front_block_x = i % front_width;
			printf("Front Cam block position = [%d, %d], Center pos= [%d, %d]\n", front_block_x, front_block_y, front_width/2, front_height/2);
			front_block_found = 1;
			break;
		}
	}
	if (front_block_found == 0)
	{
		printf("[ERROR] Front Block is not found\n");
		return 1;
	}

	// IR Cama
	skip_pixels = SKIP_LINES * ir_width;
	valid_count = 0;
	for (int i = skip_pixels; i<lSize2 - skip_pixels; i++)
	{
		pixel_value = (int) *(buffer2 + i);
		if (pixel_value < 10) // Criteria is below 10
		{
			valid_count++;
			//printf("%x\n", (int) *(buffer2 + i));
		}
		else
		{
			if (valid_count > 0) // Should continue 5 pixel to be highter than criteria
				valid_count--;
		}
		if (valid_count > 5) // The block is found. Break
		{
			ir_block_y = i / ir_width;
			ir_block_x = i % ir_width;
			printf("IR Cam block pos = [%d, %d]. Center pos= [%d, %d]\n", ir_block_x, ir_block_y, ir_width/2, ir_height/2);
			ir_block_found = 1;
			break;
		}
	}
	if (ir_block_found == 0)
	{
		printf("[ERROR] IR Block is not found\n");
		return 1;
	}

	// Check the PASS/FAIL. Criteria is that the relative position between center and block should be the same in between Front and IR.
	int front_diff_x = front_block_x - front_width / 2;
	int front_diff_y = front_block_y - front_height / 2;
	printf("Front to center Distance = [%d, %d]\n", front_diff_x, front_diff_y);
	
	int ir_diff_x = ir_block_x - ir_width / 2;
	int ir_diff_y = ir_block_y - ir_height / 2;
	printf("IR to center Distance = [%d, %d]\n", ir_diff_x, ir_diff_y);

	// If diff of front or ir is larger than criteria, then fail


	free(buffer1);
	free(buffer2);
	fclose(pFile1);
	fclose(pFile2);
    return 0;
}

