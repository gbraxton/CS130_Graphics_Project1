// Name: George Braxton
// Quarter Fall, Year: 2014
// Project 1
//
// This file is to be modified by the student.
// main.cpp
//---------CONTROLS-------------------
// Pan: arrow keys
// Zoom in: '+'
// Zoom out: '-'
// Rotate about x: 'x' (positive) 'X' (negative)
// Rotate about y: 'y' (positive) 'Y' (negative)
// Rotate about z: 'z' (positive) 'Z' (negative)
// Toggle wirefram mode: 'w'
//-----------------------------------------------
////////////////////////////////////////////////////////////
#include <algorithm>
#include <cmath>
#include <fstream>
#include <GL/glut.h>
#include <limits>
#include <stdio.h>
#include <vector>
// point3d.h is a modification of the provided point2d.h for 3-space.
#include "point3d.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const int ZMAX = INT_MAX; //each value in z buffer initialized to ZMAX to guarantee first entry is drawn
// transformations are increments by the following steps
const float ROTATION_STEP = M_PI/16.0;
const float PAN_STEP = 25.0;
const float ZOOM_STEP = 3.0/4.0;
const double PI = M_PI;

struct Triangle{
	Point3D p1, p2, p3;
	Triangle(Point3D p1, Point3D p2, Point3D p3): p1(p1), p2(p2), p3(p3){};
};
//Array for that is cycled through to give each rendered triangle a different color
GLfloat colors[10][3] = { {0.0, 0.0, 1.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 1.0}, {1.0, 0.0, 1.0}, {1.0, 1.0, 0.0}, {1.0, 1.0, 1.0}, {0.5, 0.5, 1.0}, {0.5, 1.0, 0.5}, {1.0, 0.5, 0.5} };

std::vector<Triangle> triangles; // holds all triangles in model
Point3D modelCenter; //computed after model file is loaded

int** zbuffer = new int*[WINDOW_WIDTH+1]; //array for buffer

bool wireframeMode = false;
// Triangles without a flat top or bottom are rendered bottom half then top half
// vertexInterpolation is used to know when this is the case so that
// wireframes are drawn for the interpolated vertex in the middle of the triangle
bool vertexInterpolation = false;

void renderPixel(int x, int y, int z=0)
{
	glBegin(GL_POINTS);
	if(zbuffer[x][y] >= z){
		glVertex2i(x, y);
		zbuffer[x][y] = z;
	}
	glEnd();
}

Point3D* getSortedPoints(Point3D p1, Point3D p2, Point3D p3){
	Point3D unsorted[3] = {p1, p2, p3};
	int lowestY = 0;
	int middleY = 1;
	int highestY = 2;
	if(unsorted[middleY].y < unsorted[lowestY].y){
		int temp = lowestY;
		lowestY = middleY;
		middleY = temp;
	}
	if(unsorted[highestY].y < unsorted[middleY].y){
		int temp = middleY;
		middleY = highestY;
		highestY = temp;
	}
	if(unsorted[middleY].y < unsorted[lowestY].y){
		int temp = lowestY;
		lowestY = middleY;
		middleY = temp;
	}
	Point3D* sorted = new Point3D[3];
	sorted[0] = unsorted[lowestY];
	sorted[1] = unsorted[middleY];
	sorted[2] = unsorted[highestY];
	return sorted;
}

