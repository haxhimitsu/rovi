#define OPENCV_TRAITS_ENABLE_DEPRECATED 

#include <ros/ros.h>
#include <stereo_msgs/DisparityImage.h>
#include <sensor_msgs/CameraInfo.h>
#include <sensor_msgs/Image.h>
#include <opencv2/opencv.hpp>

#include <image_proc/processor.h>
#include <image_geometry/stereo_camera_model.h>
#include <boost/version.hpp>
#if ((BOOST_VERSION / 100) % 1000) >= 53
#include <boost/thread/lock_guard.hpp>
#endif

#include <stereo_image_proc/processor.h>

#include <cv_bridge/cv_bridge.h>

//
#include <iostream>
#include <cstdio>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <string.h>

using namespace sensor_msgs;
using namespace stereo_msgs;
using namespace stereo_image_proc;

bool first = true;

ros::NodeHandle *nh;
ros::Publisher pub;

int seq_lrect = -1;
int seq_rrect = -1;
int seq_linfo = -1;
int seq_rinfo = -1;

cv::Mat_<uint8_t> l_image;
cv::Mat_<uint8_t> r_image;
CameraInfo linfo;
CameraInfo rinfo;

StereoProcessor block_matcher_;

cv::Ptr<cv::StereoSGBM> ssgbm;

void tmpSgbm()
{
	cv::Mat img[2];
	img[0]=cv::imread("/home/take/left1.pgm", CV_LOAD_IMAGE_GRAYSCALE);
	img[1]=cv::imread("/home/take/right1.pgm", CV_LOAD_IMAGE_GRAYSCALE);

	int window_size = 1;
	int minDisparity = 0;
	int numDisparities = 16;
	int blockSize = 3;
	int P1 = 8 * 3 * window_size * window_size;
	int P2 = 32 * 3 * window_size * window_size;
	int disp12MaxDiff = 1;
	int preFilterCap = 1;
	int uniquenessRatio = 1;
	int speckleWindowSize = 3;
	int speckleRange = 1;
        
	cv::Ptr<cv::StereoSGBM> tmpssgbm = cv::StereoSGBM::create(
		minDisparity,
		numDisparities,
		blockSize,
		P1,
		P2, disp12MaxDiff,
		preFilterCap,
		uniquenessRatio,
		speckleWindowSize , speckleRange, cv::StereoSGBM::MODE_HH4 );
	cv::Mat disparity;
	tmpssgbm->compute(img[0],img[1], disparity);

	double min,max;
	cv::minMaxIdx(disparity, &min, &max);
	cv::Mat adj;
	float scale = 255 / (max-min);
	disparity.convertTo(adj,CV_8UC1, scale, -min*scale); 
	cv::imshow("MyDisparity",adj);
	cv::waitKey(0);
}

void paramCheck()
{



}

int window_size = 1; // SADWindowSize
int minDisparity = 0;
int numDisparities = 16;
int blockSize = 3; // CorrelationWindowSize
int P1 = 8 * 3 * window_size * window_size; // TODO
int P2 = 32 * 3 * window_size * window_size;// TODO
int disp12MaxDiff = 1;
int preFilterCap = 1;
int uniquenessRatio = 1;
int speckleWindowSize = 3;
int speckleRange = 1;
int fullDP = 0;

/*
	int window_size = 3; // SADWindowSize
	int minDisparity = 5;
	int numDisparities = 32;
	int blockSize = 7; // CorrelationWindowSize
	int P1 = 8 * 3 * window_size * window_size; // TODO
	int P2 = 32 * 3 * window_size * window_size;// TODO
	int disp12MaxDiff = 3;
	int preFilterCap = 2;
	int uniquenessRatio = 10;
	int speckleWindowSize = 7;
	int speckleRange = 5;
	int fullDP = 1;
*/
        
