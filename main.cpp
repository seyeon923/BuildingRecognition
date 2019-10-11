#include <vector>
#include <cstdio>
#include <iostream>

#include "gis.hpp"
#include "mysql.hpp"

// windows specific
#include <io.h>
#include <fcntl.h>

using namespace std;

int main() {
	//// windows specific
	//_setmode(_fileno(stdout), _O_U16TEXT);

	//mysqlTest();
	//gisTest();

	transformTest();

	return 0;
}

