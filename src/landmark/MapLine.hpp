/*** 
 * @Author: yanyan-li yanyan.li.camp@gmail.com
 * @Date: 2022-09-17 16:49:21
 * @LastEditTime: 2022-10-21 16:39:36
 * @LastEditors: yanyan-li yanyan.li.camp@gmail.com
 * @Description: 
 * @FilePath: /venom/src/landmark/MapLine.hpp
 */

#ifndef __VENOM_SRC_LANDMARK_MAPLINE_HPP__
#define __VENOM_SRC_LANDMARK_MAPLINE_HPP__


#include "../estimator/Trajectory.hpp"


namespace simulator
{
    // Scalar -> Eigen::Vector3d  // Scalar -> cv::Matx31d
   class Trajectory; // Trajectory trajec_;

   class MapLine{
       public:
           // id of the mappoint
           int num_id_;
           // how many Trajectorys detect this point
           int observed;
           // type: 0, 1, 2
           int vanishing_direction_type_;
           // The normalized vanishing direction to which the 3D line belongs
           Eigen::Vector3d vanishing_direction_;
           // endpoints parametrization in the world coordinate
           Eigen::Matrix<double,3,2> pos_world_;
           Eigen::Matrix<double,3,2> pos_world_noise_; 
           // plucker parametrization in the world coordinate
           Eigen::Matrix<double,3,2> plucker_world_;
           Eigen::Matrix<double,3,2> plucker_world_noise_;

           Trajectory * traject_; 
           /**
            * @brief The mappoint is detected by the i_Trajectory ^{th} Trajectory,
            *        and associated to the i_pixel ^{th} pixel. 
            */
           std::map<int /*id_poxe*/, int /*i_pixel*/> observations_;
           std::vector< std::pair<int, Eigen::Matrix<double,3,2>>> vec_obs_;  // noised observation
           std::vector< std::pair<int, Eigen::Matrix<double,3,2>>> vec_obs_gt_; // gt observation
          
           std::random_device rd;
           std::default_random_engine generator_;
           std::normal_distribution<double> pixel_n_;
      
       public:
           MapLine(const int id, Trajectory* traject):num_id_(id),observed(0),traject_(traject)
           {
               // noise
               
               
               double max_nt = 0.1; double max_nq = 1.*M_PI/180.; double max_pixel_n = 1./240;
               
               //double max_pixel_n =1;

               std::random_device rd;
               std::default_random_engine generator(rd());
               std::normal_distribution<double> nt(0., max_nt);
               std::normal_distribution<double> nq(0., max_nq);
               std::normal_distribution<double> pixel_n(0, max_pixel_n);
               
               //std::normal_distribution<double> pixel_n(1, max_pixel_n);
               pixel_n_ = pixel_n;
               generator_ = generator;
               //pixel_n_ = pixel_n;

           };

           void AddNoise(Eigen::Matrix<double,3,2> &ob_gt, Eigen::Matrix<double, 3, 2> &ob_noise)
           {
            Eigen::Matrix2d pixel_line;
            //
            Reproject(ob_gt, pixel_line); 

            // 

            //std::random_device rd;
            //std::default_random_engine generator(rd());
            Eigen::Matrix2d noise;
            noise<<pixel_n_(generator_),pixel_n_(generator_),pixel_n_(generator_),pixel_n_(generator_);

            std::cout<<"pixel_line: "<<pixel_line<<", noise:"<<noise<<std::endl; 




           }

           void Reproject(Eigen::Matrix<double,3,2> &ob_gt, Eigen::Matrix2d &pixel_gt)
           {
            Eigen::Vector3d start_point = ob_gt.block(0,0,3,1);
            Eigen::Vector3d end_point = ob_gt.block(0,1,3,1);

            // (u-cx)/fx = x
            double fx = traject_->cam_intri.fx;
            double fy = traject_->cam_intri.fy;
            double cx = traject_->cam_intri.cx;
            double cy = traject_->cam_intri.cy;

            double start_u = fx * start_point(0)/start_point(2) + cx; 
            double start_v = fy * start_point(1)/start_point(2) + cy; 

            double end_u = fx * end_point(0)/end_point(2) + cx; 
            double end_v = fy * end_point(1)/end_point(2) + cy;


            pixel_gt(0,0) = start_u; pixel_gt(0,1)=end_u;
            pixel_gt(1,0) = start_v; pixel_gt(1,1)=end_v;

           }

