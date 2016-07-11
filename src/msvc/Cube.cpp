#include "Cube.h"



Cube::Cube()
{


}


Cube::~Cube()
{
	if (mesh)
		delete(mesh);
}
