/**
*
* 函数功能:读取pcl点云文件并发布到topic上去
*
*
* maker: crp 
* data: 2016-6-8
*/


#include<iostream>
#include<string>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <vector>

// #include<ros/ros.h>  
// #include<pcl/point_cloud.h>  
// #include<pcl_conversions/pcl_conversions.h>  
// #include<sensor_msgs/PointCloud2.h>  
// #include<pcl/io/pcd_io.h>
    #include<ros/ros.h>  
    #include<pcl/point_cloud.h>  
    #include<pcl_conversions/pcl_conversions.h>  
    #include<sensor_msgs/PointCloud2.h>  
    #include <pcl/common/transforms.h>
    #include<pcl/io/pcd_io.h>
using namespace std;


int main (int argc, char **argv)  
{  

    
	std::string topic,path,frame_id;
        int hz=5;

	ros::init (argc, argv, "publish_pointcloud");  
	ros::NodeHandle nh;  

    nh.param<std::string>("path", path, "/home/robot/LYD.pcd/last.pcd");
	nh.param<std::string>("frame_id", frame_id, "Vlp-16");
	nh.param<std::string>("topic", topic, "/pointcloud/output");
        nh.param<int>("hz", hz, 5);
   
	ros::Publisher pcl_pub = nh.advertise<sensor_msgs::PointCloud2> (topic, 10);  

	pcl::PointCloud<pcl::PointXYZ> cloud;  
	sensor_msgs::PointCloud2 output;  
	pcl::io::loadPCDFile (path, cloud);  
	pcl::toROSMsg(cloud,output);// 转换成ROS下的数据类型 最终通过topic发布d
    Eigen::Affine3f transform = Eigen::Affine3f::Identity();
    transform.rotate(Eigen::AngleAxisf(-M_PI/2, Eigen::Vector3f(1,0,0)));
    pcl::transformPointCloud(cloud, cloud, transform);
 	pcl::toROSMsg(cloud,output);
	output.header.stamp=ros::Time::now();
	output.header.frame_id  =frame_id;

	cout<<"path = "<<path<<endl;
	cout<<"frame_id = "<<frame_id<<endl;
	cout<<"topic = "<<topic<<endl;
	cout<<"hz = "<<hz<<endl;

	ros::Rate loop_rate(hz);  
	while (ros::ok())  
	{  
		pcl_pub.publish(output);  
		ros::spinOnce();  
		loop_rate.sleep();  
	}  
	return 0;  
}  


    // #include<ros/ros.h>  
    // #include<pcl/point_cloud.h>  
    // #include<pcl_conversions/pcl_conversions.h>  
    // #include<sensor_msgs/PointCloud2.h>  
    // #include <pcl/common/transforms.h>
    // #include<pcl/io/pcd_io.h>
      
    // int main (int argc, char **argv)  
    // {  
    //   ros::init (argc, argv, "publish_pointcloud");  
    //   ros::NodeHandle nh;  
    //   ros::Publisher pcl_pub = nh.advertise<sensor_msgs::PointCloud2> ("topic", 10);  
      
    //   pcl::PointCloud<pcl::PointXYZ> cloud1,cloud2;  
    //   sensor_msgs::PointCloud2 output;  
    //   pcl::io::loadPCDFile ("/home/robot/LYD.pcd/last.pcd", cloud1);
 
    //   Eigen::Affine3f transform_2 = Eigen::Affine3f::Identity();
 
 
    //   //绕x轴旋转一个theta角
    //   transform_2.rotate(Eigen::AngleAxisf(1.570795, Eigen::Vector3f::UnitX()));
 
    //   //执行变换
    //   //pcl::PointCloud<pcl::PointXYZ>::Ptr pPointCloudOut(new pcl::PointCloud<pcl::PointXYZ>());
    //   pcl::transformPointCloud(cloud1, cloud2, transform_2);
 
 
    //   pcl::toROSMsg(cloud2,output);// 转换成ROS下的数据类型 最终通过topic发布
	
    //   output.header.stamp=ros::Time::now();
    //   output.header.frame_id  ="/Vlp-16";
	
    //   ros::Rate loop_rate(1);  
    //   while (ros::ok())  
    //   {  
    //     pcl_pub.publish(output);  
    //     ros::spinOnce();  
    //     loop_rate.sleep();  
    //   }  
    //   return 0;  
    // }  