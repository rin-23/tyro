#include "segmentation.h"

#include <Eigen/SparseCore>

#include <igl/all_edges.h>
#include <igl/unique.h>
#include <igl/edge_lengths.h>
#include <igl/slice.h>
#include <igl/barycenter.h>
#include <igl/slice.h>
#include <igl/exact_geodesic.h>
#include <igl/is_edge_manifold.h>
#include <igl/edge_flaps.h>
#include <igl/unique_edge_map.h>

#include "all.h"
#include "RALogManager.h"
#include "GCoptimization.h"

using Eigen::VectorXi;
using Eigen::VectorXd;

using Eigen::MatrixXi;
using Eigen::MatrixXd;

using Eigen::Vector3i;
using Eigen::Vector3f;

namespace tyro 
{
struct SegmentationData 
{
	Eigen::SparseMatrix<double> S; //smooth cost
	Eigen::MatrixXd U; //data cost
};

void setNeighbors(GCoptimizationGeneralGraph *gc, const Eigen::MatrixXi& EF)
{
	for (int i = 0; i < EF.rows(); ++i)
	{	
		int f1 = EF(i, 0);
		int f2 = EF(i, 1);
		
		// face indicies thata -1 means i is a boundary edge
		if (f1 != -1 && f2 != -1)  
		{
			gc->setNeighbors(f1, f2);
		}
	}
}

GCoptimization::EnergyTermType dataFnVertex(int p, int l, void* extraData) 
{
	SegmentationData* data = (SegmentationData*)extraData;
	GCoptimization::EnergyTermType cost = data->U(p, l);
	return cost;
}

GCoptimization::EnergyTermType smoothFnVertex(int p1, int p2, int l1, int l2, void *extraData) 
{	
	if (l1 == l2) 
	{ 
		return 0;
	}

	SegmentationData* data = (SegmentationData*)extraData;
	GCoptimization::EnergyTermType cost = data->S.coeffRef(p1, p2);
	return cost;
}

void prepareSmoothMatrix(const std::vector<Eigen::MatrixXd>& v_data, //vertex data
						const Eigen::MatrixXi& F,
                        const Eigen::MatrixXd& Vavg, //average of faces
                        const Eigen::MatrixXi& EF, //edge flaps 
                        const Eigen::MatrixXi& uE, //unique edges
						const std::vector<Eigen::VectorXd>& uEL, //edge length
						double smooth_weight,
						Eigen::SparseMatrix<double>& A, 
						const Eigen::VectorXi& isBoundary, double avg_length = 0) // sparse matrix of smooth cost
{		
	A.resize(F.rows(), F.rows());
	A.setZero();
	
	for (int e = 0; e < uE.rows(); ++e) 
	{	
		int f1idx = EF(e,0);
		int f2idx = EF(e,1);
		int v1idx = uE(e,0);
		int v2idx = uE(e,1);
		
		if (f1idx == -1 || f2idx == -1) 
			continue;
		/*
		if ((isBoundary(f1idx) && !isBoundary(f2idx)) ||
			(isBoundary(f2idx) && !isBoundary(f1idx))) 
		{
			A.coeffRef(f1idx, f2idx) = 1000000;   
			A.coeffRef(f2idx, f1idx) = 1000000;
			continue;
		}			
		*/
		double cost = 0;
		for (int frame = 0; frame < v_data.size(); ++frame) 
		{

			double n1 = (v_data[frame].row(v1idx) - Vavg.row(v1idx)).norm();
			double n2 = (v_data[frame].row(v2idx) - Vavg.row(v2idx)).norm();

			cost += uEL[frame](e) * (n1 + n2 + avg_length);
		    //cost += (v1diff.norm() + v2diff.norm() + 1);
		}

		//double w_smooth = 9;
		//if (cost > 0) 
	//	{
			//RA_LOG_INFO("lul");
	//	}

		cost *= smooth_weight;
		A.coeffRef(f1idx, f2idx) = cost;   
		A.coeffRef(f2idx, f1idx) = cost;
	}
}


void prepareDataMatrix(const std::vector<Eigen::MatrixXd>& v_data, //vertex data
					   const Eigen::MatrixXi& F,
                       const Eigen::MatrixXd& Vavg, //average of faces
                       const Eigen::MatrixXi& EF, //edge flaps 
                       const Eigen::MatrixXi& uE, //unique edges
					   const Eigen::VectorXi& FS1, //ids of seed faces into F
					   const Eigen::VectorXi& FS2, //ids of seed faces into F
					   Eigen::MatrixXd& A) 
{	
	//get geodesic distance data
	VectorXi VS, VT, FT; //, FS1, FS2;
	FT.setLinSpaced(F.rows(), 0, F.rows()-1);
	
	//int split = int(seeds.size()/2);
	//FS1.resize(split);
	//FS2.resize(seeds.size() - split);

	//for (int i = 0; i < split; ++i) FS1(i) = seeds(i);
	//for (int i = split; i < seeds.size(); ++i) FS2(i-split) = seeds(i);
	
	//Assume half of the seeds are one side and other hald is the other
	//FS(0) = S(0);
	
	Eigen::VectorXd d1;
	igl::exact_geodesic(Vavg, F, VS, FS1, VT, FT, d1);
	
	Eigen::VectorXd d2;
	igl::exact_geodesic(Vavg, F, VS, FS2, VT, FT, d2);

	//MatrixXd BC;
	//igl::barycenter(Vavg, F, BC);
	//MatrixXi FS; //seed face  
	//igl::slice(F, S, 1, FS);
	//MatrixXd BCS;
	//igl::barycenter(Vavg, FS, BCS);

	//VectorXd Z = BC.col(1); // TODO assumes model is along Z axis.
	//int imin, imax;
	//double zmin = Z.minCoeff(&imin);
	//double zmax = Z.maxCoeff(&imax);


	MatrixXd Thard;
	Thard.resize(F.rows(), 2);
	Thard.setZero();
	for (int i = 0; i < FS1.size(); ++i) 
	{
		Thard(FS1(i), 0) = 0;
		Thard(FS1(i), 1) = 1;
	}	

	for (int i = 0; i < FS2.size(); ++i) 
	{
		Thard(FS2(i), 1) = 0;
		Thard(FS2(i), 0) = 1;
	}	

	MatrixXd Tsoft;
	Tsoft.resize(F.rows(), 2);
	Tsoft.setZero(); 
	Tsoft.col(0) = d1; //Znorm;
	Tsoft.col(1) = d2; //(ones - Znorm);
	//Tsoft = Tsoft.array().pow(10).matrix();
	//double ll = d1(imin);
	//double l2 = d2(imax);

	double soft_w = 1;
	double hard_w = 100000;
	A = soft_w * Tsoft + hard_w * Thard;
}




void segmentation(const std::vector<Eigen::MatrixXd>& v_data, //vertex data
                  const Eigen::MatrixXi& F, //faces, most likeley a subset of the original mesh
                  const Eigen::MatrixXd& Vavg, //average of faces
                  //const Eigen::MatrixXi& inEF, //edge flaps 
                  //const Eigen::MatrixXi& inuE, //unique edges
				  const Eigen::VectorXi& seeds1,
				  const Eigen::VectorXi& seeds2,
				  //const Eigen::VectorXi& face_map,
				  double smooth_weight,
				  Eigen::VectorXi& L, 
				  const Eigen::VectorXi& isBoundary)  
{
	//assert(seeds.size() == 2); //only support 2 labels right now
	assert(igl::is_edge_manifold(F));

	MatrixXi E, inuE;
	VectorXi EMAP;
	std::vector<std::vector<int> > uE2E;
	igl::unique_edge_map(F, E, inuE,EMAP,uE2E);

	MatrixXi inEF, EI;
	igl::edge_flaps(F, inuE, EMAP, inEF, EI);
	
	//igl::slice(FO, face_map, 1, F);


	//extract interior edges only
	Eigen::VectorXi int_man;
	tyro::all(inEF, 
              [](const Eigen::VectorXi& v) -> bool { return v(0) != -1 && v(1) != -1;},
              1,
              int_man);

	//slice edge flaps and unique edge matrix to only contain info about interior edges
	MatrixXi EF, uE;
	igl::slice(inEF, int_man, 1, EF);
	igl::slice(inuE, int_man, 1, uE);

	//Compute edge length of the mesh for every frame
	std::vector<VectorXd> uEL;
	for (int i = 0; i < v_data.size(); ++i) 
	{
		const auto& V = v_data[i];
		Eigen::MatrixXd L;
		igl::edge_lengths(V, uE, L);
		uEL.push_back(L.col(0));
	}

	SegmentationData* data = new SegmentationData();

	//data->S.resize(F.rows(), F.rows());
	//data->S.setZero();
	prepareSmoothMatrix(v_data,   //vertex data
						F,		  //face data
                        Vavg,     //average of faces
                        EF,       //edge flaps 
                        uE,       //unique edges
            			uEL,      //edge length
						smooth_weight,
    		 			data->S, 
						isBoundary); // sparse matrix of smooth cost
	
	prepareDataMatrix(v_data,
					  F, 
					  Vavg,
					  EF, 
					  uE, 
					  seeds1,
					  seeds2,
					  data->U);

	int num_data = F.rows();
	int num_labels = 2;

	try
	{
		GCoptimizationGeneralGraph *gc = new GCoptimizationGeneralGraph(num_data, num_labels);

		gc->setDataCost(&dataFnVertex, data);		
		gc->setSmoothCost(&smoothFnVertex, data);
		tyro::setNeighbors(gc, EF);

		double oldE = gc->compute_energy();
		gc->swap(-1);
		double newEnergy = gc->compute_energy();

		L.resize(num_data);
		for (int i = 0; i < num_data; i++)
		{
			L(i) = gc->whatLabel(i);
		}

		delete gc; 
	}
	catch(GCException e) 
	{
		e.Report();
	}
}


                  
void segmentation2(const std::vector<Eigen::MatrixXd>& v_data, //vertex data
                  const Eigen::MatrixXi& F, //original face data
                  const Eigen::MatrixXd& Vavg, //average of faces
				  const Eigen::VectorXi& seeds1, //seeds(indicies of faces that must belong to a label)
                  const Eigen::VectorXi& seeds2,
                  double smooth_weight,
				  double avg_length, 
                  Eigen::VectorXi& L) 
{

	assert(igl::is_edge_manifold(F));

	MatrixXi E, uE;
	VectorXi EMAP;
	std::vector<std::vector<int> > uE2E;
	igl::unique_edge_map(F, E, uE,EMAP,uE2E);

	MatrixXi EF, EI;
	igl::edge_flaps(F, uE, EMAP, EF, EI);


	//extract interior edges only
	//Eigen::VectorXi int_man;
	//tyro::all(inEF, 
     //         [](const Eigen::VectorXi& v) -> bool { return v(0) != -1 && v(1) != -1;},
     //         1,
     //         int_man);

	//slice edge flaps and unique edge matrix to only contain info about interior edges
	//MatrixXi EF, uE;
	//igl::slice(inEF, int_man, 1, EF);
	//igl::slice(inuE, int_man, 1, uE);

	//Compute edge length of the mesh for every frame
	std::vector<VectorXd> uEL;
	uEL.resize(v_data.size());
	for (int i = 0; i < v_data.size(); ++i) 
	{
		const auto& V = v_data[i];
		Eigen::MatrixXd L;
		igl::edge_lengths(V, uE, L);
		uEL[i] = L.col(0);
	}

	SegmentationData* data = new SegmentationData();

	//data->S.resize(F.rows(), F.rows());
	//data->S.setZero();
	Eigen::VectorXi isBoundary;
	prepareSmoothMatrix(v_data,   //vertex data
						F,		  //face data
                        Vavg,     //average of faces
                        EF,       //edge flaps 
                        uE,       //unique edges
            			uEL,      //edge length
						smooth_weight,
    		 			data->S, 
						isBoundary, avg_length); // sparse matrix of smooth cost
	
	prepareDataMatrix(v_data,
					  F, 
					  Vavg,
					  EF, 
					  uE, 
					  seeds1,
					  seeds2,
					  data->U);

	int num_data = F.rows();
	int num_labels = 2;

	try
	{
		GCoptimizationGeneralGraph *gc = new GCoptimizationGeneralGraph(num_data, num_labels);

		gc->setDataCost(&dataFnVertex, data);		
		gc->setSmoothCost(&smoothFnVertex, data);
		tyro::setNeighbors(gc, EF);

		double oldE = gc->compute_energy();
		gc->swap(-1);
		double newEnergy = gc->compute_energy();

		L.resize(num_data);
		for (int i = 0; i < num_data; i++)
		{
			L(i) = gc->whatLabel(i);
		}

		delete gc; 
	}
	catch(GCException e) 
	{
		e.Report();
	}
}


}

