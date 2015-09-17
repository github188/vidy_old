#include <iostream>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"


int main(int argc,char* argv[]){

  cv::VideoCapture capture(argv[1]);

  if(!capture.isOpened()){
    std::cout<<"vidoe does not open"<<std::endl;
  }
  
  cv::VideoWriter writer("video.avi",CV_FOURCC('M','J','P','G'),5.0,cv::Size(1280,720));

  char key = '\0';
  cv::Mat frame;
  
  while(key!=27){
    capture.read(frame);
    if(frame.empty()){
      std::cout<<"no frame"<<std::endl;
    }
    cv::imshow("video",frame);
    writer<<frame;
    cv::waitKey(200);  
    if(key=='s'){
      cv::imwrite("image.jpg",frame);
    } 
  }

  return 0;
}
