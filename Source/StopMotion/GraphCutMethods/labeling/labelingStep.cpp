#include "GCoptimization.h"
#include "utils.h"
#include <algorithm>    // std::min
#include "labelingStep.h"

using namespace Eigen;
using namespace std;

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
	void** data = (void**)extraData;
	MatrixXd* F = (MatrixXd*)data[0];
	MatrixXd* D = (MatrixXd*)data[1];
	
	VectorXd pObj = F->col(p);
	VectorXd lObj = D->col(l);

	double diff = (pObj - lObj).squaredNorm();
	GCoptimization::EnergyTermType sum = (GCoptimization::EnergyTermType) diff;
	return sum;
}

GCoptimization::EnergyTermType smoothFnTRUEVertex(int p1, int p2, int l1, int l2, void *extraData)
{
	void** data = (void**)extraData;
	MatrixXd* F = (MatrixXd*)data[0];
	MatrixXd* D = (MatrixXd*)data[1];

	VectorXd l1Obj = D->col(l1);
	VectorXd l2Obj = D->col(l2);

	VectorXd p1Obj = F->col(p1);
	VectorXd p2Obj = F->col(p2);

	double diff = ((l1Obj - l2Obj) - (p1Obj - p2Obj)).squaredNorm();
	GCoptimization::EnergyTermType sum = (GCoptimization::EnergyTermType) (global_w * diff);
	return sum;
	
}

void setNeighbours(GCoptimizationGeneralGraph *gc, int num_data)
{
	for (int i = 0; i < num_data; ++i)
	{
		if (i != num_data - 1) // create pairs
			gc->setNeighbors(i, i + 1);
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


void labelFacesTRUEVertex(Eigen::MatrixXd& F, Eigen::MatrixXd& D, Eigen::VectorXi& S_vec, double w_s, double& energy) 
{
	global_w = w_s;
	int num_frames = F.cols();
	int num_labels = D.cols();
	S_vec.resize(num_frames);

	try
	{
		GCoptimizationGeneralGraph *gc = new GCoptimizationGeneralGraph(num_frames, num_labels);
		void* extraData[2] = {&F, &D};
		gc->setDataCost(&dataFnTRUEVertex, extraData);
		gc->setSmoothCost(&smoothFnTRUEVertex, extraData);

		setNeighbours(gc, num_frames);

		double oldE = gc->compute_energy();
		gc->swap(-1);
		energy = gc->compute_energy();

		for (int i = 0; i < num_frames; i++)
		{
			S_vec(i) = gc->whatLabel(i);
		}

		delete gc;
	}
	catch (GCException e)
	{
		e.Report();
	}
}
