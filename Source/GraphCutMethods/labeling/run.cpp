#include "utils.h"
#include "labelingStep.h"
#include "updateStep.h"
#include "setup.h"
#include <algorithm>
#include <iostream>
#include <vector>
#include <chrono>
#include "point.h"
#include "kmeans.h"
#pragma warning(push, 0)
#include <igl\read_triangle_mesh.h>


#pragma warning(pop)

using namespace Eigen;
using namespace std;
using namespace std::chrono;
/*
void loadObjData(std::vector<MatrixXd>& verticies, MatrixXi& F)
{
	verticies.resize(NUM_DATA_OBJ);
	for (int i = 0; i < NUM_DATA_OBJ; ++i)
	{
		//MatrixXd vk;
		std::cout << "loaded data" << i << "\n";
		//igl::read_triangle_mesh(DATA_OBJ[i], verticies.at(i), F);
		//verticies.push_back(vk);
	}
}

void loadObjLabels(std::vector<MatrixXd>& verticies, MatrixXi& F)
{
	verticies.resize(NUM_LABELS_OBJ);
	for (int i = 0; i < NUM_LABELS_OBJ; ++i)
	{
		//MatrixXd vk;
		std::cout << "loaded label" << i << "\n";
		//igl::read_triangle_mesh(LABEL_OBJ[i], verticies.at(i), F);
		//verticies.push_back(vk);
	}
}
*/


void build_L_Matrix_Kmeans(std::string& fpath, MatrixXd& F, MatrixXd& L, int numLabels)
{
	std::vector<Point> points;
	KMeans::loadPoints(fpath, &points);
	KMeans kmeans(numLabels);
	kmeans.init(points);
	kmeans.run();
	//kmeans.printMeans();
	
	int numFrames = F.rows();
	int numBlendshapes = F.cols();
	L.resize(numLabels, numBlendshapes);
	L.setZero();

	for (int i = 0; i < numLabels; ++i) 
	{
		Point cluster = kmeans.means_[i];
		double* cluster_data = cluster.data_.data();
		Map<VectorXd> mapped(cluster_data, cluster.dimensions_);
		L.row(i) = mapped;
	}	
}

void build_L_Matrix(const MatrixXd& F, MatrixXd& L, int numLabels) 
{	
	int numFrames = F.rows();
	int numBlendshapes = F.cols();
	L.resize(numLabels, numBlendshapes);
	L.setZero();

	int* numbers = nullptr;
	generateRandomNumbers(&numbers, numLabels, numFrames);

	for (int i = 0; i < numLabels; ++i) 
	{
		int rndNum = numbers[i];
		L.row(i) = F.row(rndNum);
	}
	
	delete numbers;
}

void build_L_Matrix_Vertex(const MatrixXd& F, MatrixXd& L, int numLabels) 
{
	int numFrames = F.cols();
	int numVerticies = F.rows();
	L.resize(numVerticies, numLabels);
	L.setZero();

	int* numbers = nullptr;
	generateRandomNumbers(&numbers, numLabels, numFrames);

	for (int i = 0; i < numLabels; ++i)
	{
		int rndNum = numbers[i];
		L.col(i) = F.col(rndNum);
	}

	delete numbers;
}

void build_L_Matrix_Vertex_Kmeans(MatrixXd& F, MatrixXd& L, int numLabels)
{
	std::vector<Point> points;
	//KMeans::loadPoints(fpath, &points);
	
	int numFrames = F.cols();
	for (int i = 0; i < numFrames; ++i) 
	{
		VectorXd mat = F.col(i);
		vector<double> vec(mat.data(), mat.data() + mat.size());
		Point p(vec);
		points.push_back(p);
	}
	
	KMeans kmeans(numLabels);
	kmeans.init(points);
	kmeans.run();
	//kmeans.printMeans();

	
	int numVerticies= F.rows();
	L.resize(numVerticies, numLabels);
	L.setZero();

	for (int i = 0; i < numLabels; ++i)
	{
		Point cluster = kmeans.means_[i];
		double* cluster_data = cluster.data_.data();
		Map<VectorXd> mapped(cluster_data, cluster.dimensions_);
		L.col(i) = mapped;
	}
}

