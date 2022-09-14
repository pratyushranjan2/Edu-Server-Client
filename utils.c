#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

// function to return instruction code on getting string instruction
int get_itr_code(char *itr) {
	if (strcmp(itr, ADD_TEACHER) == 0) {
		return CODE_ADD_TEACHER;
	}
	else if (strcmp(itr, DEL_TEACHER) == 0) {
		return CODE_DEL_TEACHER;
	}
	else if (strcmp(itr, ADD_COURSE) == 0) {
		return CODE_ADD_COURSE;
	}
	else if (strcmp(itr, DEL_COURSE) == 0) {
		return CODE_DEL_COURSE;
	}
	return CODE_INVALID;
}