           void GenerateMapLine(const double distance, std::string axis)
           {
               std::uniform_real_distribution<double> point_generate( -4., 4. ); // width distribution
               if(axis == "vertical-left")
               {
                pos_world_(0,0) = distance; pos_world_(0,1) = distance; 
                pos_world_(1,0) = point_generate(generator_); pos_world_(1,1) = pos_world_(1,0);
                pos_world_(2,0) = 2.; pos_world_(2,1) = -3.; 

                vanishing_direction_type_ = 0;
                vanishing_direction_ = (pos_world_.block(0,0,3,1)-pos_world_.block(0,1,3,1)).normalized();
                // std::cout<<" vanishing_direction: "<<vanishing_direction_<<std::endl;

               }
               else if(axis == "vertical-right")
               {
                pos_world_(0,0) = point_generate(generator_); pos_world_(0,1) = pos_world_(0,0);
                pos_world_(1,0) = distance; pos_world_(1,1) = distance; 
                pos_world_(2,0) = 2.; pos_world_(2,1) = -3.;
                vanishing_direction_type_ = 0; 
                vanishing_direction_ = (pos_world_.block(0,0,3,1)-pos_world_.block(0,1,3,1)).normalized();
                // std::cout<<" vanishing_direction: "<<vanishing_direction_<<std::endl;

               }
               else if(axis == "horizontal-left")
               {
                // x ,y, z
                pos_world_(0,0) = point_generate(generator_); pos_world_(0,1) = pos_world_(0,0);
                pos_world_(1,0) = -4.; pos_world_(1,1) = 4.; 
                pos_world_(2,0) = -3.2; pos_world_(2,1) = -3.2;
                vanishing_direction_type_ = 1;
                vanishing_direction_ = (pos_world_.block(0,0,3,1)-pos_world_.block(0,1,3,1)).normalized();
                // std::cout<<" vanishing_direction: "<<vanishing_direction_<<std::endl;

               }
               else if(axis == "horizontal-right")
               {
                // x ,y, z
                pos_world_(0,0) = -4.; pos_world_(0,1) = 4.; 
                pos_world_(1,0) = point_generate(generator_); pos_world_(1,1) = pos_world_(1,0);
                pos_world_(2,0) = -3.2; pos_world_(2,1) = -3.2;
                vanishing_direction_type_ = 2;
                vanishing_direction_ = (pos_world_.block(0,0,3,1)-pos_world_.block(0,1,3,1)).normalized();
                // std::cout<<" vanishing_direction: "<<vanishing_direction_<<std::endl;

               }
           }
          
           /**
            * @brief reproject the 3D mapline to camera view, we than can obtain whether the mappoint can be detected or not.
            *
            * @param keyframes_Twcs
            * @param add_noise : to generate noisy mappoint
            */
           void AddObservation(std::vector<Eigen::Matrix4d> keyframes_Twcs, bool add_nose)
           {
               //
               for(int i = 0; i< keyframes_Twcs.size(); ++i)
               {
                   auto Twc = keyframes_Twcs[i];
                   Eigen::Matrix4d Tcw = Twc.inverse();
                   Eigen::Matrix3d Rcw = Tcw.block(0,0,3,3);
                   //Eigen::Vector3d tcw = Tcw.block(0,3,3,1);
                   Eigen::Matrix<double,3,2> tcw(Eigen::Matrix<double,3,2>::Zero());
                   tcw.block(0,0,3,1) = Tcw.block(0,3,3,1);
                   tcw.block(0,1,3,1) = Tcw.block(0,3,3,1);
                    
                   Eigen::Matrix<double, 3,2> ob;
 
                   // in the camera coordinate
                   ob = Rcw * pos_world_ + tcw;
 
                   // 
                   if(ob(2,0) < 0) continue; // backside of the camera
                   if(ob(2,1) < 0) continue; //

                //    ob.block(0,0,3,1) = ob.block(0,0,3,1) / ob(2,0); // 
                //    ob.block(0,1,3,1) = ob.block(0,1,3,1) / ob(2,1); //      

                   // normalized image center 
                   Eigen::Vector3d center(0,0,1);
                   
                   //
                   Eigen::Matrix<double,3,2> ob_cam = ob;
                   ob_cam.block(0,0,3,1).normalize();
                   ob_cam.block(0,1,3,1).normalize();
                   // std::cout<<"\033[0;34m ob_cam: \033[0m"<<ob_cam<<std::endl;
                   
                   // ray angle 
                   Eigen::Vector3d ray = ob_cam.block(0,0,3,1);
                   double fov0 = std::acos(center.dot( ray)); 
                   fov0 = fov0 / M_PI * 180.;
                   if(fov0 > 60) continue;
                   ray = ob_cam.block(0,1,3,1);
                   fov0 = std::acos(center.dot( ray )); 
                   fov0 = fov0 / M_PI * 180.;
                   if(fov0 > 60) continue;

                   // vec：camera id,  
                   // vec: line id, 
                   // Eigen::Matrix<double,3,2>
                //    traject_->SetKeyFrameDetects(i, this->num_id_, ob); 
                //    //std::cout<<"the "<<i<<" th camera. "<<trajec_.contain_mp_cams_[i]<<std::endl;
                //    observed++;
                  
                   // observation: <key: Trajectory_id, value: 该相机坐标系下的(x_0,y_0,1)>d
                   
                   vec_obs_gt_.emplace_back(i,ob); 

                   
                   //if(add_nose && vec_obs_gt_.size() > 1)
                    if (add_nose)
                    {
                        Eigen::Matrix<double,3,2> noise;
                        AddNoise(ob, noise);
                        noise<< pixel_n_(generator_), pixel_n_(generator_),
                                pixel_n_(generator_), pixel_n_(generator_),
                                0, 0;
                        //std::cout<<"noise:"<<noise<<std::endl;       
                        ob += noise;
                    }
                    traject_->SetKeyFrameDetects(i, this->num_id_, ob); 
                    //std::cout<<"the "<<i<<" th camera. "<<trajec_.contain_mp_cams_[i]<<std::endl;
                    observed++;   


                   vec_obs_.emplace_back(i,ob); //obs.emplace_back(i, ob);
               }
 
           }
 
           void print()
           {
               std::cout<<"\033[0;31m The global position of mappoint is  \033[0m"<<std::endl
                        <<pos_world_<<std::endl<< "which is detected by "<<observed<<"cameras"<<std::endl;
 
           }
   };
}

#endif // __VENOM_SRC_LANDMARK_MAPLINE_HPP__