 #include "console_functions.h"

  
void console_plot_error(App* app, const std::vector<std::string>& args) 
{   
    using namespace Eigen;
    
    Eigen::MatrixXf m1(2,3);
    Eigen::MatrixXf m2(2,3);
    Eigen::VectorXf v(2);

    m1 << 1, 2, 3,
            4, 5, 6;
    
    m2 << 1, 0, 0,
            0, 0, 0;
    
    VectorXf m = (m1-m2).rowwise().squaredNorm();

    std::cout << m.rows() << " " << m.cols() <<  std::endl;

    int num_parts = app->m_stop_motion.size();
    int num_frames = app->m_frame_data.v_data.size();
    app->m_error.resize(num_parts);
    app->max_error = std::numeric_limits<float>::min();
    for (int p = 0; p < num_parts; ++p) 
    {   
        if (app->m_stop_motion[p].computed) 
        {
            app->m_error[p].resize(num_frames); 
            for (int frame=0; frame < num_frames; ++frame) 
            {                   
                //VC[frame].resize(FD.v_data[0].rows());
                VectorXd lul = (app->m_pieces[p].v_data[frame] - app->m_stop_motion[p].anim.v_data[frame]).rowwise().squaredNorm();  
                float max = lul.maxCoeff();
                app->max_error = std::max(app->max_error, max); 
                app->m_error[p][frame] = lul;                          
            }
        }
    }
    app->m_computed_error =true;
}

void console_offset_frame(App* app, const std::vector<std::string>& args) 
{
    if (args.size() == 1) 
    {
        int start = std::stoi(args[0]);
//                int end = std::stoi(args[1]);
        app->m_frame_offset = start;
        app->render();
        glfwPostEmptyEvent();
    }
}

void console_laplacian_smooth_along_edges(App* app, const std::vector<std::string>& args) 
{
        using Eigen::Vector3d;
    //Taubian smoothing
    if (app->vid_list.size() > 0 ) 
    {   
        for (int iter = 0; iter < 1; ++iter) 
        {   
            const auto& FDV = app->m_frame_data.avg_v_data;
            MatrixXd Voriginal = app->m_frame_data.avg_v_data.eval();

            for (int i = 0; i < app->vid_list.size(); ++i) 
            {   
                Vector3d Lv;
                Lv.setZero();
                
                int left = i-1; 
                int right = i+1; 
                
                //check if v is boundary, then dont smooth
                if (i==0) 
                {
                    left = app->vid_list.size()-1;
                    right = 1;  
                }
                else if (i == app->vid_list.size()-1) 
                {
                    left = i-1;
                    right = 0;  
                }
                
                Vector3d v = FDV.row(app->vid_list[i]);
                Vector3d v1 = FDV.row(app->vid_list[left]);
                Vector3d v2 = FDV.row(app->vid_list[right]);  

                double w1 = 0.5;//1.0/(1000*(v1 - v).squaredNorm());
                double w2 = 0.5;//1.0/(1000*(v2 - v).squaredNorm());
                
                Lv += (w1)*v1;
                Lv += (w2)*v2;
                
                //Lv += app->m_frame_data.v_data[app->m_frame].row(vid_n);
                Lv = (1.0/(w1+w2)) * Lv - v;
                //std::cout<< Lv;
                Voriginal.row(app->vid_list[i]) +=  0.5*Lv;
                
            }
            app->m_frame_data.avg_v_data = Voriginal.eval();
        }

        
        app->render();
        glfwPostEmptyEvent;
    }
    
}


