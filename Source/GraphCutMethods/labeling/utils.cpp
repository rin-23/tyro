#include "utils.h"
#include <iostream>
#include <fstream>
#include <igl/read_triangle_mesh.h>
#include <random>
#include <chrono>
#include <string>
#include <Eigen/Dense>
#include <igl/write_triangle_mesh.h>
#include <igl/readPLY.h>
#include <igl/read_triangle_mesh.h>

using namespace Eigen;
using namespace std;

bool loadMatrix(std::string filename, Eigen::MatrixXd& m)
{
	// General structure
	// 1. Read file contents into vector<double> and count number of lines
	// 2. Initialize matrix
	// 3. Put data in vector<double> into matrix

	std::ifstream input(filename.c_str());
	if (input.fail())
	{
		std::cerr << "ERROR. Cannot find file '" << filename << "'." << std::endl;
		assert(false);
		return false;
	}
	std::string line;
	double d;

	std::vector<double> v;
	int n_rows = 0;
	while (getline(input, line))
	{	
		if (line[0] == '#') //skip comments
			continue;

		++n_rows;
		std::stringstream input_line(line);
		while (!input_line.eof())
		{
			input_line >> d;
			v.push_back(d);
		}
	}
	input.close();

	int n_cols = v.size() / n_rows;
	m.resize(n_rows, n_cols);

	for (int i = 0; i < n_rows; i++)
		for (int j = 0; j < n_cols; j++)
			m(i, j) = v[i*n_cols + j];

	return true;
}

//template <typename DerivedV>
bool saveMatrix(std::string filename, Eigen::VectorXd& matrix, bool overwrite)
{
	std::ofstream file;
	file.open(filename.c_str());
	if (!file.is_open())
	{
		std::cerr << "Couldn't open file '" << filename << "' for writing." << std::endl;
		return false;
	}

	file << std::fixed;
	file << matrix;
	file.close();

	return true;
}

//template <typename DerivedV>
bool saveMatrix(std::string filename, Eigen::VectorXi& matrix, bool overwrite)
{
	std::ofstream file;
	file.open(filename.c_str());
	if (!file.is_open())
	{
		std::cerr << "Couldn't open file '" << filename << "' for writing." << std::endl;
		return false;
	}

	file << std::fixed;
	file << matrix;
	file.close();

	return true;
}

bool saveMatrix(std::string filename, Eigen::MatrixXd& matrix, bool overwrite)
{
	std::ofstream file;
	file.open(filename.c_str());
	if (!file.is_open())
	{
		std::cerr << "Couldn't open file '" << filename << "' for writing." << std::endl;
		return false;
	}

	file << std::fixed;
	file << matrix;
	file.close();

	return true;
}

/*
void loadMatricies(Eigen::MatrixXd*& verticies, Eigen::MatrixXi& F, int num_files, char** files)
{
	verticies = new Eigen::MatrixXd[num_files];

	for (int i = 0; i < num_files; ++i)
	{
		igl::read_triangle_mesh(files[i], verticies[i], F);
	}
}
*/

bool loadFrames(const std::string& objPath, Eigen::MatrixXd& FRAMES, Eigen::MatrixXd& SAVED_FACES)
{	
	std::ifstream input(objPath.c_str());
	if (input.fail())
	{
		std::cerr << "ERROR. Cannot find file '" << objPath << "'." << std::endl;
		assert(false);
		return false;
	}
	//double d;
	//std::vector<double> v;
	//int n_rows = 0;

	std::string line;
	std::vector<VectorXd> frames;

	while (getline(input, line))
	{
		if (line[0] == '#') //skip comments
			continue;

		Eigen::MatrixXd V;
		std::cout << line << "\n";
		
		//V = V_temp - V_neut;
		//FILE * fp = fopen(line.c_str(), "r+");
		//igl::read_triangle_mesh(ext, fp, V, SAVED_FACES);
		//fclose(fp);
		//if (ext == "ply") {
		//igl::readPLY(line, V, SAVED_FACES);
		//}
		//else if (ext == "obj")
	//	{
		igl::read_triangle_mesh(line, V, SAVED_FACES);
		//}

		Matrix<double, Dynamic, Dynamic, RowMajor> M2(V);
		Map<VectorXd> v2(M2.data(), M2.size());
		VectorXd b2 = v2;
		frames.push_back(b2);
	}

	int numFrames= frames.size();
	int numVerticies = frames[0].size();

	FRAMES.resize(numVerticies, numFrames);

	int cindex = 0;
	for (auto& bb : frames)
	{
		FRAMES.col(cindex++) = bb;
	}

	input.close();

	return true;
}

