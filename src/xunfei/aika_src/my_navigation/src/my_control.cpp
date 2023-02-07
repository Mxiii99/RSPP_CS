#include <ros/ros.h>
#include <ros/console.h>
#include <nav_msgs/Path.h>
#include <sensor_msgs/Imu.h>
#include <std_msgs/String.h>
#include <geometry_msgs/Quaternion.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_listener.h>
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/PoseStamped.h>
#include <tf/tf.h>
#include <dynamic_reconfigure/server.h>
#include <my_navigation/TutorialsConfig.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>

double Path_x[200] = {0.0},Path_y[200] = {0.0};
double dx = 0.0 ,dy = 0.0,dO = 0.0,Xb = 0.0,Yb = 0.0;
double A = 0.0 ,B = 0.0;
int g_nFroehand = 0;
double sheer_speed = 0.0;
double Motor_speed = 0.0;
double teb_speed = 0.0;
double teb_sheer = 0.0;
int slowflag = 0;
double g_fP = 7,g_fD = 10; //7,10 0.6m//9 15 0.65
int point_A = 0,point_B = 0,point_C = 0;
double curvity = 0;
double err = 0,last_err = 0;
// x  =  cosdO  sindO * dx  
// y    -sindO  cosdO   dy
double My_point_x[10] = {0.0};
double My_point_y[10] = {0.0};
double My_point_z[10] = {0.0};
double Dis_point[200];
double goal_x = -0.18;
double goal_y = -5.3;
double car_x = 0;
double car_y = 0;
double real_x = 0;
double real_z = 0;
double Kp=5,KD=2,W_err=0,W_err_last=0;
void pathInfoCallback(const nav_msgs::Path& msg)
{	
		int flag = 0,size = msg.poses.size();
			 for(int i=0;i<size;i++)
			 {
				dx = msg.poses[i].pose.position.x - Xb;
				dy = msg.poses[i].pose.position.y - Yb;
				Path_x[i] = A*dx + B*dy;
				Path_y[i] = -B*dx + A*dy;
				Dis_point[i] = sqrt(Path_x[i]*Path_x[i] + Path_y[i]*Path_y[i]);
			 }

			 double Max_temp = Dis_point[0];
                   for(int i =1; i<size;i++)
		   {
		   	if(Dis_point[i]>Max_temp)
			{
				Max_temp = Dis_point[i];
				g_nFroehand = i;
			}
		   }

                  if(Dis_point[g_nFroehand]>0.9)
		  {
			 for(int i=0;i<size;i++)
			 {
			 	if(Dis_point[i] > 0.5)
				{
					point_A = i;
					break ;
				}
			 }
			 /*
			 for(int i=point_A;i<size;i++)
			 {
			 	if(Dis_point[i]>0.5)
				{
					point_B = i;
					break;
				}
			 }*/
			 point_B = point_A + 1;
			 if(point_B>size-1)
				 point_B = size -1;
			point_C = point_B + 1;
			if(point_C > size-1)
				point_C = size-1;
		}
                   else{
		   	 point_A = (int)(size/2);
			 point_B =point_A;
                         point_C = size-1;
			}
			 ROS_INFO("B.y:%f,C.y:%fDis:%f,C_D:%f,c_y:%f",Path_y[point_A],Path_y[point_B],Dis_point[point_B],Dis_point[point_C],Path_y[point_C]);
			 My_point_x[0] = msg.poses[point_A].pose.position.x;
			 My_point_y[0] = msg.poses[point_A].pose.position.y;
             My_point_z[0] = msg.poses[point_A].pose.position.z;

             My_point_x[1] = msg.poses[point_B].pose.position.x;
             My_point_y[1] = msg.poses[point_B].pose.position.y;
	         My_point_z[1] = msg.poses[point_B].pose.position.z;
			 
			 My_point_x[2] = msg.poses[point_C].pose.position.x;
			 My_point_y[2] = msg.poses[point_C].pose.position.y;
			 My_point_z[2] = msg.poses[point_C].pose.position.z;
             
			 double dis = 0,dis1 = 0,dis2 = 0,dis3 = 0;
			 double sinA = 0,cosA = 0;
			 if(Path_y[point_A]==Path_y[point_B]==Path_y[point_C])
			 {
				curvity = 0;
			 }
			 else
			 {
				dis1 = sqrt(( Path_x[point_A] - Path_x[point_B] )*( Path_x[point_A] - Path_x[point_B] ) +
							( Path_y[point_A] - Path_y[point_B] )*( Path_y[point_A] - Path_y[point_B] ));
				dis2 = sqrt(( Path_x[point_A] - Path_x[point_C] )*( Path_x[point_A] - Path_x[point_C] )+
							( Path_y[point_A] - Path_y[point_C] )*( Path_y[point_A] - Path_y[point_C] ));
				dis3 = sqrt(( Path_x[point_C] - Path_x[point_B] )*( Path_x[point_C] - Path_x[point_B] )+
							( Path_y[point_C] - Path_y[point_B] )*( Path_y[point_C] - Path_y[point_B] ));
				 dis = dis1*dis1 + dis3*dis3 - dis2*dis2;
				 if(dis1*dis3*dis2 != 0)
				 {
				 cosA = dis/(2*dis1*dis3);//余弦定理求角度
				 if((1-cosA*cosA)> 0)
				 sinA = sqrt(1 - cosA*cosA);//求正弦
				 else
						 curvity = 0;
				 if(sinA != 0)
				 curvity = 0.5*dis2/sinA;//正弦定理求外接圆半径
				 else
						 curvity = 0;

				 if(curvity != 0)
				 curvity = 1/curvity;//半径的倒数是曲率，半径越小曲率越大

        		 }
				 else curvity = 0;	 
	         }
			 err = Path_y[point_A];
			 ROS_INFO("curity:%f,A:%d,B:%d,c:%d",curvity,point_A,point_B,point_C);
			/* if((Path_y[point_C]<=0.1&&Path_y[point_C]>=-0.1)&&(Path_y[point_C]*Path_y[point_C]>=Path_y[point_B]*Path_y[point_B])&&(Path_y[point_B]*Path_y[point_B]<0.01))
				Motor_speed = 1.5;
			 else
				 Motor_speed = 0.6;*/
			 double different;
			 different = err - last_err;
                sheer_speed = g_fP*err+g_fD*different;
		
		last_err = err;
//	        Motor_speed = -40*Path_y[point_B]*Path_y[point_B]+1;//90
//		Motor_speed = -1;
	//	sheer_speed = 0;
	if(sheer_speed>4)//4 0.6
			sheer_speed = 4;
	if(sheer_speed<-4)
			sheer_speed = -4;
		if(curvity>1&&Path_y[point_A]*Path_y[point_A]>0.09)
		{
			slowflag = 1;
		}
		if(slowflag ==1 &&Path_y[point_A]*Path_y[point_A]>0.01)
		{
		
		slowflag = 0;
		}

		if(slowflag == 0 )
		{
		Motor_speed = teb_speed;
		sheer_speed = teb_sheer;
		}
		if(slowflag == 1)
		{
			Motor_speed = 0.6;
			sheer_speed = sheer_speed;
		}
//纯路径跟踪/*
                      /* double Ld = Dis_point[point_B],k;
		       if(dO > 0)
			       dO = dO;
		       else
			       dO = -dO;

		       if(dO *57.3 >= 90 && dO *57.3<=180)
			       dO = dO - 1.57;
		       if(dO *57.3 >180&&dO*57.3<=270)
			       dO = dO - 3.14;
		       if(dO*57.3>270&&dO*57.3<=360)
			       dO = dO - 4.71;
			if(Path_y[point_B]>=0)
				dO = dO;
			if(Path_y[point_B]<0)
				dO = -dO;
			k = 2*sin(dO)/Ld;
			Motor_speed = 0.6;
			W_err = Motor_speed*k;
			sheer_speed = Kp*W_err+KD*(W_err_last-W_err);
			W_err_last = W_err;
			*/
			double goal_car_dis = 5;
			 goal_car_dis = sqrt((car_x-goal_x)*(car_x-goal_x)+(car_y-goal_y)*(car_y-goal_y));
			 if(goal_car_dis < 0.4)//0.3
			 {
			 	sheer_speed = 0;
				Motor_speed = 0;
				ROS_INFO("goal received!!!");
			 }

	 ROS_INFO("sheer:%lf,real_z:%lf,speed:%f,real_x:%lf,fore:%d,seiz:%ld,flag:%d",sheer_speed,real_z,Motor_speed,real_x,g_nFroehand,msg.poses.size(),slowflag);

}