void makeSgbm()
{
/*
	minDisparity,
	numDisparities,
	blockSize,
	P1,
	P2,
	disp12MaxDiff,
	preFilterCap,
	uniquenessRatio,
	speckleWindowSize,
	speckleRange,
*/

/*
	// TODO
//	int window_size = 3; // SADWindowSize

	if (minDisparity < -128) {
		ROS_ERROR("minDisparity(%d) is set to -128", minDisparity);
		minDisparity = -128;
	}
	else if (minDisparity > 128) {
		ROS_ERROR("minDisparity(%d) is set to 128", minDisparity);
		minDisparity = 128;
	}

	if (numDisparities < 32) {
		ROS_ERROR("numDisparities(%d) is set to 32", numDisparities);
		numDisparities = 32;
	}
	else if (numDisparities > 256) {
		ROS_ERROR("numDisparities(%d) is set to 256", numDisparities);
		numDisparities = 256;
	}
	int ndred = numDisparities % 16;
	numDisparities -= ndred;
	ROS_ERROR("ndred=%d, now numDisparities=%d", ndred, numDisparities);

	// TODO
//	int blockSize = 7; // CorrelationWindowSize
//	int P1 = 8 * 3 * window_size * window_size; // TODO
//	int P2 = 32 * 3 * window_size * window_size;// TODO

	if (disp12MaxDiff < 0) {
		ROS_ERROR("disp12MaxDiff(%d) is set to 0", disp12MaxDiff);
		disp12MaxDiff = 0;
	}
	else if (disp12MaxDiff > 128) {
		ROS_ERROR("disp12MaxDiff(%d) is set to 128", disp12MaxDiff);
		disp12MaxDiff = 128;
	}

	if (preFilterCap < 1) {
		ROS_ERROR("preFilterCap(%d) is set to 1", preFilterCap);
		preFilterCap = 1;
	}
	else if (preFilterCap > 63) {
		ROS_ERROR("preFilterCap(%d) is set to 63", preFilterCap);
		preFilterCap = 63;
	}
*/

	// TODO preFilsterSize
/*
	if (blockSize < 5) {
		ROS_ERROR("blockSize(%d) is set to 5", blockSize);
		blockSize = 5;
	}
	if (blockSize > 255) {
		ROS_ERROR("blockSize(%d) is set to 255", blockSize);
		blockSize = 255;
	}
*/

/*
	// TODO float?
	if (uniquenessRatio < 0) {
		ROS_ERROR("uniquenessRatio(%d) is set to 0", uniquenessRatio);
		uniquenessRatio = 0;
	}
	else if (uniquenessRatio > 100) {
		ROS_ERROR("uniquenessRatio(%d) is set to 100", uniquenessRatio);
		uniquenessRatio = 100;
	}

	int speckleWindowSize = 7;
	int speckleRange = 5;
	int fullDP = 1;
*/

	ssgbm = cv::StereoSGBM::create(
		minDisparity,
		numDisparities,
		blockSize,
		P1,
		P2,
		disp12MaxDiff,
		preFilterCap,
		uniquenessRatio,
		speckleWindowSize,
		speckleRange,
		cv::StereoSGBM::MODE_HH4);

//  config.prefilter_size |= 0x1; // must be odd
  
	block_matcher_.setPreFilterCap(preFilterCap);
	block_matcher_.setCorrelationWindowSize(blockSize);
	block_matcher_.setMinDisparity(minDisparity);
	block_matcher_.setDisparityRange(numDisparities);
	block_matcher_.setUniquenessRatio(uniquenessRatio);
	block_matcher_.setSpeckleSize(speckleWindowSize);
	block_matcher_.setSpeckleRange(speckleRange);

	block_matcher_.setStereoType(StereoProcessor::SGBM);
	block_matcher_.setSgbmMode(fullDP);
	block_matcher_.setP1(P1);
	block_matcher_.setP2(P2);
	block_matcher_.setDisp12MaxDiff(disp12MaxDiff);
}

