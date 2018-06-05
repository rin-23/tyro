#include "GCoptimization.h"
#include "utils.h"
#include <algorithm>    // std::min
#include "labelingStep.h"
#include "RALogManager.h"
#include <chrono>
#include <omp.h>
#include <unordered_map>

using namespace Eigen;
using namespace std;
using namespace std::chrono;
double global_w = 1.0f;

GCoptimization::EnergyTermType smoothFnBlendshapes(int p1, int p2, int l1, int l2, void *extraData)
{   
	// return 0;
	// if ((l1 - l2)*(l1 - l2) <= 4) return((l1 - l2)*(l1 - l2));
	// else return(4);
	
	MatrixXd** data = (MatrixXd**)extraData;
	MatrixXd* bWeightsData = data[0];
	MatrixXd* bWeightsLabel = data[1];
	MatrixXd* m = data[2];

	VectorXd l1Obj = bWeightsLabel->row(l1);
	VectorXd l2Obj = bWeightsLabel->row(l2);

	VectorXd p1Obj = bWeightsData->row(p1);
	VectorXd p2Obj = bWeightsData->row(p2);

	VectorXd diff = *m * ((l1Obj - l2Obj) - (p1Obj - p2Obj));
	//VectorXd diff2 = (l1Obj - l2Obj) - (p1Obj - p2Obj);
	
	GCoptimization::EnergyTermType sum = (GCoptimization::EnergyTermType) (global_w * diff.squaredNorm());
	return sum;
}

GCoptimization::EnergyTermType dataFnBlendshape(int p, int l, void *extraData)
{
	MatrixXd** data = (MatrixXd**)extraData;
	MatrixXd* bWeightsData = data[0];
	MatrixXd* bWeightsLabel = data[1];
	MatrixXd* m = data[2];

	VectorXd pObj = bWeightsData->row(p);
	VectorXd lObj = bWeightsLabel->row(l);

	VectorXd diff = *m * (pObj - lObj);
	GCoptimization::EnergyTermType sum = (GCoptimization::EnergyTermType) diff.squaredNorm();
	//std::cout << p << " " << l << " " << sum << "\n";
	return sum;
}

GCoptimization::EnergyTermType dataFnVertex(int p, int l, void *extraData)
{
	void** data = (void**)extraData;
	MatrixXd* W = (MatrixXd*)data[0];
	MatrixXd* D = (MatrixXd*)data[1];
	MatrixXd* MB = (MatrixXd*)data[2];
	//DiagonalMatrix<double, Eigen::Dynamic>* M = (DiagonalMatrix<double, Eigen::Dynamic>*)data[3];


	VectorXd pObj = W->row(p);
	VectorXd lObj = D->row(l);

	double diff = (*MB * (pObj - lObj)).squaredNorm();
	//VectorXd diff2 = pObj - lObj;
	GCoptimization::EnergyTermType sum = (GCoptimization::EnergyTermType) diff;
	//std::cout << p << " " << l << " " << sum << "\n";
	return sum;
}


GCoptimization::EnergyTermType smoothFnVertex(int p1, int p2, int l1, int l2, void *extraData)
{
	//return 0;
	//if ((l1 - l2)*(l1 - l2) <= 4) return((l1 - l2)*(l1 - l2));
	//else return(4);

	void** data = (void**)extraData;
	MatrixXd* W = (MatrixXd*)data[0];
	MatrixXd* D = (MatrixXd*)data[1];
	MatrixXd* MB = (MatrixXd*)data[2];
	//DiagonalMatrix<double, Eigen::Dynamic>* M = (DiagonalMatrix<double, Eigen::Dynamic>*)data[3];
	
	VectorXd l1Obj = D->row(l1);
	VectorXd l2Obj = D->row(l2);

	VectorXd p1Obj = W->row(p1);
	VectorXd p2Obj = W->row(p2);

	double diff = (*MB * ((l1Obj - l2Obj) - (p1Obj - p2Obj))).squaredNorm();
	//VectorXd diff2 = (l1Obj - l2Obj) - (p1Obj - p2Obj);
	GCoptimization::EnergyTermType sum = (GCoptimization::EnergyTermType) (global_w * diff);
	return sum;
}