void callback(dynamic_tutorials::TutorialsConfig &config,uint32_t level){
		ROS_INFO("Reconfigure Request: %f %f %f",
				  config.dir_P,config.rad_max,
				  config.speed_max);
//		g_fP = config.dir_P;
//		Motor_speed = config.speed_max;
		if(Motor_speed > 4)
				Motor_speed = 4;
		if(Motor_speed < -4)
				Motor_speed = -4;
}

void ImuInfoCallback(const sensor_msgs::Imu& imu_raw)
{
	float x = imu_raw.orientation.x;
	float y = imu_raw.orientation.y;
	float z = imu_raw.orientation.z;
	float w = imu_raw.orientation.w;

	double tmp,yaw;
	tf::Quaternion q(x,y,z,w);
	tf::Matrix3x3 quaternion(q);
	quaternion.getRPY(tmp,tmp,yaw);
//    ROS_INFO("YAW:%f",yaw);
       dO = yaw;
    A = cos(yaw);
	B = sin(yaw);

}

void carPoseCallback(const geometry_msgs::PoseWithCovarianceStamped& car)
{
	car_x = car.pose.pose.position.x;
	car_y = car.pose.pose.position.y;
}
void goalPoseCallback(const geometry_msgs::PoseStamped& goal)
{
	goal_x = goal.pose.position.x;
	goal_y = goal.pose.position.y;
}