void outputDisparity()
{
	ROS_ERROR("outputDisparity called.");

	{
		image_geometry::StereoCameraModel model_;
		model_.fromCameraInfo(linfo, rinfo);

		DisparityImagePtr disp_msg = boost::make_shared<DisparityImage>();

		disp_msg->header         = linfo.header;
		disp_msg->image.header   = linfo.header;

		int border = block_matcher_.getCorrelationWindowSize() / 2;

		ROS_ERROR("b_m__ corrws=%d", block_matcher_.getCorrelationWindowSize());
		ROS_ERROR("b_m__ border=%d", border);
		ROS_ERROR("b_m__ disparityrange=%d", block_matcher_.getDisparityRange());

		int left   = block_matcher_.getDisparityRange() + block_matcher_.getMinDisparity() + border - 1;
		int wtf   = (block_matcher_.getMinDisparity() >= 0) ? border + block_matcher_.getMinDisparity() : std::max(border, -block_matcher_.getMinDisparity());
		int right  = disp_msg->image.width - 1 - wtf;
		int top    = border;
		int bottom = disp_msg->image.height - 1 - border;

		ROS_ERROR("b_m__ left=%d, right=%d, top=%d, bottom=%d", left, right, top, bottom);

		disp_msg->valid_window.x_offset = left;
		disp_msg->valid_window.y_offset = top;
		disp_msg->valid_window.width    = right - left;
		disp_msg->valid_window.height   = bottom - top;

		block_matcher_.processDisparity(l_image, r_image, model_, *disp_msg);

		for (int i = 0; i < 4; i++) {
			ROS_ERROR("b_m__ disp_msg.data[%d]=%d", i, disp_msg->image.data[i]);
		}
		const float *dfp = (float*)&disp_msg->image.data[0];
		ROS_ERROR("b_m__ disp_msg.datafloat=%f", *dfp);


		double cx_l = model_.left().cx();
		double cx_r = model_.right().cx();
		ROS_ERROR("b_m__ real     datafloat=%f", *dfp + (cx_l - cx_r));
	}

	{
		image_geometry::StereoCameraModel model_;
		model_.fromCameraInfo(linfo, rinfo);

		DisparityImagePtr disp_msg = boost::make_shared<DisparityImage>();

		disp_msg->header         = linfo.header;
		disp_msg->image.header   = linfo.header;

		int border = ssgbm->getBlockSize() / 2;

		ROS_ERROR("ssgbm bls=%d", ssgbm->getBlockSize());
		ROS_ERROR("ssgbm border=%d", border);
		ROS_ERROR("ssgbm disparityrange=%d", ssgbm->getNumDisparities());

		int left   = ssgbm->getNumDisparities() + ssgbm->getMinDisparity() + border - 1;
		int wtf   = (ssgbm->getMinDisparity() >= 0) ? border + ssgbm->getMinDisparity() : std::max(border, -ssgbm->getMinDisparity());
		int right  = disp_msg->image.width - 1 - wtf;
		int top    = border;
		int bottom = disp_msg->image.height - 1 - border;

		ROS_ERROR("ssgbm left=%d, right=%d, top=%d, bottom=%d", left, right, top, bottom);

		disp_msg->valid_window.x_offset = left;
		disp_msg->valid_window.y_offset = top;
		disp_msg->valid_window.width    = right - left;
		disp_msg->valid_window.height   = bottom - top;

		{
			cv::Mat disparity;
			ssgbm->compute(l_image, r_image, disparity);
			ROS_ERROR("ssgbm compute done");

			double min,max;
			cv::minMaxIdx(disparity, &min, &max);
			ROS_ERROR("min=%f, max=%f", min, max);
			cv::Mat adj;
			float scale = 255 / (max-min);
			ROS_ERROR("scale=%f", scale);
			disparity.convertTo(adj, CV_8UC1, scale, -min*scale);
			
			ROS_ERROR("d89=%f, a89=%f", disparity.at<float>(8, 9), adj.at<float>(8, 9));

			uint8_t *adjp = (uint8_t *)&adj;
			for (int i = 0; i < 8; i++, adjp++) {
				ROS_ERROR("ssgbm adj[%d]=%d", i, *adjp);
			}
			const float *dfp = (float *)&adj;
			ROS_ERROR("ssgbm adj float=%f", *dfp);

			if (first) {
				cv::imshow("Disparity",adj);
				first = false;
				cv::waitKey(0);
			}
		}

		cv::Mat_<int16_t> disparity16_;
		ssgbm->compute(l_image, r_image, disparity16_);
		ROS_ERROR("ssgbm compute done!!");

		static const int DPP = 16; // disparities per pixel
		static const double inv_dpp = 1.0 / DPP;

		sensor_msgs::Image& dimage = disp_msg->image;
		dimage.height = disparity16_.rows;
		dimage.width = disparity16_.cols;
		dimage.encoding = sensor_msgs::image_encodings::TYPE_32FC1;
		dimage.step = dimage.width * sizeof(float);
		dimage.data.resize(dimage.step * dimage.height);

		cv::Mat_<float> dmat(dimage.height, dimage.width, (float*)&dimage.data[0], dimage.step);
		disparity16_.convertTo(dmat, dmat.type(), inv_dpp);

		disp_msg->f = model_.right().fx();
		disp_msg->T = model_.baseline();

		disp_msg->min_disparity = minDisparity;
		disp_msg->max_disparity = minDisparity + numDisparities - 1;
		disp_msg->delta_d = inv_dpp;

		for (int i = 0; i < 4; i++) {
			ROS_ERROR("ssgbm disp_msg.data[%d]=%d", i, disp_msg->image.data[i]);
		}
		const float *dfp = (float*)&disp_msg->image.data[0];
		ROS_ERROR("ssgbm disp_msg.datafloat=%f", *dfp);

		pub.publish(disp_msg);
	}

/*
	{

		cv::Mat img[2];
		img[0]=cv::imread("/home/take/left1.pgm", CV_LOAD_IMAGE_GRAYSCALE);
		img[1]=cv::imread("/home/take/right1.pgm", CV_LOAD_IMAGE_GRAYSCALE);

		cv::Mat disparity;
		ssgbm->compute(img[0],img[1], disparity);

		double min,max;
		cv::minMaxIdx(disparity, &min, &max);
		cv::Mat adj;
		float scale = 255 / (max-min);
		disparity.convertTo(adj,CV_8UC1, scale, -min*scale); 

		if (first) {
			cv::imshow("aaDisparity",adj);
			first = false;
			cv::waitKey(0);
		}
	}
*/

}