void console_taubin_smooth_along_edges(App* app, const std::vector<std::string>& args) 
{   
    using Eigen::Vector3d;
    //Taubian smoothing
    if (app->vid_list.size() > 0) 
    {   

        for (int iter = 0; iter < 6; ++iter) 
        {   
            const auto& FDV = app->m_frame_data.avg_v_data;
            MatrixXd Voriginal = app->m_frame_data.avg_v_data.eval();

            for (int i = 0; i < app->vid_list.size(); ++i) 
            {   
                Vector3d Lv;
                Lv.setZero();
                
                int left = i-1; 
                int right = i+1; 
                
                //check if v is boundary, then dont smooth
                if (i==0) 
                {
                    left = app->vid_list.size()-1;
                    right = 1;  
                }
                else if (i == app->vid_list.size()-1) 
                {
                    left = i-1;
                    right = 0;  
                }
                
                Vector3d v = FDV.row(app->vid_list[i]);
                Vector3d v1 = FDV.row(app->vid_list[left]);
                Vector3d v2 = FDV.row(app->vid_list[right]);  

                double w1 = 0.5;//1.0/(1000*(v1 - v).squaredNorm());
                double w2 = 0.5;//1.0/(1000*(v2 - v).squaredNorm());
                
                Lv += (w1)*v1;
                Lv += (w2)*v2;
                
                //Lv += app->m_frame_data.v_data[app->m_frame].row(vid_n);
                Lv = (1.0/(w1+w2)) * Lv - v;
                //std::cout<< Lv;
                if (iter%2 == 0)
                    Voriginal.row(app->vid_list[i]) +=  0.5*Lv;
                else
                    Voriginal.row(app->vid_list[i]) += -0.55*Lv;
                
            }
            app->m_frame_data.avg_v_data = Voriginal.eval();
        }

        
        app->render();
        glfwPostEmptyEvent;
    }
    
}

void console_save_serialised_data(App* app, const std::vector<std::string>& args) 
{   
    if (args.size() != 2) 
        return; 

    auto type = args[0];
    auto filename = args[1];

    auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
    auto p = f/filesystem::path(filename);
    std::ofstream os(p.str(), std::ios::binary);
    cereal::BinaryOutputArchive archive(os);
    
    if (type == "frames") 
    {
        archive(app->m_frame_data);
    } 
    else if (type == "deform") 
    {
        archive(app->m_frame_deformed_data);
    }
    else if (type == "split") 
    {
        archive(app->m_pieces);
    }
    else if (type == "stop") 
    {
        archive(app->m_stop_motion);
    }
    else if (type == "stop_low") 
    {
        archive(app->m_stop_motion[1]);
    }
    else if (type == "stop_up") 
    {
        archive(app->m_stop_motion[0]);
    }
}

void console_load_serialised_data(App* app, const std::vector<std::string>& args) 
{   
    if (args.size() < 2) 
        return; 
    
    auto type = args[0];
    auto filename = args[1];

    auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
    auto p = f/filesystem::path(filename);

    std::ifstream in = std::ifstream(p.str(), std::ios::binary);
    cereal::BinaryInputArchive archive(in);
    
    if (type == "frames") 
    {
        archive(app->m_frame_data);

        app->m_timeline->SetFrameRange(app->m_frame_data.v_data.size()-1);

        //Compute radius of the bounding box of the model
        AxisAlignedBBox bbox;
        MatrixXd VT = app->m_frame_data.v_data[0].transpose();
        bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
        app->m_model_offset = bbox.GetRadius(); 

        app->m_update_camera = true;
        app->m_state = App::State::LoadedModel;
    } 
    else if (type == "deform") 
    {
        archive(app->m_frame_deformed_data);
        app->m_computed_deformation = true;
        app->m_update_camera = true;
    }
    else if (type == "split") 
    {
        archive(app->m_pieces);
        app->m_computed_parts = true;            
        app->m_update_camera = true;
    }
    else if (type == "stop") 
    {
        archive(app->m_stop_motion);
        for (int i = 0; i < app->m_stop_motion.size(); ++i)
            if (app->m_stop_motion[i].anim.v_data.size() > 0)
                app->m_stop_motion[i].computed = true;
        
        app->m_computed_stop_motion = true;
        app->m_update_camera = true;

    }
    else if (type == "stop_low") 
    {   
        if (app->m_stop_motion.empty())
            app->m_stop_motion.resize(2);
        
        archive(app->m_stop_motion[1]);
        app->m_stop_motion[1].computed = true;
        
        app->m_computed_stop_motion = true;
        app->m_update_camera = true;
    }
    else if (type == "stop_up") 
    {
        if (app->m_stop_motion.empty())
            app->m_stop_motion.resize(2);
        
        archive(app->m_stop_motion[0]);
        app->m_stop_motion[0].computed = true;
        
        app->m_computed_stop_motion = true;
        app->m_update_camera = true;
    }
    app->render();
    glfwPostEmptyEvent();
}

void console_clear_selection(App* app, const std::vector<std::string>& args) 
{
    app->vid_list.clear();
    app->ball_list.clear();
    
    for (int fid = 0; fid < app->m_frame_data.f_data.rows(); ++fid) 
    {
        app->setFaceColor(fid, false);
    }
    app->fid_list.clear();        
    app->fid_list2.clear();
    app->fid_list3.clear();
    app->render();
}

