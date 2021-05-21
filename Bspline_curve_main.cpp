
#ifdef _WIN32
#include <Windows.h>
#endif

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include "opencv2/opencv_modules.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/opencv.hpp>
#include "include/GL/glew.h"		
#include "include/GLFW/glfw3.h" 
#include "include/glm/glm.hpp" 
#include <vector>
#include <math.h>
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "lib/glew32.lib")
#pragma comment(lib, "lib/glfw3.lib")

#ifdef HAVE_OPENCV_XFEATURES2D
#include "opencv2/xfeatures2d/nonfree.hpp"
#endif


#define C(msg) std::cout << msg
#define CN(msg) std::cout << msg << std::endl
#define num_piece 7
using namespace std;
using namespace cv;
using namespace cv::detail;
using namespace glm;


vector<double> knots;		// knot sequence vector

vector<Point3f> control = {
   { 100,400,0 },
   {  150,580,20 },
   { 250,350,00 },
   {500,500,-20},
   {640,700,0},
   {640,850,2},
   {980,620,0}
};

unsigned int num_ctr = num_piece; // the number of control points 
unsigned int degree = 3;// B-spline curve of degree
unsigned int order = degree + 1;
unsigned int num_knots = num_ctr + order;// knot = num_ctr + degree + 1
unsigned int LOD = 2000;
double uinc = 0.001;	// u parameter increment (for B-spline curve)


void generateKnots() {
	knots.clear();
	int times = 0;
	for (int i = 0; i < degree; i++) {
		knots.push_back(0);
	}
	int middle = num_ctr - degree + 1;
	for (int i = 0; i < middle; i++) {
		knots.push_back(static_cast<float>(i));
		times = static_cast<float>(i);
	}
	for (int i = 0; i < degree; i++) {
		knots.push_back(times);
	}
}


//------------------------------------------------------------	CoxDeBoor()
double CoxDeBoor(double u, int i, int k, vector<double> Knots) {
	if (k == 1)
	{
		if (Knots[i] <= u && u <= Knots[i + 1]) {
			return 1.0f;
		}
		return 0.0f;
	}
	double Den1 = Knots[i + k - 1] - Knots[i];
	double Den2 = Knots[i + k] - Knots[i + 1];
	double Eq1 = 0, Eq2 = 0;
	if (Den1 > 0) {
		Eq1 = ((u - Knots[i]) / Den1) * CoxDeBoor(u, i, k - 1, Knots);
	}
	if (Den2 > 0) {
		Eq2 = (Knots[i + k] - u) / Den2 * CoxDeBoor(u, i + 1, k - 1, Knots);
	}
	return Eq1 + Eq2;
}

Point3f GetOutpoint(double t) {
	Point3f OutPoint(1, 0, 0);
	// sum the effect of all CV's on the curve at this point to 
	// get the evaluated curve point
	// 
	for (unsigned int i = 0; i != num_ctr; ++i) {
		// calculate the effect of this point on the curve
		double Val = CoxDeBoor(t, i, order, knots);
		if (Val > 0.001f) {
			// sum effect of CV on this part of the curve
			OutPoint.x += Val * control[i].x;
			OutPoint.y += Val * control[i].y;
			OutPoint.z += Val * control[i].z;
		}
	}
	return   OutPoint;
}


vector<Point3f>render() {
	// populate (standard) knot sequence
	vector<Point3f> rendered_point;
	generateKnots();
	// generate B-spline
	for (int i = 0; i != LOD; ++i) {
		float t = knots[num_knots - 1] * i / (float)(LOD - 1);
		CN(t);
		if (i == LOD - 1)
			t -= 0.001f;
		Point3f point;
		point = GetOutpoint(t);
		rendered_point.push_back(point);
	}
	return rendered_point;
}

int main() {
	int width = 1000;
	int height = 1000;
	Mat img(height, width, CV_8UC3, Scalar(0, 0, 0));
	for (size_t i = 0; i < num_ctr; i++) {
		Point r(0, 0);
		r.x = control[i].x;
		r.y = control[i].y;
		circle(img, r, 5, Scalar(100, 100, 0), -1);
	}
	
	vector<Point3f> rendered_point;
	rendered_point = render();
	CN(rendered_point.size());
	for (size_t i = 0; i < rendered_point.size(); i++) {
		Point r(0, 0);
		r.x = rendered_point[i].x;
		r.y = rendered_point[i].y;
		circle(img, r, 1, Scalar(0, 100, 255), -1);
		C(".");
	}
	imshow("result", img);
	waitKey(0);
	return 0;

}