void build_importance_matrix(std::string importance, DiagonalMatrix<double, Eigen::Dynamic>& M, int num_coordinates)
{	
	

	MatrixXd Mv;
	loadMatrix(importance, Mv);
	VectorXd M_vec = Mv.col(0);
	M.resize(num_coordinates);
	M.setIdentity();
		
	double V_WEIGHT = 2.5;
	for (int i = 0; i < M_vec.size(); ++i) 
	{
		int indx = M_vec[i];
		int indx1 = 3 * indx;
		int indx2 = indx1 + 1;
		int indx3 = indx1 + 2;
		M.diagonal()[indx1]= V_WEIGHT;
		M.diagonal()[indx2]  = V_WEIGHT;
		M.diagonal()[indx3] = V_WEIGHT;
	}
}

int runlabeling_bshape(int num_labels, double& result_energy) 
{
	/*
	SETUP
	*/
#if D_OPTIMIZE_LOWER
	std::string objPath = string(FOLDER_PATH) + string("/BlendshapesOBJ/Lower/objlist.txt");
	std::string neut_path = string(FOLDER_PATH) + string("/BlendshapesOBJ/Lower/neuteral.obj");
	std::string wPathData = string(FOLDER_PATH) + string("/WeightsLabels/Lower/weights.txt");
	std::string labelPath = string(FOLDER_PATH) + string("/WeightsLabels/Lower/updated_labels.txt");
	std::string wPathNewLabel = string(FOLDER_PATH) + string("/WeightsLabels/Lower/updated_weightslabels.txt");
	std::string importance = string(FOLDER_PATH) + string("/WeightsLabels/Lower/importance.txt");
#else
	std::string objPath = string(FOLDER_PATH) + string("/BlendshapesOBJ/Upper/objlist.txt");
	std::string neut_path = string(FOLDER_PATH) + string("/BlendshapesOBJ/Upper/neuteral.obj");
	std::string wPathData = string(FOLDER_PATH) + string("/WeightsLabels/Upper/weights.txt");
	std::string labelPath = string(FOLDER_PATH) + string("/WeightsLabels/Upper/updated_labels.txt");
	std::string wPathNewLabel = string(FOLDER_PATH) + string("/WeightsLabels/Upper/updated_weightslabels.txt");
	std::string importance = string(FOLDER_PATH) + string("/WeightsLabels/Upper/importance.txt");
#endif
	MatrixXd F; //frame blendshape data
	loadMatrix(wPathData, F);
	std::cout << "Number of frames" << F.rows() << "\n";

	MatrixXd Mv; //weights to determin how important certain blendshapes are 
	loadMatrix(importance, Mv);
	VectorXd M_vec = Mv.col(0);
	MatrixXd M = M_vec.asDiagonal();
	
	double w_s = 100.0f; //smooth weight
	int num_steps = 30;
	double tolerance = 0.000001;
	int n_init = 10; // number of times the clustering algorithm will be run
	
	/*
	RUN
	*/
	std::vector<Eigen::MatrixXd> n_init_L;
	std::vector<Eigen::VectorXi> n_init_S;
	std::vector<double> n_init_energy;
	
	std::cout << "Energie for " << num_labels << " labels\n";

	for (int j = 0; j < n_init; ++j) 
	{	
		MatrixXd L; //label blendshape data
#if D_USE_KMEANS_INITALIZATION
		build_L_Matrix_Kmeans(wPathData, F, L, num_labels);
#else
		build_L_Matrix(F, L, num_labels);
#endif
		VectorXi S_vec;
		double graphEnergy, oldEnergy, newEnergy;
		
		std::cout << "#### Trial " << j<< " ####\n";

		for (int i = 0; i < num_steps; ++i)
		{
			std::cout << "Iteration " << i << "\n";

			high_resolution_clock::time_point t1 = high_resolution_clock::now();
			labelFacesBShape(F, L, M, S_vec, w_s, graphEnergy);
			high_resolution_clock::time_point t2 = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(t2 - t1).count();
			cout << "label time " << duration << "\n";
			
			t1 = high_resolution_clock::now();
			updateStepBShape(F, M, S_vec, w_s, L, oldEnergy, newEnergy);
			t2 = high_resolution_clock::now();
			duration = duration_cast<microseconds>(t2 - t1).count();
			cout << "updte time " << duration << "\n";

			
			std::cout << "Energy after graph cuts " << graphEnergy << "\nEnergy before update step " << oldEnergy << "\nEnergy after update step " << newEnergy << "\n\n";

			if (abs(newEnergy - oldEnergy) < tolerance)
			{
				std::cout << "Difference between old and new energies is lower than tolerance of " << tolerance << "\n";
				break;
			}
		}
		n_init_L.push_back(L);
		n_init_S.push_back(S_vec);
		n_init_energy.push_back(oldEnergy);
	}

	//Choose best energy
	std::vector<double>::iterator result = std::min_element(std::begin(n_init_energy), std::end(n_init_energy));
	int min_idx = std::distance(std::begin(n_init_energy), result);
	
	MatrixXd L = n_init_L.at(min_idx);
	saveMatrix(wPathNewLabel, L, true);
	
	VectorXi S_vec = n_init_S.at(min_idx);
	saveMatrix(labelPath, S_vec, true);
	
	result_energy = n_init_energy.at(min_idx);
	
	return 1;
}

