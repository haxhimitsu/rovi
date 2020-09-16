#pragma once 

#include <string>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include "Aravis.h"
#include "YCAM3D.h"

class CameraYCAM3D;

namespace camera{
	namespace ycam3d{
		//constexpr int CAM_EXPOSURE_TIME_DEFAULT   = aravis::ycam3d::CAM_EXPOSURE_TIME_DEFAULT;
		//constexpr int CAM_EXPOSURE_TIME_MAX       = aravis::ycam3d::CAM_EXPOSURE_TIME_MAX;
		//constexpr int CAM_EXPOSURE_TIME_MIN       = aravis::ycam3d::CAM_EXPOSURE_TIME_MIN;
		
		constexpr int CAM_DIGITAL_GAIN_DEFAULT   = aravis::ycam3d::CAM_DIGITAL_GAIN_DEFAULT;
		constexpr int CAM_DIGITAL_GAIN_MAX       = aravis::ycam3d::CAM_DIGITAL_GAIN_MAX;
		constexpr int CAM_DIGITAL_GAIN_MIN       = aravis::ycam3d::CAM_DIGITAL_GAIN_MIN;
		
		constexpr int CAM_ANALOG_GAIN_DEFAULT   = aravis::ycam3d::CAM_ANALOG_GAIN_DEFAULT;
		constexpr int CAM_ANALOG_GAIN_MAX       = aravis::ycam3d::CAM_ANALOG_GAIN_MAX;
		constexpr int CAM_ANALOG_GAIN_MIN       = aravis::ycam3d::CAM_ANALOG_GAIN_MIN;
		
		//constexpr int PROJ_EXPOSURE_TIME_DEFAULT  = aravis::ycam3d::PROJ_EXPOSURE_TIME_DEFAULT;
		//constexpr int PROJ_EXPOSURE_TIME_MIN      = aravis::ycam3d::PROJ_EXPOSURE_TIME_MIN;
		//constexpr int PROJ_EXPOSURE_TIME_MAX      = aravis::ycam3d::PROJ_EXPOSURE_TIME_MAX;
		
		constexpr int PROJ_BRIGHTNESS_DEFAULT     = aravis::ycam3d::PROJ_BRIGHTNESS_DEFAULT;
		constexpr int PROJ_BRIGHTNESS_MIN         = aravis::ycam3d::PROJ_BRIGHTNESS_MIN;
		constexpr int PROJ_BRIGHTNESS_MAX         = aravis::ycam3d::PROJ_BRIGHTNESS_MAX;
		
		constexpr int PROJ_FLASH_INTERVAL_DEFAULT = aravis::ycam3d::PROJ_FLASH_INTERVAL_DEFAULT;
		constexpr int PROJ_FLASH_INTERVAL_MIN     = aravis::ycam3d::PROJ_FLASH_INTERVAL_MIN;
		constexpr int PROJ_FLASH_INTERVAL_MAX     = aravis::ycam3d::PROJ_FLASH_INTERVAL_MAX;
		
		constexpr int YCAM_EXPOSURE_TIME_LEVEL_DEFAULT = aravis::ycam3d::EXPOSURE_TIME_SET_DEFAULT;
		constexpr int YCAM_EXPOSURE_TIME_LEVEL_MIN = 0;
		const int YCAM_EXPOSURE_TIME_LEVEL_MAX = aravis::ycam3d::EXPOSURE_TIME_SET_SIZE -1;
		
		struct CameraImage {
			bool result =false;
			int width = -1;
			int height = -1;
			int step = -1;
			int color_ch = -1;
			
			std::vector<unsigned char> data;
			
			CameraImage(){}
			
			CameraImage(const int a_width,const int a_height,const int a_step,const int a_color_ch=1):
				width(a_width),
				height(a_height),
				step(a_step),
				color_ch(a_color_ch)
			{
				
			}
				
			bool valid()const{
				return width > 0 && height > 0 && color_ch > 0 && step > 0 && data.size() == byte_count();
			}
			
			int byte_count()const{
				return step * height;
			}
			
			bool alloc(){
				const int buf_size=byte_count();
				if( buf_size <= 0 ) { return false; }
				data.assign(buf_size,0);
				return data.size() == buf_size;
			}
			