void disparityCallback(const DisparityImageConstPtr& msg)
{
	ROS_ERROR("disparityCallback");
	ROS_ERROR("f=%f", msg->f);
	ROS_ERROR("T=%f", msg->T);
	ROS_ERROR("min_disparity=%f", msg->min_disparity);
	ROS_ERROR("max_disparity=%f", msg->max_disparity);
	ROS_ERROR("width=%u", msg->image.height);
	ROS_ERROR("height=%u", msg->image.width);
	ROS_ERROR("step=%u", msg->image.step);
	ROS_ERROR("image.data.size()=%d", msg->image.data.size());
	for (int i = 0; i < 12; i++) {
		ROS_ERROR("image.data[%d]=%d", i, msg->image.data[i]);
	}
	ROS_ERROR("sizeof(float)=%d", sizeof(float));
        const float *datafloatp = (float*)&msg->image.data[0];
	ROS_ERROR("datafloat=%f", *datafloatp);
	char okdata[] = {0, 0, 128, 191};
        const float *okfloatp = (float*)okdata;
	ROS_ERROR("okfloat=%f", *okfloatp);

  cv::Mat_<cv::Vec3b> disparity_color_;

  float min_disparity = msg->min_disparity;
  float max_disparity = msg->max_disparity;
  float multiplier = 255.0f / (max_disparity - min_disparity);

	ROS_ERROR("multiplier=%f", multiplier);

  const cv::Mat_<float> dmat(msg->image.height, msg->image.width,
                             (float*)&msg->image.data[0], msg->image.step);
  disparity_color_.create(msg->image.height, msg->image.width);
    
	ROS_ERROR("disparity_color_.rows=%d", disparity_color_.rows);

  for (int row = 0; row < disparity_color_.rows; ++row) {

	if (row < 1) {
		ROS_ERROR("row=%d", row);
	}

	int di = 0;

    const float* d = dmat[row];
    cv::Vec3b *disparity_color = disparity_color_[row],
              *disparity_color_end = disparity_color + disparity_color_.cols;
    for (; disparity_color < disparity_color_end; ++disparity_color, ++d, ++di) {
      int index = (*d - min_disparity) * multiplier + 0.5;
	if (row < 1 && di < 4) {
		ROS_ERROR("*d=%f ... before index=%d", *d, index);
	}
      index = std::min(255, std::max(0, index));
//	ROS_ERROR("after  index=%d", index);
    }
  }

}

