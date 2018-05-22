#include "TyroApp.h"

int main(int argc, char **argv) 
{	
	tyro::App appr;
	int r_code = appr.Launch();
	//int r_code = appr.VideoToImages();
	return r_code;
}