void console_clear_vertex_selection(App* app, const std::vector<std::string>& args) 
{
    app->vid_list.clear();
    app->ball_list.clear();
    app->render();
}

void console_clear_face_selection(App* app, const std::vector<std::string>& args) 
{
    for (int fid = 0; fid < app->m_frame_data.f_data.rows(); ++fid) 
    {
        app->setFaceColor(fid, false);
    }
    app->fid_list.clear();        
    app->fid_list2.clear();
    app->fid_list3.clear();
    app->render();
}

void console_set_vertex_weight(App* app, const std::vector<std::string>& args) 
{
    if (args.size()>0) 
    {   
        float w = std::stof(args[0]);
        for (auto vid : app->vid_list) 
        {
            app->m_weights.VW(vid) = w;
        }
    }
}

void console_set_face_weight(App* app, const std::vector<std::string>& args) 
{
    if (args.size()>0) 
    {   
        float w = std::stof(args[0]);
        for (auto fid : app->fid_list) 
        {
            app->m_weights.FW(fid) = w;
        }
    }
}

void console_draw_vertex_weight_map(App* app, const std::vector<std::string>& args) 
{   
    double maxC = app->m_weights.VW.maxCoeff();
    double minC = app->m_weights.VW.minCoeff(); //should be 1
    
    for (int vid = 0; vid < app->m_weights.VW.size(); ++vid) 
    {   
        double w = app->m_weights.VW(vid); 
        if (w > 1) 
        {   
            double red = w / (maxC - minC); 
            app->addSphere(vid, Wm5::Vector4f(red, 0, 0, 1));
        }
    }
}

void console_draw_face_weight_map(App* app, const std::vector<std::string>& args) 
{   
    double maxC = app->m_weights.FW.maxCoeff();
    double minC = app->m_weights.FW.minCoeff(); //should be 1
    
    for (int fid = 0; fid < app->m_weights.FW.size(); ++fid) 
    {   
        double w = app->m_weights.FW(fid); 
        //if (w > 1) 
        //{   
            double green = w / (maxC - minC); 
            Eigen::Vector3d clr(0, green, 0);
            app->setFaceColor(fid, clr);
        //}
    }
} 

void console_segmentation(App* app, const std::vector<std::string>& args) 
{  
    if (!(app->fid_list.size() > 0 && 
        app->fid_list2.size() > 0 &&
        app->fid_list3.size() > 0 &&
        args.size() == 1)) 
    { 
        return;   
    }   

    double smooth_weight = (double)std::stoi(args[0]);
    
    const auto& FD = app->m_frame_data;

    VectorXi L;
    VectorXi flist1 = Eigen::Map<VectorXi>(app->fid_list.data(), 
                                            app->fid_list.size());
    MatrixXi F;
    igl::slice(FD.f_data, flist1, 1, F); 

    VectorXi seeds2, seeds3; //foreground and background         
    seeds2.resize(app->fid_list2.size());
    seeds3.resize(app->fid_list3.size());
    
    for (int i = 0; i < app->fid_list2.size(); ++i)
    {
        for (int j = 0; j < app->fid_list.size(); ++j) 
        {
            if (app->fid_list2[i] == app->fid_list[j]) 
            {
                seeds2(i) = j;
            }
        }
    }

    for (int i = 0; i < app->fid_list3.size(); ++i)
    {
        for (int j = 0; j < app->fid_list.size(); ++j) 
        {
            if (app->fid_list3[i] == app->fid_list[j]) 
            {
                seeds3(i) = j;
            }
        }
    }

    assert(seeds2.maxCoeff() < F.rows());
    assert(seeds3.maxCoeff() < F.rows());

    std::vector<MatrixXd> v_data;
    MatrixXi NF, I;
    for (int i = 0; i < FD.v_data.size(); ++i) 
    {   
        MatrixXd NV;
        igl::remove_unreferenced(FD.v_data[i], F, NV, NF, I);
        v_data.push_back(NV);
    }

    MatrixXd Vavg;
    igl::remove_unreferenced(FD.avg_v_data, F, Vavg, NF, I);
            
    tyro::segmentation(v_data, 
                        NF,
                        Vavg,
                        seeds2,
                        seeds3,
                        smooth_weight,
                        L);
    
    for (int i = 0; i < L.size(); ++i) 
    {
        if (L(i) == 1) 
        {
            app->setFaceColor(flist1[i], Eigen::Vector3d(0,0.5,0));
        }
        else 
        {
            app->setFaceColor(flist1[i], Eigen::Vector3d(0.5,0,0));                
        }
    }

    //TODO:find boundary
    /*
    const auto & cE = FD.ue_data;
    const auto & cEMAP = FD.EMAP;
    MatrixXi EF, EI;

    igl::edge_flaps(FD.f_data, 
                    cE, 
                    cEMAP, 
                    EF, 
                    EI);
    
    for (int i = 0; i < FD.ue_data.rows(); ++i) 
    {               
        int f1 = EF(i, 0);
        int f2 = EF(i, 1);
        
        auto it1 = std::find(app->fid_list.begin(), app->fid_list.end(), f1);
        auto it2 = std::find(app->fid_list.begin(), app->fid_list.end(), f2);
        
        if (it1 != app->fid_list.end() && it2 != app->fid_list.end()) 
        {   
            int idx1 = it1 - app->fid_list.begin();
            int idx2 = it2 - app->fid_list.begin();
            if (L(idx1) != L(idx2)) 
            {
                app->eid_list.push_back(i);                        
                app->m_frame_data.ec_data.row(i) = Eigen::Vector3d(0,0.0,0.8);

                Eigen::Vector2i evec = FD.ue_data.row(i);
                //if direction is switched after a mapping directed to undirected
                if (!(evec(0)== e1(0) && evec(1) == e1(1))) 
                {
                    DMAP(i) = 1;
                }
            }
        }
    }    */ 
}