bool saveDictionary(const std::string& labelOBJ, Eigen::MatrixXd& D, Eigen::MatrixXd& SAVED_FACES)
{
	int numLabels = D.cols();
	for (int i = 0; i < numLabels; ++i) 
	{
		VectorXd objCol = D.col(i);
		MatrixXd V = Map<Matrix<double,Dynamic, Dynamic,RowMajor>>(objCol.data(), D.rows()/3, 3);
		std::string save_path = labelOBJ + std::string("/frame") + std::to_string(i) + std::string(".obj");
		igl::write_triangle_mesh(save_path, V, SAVED_FACES);
	}
}



bool loadBlendshapeTargets(const std::string& objPath, const std::string& neut_path, Eigen::MatrixXd& B)
{
	std::ifstream input(objPath.c_str());
	if (input.fail())
	{
		std::cerr << "ERROR. Cannot find file '" << objPath << "'." << std::endl;
		assert(false);
		return false;
	}
	//double d;
	//std::vector<double> v;
	//int n_rows = 0;

	std::string line;
	std::vector<VectorXd> bshapes;
	
	Eigen::MatrixXd V_neut, F_neut;
	igl::read_triangle_mesh(neut_path, V_neut, F_neut);

	while (getline(input, line))
	{
		if (line[0] == '#') //skip comments
			continue;

		//++n_rows;
		//Eigen::MatrixXd b;
		//loadMatrix(line, b);
		Eigen::MatrixXd V, F_temp, V_temp;
		std::cout << line << "\n";
		igl::read_triangle_mesh(line, V_temp, F_temp);
		V = V_temp - V_neut;
		
		Matrix<double, Dynamic, Dynamic, RowMajor> M2(V);
		Map<VectorXd> v2(M2.data(), M2.size());
		VectorXd b2 = v2;
		bshapes.push_back(b2);
	}

	int numBshapes = bshapes.size();
	int numVerticies = bshapes[0].size();

	B.resize(numVerticies, numBshapes);

	int cindex = 0;
	for (auto& bb : bshapes) 
	{
		B.col(cindex++) = bb;
	}

	input.close();

	return true;
}

void printMatrix(const Eigen::MatrixXd& matrix, std::string& header) 
{	
	bool debug = 0;
	if (debug)
	{
		std::cout << header << "\n";
		std::cout << matrix << "\n";
	}
}

int generateRandomNumbers(int** numbers, int nSimulations, int numFrames)
{
	*numbers = new int[nSimulations];
	
	std::random_device rd;     // only used once to initialise (seed) engine
	std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
	std::uniform_int_distribution<int> uni(0, numFrames-1); // guaranteed unbiased

	for (int n = 0; n < nSimulations; ++n) 
	{
		int k = uni(rng);
		//std::cout << std::to_string(k) << " ";
		(*numbers)[n] = k;
	}
	//std::cout << "\n";
	return 0;
}


std::string getFileName(const std::string& s) {

	char sep = '/';
	
	size_t i = s.rfind(sep, s.length());
	if (i != std::string::npos) {
		return(s.substr(i + 1, s.length() - i));
	}

	return("");
}


int convertOFFtoOBJ(std::string& objPath)
{
	std::ifstream input(objPath.c_str());
	if (input.fail())
	{
		std::cerr << "ERROR. Cannot find file '" << objPath << "'." << std::endl;
		assert(false);
		return false;
	}

	std::string line;
	while (getline(input, line))
	{
		if (line[0] == '#') //skip comments
			continue;

		Eigen::MatrixXd V, F;
		std::cout << line << "\n";

		igl::read_triangle_mesh(line, V, F);
		
		//string saveto = folderPath + string("/") + getFileName(objPath) + string(".obj");

		igl::write_triangle_mesh(line + string(".obj"), V, F, true);

	}
}