int runlabeling_vertex(int num_labels, double& result_energy) 
{
	/*
	SETUP
	*/ 
#if D_OPTIMIZE_LOWER
	std::string objPath = string(FOLDER_PATH) + string("/BlendshapesOBJ/Lower/objlist.txt");
	std::string neut_path = string(FOLDER_PATH) + string("/BlendshapesOBJ/Lower/neuteral.obj");
	std::string wPathData = string(FOLDER_PATH) + string("/WeightsLabels/Lower/weights.txt");
	std::string labelPath = string(FOLDER_PATH) + string("/WeightsLabels/Lower/updated_labels.txt");
	std::string wPathNewLabel = string(FOLDER_PATH) + string("/WeightsLabels/Lower/updated_weightslabels.txt");
	std::string importance = string(FOLDER_PATH) + string("/WeightsLabels/Lower/importance_vertex.txt");
#else
	std::string objPath = string(FOLDER_PATH) + string("/BlendshapesOBJ/Upper/objlist.txt");
	std::string neut_path = string(FOLDER_PATH) + string("/BlendshapesOBJ/Upper/neuteral.obj");
	std::string wPathData = string(FOLDER_PATH) + string("/WeightsLabels/Upper/weights.txt");
	std::string labelPath = string(FOLDER_PATH) + string("/WeightsLabels/Upper/updated_labels.txt");
	std::string wPathNewLabel = string(FOLDER_PATH) + string("/WeightsLabels/Upper/updated_weightslabels.txt");
	std::string importance = string(FOLDER_PATH) + string("/WeightsLabels/Lower/importance_vertex.txt");
#endif

	MatrixXd W; //frame blendshape data
	loadMatrix(wPathData, W);

	MatrixXd B; //blendshape targets
	loadBlendshapeTargets(objPath, neut_path, B);
	//B = 0.01*B;
	
	int num_coordinates = B.rows();
	DiagonalMatrix<double, Eigen::Dynamic> M(num_coordinates); //weights to determin how important certain verticies are
	build_importance_matrix(importance, M, num_coordinates);
	//loadMatrix(importance, Mv);
	//VectorXd M_vec = Mv.col(0);
	//MatrixXd M = M_vec.asDiagonal();
	

	double w_s = 1.0f; //smooth weight
	int num_steps = 20;// 150;
	double tolerance = 0.000001;
	int n_init = 4; // number of times the clustering algorithm will be run

	/*
	RUN 
	*/
	std::vector<Eigen::MatrixXd> n_init_D;
	std::vector<Eigen::VectorXi> n_init_S;
	std::vector<double> n_init_energy;

	std::cout << "Energie for " << num_labels << " labels\n";
	
	for (int j = 0; j < n_init; ++j)
	{
		MatrixXd D; //label blendshape data
		//assert(W.cols() == num_labels);
#if D_USE_KMEANS_INITALIZATION
		build_L_Matrix_Kmeans(wPathData, W, D, num_labels);
#else
		build_L_Matrix(W, D, num_labels);
#endif
		VectorXi S_vec;
		double graphEnergy, oldEnergy, newEnergy;

		std::cout << "#### Trial " << j << " ####\n";

		for (int i = 0; i < num_steps; ++i)
		{
			std::cout << "Iteration " << i << "\n";
			
			high_resolution_clock::time_point t1 = high_resolution_clock::now();
			labelFacesVertex(M, W, B, D, S_vec, w_s, graphEnergy);
			high_resolution_clock::time_point t2 = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(t2 - t1).count();
			cout << "label time " << duration << "\n";

			t1 = high_resolution_clock::now();
			updateStepVertex(M, W, B, D, S_vec, w_s, oldEnergy, newEnergy);
			t2 = high_resolution_clock::now();
			duration = duration_cast<microseconds>(t2 - t1).count();
			cout << "updte time " << duration << "\n";

			
			std::cout << "Energy after graph cuts " << graphEnergy << "\nEnergy before update step " << oldEnergy << "\nEnergy after update step " << newEnergy << "\n\n";
			double diff = abs(newEnergy - oldEnergy);
			if (oldEnergy >= newEnergy && diff < tolerance)
			{
				std::cout << "Difference diff " << diff << " between old and new energies is lower than tolerance of " << tolerance << "\n\n";
				break;
			}
		}
		n_init_D.push_back(D);
		n_init_S.push_back(S_vec);
		n_init_energy.push_back(oldEnergy);
	}

	//Choose best energy
	std::vector<double>::iterator result = std::min_element(std::begin(n_init_energy), std::end(n_init_energy));
	int min_idx = std::distance(std::begin(n_init_energy), result);

	MatrixXd D = n_init_D.at(min_idx);
	saveMatrix(wPathNewLabel, D, true);

	VectorXi S_vec = n_init_S.at(min_idx);
	saveMatrix(labelPath, S_vec, true);

	result_energy = n_init_energy.at(min_idx);
	
	return 1;
}

