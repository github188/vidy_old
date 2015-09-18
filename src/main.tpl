#include <iostream>
#include <vector>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/objdetect/objdetect.hpp"


int main(int argc,char* argv[]){

  cv::VideoCapture capture(argv[1]);

  if(!capture.isOpened()){
    std::cout<<"vidoe does not open"<<std::endl;
  }
  
  cv::VideoWriter writer("video.avi",CV_FOURCC('M','J','P','G'),5.0,cv::Size(1280,720));

  bool record = false;

  char key = '\0';

  cv::Mat frame;

  //MOG background.
  cv::BackgroundSubtractorMOG2 mog;

  //HOG detect.
  cv::HOGDescriptor hog;
  hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

  while(key!=27){
    capture.read(frame);
    if(frame.empty()){
      std::cout<<"no frame"<<std::endl;
    }

    //-- test code -----------
    std::vector<cv::Rect> people;
    hog.detectMultiScale(frame,people,0,cv::Size(8,8),cv::Size(32,32),1.05,1);
    for(unsigned int i=0;i<people.size();i++){
      if(people[i].x>650&&people[i].y>200&&people[i].x<900){
        cv::rectangle(frame,people[i],cv::Scalar(0,255,0),2);
      }
    }

    /*
    cv::Mat mask;
    mog(frame,mask,0.1);
    for(unsigned int i=0;i<3;i++){
      //cv::dilate(mask,mask,cv::Mat(),cv::Point(1,1),1);
      cv::erode(mask,mask,cv::Mat(),cv::Point(1,1),1);
      cv::dilate(mask,mask,cv::Mat(),cv::Point(1,1),1);
    }
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(mask,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
    std::vector<std::vector<cv::Point> >::iterator it=contours.begin();
    while(it!=contours.end()){
      cv::Rect r0 = boundingRect(*it);
      if(it->size()<500|| r0.y<100){
        it = contours.erase(it);
      }else{
        ++it;
      }
    }
    cv::Mat mask_new = cv::Mat::zeros(mask.size(),CV_8UC1);
    cv::drawContours(mask_new,contours,-1,cv::Scalar(255),CV_FILLED);
    cv::dilate(mask_new,mask_new,cv::Mat(),cv::Point(1,1),1);
    cv::imshow("mask_new",mask_new);
    */
    
    //-------------------------

    cv::imshow("video",frame);
    cv::waitKey(200);
    
    //--control record video.--
    if(key=='r'){
      if(!record){
        record=true;
      }else{
        record=false;
      }
    }
    //--record video--
    if(record){
      writer<<frame;
    } 

    //--save image.-- 
    if(key=='s'){
      cv::imwrite("image.jpg",frame);
    } 
  }

  return 0;
}