void console_upsample(App* app, const std::vector<std::string> & args) 
{
    auto& FD = app->m_frame_data;
    Eigen::SparseMatrix<double> S;
    Eigen::MatrixXi newF;
    igl::upsample(FD.v_data[0].rows(), FD.f_data, S, newF);
    FD.f_data = newF;

    for (int i =0; i < FD.v_data.size(); ++i) 
    {
        FD.v_data[i] = S * FD.v_data[i];
        
        Eigen::MatrixXd N;
        int num_face = FD.f_data.rows();
        igl::per_vertex_normals(FD.v_data[i], FD.f_data, N); 
        FD.n_data[i] = N;

        std::vector<std::vector<int> > uE2E;
        igl::unique_edge_map(FD.f_data,
                                FD.e_data,
                                FD.ue_data,
                                FD.EMAP,
                                uE2E);
        Eigen::Vector3d face_color(0.5,0.5,0.5);
        tyro::color_matrix(FD.f_data.rows(), face_color, FD.fc_data);
        tyro::color_black_matrix(FD.ue_data.rows(), FD.ec_data);
    }

    app->compute_average();                       
}

void console_show_wireframe(App* app, const std::vector<std::string> & args) 
{
    if (args.size() == 1) 
    {
        int show = std::stoi(args[0]);
        app->m_show_wireframe = show;
        app->render();                
    }
}

void console_deselect_faces(App* app, const std::vector<std::string> & args) 
{
    for (auto fid : app->fid_list) 
    {
        app->setFaceColor(fid, false);
    }
    app->fid_list.clear();
    app->render();
    glfwPostEmptyEvent(); 
}

void console_deselect_verticies(App* app, const std::vector<std::string> & args) 
{
    app->removeSpheres(app->vid_list);
    app->vid_list.clear();
    app->render();
    glfwPostEmptyEvent();
}

void console_show_edge_selection(App* app, const std::vector<std::string> & args) 
{
    RA_LOG_INFO("Converting vertex to edge selection");
    
    MatrixXi eid_list;
    VectorXi EI, uEI, DMAP;
    convert_vertex_to_edge_selection(app->vid_list, 
                                        app->m_frame_data.e_data, 
                                        app->m_frame_data.ue_data, 
                                        app->m_frame_data.EMAP,
                                        eid_list, 
                                        EI, 
                                        uEI, 
                                        DMAP);

    for (int i = 0; i < uEI.size(); ++i) 
    {   
        app->m_frame_data.ec_data.row(uEI(i)) = Eigen::Vector3d(0,0.8,0);
    }
    
    //debug_show_faces_near_edge_selection(uEI, DMAP);       
            
    app->render();
    glfwPostEmptyEvent();
    
    return;
}

