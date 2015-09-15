#ifndef VIDY_INC_BLOB_COMPARE_H_                                            
#define VIDY_INC_BLOB_COMPARE_H_                                            
                                                                            
#include "blob.h"                                                           
                                                                            
namespace vidy{                                                             
                                                                            
class BlobCompare{                                                          
public:                                                                     
  BlobCompare();                                                            
  ~BlobCompare();                                                           
                                                                            
  //@param distance: distance to compare                                    
  //@return 1: blobs are in the distance.                                   
  //@return 0: blobs are not in the distance.                               
  int CompareDistance(const BlobNode current_blobnode,const BlobNode compare_blobnode,float distance);

  //TODO:compare two blobs by their features.
  //@return 1: blobs are similar.                                           
  //@return 0: blobs are not similar.                                       
  int CompareFeature(const BlobNode current_blobnode,const BlobNode compare_blobnode) const;

}; //class BlobCompare

} //namespace vidy
                                                                            
#endif

