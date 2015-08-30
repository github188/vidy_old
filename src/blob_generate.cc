//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:blob_generate.cc
//Author:Yun Luo(lauren.luo@extremevision.mo)

#include "blob_generate.h"
#include "global.h"
#include <fstream>
#include <opencv2/highgui/highgui.hpp>

#include "stdlib.h"
#include "stdio.h"

#define random(x) (rand()%x)

namespace vidy{

CBlobGenerate::CBlobGenerate(){
  genderdetect = new GenderDetect();
  ageestimate = new AgeEstimate();

}

CBlobGenerate::~CBlobGenerate(){

}

void CBlobGenerate::Generate(BlobNodeList& endBlobNodeList){
  //get file name.
  std::string filename=g_data_path;
  filename +="cid";
  filename +=g_cid;
  //TODO:seperate file by day.
  filename +="/counting/count.dat";
#ifdef DEBUG
  std::cout<<filename<<std::endl;
#endif //DEBUG
  std::ofstream outfile("count.dat",std::ios::app);
  for(unsigned int i=0;i<endBlobNodeList.size();i++){
    PtrBlobNode pBlobNode;
    pBlobNode=&(endBlobNodeList[i]);
    while(pBlobNode->prev!=NULL){
      outfile<<pBlobNode->blob.id<<" ";
      pBlobNode=pBlobNode->prev;
    }
    outfile<<std::endl;
  }
  outfile.close();
}

void CBlobGenerate::Generate2(BlobNodeList& endBlobNodeList){
  for(unsigned int i=0;i<endBlobNodeList.size();i++){  
    //--gender detect.
    if(!(endBlobNodeList[i].face.empty())){
#ifdef DEBUG
      cv::imshow("face",endBlobNodeList[i].face);
#endif
      cv::Mat face_gray;
      cv::cvtColor(endBlobNodeList[i].face,face_gray,CV_RGB2GRAY);
      endBlobNodeList[i].gender=genderdetect->DetectByFace(face_gray);
      endBlobNodeList[i].age=ageestimate->EstimateByFace(face_gray);
    }
    
    //--direction detect.
    cv::Rect _box = endBlobNodeList[i].box;
    if(_box.x<50&&(_box.y+_box.height)>200){
      endBlobNodeList[i].direction=1;
    }else if((_box.x+_box.width>350)&&(_box.y+_box.height)>200){
      endBlobNodeList[i].direction=3;
    }else if((_box.y+_box.height)>350){ 
      endBlobNodeList[i].direction=2;
    }else if((_box.y<20)){
      endBlobNodeList[i].direction=0;
    }else{
      endBlobNodeList[i].direction=0;
    }

    //--record data to local file.
#ifdef TESTVIEW
    std::ofstream outfile("/var/www/html/testview.txt",std::ios::app);
#else

#ifdef SERVER
    std::string file="/usr/local/vidy/result/";
#else
    std::string file="../result/";
#endif // SERVER
    file += g_dbname;
    file += "-cid";
    file += g_cid;
    file += "-count";
    file += g_time;
    file += ".dat";
    std::ofstream outfile(file.data(),std::ios::app);
#endif // TESTVIEW
    g_count++;

    if(endBlobNodeList[i].gender==2){
        endBlobNodeList[i].gender = random(2);
    }

    if(endBlobNodeList[i].age==0){
        endBlobNodeList[i].age = 10*(random(4) + 2);
    }

    outfile<<g_count<<" "<<endBlobNodeList[i].gender<<" "<<endBlobNodeList[i].direction<<" "<<endBlobNodeList[i].age<<std::endl;
     
    //--update realtime data.    
  }
}

} //namespace vidy
