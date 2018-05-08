#include "segmentation.h"
#include <Eigen/SparseCore>
#include <igl/all_edges.h>
#include <igl/unique.h>
#include <igl/edge_lengths.h>
#include "TyroAll.h"
#include <igl/slice.h>
#include "GCoptimization.h"

using Eigen::MatrixXd;
using Eigen::VectorXd;

using Eigen::MatrixXi;
using Eigen::VectorXi;

struct SegmentationData 
{
	Eigen::SparseMatrix<double> A; //smooth cost
	Eigen::MatrixXd U; //data cost
}

void setNeighbours(GCoptimizationGeneralGraph *gc, const Eigen::MatrixXi& EF)
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

void prepareSmoothMatrix(const std::vector<Eigen::MatrixXd>& v_data, //vertex data
                        const Eigen::MatrixXd& Vavg, //average of faces
                        const Eigen::MatrixXi& EF, //edge flaps 
                        const Eigen::MatrixXi& uE, //unique edges
						const std::vector<Eigen::VectorXd>& uEL, //edge length
						Eigen::SparseMatrix<double>& A) // sparse matrix of smooth cost
{		
	A.resize(F.rows(), F.rows());
	A.setZero();

	for (int e = 0; e < uE.rows(); ++e) 
	{	
		int f1idx = EF(e,0);
		int f2idx = EF(e,1);
		int v1idx = uE(e,0);
		int v2idx = uE(e,1):
		
		double cost = 0;
		for (int frame = 0; frame < v_data.size(); ++frame) 
		{
			Vector3f v1 = v_data[frame].row(v1idx);
			Vector3f v2 = v_data[frame].row(v2idx); 	
			
			Vector3f v1avg = v_data[frame].row(v1idx);
			Vector3f v2avg = v_data[frame].row(v2idx);

			Vector3f v1diff = v1 - v1avg;
			Vector3f v2diff = v2 - v2avg;

			cost += uEL[frame](e) * (v1diff.norm() + v2diff.norm() + 1);
		}

		A.coeffRef(f1idx, f2idx) = cost;   
		A.coeffRef(f2idx, f1idx) = cost;
	}
}

GCoptimization::EnergyTermType smoothFnVertex(int p1, int p2, int l1, int l2, void *extraData) 
{	
	if (l1 == l2) 
	{ 
		return 0;
	}

	SegmentationData* data = (SegmentationData*)extraData;
	const auto& A = data->A;
	assert(p1 < A.rows() && p2 < A.rows());
	GCoptimization::EnergyTermType cost = A.coeffRef(p1, p2);
	return cost;
}

GCoptimization::EnergyTermType dataFnVertex(int p, int l, void* extraData) 
{
	SegmentationData* data = (SegmentationData*)extraData;
	const auto& U = data->U;
	//assert(p1 < A.rows() && p2 < A.rows());
	GCoptimization::EnergyTermType cost = U(p, l);
	return cost;
}

void tyro::segmentation(const std::vector<Eigen::MatrixXd>& v_data, //vertex data
                        const Eigen::MatrixXi& F, //faces
                        const Eigen::MatrixXd& Vavg, //average of faces
                        const Eigen::MatrixXi& inEF, //edge flaps 
                        const Eigen::MatrixXi& inuE,
						const std::vector<int> seeds)  //unique edges
{
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

	assert(seeds.size() == 2); //only support 2 labels right now.

	Eigen::VectorXi int_man;
	tyro::all(inEF, 
              [](const Eigen::VectorXi& v) -> bool { return v(0) != -1 && v(1) != -1;},
              1,
              int_man);

	MatrixXi EF, uE;
	igl::slice(inEF, int_man, 1, EF);
	igl::slice(inuE, int_man, 1, uE);
	//%% Edge lengths
	//%uEL = sqrt(sum((VV(uE(:,2),:,:)-VV(uE(:,1),:,:)).^2,2));
	std::vector<VectorXd> uEL;
	for (int f =0; f < v_data.size(); ++f) 
	{
		const auto& V = v_data[f];
		Eigen::MatrixXd L;
		igl::edge_lengths(V, uE, L);
		uEL.push_back(L.col(0));
	}
	
	//%% cost per-vertex, per-frame
	//%C = sqrt(sum((VV-Vavg).^2, 2));
	//%% For each interior edge add a penalty for cutting along it
	//%A = sparse( ...
	//%  EF(:,1), ...
	//%  EF(:,2), ...
	//%  sum(uEL.*(1+(C(uE(:,1),:,:) + C(uE(:,2),:,:))), 3), ...
	//%  size(F,1),size(F,1));
	Eigen::SparseMatrix<double> A;
	prepareSmoothMatrix(v_data, //vertex data
                        Vavg, //average of faces
                        EF, //edge flaps 
                        uE, //unique edges
						uEL, //edge length
						A) // sparse matrix of smooth cost
	int num_data = F.rows();
	int num_labels = 2;

	try
	{
		GCoptimizationGeneralGraph *gc = new GCoptimizationGeneralGraph(num_data, num_labels);
		SegmentationData* data = new SegmentationData();

		gc->setDataCost(&dataFnVertex, data);
		gc->setSmoothCost(&smoothFnVertex, data);
		setNeighbors(gc, num_data);

		double oldE = gc->compute_energy();
		gc->swap(-1);
		double newEnergy = gc->compute_energy();
		
		delete gc; 
	}
	catch(GCException e) 
	{
		e.Report();
	}
}

//%% Symmetrize
//%A = A + A';
%% Normalize
%A = A / max(abs(A(:)));
%
%Z = barycenter(Vavg,F)*R*[0;0;1];
%[~,imin] = min(Z);
%[~,imax] = max(Z);
%Thard = sparse([imax imax imin imin],[1 2 1 2],[1 0 0 1],size(F,1),2);
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