// renders a triangle with a flat top
void flatTopTriangle(Point3D bottom, Point3D middle, Point3D top){
	Point3D left = top, right = middle;
	if(left.x > right.x){
		left = middle;
		right = top;
	}
	float leftSlope = (left.y-bottom.y)/(left.x-bottom.x);
	float rightSlope = (right.y-bottom.y)/(right.x-bottom.x);
	float vertZSlope = (left.y-bottom.y)/(left.z-bottom.z);
	float horiZSlope = (left.x-right.x)/(left.z-right.z);
	float nextLeft = bottom.x;
	float nextRight = bottom.x;
	float nextVertZ = bottom.z;
	int scanline = (int)(bottom.y+0.5);
	int endscan = (int)(top.y+0.5);
	while(scanline <= endscan){
		float nextHoriZ = nextVertZ;
		for(int xPixel = (int)(nextLeft+0.5); xPixel <= (int)(nextRight+0.5); xPixel++){
			if( (xPixel>=0) && (xPixel<=WINDOW_WIDTH) && (scanline>=0) && (scanline<=WINDOW_HEIGHT) ){
				//check for wireframe mode if so just render edges
				if(!wireframeMode || ((scanline==endscan) && !vertexInterpolation) || (xPixel == (int)(nextLeft+0.5)) || (xPixel==(int)(nextRight+0.5))){
					renderPixel(xPixel, scanline, (int)(nextHoriZ+0.5));
				}
			}
			nextHoriZ += 1.0/horiZSlope;
		}
		nextLeft += 1.0/leftSlope;
		nextRight += 1.0/rightSlope;
		nextVertZ += 1.0/vertZSlope;
		scanline++;
	}
}

// renders a flat bottom triangle
void flatBottomTriangle(Point3D left, Point3D top, Point3D right){
	float leftSlope = (top.y-left.y)/(top.x-left.x);
	float rightSlope = (top.y-right.y)/(top.x-right.x);
	float vertZSlope = (top.y-left.y)/(top.z-left.z);
	float horiZSlope = (left.x-right.x)/(left.z-right.z);
	float nextLeft = left.x;
	float nextRight = right.x;
	float nextVertZ = left.z;
	int scanline = (int)(left.y+0.5);
	int endscan = (int)(top.y+0.5);
	while(scanline <= endscan){
		float nextHoriZ = nextVertZ;
		for(int xPixel = (int)(nextLeft+0.5); xPixel <= (int)(nextRight+0.5); xPixel++){
			if( (xPixel>=0) && (xPixel<=WINDOW_WIDTH) && (scanline>=0) && (scanline<=WINDOW_HEIGHT) ){
				//check for wireframe mode if so just render edges
				if(!wireframeMode || ((scanline==(int)(left.y+0.5)) && !vertexInterpolation) || (xPixel == (int)(nextLeft+0.5)) || (xPixel==(int)(nextRight+0.5)) ){
					renderPixel(xPixel, scanline, (int)(nextHoriZ+0.5));
				}
			}
			nextHoriZ += 1.0/horiZSlope;
		}
		nextLeft += 1.0/leftSlope;
		nextRight += 1.0/rightSlope;
		nextVertZ += 1.0/vertZSlope;
		scanline++;
	}
}

void triangleRender(Point3D p1, Point3D p2, Point3D p3){
	//sort points by y coordinate
	Point3D* points = getSortedPoints(p1, p2, p3);
	if(points[0].y == points[1].y){
		vertexInterpolation = false;
		Point3D left = points[0], right = points[1];
		if(left.x > right.x){
			left = points[1];
			right = points[0];
		}
		flatBottomTriangle(left, points[2], right);
	} else if(points[1].y == points[2].y){
		vertexInterpolation = false;
		flatTopTriangle(points[0], points[1], points[2]);
	} else {
		vertexInterpolation = true;
		// here I find the point between the top and bottom points to create two separate triangles to draw (one flat top, one flat bottom)
		float xDiff = points[2].x - points[0].x;
		float zDiff = points[2].z - points[0].z;
		float midX = points[0].x;
		float midZ = points[0].z;
		if(xDiff != 0.0){
			midX = points[0].x + (xDiff/(points[2].y - points[0].y))*(points[1].y-points[0].y);
		}
		if(zDiff != 0.0){
			midZ = points[0].z + (zDiff/(points[2].y - points[0].y))*(points[1].y-points[0].y);
		}
		Point3D centerMid(midX, points[1].y, midZ);
		//flatTop
		flatTopTriangle(points[0], points[1], centerMid);
		//flatBottom
		Point3D left = centerMid, right = points[1];
		if(left.x > right.x){
			left = points[1];
			right = centerMid;
		}
		flatBottomTriangle(left, points[2], right);
	}
}

