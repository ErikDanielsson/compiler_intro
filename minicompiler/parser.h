#pragma once

void lr_parser(char verbose);
void parser_error(int length, const char* expected,
		  int fatal, int line, int column);
