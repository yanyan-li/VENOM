/*
* @Author: yanyan.li yanyan.li.camp@gmail.com
* @Date: 2022-08-30 07:38:57
* @LastEditors: yanyan.li yanyan.li.camp@gmail.com
* @LastEditTime: 2022-09-07 12:09:08
* @FilePath: 
* @Description: test
*/
#include "basics.hpp"
#include "optimization.hpp"
#include<iostream>
#include<string>
 
int main(int argv, char** argc)
{
   if(argv==1)
       std::cout<<"usage:  "<<std::endl;
 
   // keyframe generation
   int frame_num = 100;
   ParaLi::Trajectory robot_trajectory;
   // generate a circular trajectory with 100 keyframes
   robot_trajectory.GenerateTrajectory(ParaLi::Trajectory::SPHERE, frame_num); 
   // robot_trajectory.PrintTrajectory();
 
   // landmarks generation
   double distance = 5.0; // distance between wall and the trajectory center
   bool add_noise_to_meas = true;
   std::vector<Eigen::Vector3d> points_gt;
   std::vector<std::vector<std::pair< int,Eigen::Vector3d>>> vec_meas_keyframe_mp;
   std::vector<std::vector<std::pair< int,Eigen::Vector3d>>> vec_gt_keyframe_mp;
 
   for(int id=0; id<200; id++)
   {
       ParaLi::MapPoint * ptr_mp = new ParaLi::MapPoint(id);
       if(id<50)
           ptr_mp->GenerateMapPoint(distance, 'x');   //left side of the wall
       else if(id<100)
           ptr_mp->GenerateMapPoint(-distance, 'x');  //right side
       else if(id<150)
           ptr_mp->GenerateMapPoint(distance, 'y');   // front side
       else if(id<200)
           ptr_mp->GenerateMapPoint(-distance, 'y');  // back side
 
       ptr_mp->AddObservation( robot_trajectory.traject_gt_Twc_, add_noise_to_meas);   
       //ptr_mp->print();
       points_gt.push_back(ptr_mp->pos_world_);
       // vec_meas_keyframe_mp: mappoint_id<camera_id, mappoint_value>
       vec_meas_keyframe_mp.push_back(ptr_mp->obs);
       vec_gt_keyframe_mp.push_back(ptr_mp->obs_gt);
 
   }
 
   ParaLi::Reconstruct recon;
   recon.Triangulation(vec_meas_keyframe_mp,robot_trajectory.traject_gt_Twc_);
  
   std::vector<Eigen::Matrix4d> vec_traject_Twc_opti;
   ParaLi::pointLocalBundleAdjustment::optimize(recon.tri_point_xyz_, vec_meas_keyframe_mp, robot_trajectory.traject_gt_Twc_, vec_traject_Twc_opti);
 
   //  visualization
   ParaLi::Visualizer viewer;
   viewer.SetParameter(points_gt, robot_trajectory.traject_gt_Twc_,
               vec_traject_Twc_opti, vec_meas_keyframe_mp,
               recon.tri_point_inverse_depth_, recon.tri_point_xyz_);
   viewer.show();
 
 
   //   
   return 0;
 
}
 
 