//R = axisangle2matrix([0 1 0],-pi/2)*axisangle2matrix([0 0 1],-pi/2);
//Vavg = mean(VV,3);
//Average onto faces
//F2V = sparse(F, repmat(1:size(F,1), 3, 1)',1/3,size(VV,1),size(F,1))';
//%allE = [F(:,[2 3]); F(:,[3 1]); F(:,[1 2])]; // non unique edges stacked together allE is (F*3, 2)
//%[sE, I] = sort(allE, 2); // sort each row so that [2 1; 3 0] turns into [1 2; 0 3]. sE = allE(I)
//%[uE,~,J] = unique(sE,'rows');  // treat each row of sE as a single entity and return the unique rows of sE in sorted order. uE(J) = sE
//%% face indices on either side of each edge
//%EF = full(sparse(J, 
//                  I(:,1), 
//                  repmat(1:size(F,1), 1, 3)', 
//                  size(uE,1), //uE.rows() // number of unique edges
//                  2)); 
//%% only keep interior manifold edges
//%int_man = all(EF>0,2);
//%EF = EF(int_man,:);
//%uE = uE(int_man,:);

//%% Edge lengths
//%uEL = sqrt(sum((VV(uE(:,2),:,:)-VV(uE(:,1),:,:)).^2,2));