void clearZBuffer(){
	for(int i = 0; i < WINDOW_WIDTH+1; i++){
		for(int j = 0; j < WINDOW_HEIGHT+1; j++){
			zbuffer[i][j] = ZMAX;
		}
	}
}

void computeModelCenter(){
	int totalX = 0, totalY = 0, totalZ = 0;
	for(int i = 0; i < triangles.size(); i++){
		totalX += (int)triangles[i].p1.x;
		totalX += (int)triangles[i].p2.x;
		totalX += (int)triangles[i].p3.x;
		totalY += (int)triangles[i].p1.y;
		totalY += (int)triangles[i].p2.y;
		totalY += (int)triangles[i].p3.y;
		totalZ += (int)triangles[i].p1.z;
		totalZ += (int)triangles[i].p2.z;
		totalZ += (int)triangles[i].p3.z;
	}
	modelCenter.x = ((float)totalX)/((float)triangles.size()*3.0);
	modelCenter.y = ((float)totalY)/((float)triangles.size()*3.0);
	modelCenter.z = ((float)totalZ)/((float)triangles.size()*3.0);
}

void polygonRender(){
	clearZBuffer();
	for(int i = 0; i < triangles.size(); i++){
		glColor3f(colors[i%10][0], colors[i%10][1], colors[i%10][2]);
		triangleRender(triangles[i].p1, triangles[i].p2, triangles[i].p3);
	}
}

void GL_render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    polygonRender();
    glutSwapBuffers();
}

void rotateX(bool positive){
	float multiplier = positive ? 1.0 : -1.0;
	float rotation = ROTATION_STEP * multiplier;
	double xCoord, yCoord, zCoord;
	for(int i = 0; i < triangles.size(); i++){
		//first translate to origin
		triangles[i].p1.x -= modelCenter.x;
		triangles[i].p2.x -= modelCenter.x;
		triangles[i].p3.x -= modelCenter.x;
		triangles[i].p1.y -= modelCenter.y;
		triangles[i].p2.y -= modelCenter.y;
		triangles[i].p3.y -= modelCenter.y;
		triangles[i].p1.z -= modelCenter.z;
		triangles[i].p2.z -= modelCenter.z;
		triangles[i].p3.z -= modelCenter.z;
		//apply rotation
		yCoord = triangles[i].p1.y;
		zCoord = triangles[i].p1.z;
		triangles[i].p1.y = cos(rotation)*yCoord - sin(rotation)*zCoord;
		triangles[i].p1.z = sin(rotation)*yCoord + cos(rotation)*zCoord;
		yCoord = triangles[i].p2.y;
		zCoord = triangles[i].p2.z;
		triangles[i].p2.y = cos(rotation)*yCoord - sin(rotation)*zCoord;
		triangles[i].p2.z = sin(rotation)*yCoord + cos(rotation)*zCoord;
		yCoord = triangles[i].p3.y;
		zCoord = triangles[i].p3.z;
		triangles[i].p3.y = cos(rotation)*yCoord - sin(rotation)*zCoord;
		triangles[i].p3.z = sin(rotation)*yCoord + cos(rotation)*zCoord;
		//translate back to center
		triangles[i].p1.x += modelCenter.x;
		triangles[i].p2.x += modelCenter.x;
		triangles[i].p3.x += modelCenter.x;
		triangles[i].p1.y += modelCenter.y;
		triangles[i].p2.y += modelCenter.y;
		triangles[i].p3.y += modelCenter.y;
		triangles[i].p1.z += modelCenter.z;
		triangles[i].p2.z += modelCenter.z;
		triangles[i].p3.z += modelCenter.z;
	}
}