			cv::Mat to_mat()const {
				if(!valid()){ return cv::Mat(); }
				
				cv::Mat img=cv::Mat(height,width,CV_8UC1,cv::Scalar(0));
				memcpy(img.data,data.data(),byte_count());
				return img;
			}
		};
		using f_camera_open_finished = std::function<void(const bool result)>;
		using f_camera_closed = std::function<void(void)>;
		using f_pattern_img_received = std::function<void(const bool result,const int elapsed, const std::vector<camera::ycam3d::CameraImage> &imgs_l,const std::vector<camera::ycam3d::CameraImage> &imgs_r)>;
		using f_capture_img_received = std::function<void(const bool result,const int elapsed, camera::ycam3d::CameraImage img_l,const camera::ycam3d::CameraImage &img_r)>;

	}
}



class CameraYCAM3D {
protected:
	
	enum CaptureStatus {
		CaptStat_Ready,
		CaptStat_Single,
		CaptStat_Pattern
	};
	
	struct CameraImageReceivedCallback : public OnRecvImage
	{
		CameraYCAM3D * const m_self;
		explicit CameraImageReceivedCallback(CameraYCAM3D *obj);
		void operator()(int camno, int frmidx, int width, int height, int color, void *mem) override;
	};
		
	struct CameraDisconnectCallbck : public OnLostCamera
	{
		CameraYCAM3D * const m_self;
		explicit CameraDisconnectCallbck(CameraYCAM3D *obj);
		void operator()(int camno) override;
	};
	
private:
	YCAM_RES m_ycam_res;
	std::unique_ptr<Aravis> m_arv_ptr;
	std::vector<unsigned char> m_arv_img_buf;
	
	std::atomic<bool> m_open_stat;
	std::timed_mutex m_camera_mutex;
	
	bool m_auto_connect_abort;
	std::thread m_auto_connect_thread;
	
	std::timed_mutex m_auto_connect_mutex;
	
	CameraImageReceivedCallback m_on_image_received;
	CameraDisconnectCallbck m_on_disconnect;
	
	int m_capture_timeout_period;
	int m_trigger_timeout_period;
	std::thread m_capture_thread;
	
	camera::ycam3d::f_camera_open_finished m_callback_cam_open_finished;
	camera::ycam3d::f_camera_closed m_callback_cam_closed;
	camera::ycam3d::f_capture_img_received m_callback_capt_img_recv;
	camera::ycam3d::f_pattern_img_received m_callback_trig_img_recv;
	
	bool reset_image_buffer();
	
	bool get_camera_param_int(const std::string &label,std::function<bool(int*)> func,int *val);
	bool set_camera_param_int(const std::string &label,std::function<bool(int)> func,const int val);
protected:
	int m_camno;
	
	std::atomic<CaptureStatus> m_capt_stat;
	std::timed_mutex m_capt_finish_wait_mutex;
	
	std::timed_mutex m_img_update_mutex; //m_imgs_left, m_imgs_right, m_img_recv_flags �Ώ�
	std::vector<camera::ycam3d::CameraImage> m_imgs_left;
	std::vector<camera::ycam3d::CameraImage> m_imgs_right;
	std::vector<bool> m_img_recv_flags;
	
	
public:
	CameraYCAM3D();
	virtual ~CameraYCAM3D();
	
	bool init(const std::string &camera_res);
	
	int width()const;
	int height()const;
	
	bool is_open()const;
	void open();
	void close();
	
	bool is_auto_connect_running();
	
	bool is_busy();
	
	void set_capture_timeout_period(const int timeout);
	
	void set_trigger_timeout_period(const int timeout);
	
	bool capture();
	
	bool capture_strobe();
	
	bool capture_pattern();
	
	void start_auto_connect();
	
	bool get_exposure_time_level(int *val);
	bool set_exposure_time_level(const int val);
	
	bool get_exposure_time(int *val);
	//bool set_exposure_time(const int val);
	
	bool get_gain_digital(int *val);
	bool set_gain_digital(const int val);
	
	bool get_gain_analog(int *val);
	bool set_gain_analog(const int val);
	
	bool get_projector_exposure_time(int *val);
	//bool set_projector_exposure_time(const int val);
	
	bool get_projector_brightness(int *val);
	bool set_projector_brightness(const int val);
	
	//bool get_projector_interval(int *val);
	//bool set_projector_interval(const int val);

	void set_callback_camera_open_finished(camera::ycam3d::f_camera_open_finished callback);
	
	void set_callback_camera_closed(camera::ycam3d::f_camera_closed);
	
	void set_callback_capture_img_received(camera::ycam3d::f_capture_img_received callback);

	void set_callback_pattern_img_received(camera::ycam3d::f_pattern_img_received callback);
};