GCoptimization::EnergyTermType dataFnTRUEVertex(int p, int l, void *extraData)
{
	//high_resolution_clock::time_point t1 = high_resolution_clock::now();
	void** data = (void**)extraData;
	MatrixXd* F = (MatrixXd*)data[0];
	MatrixXd* D = (MatrixXd*)data[1];
	
	//VectorXd pObj = ;
	//VectorXd lObj = ;

	double diff = 0;
	for (int i = 0; i < D->rows(); ++i) 
	{
		double a = (*F)(i, p) - (*D)(i, l);
		diff += a*a;
	}

	//double diff = (F->col(p) - D->col(l)).squaredNorm();
	GCoptimization::EnergyTermType sum = (GCoptimization::EnergyTermType) diff;

	//high_resolution_clock::time_point t2 = high_resolution_clock::now();
	//auto duration = duration_cast<milliseconds>(t2 - t1).count();
	//RA_LOG_INFO("dataFnTRUEVertex %f sec", diff);
	return sum;
}

GCoptimization::EnergyTermType smoothFnTRUEVertex(int p1, int p2, int l1, int l2, void *extraData)
{	
	//high_resolution_clock::time_point t1 = high_resolution_clock::now();

	void** data = (void**)extraData;
	MatrixXd* F = (MatrixXd*)data[0];
	MatrixXd* D = (MatrixXd*)data[1];
	VectorXd* VW = (VectorXd*)data[2];
	std::unordered_map<std::string, double>* m = (std::unordered_map<std::string, double>*)data[3];
	//VectorXd l1Obj = ;
	//VectorXd l2Obj = ;
	//VectorXd p1Obj = ;
	//VectorXd p2Obj = ;
	using namespace std;
	std::string ky =  std::to_string(p1) 
	                 + string("-") 
	                 + std::to_string(p2)
					 + string("-") 
					 + std::to_string(l1) 
					 + string("-")
					 + std::to_string(l2);
	auto it = m->find(ky);
	
	if (it != m->end()) return (*m)[ky];
	
	double diff = 0;
	for (int i = 0; i < D->rows(); ++i) 
	{
		double a =  (*VW)(i) *((*D)(i, l1) - (*D)(i, l2) - (*F)(i, p1) + (*F)(i, p2));
		diff += a*a;
	}
	
	//double diff = (D->col(l1) - D->col(l2) - F->col(p1) + F->col(p2)).squaredNorm();
	GCoptimization::EnergyTermType sum = (GCoptimization::EnergyTermType) (global_w * diff);
	//high_resolution_clock::time_point t2 = high_resolution_clock::now();
	//auto duration = duration_cast<milliseconds>(t2 - t1).count();
	//RA_LOG_INFO("smoothFnTRUEVertex %i milisec", duration);
	//RA_LOG_INFO("%i,%i,%i,%i", p1, p2, l1, l2);
	m->insert({ky, sum});	
	return sum;	
}

//sequenceIdx contains info where scenes breaks
void setNeighbours(GCoptimizationGeneralGraph *gc, int num_data) 
{	
	//should use one with sequenceIdx
	assert(false);
}

void setNeighbours(GCoptimizationGeneralGraph *gc, int num_data, std::vector<int>& sequenceIdx)
{	

	std::vector<int> ff;
	int sofar = 0;
	for (int i = 0; i < sequenceIdx.size(); ++i) 
	{
		sofar += sequenceIdx[i];
		ff.push_back(sofar);
	}

	for (int i = 0; i < num_data - 1; ++i)
	{	
		auto it = std::find(ff.begin(), ff.end(), i + 1); //dont connect one scene to another
		if (it == ff.end()) 
        { 
			gc->setNeighbors(i, i + 1); 
		}
	}
}

void labelFacesVertex(Eigen::DiagonalMatrix<double, Eigen::Dynamic>& M, Eigen::MatrixXd& W, Eigen::MatrixXd& B, Eigen::MatrixXd& D, Eigen::VectorXi& S_vec, double w_s, double& energy)
{
	global_w = w_s;
	int num_data = W.rows();
	int num_labels = D.rows();
	S_vec.resize(num_data);

	try
	{
		GCoptimizationGeneralGraph *gc = new GCoptimizationGeneralGraph(num_data, num_labels);
		MatrixXd MB = M*B;
		void* extraData[3] = { &W, &D, &MB };
		gc->setDataCost(&dataFnVertex, extraData);
		gc->setSmoothCost(&smoothFnVertex, extraData);

		setNeighbours(gc, num_data);

		double oldE = gc->compute_energy();
		gc->swap(-1);
		energy = gc->compute_energy();
		//printf("Before graph opt %0.2f, after %0.2f\n", oldE, energy);

		for (int i = 0; i < num_data; i++)
		{
			S_vec(i) = gc->whatLabel(i);
			//std::cout << S_vec(i) << "\n";
		}

		//if (S_vec_path != nullptr) // want to save labels to file
		//{
		//	saveMatrix(S_vec_path, S_vec);
		//}

		delete gc;
	}
	catch (GCException e)
	{
		e.Report();
	}
}