void console_split_mesh(App* app, const std::vector<std::string> & args) 
{
    RA_LOG_INFO("Splitting mesh");
    
    if (app->vid_list.size() == 0 || app->m_frame_deformed_data.v_data.size() == 0) 
    {   
        RA_LOG_INFO("Not enough information");
        return;
    }

    if (!igl::is_edge_manifold(app->m_frame_deformed_data.f_data)) 
    {   
        Eigen::MatrixXi P;
        igl::extract_manifold_patches(app->m_frame_deformed_data.f_data, P);
        //int a = P.minCoeff();
        //int b = P.maxCoeff();
        using Eigen::Vector3d;
        int a = 0, b = 0, c = 0;

        for (int i = 0; i < P.rows(); ++i) 
        {   
            Vector3d clr;
            if (P(i,0)==0) {
                clr = Vector3d(1,0,0);
                a++;}
            
            if (P(i,0)==1){ 
                clr = Vector3d(0,1,0);
                b++;}

            if (P(i,0)==2) { 
                clr = Vector3d(0,0,1);
                c++;}

            app->setFaceColor(i, clr);
        }
        RA_LOG_ERROR("Mesh is not edge manifold");
        return;
    }

    MatrixXi eid_list;
    VectorXi EI, uEI, DMAP;
    convert_vertex_to_edge_selection(app->vid_list, 
                                        app->m_frame_deformed_data.e_data, 
                                        app->m_frame_deformed_data.ue_data, 
                                        app->m_frame_deformed_data.EMAP,
                                        eid_list, 
                                        EI, 
                                        uEI, 
                                        DMAP);


    MatrixXi F1, F2;
    tyro::mesh_split(app->m_frame_deformed_data.f_data,
                        uEI,
                        DMAP, 
                        F1, 
                        F2);
    
    app->m_pieces.resize(2);
    auto& A1 = app->m_pieces[0];
    auto& A2 = app->m_pieces[1];
    A1.v_data.resize(app->m_frame_deformed_data.v_data.size());
    A2.v_data.resize(app->m_frame_deformed_data.v_data.size());
    A1.n_data.resize(app->m_frame_deformed_data.v_data.size());
    A2.n_data.resize(app->m_frame_deformed_data.v_data.size());
    
    A1.sequenceIdx = app->m_frame_data.sequenceIdx;
    A2.sequenceIdx = app->m_frame_data.sequenceIdx;
    
    for (int i = 0; i < app->m_frame_data.v_data.size(); ++i) 
    {                   
        MatrixXi I1, I2;    
        igl::remove_unreferenced(app->m_frame_deformed_data.v_data[i], 
                                    F1, 
                                    A1.v_data[i], 
                                    A1.f_data, 
                                    I1);

        igl::per_vertex_normals(A1.v_data[i], A1.f_data, A1.n_data[i]);
        std::vector<std::vector<int> > uE2E1;
        igl::unique_edge_map(A1.f_data,A1.e_data,A1.ue_data,A1.EMAP,uE2E1);
        tyro::color_matrix(A1.f_data.rows(), Eigen::Vector3d(0.2,0.2,0.2), A1.fc_data);
        tyro::color_black_matrix(A1.e_data.rows(), A1.ec_data);

        igl::remove_unreferenced(app->m_frame_deformed_data.v_data[i], 
                                    F2, 
                                    A2.v_data[i], 
                                    A2.f_data, 
                                    I2);
        
        igl::per_vertex_normals(A2.v_data[i], A2.f_data, A2.n_data[i]);
        std::vector<std::vector<int> > uE2E2;
        igl::unique_edge_map(A2.f_data,A2.e_data,A2.ue_data,A2.EMAP,uE2E2);
        tyro::color_matrix(A2.f_data.rows(), Eigen::Vector3d(0.6,0.6,0.6), A2.fc_data);
        tyro::color_black_matrix(A2.e_data.rows(), A2.ec_data);
    }

        
    app->m_computed_parts = true;            
    app->m_update_camera = true;

    return;
}

void console_frame(App* app, const std::vector<std::string> & args) 
{
    RA_LOG_INFO("Jump to a new frame");

    if (args.size() == 1)
    {   int frame = std::stoi(args[0]);
        app->m_timeline->SetFrame(frame);
        return;
    }
}