void rotateY(bool positive){
	float multiplier = positive ? 1.0 : -1.0;
	float rotation = ROTATION_STEP * multiplier;
	double xCoord, yCoord, zCoord;
	for(int i = 0; i < triangles.size(); i++){
		//first translate to origin
		triangles[i].p1.x -= modelCenter.x;
		triangles[i].p2.x -= modelCenter.x;
		triangles[i].p3.x -= modelCenter.x;
		triangles[i].p1.y -= modelCenter.y;
		triangles[i].p2.y -= modelCenter.y;
		triangles[i].p3.y -= modelCenter.y;
		triangles[i].p1.z -= modelCenter.z;
		triangles[i].p2.z -= modelCenter.z;
		triangles[i].p3.z -= modelCenter.z;
		//apply rotation
		xCoord = triangles[i].p1.x;
		zCoord = triangles[i].p1.z;
		triangles[i].p1.x = cos(rotation)*xCoord + sin(rotation)*zCoord;
		triangles[i].p1.z = -sin(rotation)*xCoord + cos(rotation)*zCoord;
		xCoord = triangles[i].p2.x;
		zCoord = triangles[i].p2.z;
		triangles[i].p2.x = cos(rotation)*xCoord + sin(rotation)*zCoord;
		triangles[i].p2.z = -sin(rotation)*xCoord + cos(rotation)*zCoord;
		xCoord = triangles[i].p3.x;
		zCoord = triangles[i].p3.z;
		triangles[i].p3.x = cos(rotation)*xCoord + sin(rotation)*zCoord;
		triangles[i].p3.z = -sin(rotation)*xCoord + cos(rotation)*zCoord;
		//translate back to center
		triangles[i].p1.x += modelCenter.x;
		triangles[i].p2.x += modelCenter.x;
		triangles[i].p3.x += modelCenter.x;
		triangles[i].p1.y += modelCenter.y;
		triangles[i].p2.y += modelCenter.y;
		triangles[i].p3.y += modelCenter.y;
		triangles[i].p1.z += modelCenter.z;
		triangles[i].p2.z += modelCenter.z;
		triangles[i].p3.z += modelCenter.z;
	}
}

void rotateZ(bool positive){
	float multiplier = positive ? 1.0 : -1.0;
	float rotation = ROTATION_STEP * multiplier;
	double xCoord, yCoord, zCoord;
	for(int i = 0; i < triangles.size(); i++){
		//first translate to origin
		triangles[i].p1.x -= modelCenter.x;
		triangles[i].p2.x -= modelCenter.x;
		triangles[i].p3.x -= modelCenter.x;
		triangles[i].p1.y -= modelCenter.y;
		triangles[i].p2.y -= modelCenter.y;
		triangles[i].p3.y -= modelCenter.y;
		triangles[i].p1.z -= modelCenter.z;
		triangles[i].p2.z -= modelCenter.z;
		triangles[i].p3.z -= modelCenter.z;
		//apply rotation
		xCoord = triangles[i].p1.x;
		yCoord = triangles[i].p1.y;
		triangles[i].p1.x = cos(rotation)*xCoord - sin(rotation)*yCoord;
		triangles[i].p1.y = sin(rotation)*xCoord + cos(rotation)*yCoord;
		xCoord = triangles[i].p2.x;
		yCoord = triangles[i].p2.y;
		triangles[i].p2.x = cos(rotation)*xCoord - sin(rotation)*yCoord;
		triangles[i].p2.y = sin(rotation)*xCoord + cos(rotation)*yCoord;
		xCoord = triangles[i].p3.x;
		yCoord = triangles[i].p3.y;
		triangles[i].p3.x = cos(rotation)*xCoord - sin(rotation)*yCoord;
		triangles[i].p3.y = sin(rotation)*xCoord + cos(rotation)*yCoord;
		//translate back to center
		triangles[i].p1.x += modelCenter.x;
		triangles[i].p2.x += modelCenter.x;
		triangles[i].p3.x += modelCenter.x;
		triangles[i].p1.y += modelCenter.y;
		triangles[i].p2.y += modelCenter.y;
		triangles[i].p3.y += modelCenter.y;
		triangles[i].p1.z += modelCenter.z;
		triangles[i].p2.z += modelCenter.z;
		triangles[i].p3.z += modelCenter.z;
	}
}
// pan is a simple translation
void pan(float x, float y, float z){
	for(int i = 0; i < triangles.size(); i++){
		triangles[i].p1.x += x;
		triangles[i].p1.y += y;
		triangles[i].p1.z += z;
		triangles[i].p2.x += x;
		triangles[i].p2.y += y;
		triangles[i].p2.z += z;
		triangles[i].p3.x += x;
		triangles[i].p3.y += y;
		triangles[i].p3.z += z;
	}
	computeModelCenter();
}
// zoom is a scale transformatio
// when zooming in each point is multiplied by (2 * 3/4) or 1.5
// when zooming out points are multiplied by 3/4
void zoom(bool positive){
	float multiplier = positive ? 2.0 : 1.0;
	for(int i = 0; i < triangles.size(); i++){
		triangles[i].p1.x *= (ZOOM_STEP*multiplier);
		triangles[i].p1.y *= (ZOOM_STEP*multiplier);
		triangles[i].p1.z *= (ZOOM_STEP*multiplier);
		triangles[i].p2.x *= (ZOOM_STEP*multiplier);
		triangles[i].p2.y *= (ZOOM_STEP*multiplier);
		triangles[i].p2.z *= (ZOOM_STEP*multiplier);
		triangles[i].p3.x *= (ZOOM_STEP*multiplier);
		triangles[i].p3.y *= (ZOOM_STEP*multiplier);
		triangles[i].p3.z *= (ZOOM_STEP*multiplier);
	}
	computeModelCenter();
}

