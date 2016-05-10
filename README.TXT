README.TXT

Homework 3: Textured OBJ Viewer - Johann Wentzel

To compile, use "make" command.
To run, use the form "./standaloneExample.cpp <objfile>.obj <texfile>.tga <MTLfile.mtl>"

All code is original, barring provided skeleton code and that taught in tutorials.

All implemented features:
	- Compiles and runs in MS239
	- Parses and interprets OBJ files for vertices and faces, etc.
	- Parses and interprets MTL files for custom diffuse, ambient, and specular albedo values.
	- Stores the information to draw the object
	- Calculates normals for each vertex, and draws the object
	- Phong lighting, with adjustable specular reflectivity and intensity
	- Adjusts scaling automatically for non-clipped viewing.
	- Rotation of viewpoint about 3 axes: 
		- x (adjusted with W and S keys)
		- y (adjusted with A and D keys)
		- z (adjusted with Q and E keys)
	- Ability to toggle into wireframe mode (mouse left-click)
	- Ability to change scaling level (adjusted with UP and DOWN arrow keys)
	- Ability to change ambient lighting factor (adjusted tih T and G keys)
	- Ability to change specular power (adjusted with R and F keys)
	- Ability to change specular reflectivity (adjusted with Y and H keys)
		- Sub-adjust RED component of specular highlight with Z and X keys)
		- Sub-adjust GREEN component of specular highlight with C and V keys)
		- Sub-adjust BLUE component of specular highlight with B and N keys)
	- Ability to change intensity of ambient light (adjusted with P and ; keys)
		- Sub-adjust RED component of specular highlight with U and J keys)
		- Sub-adjust GREEN component of specular highlight with I and K keys)
		- Sub-adjust BLUE component of specular highlight with O and L keys)
	- Ability to switch texture filtering between bilinear, nearest-neighbor, and trilinear filtering.


	Controls:

	UP/DOWN: zoom in/out
	W/S: Rotate about x-axis
	A/D: Rotate about y-axis
	Q/E: Rotate about z-axis
	T/G: Change diffuse reflectivity
	Y/H: Change specular reflectivity
		- Z/X: Sub-adjust RED 	
		- C/V: Sub-adjust BLUE
		- B/N: Sub-adjust GREEN
	U/J: Change ambient light intensity
		- 	P/Semicolon: Sub-adjust RED
		- 	I/K: Sub-adjust GREEN
		- 	O/L: Sub-adjust BLUE
	1: Switch to nearest neighbor filtering.
	2: Switch to bilinear filtering.
	3: Switch to trilinear filtering.

	MOUSE LEFT-CLICK: Toggle wireframe mode