void console_stop_motion(App* app, const std::vector<std::string> & args) 
{
    RA_LOG_INFO("running stop motion");

    if (args.size() == 4)
    {   
        int num_labels = std::stoi(args[0]);
        double smooth_weight = std::stof(args[1]);
        int part_id = std::stoi(args[2]);
        std::string initmethod = args[3]; 
        
        if (app->m_stop_motion.empty())
        //app->m_stop_motion.clear();
            app->m_stop_motion.resize(app->m_pieces.size());

        int start, end;
        if (part_id == -1) 
        {
            start = 0;
            end = app->m_stop_motion.size();
        }
        else
        {   
            //just do one part
            start = part_id;
            end = part_id+1;
        }
        
        for (int i = start; i < end; ++i) 
        {   
            double result_energy;
            auto& sm = app->m_stop_motion[i]; 
            auto& piece = app->m_pieces[i];
            bool kmeans = false;
            if (initmethod == "kmeans") kmeans = true; 
            tyro::stop_motion_vertex_distance(num_labels, 
                                                smooth_weight,
                                                kmeans,
                                                piece.v_data,
                                                piece.sequenceIdx,
                                                piece.f_data,
                                                sm.D, //dictionary
                                                sm.L, //labels,  
                                                result_energy);
            
            sm.anim.f_data = piece.f_data;
            sm.anim.e_data = piece.e_data;
            sm.anim.ue_data = piece.ue_data;
            sm.anim.EMAP = piece.EMAP;
            sm.anim.ec_data = piece.ec_data;
            sm.anim.fc_data = piece.fc_data;
            //precompute normals
            std::vector<MatrixXd> normals;
            normals.resize(sm.D.size());
            for (int j = 0; j < sm.D.size(); ++j) 
            {   
                igl::per_vertex_normals(sm.D[j], sm.anim.f_data, normals[j]);
            }

            for (int j = 0; j < sm.L.size(); ++j) 
            {
                int l_idx = sm.L(j);
                sm.anim.v_data.push_back(sm.D[l_idx]);
                sm.anim.n_data.push_back(normals[l_idx]);
            }
            sm.computed = true;                   
        }

        app->m_computed_stop_motion = true;
        app->m_update_camera = true;

        app->render();
        glfwPostEmptyEvent();
    }
}

void console_align_all_models(App* app, const std::vector<std::string> & args) 
{
    RA_LOG_INFO("aligning models");
    app->align_all_models();
    return;
}

void console_save_mesh_sequence_with_selected_faces(App* app, const std::vector<std::string> & args) 
{
    RA_LOG_INFO("Saving mesh sequence with selected faces");
    
    if (args.size() == 2 && app->fid_list.size() > 0) 
    {   
        auto& type = args[0];
        auto wheretosave = filesystem::path(args[1]);
        
        MatrixXi newF;
        VectorXi slice_list = Eigen::Map<VectorXi>(app->fid_list.data(), app->fid_list.size());
        igl::slice(app->m_frame_data.f_data, slice_list, 1, newF);
        int start_frame = 0;

        //check that the slice is manifold
        if (!igl::is_edge_manifold(newF)) 
        {
            Eigen::MatrixXi P;
            igl::extract_manifold_patches(newF, P);
            std::vector<int> Pindx;
            for (int fid=0; fid < P.rows(); ++fid) 
            {
                if (P(fid,0) == 0) Pindx.push_back(fid); //TODO HACK assume path 0 is the largest one
            }
            MatrixXi manifoldF;
            slice_list = Eigen::Map<VectorXi>(Pindx.data(), Pindx.size());
            igl::slice(newF, slice_list, 1, manifoldF);
            newF = manifoldF;
        }

        if (type == "obj") 
        {
            for (int file = 0; file < app->m_frame_data.sequenceIdx.size(); ++file) 
            {   
                auto folder = filesystem::path(app->FOLDERS[file])/wheretosave;

                if (!folder.exists()) 
                {
                    filesystem::create_directory(folder);
                }
                
                auto objlist_path = folder/filesystem::path("objlist.txt");
                int num_frames = app->m_frame_data.sequenceIdx[file];
                
                ofstream objlist_file;
                objlist_file.open (objlist_path.str());
            
                for (int frame = start_frame; frame < start_frame + num_frames; ++frame) 
                {   
                    assert(frame < app->m_frame_data.v_data.size());
                    MatrixXd temp_V;
                    MatrixXi temp_F;
                    VectorXi I;
                    igl::remove_unreferenced(app->m_frame_data.v_data[frame], newF, temp_V, temp_F, I);
                    auto file_name = filesystem::path(tyro::pad_zeros(frame) + std::string(".obj"));
                    auto file_path = folder/file_name; 
                    igl::writeOBJ(file_path.str(), temp_V, temp_F);
                    objlist_file << file_name << "\n";
                }
                
                start_frame += num_frames;
                
                objlist_file.close();             
            } 
        }
        else if (type == "binary")
        {     
            App::MAnimation to_save;
            to_save.v_data.resize(app->m_frame_data.v_data.size());
            to_save.n_data.resize(app->m_frame_data.v_data.size());
            for (int frame = 0; frame < app->m_frame_data.v_data.size(); ++frame) 
            {   
                MatrixXi temp_F;
                VectorXi I;
                igl::remove_unreferenced(app->m_frame_data.v_data[frame], newF, to_save.v_data[frame], temp_F, I);
                igl::per_vertex_normals(to_save.v_data[frame], temp_F, to_save.n_data[frame]);
                if (frame == 0) 
                {
                    to_save.f_data =temp_F;
                }
            }  
        
            std::vector<std::vector<int> > uE2E;
            igl::unique_edge_map(to_save.f_data, 
                                    to_save.e_data,
                                    to_save.ue_data,
                                    to_save.EMAP,
                                    uE2E);
            
            //@TODO need this to update camera
            Eigen::Vector3d face_color(0.5,0.5,0.5);
            tyro::color_matrix(to_save.f_data.rows(), face_color, to_save.fc_data);
            tyro::color_black_matrix(to_save.ue_data.rows(), to_save.ec_data);

            to_save.sequenceIdx = app->m_frame_data.sequenceIdx;

            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
            auto p = f/filesystem::path(wheretosave);
            std::ofstream os(p.str(), std::ios::binary);
            cereal::BinaryOutputArchive archive(os);
            archive(to_save);
        }
    }

    return;

}