void lrectCallback(const ImageConstPtr &msg)
{
	seq_lrect = msg->header.seq;
//	ROS_ERROR("lrectCallback. now seq_lrect=%d", seq_lrect);

	// TODO
//	l_image = cv_bridge::toCvShare(msg, sensor_msgs::image_encodings::MONO8)->image;
	l_image = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::MONO8)->image;
	ROS_ERROR("now l89=%d", l_image(8, 9));

	if ((seq_lrect == seq_rrect) && (seq_lrect == seq_linfo) && (seq_lrect == seq_rinfo)) {
/*
		ROS_ERROR("before lrect l89");
		ROS_ERROR("lrect l89=%d", l_image(8, 9));
		ROS_ERROR("after  lrect l89");
*/
		outputDisparity();
	}
}

void rrectCallback(const ImageConstPtr &msg)
{
	seq_rrect = msg->header.seq;
//	ROS_ERROR("rrectCallback. now seq_rrect=%d", seq_rrect);

	// TODO
//	r_image = cv_bridge::toCvShare(msg, sensor_msgs::image_encodings::MONO8)->image;
	r_image = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::MONO8)->image;
	ROS_ERROR("now r89=%d", r_image(8, 9));

	if ((seq_rrect == seq_lrect) && (seq_rrect == seq_linfo) && (seq_rrect == seq_rinfo)) {
/*
		ROS_ERROR("before rrect l89");
		ROS_ERROR("rrect l89=%d", l_image(8, 9));
		ROS_ERROR("after  rrect l89");
*/
		outputDisparity();
	}
}

void linfoCallback(const CameraInfoConstPtr &msg)
{
	seq_linfo = msg->header.seq;
//	ROS_ERROR("linfoCallback. now seq_linfo=%d", seq_linfo);

	linfo = *msg;

	if ((seq_linfo == seq_lrect) && (seq_linfo == seq_rrect) && (seq_linfo == seq_rinfo)) {
/*
		ROS_ERROR("before linfo l89");
		ROS_ERROR("linfo l89=%d", l_image(8, 9));
		ROS_ERROR("after  linfo l89");
*/
		outputDisparity();
	}
}

void rinfoCallback(const CameraInfoConstPtr &msg)
{
	seq_rinfo = msg->header.seq;
//	ROS_ERROR("rinfoCallback. now seq_rinfo=%d", seq_rinfo);

	rinfo = *msg;

	if ((seq_rinfo == seq_lrect) && (seq_rinfo == seq_rrect) && (seq_rinfo == seq_linfo)) {
/*
		ROS_ERROR("before rinfo l89");
		ROS_ERROR("rinfo l89=%d", l_image(8, 9));
		ROS_ERROR("after  rinfo l89");
*/
		outputDisparity();
	}
}

int main(int argc, char **argv){
//	tmpSgbm();
	makeSgbm();
	ros::init(argc,argv,"disparity_node");
	ros::NodeHandle n;
	nh=&n;
	pub=n.advertise<stereo_msgs::DisparityImage>("disparity",1);
	ros::Subscriber sub_dsp = n.subscribe("disparity", 1, disparityCallback);
	ros::Subscriber sub_lr = n.subscribe("left/image_rect", 1, lrectCallback);
	ros::Subscriber sub_rr = n.subscribe("right/image_rect", 1, rrectCallback);
	ros::Subscriber sub_li = n.subscribe("left/camera_info", 1, linfoCallback);
	ros::Subscriber sub_ri = n.subscribe("right/camera_info", 1, rinfoCallback);
	ros::spin();
	return 0;
}