//%% cost per-vertex, per-frame
//%C = sqrt(sum((VV-Vavg).^2, 2));
//%% For each interior edge add a penalty for cutting along it
//%A = sparse( ...
//%  EF(:,1), ...
//%  EF(:,2), ...
//%  sum(uEL.*(1+(C(uE(:,1),:,:) + C(uE(:,2),:,:))), 3), ...
//%  size(F,1),size(F,1));

/*
//%% Symmetrize
//%A = A + A';
//%% Normalize
//%A = A / max(abs(A(:)));
//%
%Z = barycenter(Vavg,F)*R*[0;0;1];
%[~,imin] = min(Z);
%[~,imax] = max(Z);
%Thard = sparse([imax imax imin imin], [1 2 1 2], [1 0 0 1], size(F,1),2);
%Tsoft = matrixnormalize(Z);
%Tsoft = sparse([Tsoft 1-Tsoft]).^10;
%  
%w_smooth = 1024;
%w_hard = 10000;
%w_soft = 100;
%[~,L] = maxflow(w_smooth * A, w_hard * Thard + w_soft * Tsoft);
%L = double(L);
%
%V2F = sparse(F,repmat(1:size(F,1),3,1)',repmat(doublearea(Vavg,F),1,3),size(VV,1),size(F,1));
%V2FIS = 1./sum(V2F,2);
%V2FIS(V2FIS==0) = 1;
%V2F = (diag(sparse(V2FIS))*V2F);
%tsh = tsurf(F,Vavg*R,'CData',sum(C,3).*(2*V2F*double(L)-1),'EdgeColor','none',fphong,fsoft);
%axis equal;view(0,0);camlight;colormap(cbrewer('RdBu',15));
%drawnow;
%
%
%% Count how many faces of each label are incident on each vertex
%LV = sparse(F,repmat(L+1,1,3),1,size(VV,1),2);
%% fix entire one ring around boundary: hacky, Neumann boundary conditions
%A = adjacency_matrix(F);
%b = find(A*all(LV>0,2));
%% boundary conditions: fix to average value
%bc = Vavg(b,:);
%% Biharmonic operator on average mesh (this allows 
%K = cotmatrix(Vavg,F);
%M = massmatrix(Vavg,F);
%w_data = 0.00001*max(M(:));
%Q = K*(M\K) + w_data*M;
%% Deal with unreferenced vertices
%unref = find(sum(A,2) == 0);
%Q(unref,unref) = 1;
 f_data
mqwf = [];
UU = zeros(size(VV));
tsh = tsurf(F,Vavg*R,'CData',C(:,:,1).*(2*V2F*double(L)-1),'EdgeColor','none',fphong,fsoft);
axis equal;view(0,0);camlight;colormap(cbrewer('RdBu',15));
axis([vec([min(min(BRV,[],3),[],1);max(max(BRV,[],3),[],1)])'])
axis manual;
view(20,14);
caxis([-5 5]);
caxis manual;
mkdir('faces-bottom');
mkdir('faces-top');
 
[~,TI,TJ] = remove_unreferenced(Vavg,F(L<=0,:));
TF = TI(F(L<=0,:));
%TV = Vavg(TJ,:);
[~,BI,BJ] = remove_unreferenced(Vavg,F(L>0,:));
BF = BI(F(L>0,:));
%BV = Vavg(BJ,:);

for f = 1:size(VV,3);
  [UU(:,:,f),mqwf] = min_quad_with_fixed(Q,-2*Q*VV(:,:,f),b,bc,[],[],mqwf);
  writeOBJ(sprintf('faces-bottom/bottom-face_mesh_%06d.obj',f),UU(BJ,:,f),BF);
  writeOBJ(sprintf('faces-top/top-face_mesh_%06d.obj',f),      UU(TJ,:,f),TF);
  tsh.Vertices = UU(:,:,f)*R;
  %tsh.CData = normrow(UU(:,:,f)-VV(:,:,f)).*(2*V2F*double(L)-1);
  title(sprintf('%d/%d',f,size(VV,3)),'Fontsize',30);
  drawnow;
  %pause
end
}
*/
