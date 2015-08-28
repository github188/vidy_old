//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File: age_estimate.cc
//Author: Yun Luo(lauren.luo@extremevision.mo)

#include "age_estimate.h"

#include <iostream>
#include <fstream>

namespace vidy{

AgeEstimate::AgeEstimate(){
  graydata = new int[1600];
  LDAMatrix = new double[1600*47];
  LoadLDA(LDAMatrix);
#ifdef SERVER
  model = svm_load_model("/root/vidy/data/age/LDA_morph.model");
#else
  model = svm_load_model("../data/age/LDA_morph.model");
#endif // SERVER
}

AgeEstimate::~AgeEstimate(){

}

int AgeEstimate::EstimateByFace(cv::Mat face_original){
  cv::Mat normal;
  if(face_original.channels()>1){
    cv::Mat face_gray;
    cv::cvtColor(face_original,face_gray,CV_RGB2GRAY);
    cv::resize(face_gray,normal,cv::Size(40,40),0,0,CV_INTER_LINEAR);
  }else{
    cv::resize(face_original,normal,cv::Size(40,40),0,0,CV_INTER_LINEAR);
  }
  graydata = (int*)normal.data;
  node = new svm_node[48];
  LDAFeature(graydata,LDAMatrix,node);
  return svm_predict(model,node);
}

        
void AgeEstimate::LoadLDA(double *LDAMatrix){
#ifdef SERVER
        std::ifstream LDAEig("/root/vidy/data/age/LDAEig_morph.txt");
#else
        std::ifstream LDAEig("../data/age/LDAEig_morph.txt");
#endif // SERVER

        for(int i=0;i<1600*47;i++){
                LDAEig >> LDAMatrix[i];
        }

        LDAEig.close();
}

//---------------------------------------------------------------------------
void AgeEstimate::LDAFeature(int *graydata, double *LDAMatrix, svm_node *node){
        for(int i=0;i<47;i++){
                node[i].index = i;
                for(int j=0;j<1600;j++){
                        node[i].value += (double)graydata[j] * LDAMatrix[j*47+i];
                }
        }
        node[47].index = -1;
}

} //namespace vidy 
