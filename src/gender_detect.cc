//Copyright(c) 2015 Extreme Vision Ltd. Alll rights reserved.
//File: gender_detect.cc
//Author:Yun Luo(lauren.luo@extremevision.mo)

#include "gender_detect.h"
#include <opencv2/highgui/highgui.hpp>

namespace vidy{

GenderDetect::GenderDetect(){
  //-- Gender Estimation
#ifdef SERVER
  model = cv::createEigenFaceRecognizer();
  model->load("/usr/local/vidy/data/gender/eigenface.yml");
  eigenvalues=model->getMat("eigenvalues");
  W=model->getMat("eigenvectors");
  xth=121;
  evs=cv::Mat(W,cv::Range::all(),cv::Range(0,xth));
  mean=model->getMat("mean");
  fishermodel = cv::createFisherFaceRecognizer();
  fishermodel->load("/usr/local/vidy/data/gender/fisher.yml");
  sample=cv::imread("/usr/local/vidy/data/gender/1.png");
#else	
  model = cv::createEigenFaceRecognizer();
  model->load("../data/gender/eigenface.yml");
  eigenvalues=model->getMat("eigenvalues");
  W=model->getMat("eigenvectors");
  xth=121;
  evs=cv::Mat(W,cv::Range::all(),cv::Range(0,xth));
  mean=model->getMat("mean");
  fishermodel = cv::createFisherFaceRecognizer();
  fishermodel->load("../data/gender/fisher.yml");
  sample=cv::imread("../data/gender/1.png");
#endif // SERVER
}

GenderDetect::~GenderDetect(){
}

int GenderDetect::DetectByFace(cv::Mat face_gray){
  //-- gender estimation -------------
  cv::Mat reduceDemensionimage;
  cv::Mat face_resize(sample.size(),sample.type());
  face_resize.zeros(sample.size(),sample.type());
  cv::resize(face_gray,face_resize,face_resize.size(),CV_INTER_LINEAR);
  cv::Mat projection=subspaceProject(evs,mean,face_resize.reshape(1,1));
  reduceDemensionimage=projection.reshape(1,sqrt(xth*1.0));
  int gender=fishermodel->predict(reduceDemensionimage);
  return gender;
}

} //namespace vidy