void console_load_selected_faces(App* app, const std::vector<std::string> & args) 
{
    RA_LOG_INFO("Loading selected faces from tmp folder");

    if (args.size() == 1)
    {   
        app->load_selected_faces(args[0]);
        return;
    }
}

void console_save_selected_faces(App* app, const std::vector<std::string> & args) 
{
    RA_LOG_INFO("Saving selected faces to tmp folder");

    if (args.size() == 1)
    {   
        app->save_selected_faces(args[0]);
        return;
    }
}

void console_save_selected_verticies(App* app, const std::vector<std::string> & args) 
{
    RA_LOG_INFO("Saving selected verticies to tmp folder");

    if (args.size() == 1)
    {   
        app->save_selected_verticies(args[0]);
        return;
    }
}

void console_load_selected_verticies(App* app, const std::vector<std::string> & args) 
{
    RA_LOG_INFO("Loading selected verticies from tmp folder");

    if (args.size() == 1)
    {   
        app->load_selected_verticies(args[0]);
        return;
    }
}

void console_set_sel_primitive(App* app, const std::vector<std::string> & args)
{
    RA_LOG_INFO("Set selection type");

    if (args.size() == 1)
    {      
        if (args[0] == "vertex") 
            app->set_sel_primitive(App::SelectionPrimitive::Vertex);
        else if (args[0] == "faces")
            app->set_sel_primitive(App::SelectionPrimitive::Faces);
        else if (args[0] == "edges")
            app->set_sel_primitive(App::SelectionPrimitive::Edges);
        return;
    }
}

void console_set_sel_method(App* app, const std::vector<std::string> & args)
{
    RA_LOG_INFO("Set selection type");

    if (args.size() == 1)
    {      
        if (args[0] == "oneclick") 
            app->set_sel_method(App::SelectionMethod::OneClick);
        else if (args[0] == "square")
            app->set_sel_method(App::SelectionMethod::Square);
        
        return;
    }
}

void console_load_oldman(App* app, const std::vector<std::string> & args) 
{
    RA_LOG_INFO("Loading oldman obj sequence");
    app->load_oldman();
    return;
}