void keyboard(unsigned char key, int x, int y){
	switch(key){
		case 'x':
			rotateX(true);
			break;
		case 'X':
			rotateX(false);
			break;
		case 'y':
			rotateY(true);
			break;
		case 'Y':
			rotateY(false);
			break;
		case 'z':
			rotateZ(true);
			break;
		case 'Z':
			rotateZ(false);
			break;
		case 'w':
			wireframeMode = !wireframeMode;
			break;
		case '+':
			zoom(true);
			break;
		case '-':
			zoom(false);
			break;
		default:
			break;
	}
	glutPostRedisplay();
}

// This is the callback for arrow key events
void specialInput(int key, int x, int y){
	switch(key){
		case GLUT_KEY_UP:
			pan(0, -PAN_STEP, 0);
			break;
		case GLUT_KEY_DOWN:
			pan(0, PAN_STEP, 0);
			break;
		case GLUT_KEY_LEFT:
			pan(PAN_STEP, 0, 0);
			break;
		case GLUT_KEY_RIGHT:
			pan(-PAN_STEP, 0, 0);
			break;
		default:
			break;
	}
	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y){}

void GLInit(int* argc, char** argv)
{
	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("CS 130 - George Braxton: Project 1");
	glutDisplayFunc(GL_render);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialInput);
	glMatrixMode(GL_PROJECTION_MATRIX);
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
}

void loadFile(char* filename){
	std::ifstream file;
	file.open(filename);
	int numPoints;
	int numTriangles;
	file >> numPoints;
	file >> numTriangles;
	Point3D* points = new Point3D[numPoints];
	for(int i = 0; i < numPoints; i++){
		int x, y, z;
		file >> x;
		file >> y;
		file >> z;
		points[i] = Point3D((float)x, (float)y, (float)z);
	}
	for(int i=0; i < numTriangles; i++){
		int p1, p2, p3;
		file >> p1;
		file >> p2;
		file >> p3;
		Triangle newTriangle(points[p1], points[p2], points[p3]);
		triangles.push_back(newTriangle);
	}
	computeModelCenter();
	file.close();
}

void initZBuffer(){
	for(int i = 0; i < WINDOW_WIDTH+1; i++){
		zbuffer[i] = new int[WINDOW_HEIGHT+1];
	}
}

int main(int argc, char** argv)
{
	if(argc != 2){
		printf("invalid # of arguments. usage assn1 modelFile\n");
		exit(0);
	}	
	loadFile(argv[1]);
	initZBuffer();
	GLInit(&argc, argv);
	GL_render();
	glutMainLoop();

	return 0;
}