int runlabeling_true_VERTEX(int num_labels, double& result_energy)
{
	/*
	SETUP
	*/
	//auto framesOBJ = string(FOLDER_PATH) + string("/FramesOBJ/Lower/objlist.txt");
	//auto labelOBJ = string(FOLDER_PATH) + string("/LabelsOBJ/Lower/30");
	//auto labelPath = string(FOLDER_PATH) + string("/LabelsOBJ/Lower/30/updated_labels.txt");
	
	auto framesOBJ = string(FOLDER_PATH) + string("/FramesOBJ/Upper/objlist.txt");
	auto labelOBJ = string(FOLDER_PATH) + string("/LabelsOBJ/Upper/40");
	auto labelPath = string(FOLDER_PATH) + string("/LabelsOBJ/Upper/40/updated_labels.txt");


	MatrixXd F, SAVED_FACES; //frame data
	loadFrames(framesOBJ, F, SAVED_FACES);
	cout << "number of frames " << F.cols() << "\n";
	double w_s = 1.0f; //smooth weight
	int num_steps = 25;// 150;
	double tolerance = 0.0001;
	int n_init = 3; // number of times the clustering algorithm will be run

	std::vector<Eigen::MatrixXd> n_init_D;
	std::vector<Eigen::VectorXi> n_init_S;
	std::vector<double> n_init_energy;

	std::cout << "Energie for " << num_labels << " labels\n";

	for (int j = 0; j < n_init; ++j)
	{
		MatrixXd D; //label blendshape data
		
#if D_USE_KMEANS_INITALIZATION
		build_L_Matrix_Vertex_Kmeans(F, D, num_labels);
#else
		build_L_Matrix_Vertex(F, D, num_labels);
#endif

		

		VectorXi S_vec;
		double oldEnergy = 0; 
		double newEnergy = 0;
		double graphEnergy = 0;

		std::cout << "#### Trial " << j << " ####\n";

		for (int i = 0; i < num_steps; ++i)
		{
			std::cout << "Iteration " << i << "\n";

			high_resolution_clock::time_point t1 = high_resolution_clock::now();
			labelFacesTRUEVertex(F, D, S_vec, w_s, graphEnergy);
			high_resolution_clock::time_point t2 = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(t2 - t1).count();
			cout << "label time " << duration << "\n";
			
			t1 = high_resolution_clock::now();
			updateStepTRUEVertex(F, D, S_vec, w_s, oldEnergy, newEnergy);
			t2 = high_resolution_clock::now();
			duration = duration_cast<microseconds>(t2 - t1).count();
			cout << "updte time " << duration << "\n";
			
			std::cout << "Energy after graph cuts " << graphEnergy << "\nEnergy before update step " << oldEnergy << "\nEnergy after update step " << newEnergy << "\n\n";
			double diff = abs(newEnergy - oldEnergy);
			if (oldEnergy >= newEnergy && diff < tolerance)
			{
				std::cout << "Difference diff " << diff << " between old and new energies is lower than tolerance of " << tolerance << "\n\n";
				break;
			}

		}
		n_init_D.push_back(D);
		n_init_S.push_back(S_vec);
		n_init_energy.push_back(oldEnergy);
	}

	//Choose best energy
	std::vector<double>::iterator result = std::min_element(std::begin(n_init_energy), std::end(n_init_energy));
	int min_idx = std::distance(std::begin(n_init_energy), result);

	MatrixXd D = n_init_D.at(min_idx);
	saveDictionary(labelOBJ, D, SAVED_FACES);
	
	VectorXi S_vec = n_init_S.at(min_idx);
	saveMatrix(labelPath, S_vec, true);

	result_energy = n_init_energy.at(min_idx);

	return 1;
}