void console_load_monka(App* app, const std::vector<std::string> & args) 
{
    RA_LOG_INFO("load monka");
    bool serialized = true;
    if (args.size() == 1) 
        serialized = std::stoi(args[0]);

    //Picked face_id 26093 vertex_id 13681 coord 11.950990 2.934150 16.501955
    //face_id 40217 vertex_id 10585 coord -1.194749 -0.823966 1.658947
    //Picked face_id 31779 vertex_id 12996 coord 0.012255 1.557947 0.744930
    int offset_vid = 12996; // 1222;
    auto offset = Eigen::Vector3d(0.012255, 1.557947, 0.744930) ; //Eigen::Vector3d(0.613322, 2.613381, 2.238946);

    app->FOLDERS = 
    {   
        std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/01/02/"),
        std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/01/03/"),
        std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/01/06/"),
        std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/01/08/"),
        std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/01/09/"),
        std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/01/10/"),
        std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/01/11/"),
        std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/01/12/"),
        //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/01/13/"),
        
        std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/03/01/"),
        
        std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/05/03/"),
        std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/05/04/"),
        std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/05/06/"),
    
        std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/06/01/")
    };

    if (serialized) 
    {
        auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
        auto p = f/filesystem::path("monka_manifold");
        std::ifstream in = std::ifstream(p.str(), std::ios::binary);
        cereal::BinaryInputArchive archive_i(in);
        archive_i(app->m_frame_data);
    }
    else
    {
        std::vector<std::string> obj_paths;
        for (int i = 0; i < app->FOLDERS.size(); ++i) 
        {
            int num_files_read;

            //Add smth
            auto folder = app->FOLDERS[i];
            folder += std::string("face/");

            RA_LOG_INFO("loading folder %s", folder.data());
            tyro::obj_file_path_list(folder, "objlist.txt", obj_paths, num_files_read);
            RA_LOG_INFO("frames read %i", num_files_read);
            app->m_frame_data.sequenceIdx.push_back(num_files_read);
            app->load_mesh_sequence(obj_paths, true); //use IGL obj loader
        }
    }

    
    if (!igl::is_edge_manifold(app->m_frame_data.f_data)) 
    {   
        RA_LOG_ERROR_ASSERT("not manifold");
        return;
    }
    app->m_timeline->SetFrameRange(app->m_frame_data.v_data.size()-1);

    //app->align_all_models(offset_vid, offset);
        
    //Compute radius of the bounding box of the model
    AxisAlignedBBox bbox;
    MatrixXd VT = app->m_frame_data.v_data[0].transpose();
    bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
    app->m_model_offset = bbox.GetRadius(); 

    app->m_update_camera = true;
    app->m_state = App::State::LoadedModel;
    app->compute_average();

    app->m_weights.VW.resize(app->m_frame_data.v_data[0].rows());
    app->m_weights.VW.setOnes();

    app->m_weights.FW.resize(app->m_frame_data.f_data.rows());
    app->m_weights.FW.setOnes();
    
    app->compute_average();

    app->render();
}

void console_load_bunny(App* app, const std::vector<std::string> & args) 
{   
    RA_LOG_INFO("Loading bunny obj sequence");
    
    if (args.size() == 1) 
    {
        int a = std::stoi(args[0]);
        app->load_bunny(a);
    }
    else 
    {
        app->load_bunny();
    }
    return;
}

void console_load_blobby(App* app, const std::vector<std::string> & args) 
{
    RA_LOG_INFO("Loading blobby obj sequence");
    app->load_blobby();
}

void console_compute_average(App* app, const std::vector<std::string> & args) 
{
    RA_LOG_INFO("compute_average");
    app->compute_average();
}

void console_compute_deformation(App* app, const std::vector<std::string> & args)
{
    RA_LOG_INFO("computing deformation");

    if (app->vid_list.empty() || app->fid_list.empty() || !app->m_computed_avg) 
    { 
        RA_LOG_WARN("Need vertex/face selection and average mesh to compute deformation")
        return;
    }
    //app->m_frame_deformed_data.v_data.clear();
    bool result = tyro::compute_deformation(app->vid_list, 
                                            app->fid_list,
                                            app->m_frame_data.v_data,
                                            app->m_frame_data.f_data,
                                            app->m_frame_data.avg_v_data,
                                            app->m_frame_deformed_data.v_data);
    assert(result);
    tyro::copy_animation(app->m_frame_data, app->m_frame_deformed_data, true, true, true);
    app->m_computed_deformation = true;
    app->m_update_camera = true;
    //app->render();
}

    void console_invert_face_selection(App* app, const std::vector<std::string> & args) 
{
    RA_LOG_INFO("invert face selection");
    app->invert_face_selection();
    return;
}

