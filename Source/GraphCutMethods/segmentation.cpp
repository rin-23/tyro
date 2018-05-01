#include "segmentation.h"
#include <Eigen/SparseCore>
#include <igl/all_edges.h>
#include <igl/unique.h>

void tyro::segmentation(const std::vector<Eigen::MatrixXd>& v_data,
                        const Eigen::MatrixXi& F,
                        const Eigen::MatrixXd& Vavg) 
{
//R = axisangle2matrix([0 1 0],-pi/2)*axisangle2matrix([0 0 1],-pi/2);

//Vavg = mean(VV,3);
//Average onto faces
//F2V = sparse(F, repmat(1:size(F,1), 3, 1)',1/3,size(VV,1),size(F,1))';

%allE = [F(:,[2 3]); F(:,[3 1]); F(:,[1 2])]; // non unique edges stacked together allE is (F*3, 2)
%[sE, I] = sort(allE, 2); // sort each row so that [2 1; 3 0] turns into [1 2; 0 3]. sE = allE(I)
%[uE,~,J] = unique(sE,'rows');  // treat each row of sE as a single entity and return the unique rows of sE in sorted order. uE(J) = sE
%% face indices on either side of each edge
%EF = full(sparse(J, 
                  I(:,1), 
                  repmat(1:size(F,1), 1, 3)', 
                  size(uE,1), //uE.rows() // number of unique edges
                  2)); 

%% only keep interior manifold edges
%int_man = all(EF>0,2);
%EF = EF(int_man,:);
%uE = uE(int_man,:);
%% Edge lengths
%uEL = sqrt(sum((VV(uE(:,2),:,:)-VV(uE(:,1),:,:)).^2,2));
%% cost per-vertex, per-frame
%C = sqrt(sum((VV-Vavg).^2,2));
%% For each interior edge add a penalty for cutting along it
%A = sparse( ...
%  EF(:,1), ...
%  EF(:,2), ...
%  sum(uEL.*(1+(C(uE(:,1),:,:) + C(uE(:,2),:,:))),3), ...
%  size(F,1),size(F,1));
%% Symmetrize
%A = A + A';
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
%[~,L] = maxflow(w_smooth*A,w_hard*Thard+w_soft*Tsoft);
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