void carspeedCallback(const nav_msgs::Odometry& car)
{
	real_x = car.twist.twist.linear.x;
	real_z = car.twist.twist.angular.z;
}
void velCallback(const geometry_msgs::Twist& vel)
{
	teb_speed = vel.linear.x;
	teb_sheer = vel.angular.z;
}
int main(int argc,char **argv)
{
	ros::init (argc,argv,"my_control");
	ros::NodeHandle ctrl,m,n,po1,po2,po3,a,b,c,d;

	ros::Subscriber car_speed_sub = c.subscribe("/odom",10,carspeedCallback);
	ros::Subscriber car_pose_sub = a.subscribe("/amcl_pose",10,carPoseCallback);
	ros::Subscriber goal_pose_sub = b.subscribe("/move_base/current_goal",10,goalPoseCallback);
        ros::Subscriber goal_vel_sub = d.subscribe("/re/cmd_vel",10,velCallback);
        ros::Subscriber imu_sub = n.subscribe("/imu",10,ImuInfoCallback);
ros::Subscriber goal_path = ctrl.subscribe("/move_base/TebLocalPlannerROS/local_plan", 10, pathInfoCallback);
    ros::Publisher goal_speed_pub = m.advertise<geometry_msgs::Twist>("/cmd_vel",10);
	ros::Publisher goal_point1_pub = po1.advertise<geometry_msgs::PointStamped>("/my_point1",10);
	ros::Publisher goal_point2_pub = po2.advertise<geometry_msgs::PointStamped>("/my_point2",10);
    ros::Publisher goal_point3_pub = po3.advertise<geometry_msgs::PointStamped>("/my_point3",10);

	geometry_msgs::PointStamped this_pose1_stamped;
	geometry_msgs::PointStamped this_pose2_stamped;
    geometry_msgs::PointStamped this_pose3_stamped;

	ros::Time current_time, last_time;
	 current_time = ros::Time::now();
	 last_time = ros::Time::now();
 	 //ros::spin();
	tf::TransformListener listener;
	dynamic_reconfigure::Server<dynamic_tutorials::TutorialsConfig> server;
	dynamic_reconfigure::Server<dynamic_tutorials::TutorialsConfig>::CallbackType f;
	f = boost::bind(&callback, _1, _2);
	server.setCallback(f);
	ros::Rate loop_rate(10.0);
	while (ros::ok())
	{
		tf::StampedTransform transform;
		try
		{
			listener.waitForTransform("/odom","/base_link",ros::Time(0),ros::Duration(3.0));
			listener.lookupTransform("/odom","/base_link",ros::Time(0),transform);
	         // ROS_INFO_STREAM(" Transform: " << transform.getOrigin().x() << ", " << transform.getOrigin().y() << ", " <<transform.getOrigin().z() << ", " << transform.getRotation().x() << ", " << transform.getRotation().y() << ", " << transform.getRotation().z());
		Xb = transform.getOrigin().x();
		Yb = transform.getOrigin().y();
		}
		catch (tf::TransformException &ex)
		{
			ROS_ERROR("%s",ex.what());
			ros::Duration(1.0).sleep();
			continue;
		}
		geometry_msgs::Twist my_vel;

		my_vel.linear.x = Motor_speed;  //m/s
		my_vel.angular.z = sheer_speed; //rad/s
		goal_speed_pub.publish(my_vel);
		 current_time = ros::Time::now();
	this_pose1_stamped.header.stamp = current_time;
	this_pose2_stamped.header.stamp = current_time;
	this_pose3_stamped.header.stamp = current_time;
	this_pose1_stamped.header.frame_id = "odom";
	this_pose2_stamped.header.frame_id = "odom";
	this_pose3_stamped.header.frame_id = "odom";
   	this_pose1_stamped.point.x = My_point_x[0];
	this_pose1_stamped.point.y = My_point_y[0];
	this_pose1_stamped.point.z = My_point_z[0];
 
	this_pose2_stamped.point.x = My_point_x[1];
	this_pose2_stamped.point.y = My_point_y[1];
	this_pose2_stamped.point.z = My_point_z[1];

	this_pose3_stamped.point.x = My_point_x[2];
	this_pose3_stamped.point.y = My_point_y[2];
	this_pose3_stamped.point.z = My_point_z[2];

    goal_point1_pub.publish(this_pose1_stamped);
    goal_point2_pub.publish(this_pose2_stamped);
    goal_point3_pub.publish(this_pose3_stamped);
    ros::spinOnce();
    last_time = current_time;	
		loop_rate.sleep();
	}
	return 0;
}