int compute_different_energies()
{
	/*
	SETUP
	*/
#if D_OPTIMIZE_LOWER
	auto wPathData = string(FOLDER_PATH) + string("/WeightsLabels/Lower/weights.txt");
#else
	auto wPathData = string(FOLDER_PATH) + string("/WeightsLabels/Upper/weights.txt");
#endif
	MatrixXd F; //frame blendshape data
	loadMatrix(wPathData, F);
	int num_frames = F.rows();

	int num_energy_tries = 30;
	std::cout << "Number of tries " << num_energy_tries << "\n";
	VectorXd energiesVec(num_energy_tries - 2);
	energiesVec.setZero();

	/*
	RUN
	*/
	for (int label_index = 2; label_index < num_energy_tries; ++label_index)
	{
		double energy;

		if (D_WORK_TYPE == COMPUTE_DIFFERENT_ENERGIES_BSHAPES)
			runlabeling_bshape(label_index, energy);
		else if (D_WORK_TYPE == COMPUTE_DIFFERENT_ENERGIES_VERTEX)
			runlabeling_vertex(NUM_LABELS, energy);
		else
			assert(false);
		
		energiesVec(label_index - 2) = energy;
	}

#if D_OPTIMIZE_LOWER
	auto energiesPath = string(FOLDER_PATH) + string("/WeightsLabels/Lower/energies.txt");
#else
	auto energiesPath = string(FOLDER_PATH) + string("/WeightsLabels/Upper/energies.txt");
#endif

	saveMatrix(energiesPath, energiesVec);

	return 0;
}
int main(int argc, char **argv)
{
	convertOFFtoOBJ(std::string());
	return 0;
}
/*
int main(int argc, char **argv) 
{ 
	double energy;
	
	switch (D_WORK_TYPE) 
	{	
		case OPTIMIZE_TRUE_VERTEX: 
		{
			runlabeling_true_VERTEX(NUM_LABELS, energy);
			break;
		}
		case OPTIMIZE_BSHAPES: 
		{
			runlabeling_bshape(NUM_LABELS, energy);
			break;
		}
		case OPTIMIZE_VERTEX: 
		{
			runlabeling_vertex(NUM_LABELS, energy);
			break;
		}
		case COMPUTE_DIFFERENT_ENERGIES_BSHAPES:
		{
			compute_different_energies();
			break; 
		}
		case COMPUTE_DIFFERENT_ENERGIES_VERTEX:
		{
			compute_different_energies();
			break;
		}
	}
	std::cout << "Done\n";
	system("pause");
	return 0; 
}                                            

*/