void labelFacesBShape(Eigen::MatrixXd& F, Eigen::MatrixXd& L, Eigen::MatrixXd& M, Eigen::VectorXi& S_vec, double w_s, double& energy)
{
	global_w = w_s;
	int num_data = F.rows();
	int num_labels = L.rows();
	S_vec.resize(num_data);

	try
	{
		GCoptimizationGeneralGraph *gc = new GCoptimizationGeneralGraph(num_data, num_labels);
			
		MatrixXd* extraData[3] = {&F, &L, &M};
		gc->setDataCost(&dataFnBlendshape, extraData);
		gc->setSmoothCost(&smoothFnBlendshapes, extraData);
				
		setNeighbours(gc, num_data);

		double oldE = gc->compute_energy();
		gc->swap(-1);
		energy = gc->compute_energy();
		//printf("Before graph opt %0.2f, after %0.2f\n", oldE, energy);

		for (int i = 0; i < num_data; i++)
		{
			S_vec(i) = gc->whatLabel(i);
			//std::cout << S_vec(i) << "\n";
		}

		//if (S_vec_path != nullptr) // want to save labels to file
		//{
		//	saveMatrix(S_vec_path, S_vec);
		//}
		
		delete gc;
	}
	catch (GCException e)
	{
		e.Report();
	}
}

void prepareDataMatrixTRUEVertex(Eigen::MatrixXd& F, Eigen::VectorXd& VW, Eigen::MatrixXd& D, double*& dataArray) 
{	
	int num_frames = F.cols();
	int num_labels = D.cols();
	int num_vertex = F.rows();

	dataArray = new double[num_frames*num_labels];
	
	#pragma omp parallel for collapse(2)
	for (int i = 0; i < num_frames; ++i) 
	{	
		//#pragma omp parallel for
		for (int j = 0; j < num_labels; ++j) 
		{	
			double diff = 0;
			for (int v = 0; v < num_vertex; ++v) 
			{	
				//if (VW(v) > 1)
			//		RA_LOG_INFO("WEIGHT IS %f", VW(v));

				double a = VW(v) * (F(v, i) - D(v, j));
				diff +=  a*a;
			}
			dataArray[i*num_labels+j] = diff;
		}
	}
}

void labelFacesTRUEVertex(Eigen::MatrixXd& F, Eigen::VectorXd& VW, Eigen::MatrixXd& D,  Eigen::VectorXi& S_vec, std::vector<int>& sequenceIdx, double w_s, double& energy) 
{
	global_w = w_s;
	int num_frames = F.cols();
	int num_labels = D.cols();
	S_vec.resize(num_frames);

	for (int i=0; i < VW.size(); ++i) 
	{
		//if (VW(i) > 1) 
		//	RA_LOG_INFO("SUKA %f", VW(i));
	}

	try
	{
		GCoptimizationGeneralGraph *gc = new GCoptimizationGeneralGraph(num_frames, num_labels);
		std::unordered_map<std::string, double> wordMap;
		void* extraData[4] = {&F, &D, &VW, &wordMap};
		double* dataArray;
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		prepareDataMatrixTRUEVertex(F, VW, D, dataArray);
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(t2 - t1).count();

		RA_LOG_INFO("data matrix computation %i msec", duration);
		
		//gc->setDataCost(&dataFnTRUEVertex, extraData);
		gc->setDataCost(dataArray);
		
		gc->setSmoothCost(&smoothFnTRUEVertex, extraData);
		
	

		setNeighbours(gc, num_frames, sequenceIdx);

		double oldE = gc->compute_energy();
		gc->swap(-1);
		energy = gc->compute_energy();

		for (int i = 0; i < num_frames; i++)
		{
			S_vec(i) = gc->whatLabel(i);
		}

		/*
		for (auto it : wordMap) 
		{
			if (it.second > 1)
				RA_LOG_INFO("%s : %i", it.first.data(), it.second);
		}
		*/
		delete gc;
	}
	catch (GCException e)
	{
		e.Report();
	}
}
