//non-recursive biggest blob finding
//find the largest non-zero value blob in a binary image
//
//written by Li Guan
//lguan@cs.unc.edu
//Oct 2nd, 2008

#ifndef BLOB_H
#define BLOB_H

struct blobLabel
{
	int label;
	int size;
};

//non-recursive biggest blob finding
int findLargestBlob(unsigned char *I, int ih, int iw)
{
	int largestBlobSize = 0;
	int labelNum = 0;

	//initialize label map with -1
	blobLabel *labels = new blobLabel[ih*iw];
	for(int i=0;i<ih*iw;i++){
		if(I[i]>0){
			labels[i].label = i;
			labels[i].size = 1;
		}
		else{
			labels[i].label = i;
			labels[i].size = 0;
		}
	}

	bool isMerge = true;
	int iter = 1;
	while(isMerge){
		isMerge = false;
		//printf("Mergeing iteration %d.\n", iter++);
		for(int i=0;i<ih;i++){
			for(int j=0;j<iw;j++){
				if(I[i*iw+j]>0){//we need to assign a label to the pixel in labelMap
					//check its neighbors CONN-4
					if(i-1>=0&&I[(i-1)*iw+j]>0&&labels[i*iw+j].label!=labels[(i-1)*iw+j].label){
						//both pixels are on, and they are of different labels
						//merge towards the smaller label
						if(labels[i*iw+j].label>labels[(i-1)*iw+j].label){
							labels[labels[(i-1)*iw+j].label].size += labels[labels[i*iw+j].label].size;
							labels[labels[i*iw+j].label].size = 0;
							labels[i*iw+j].label = labels[(i-1)*iw+j].label;
							labels[i*iw+j].size = 0;
						}
						else{
							labels[labels[i*iw+j].label].size += labels[labels[(i-1)*iw+j].label].size;
							labels[labels[(i-1)*iw+j].label].size = 0;
							labels[(i-1)*iw+j].label = labels[i*iw+j].label;
							labels[(i-1)*iw+j].size = 0;
						}
						isMerge = true;
						continue;
					}
					if(i+1<ih&&I[(i+1)*iw+j]>0&&labels[i*iw+j].label!=labels[(i+1)*iw+j].label){
						//both pixels are on, and they are of different labels
						//merge towards the smaller label						
						if(labels[i*iw+j].label<labels[(i+1)*iw+j].label){
							labels[labels[i*iw+j].label].size += labels[labels[(i+1)*iw+j].label].size;
							labels[labels[(i+1)*iw+j].label].size = 0;
							labels[(i+1)*iw+j].label = labels[i*iw+j].label;
							labels[(i+1)*iw+j].size = 0;
						}
						else{
							labels[labels[(i+1)*iw+j].label].size += labels[labels[i*iw+j].label].size;
							labels[labels[i*iw+j].label].size = 0;
							labels[i*iw+j].label = labels[(i+1)*iw+j].label;
							labels[i*iw+j].size = 0;
						}

						isMerge = true;
						continue;
					}
					if(j-1>=0&&I[i*iw+j-1]>0&&labels[i*iw+j].label!=labels[i*iw+j-1].label){
						//both pixels are on, and they are of different labels
						//merge towards the smaller label
						if(labels[i*iw+j].label>labels[i*iw+j-1].label){
							labels[labels[i*iw+j-1].label].size += labels[labels[i*iw+j].label].size;
							labels[labels[i*iw+j].label].size = 0;
							labels[i*iw+j].label = labels[i*iw+j-1].label;
							labels[i*iw+j].size = 0;
						}
						else{
							labels[labels[i*iw+j].label].size += labels[labels[i*iw+j-1].label].size;
							labels[labels[i*iw+j-1].label].size = 0;
							labels[i*iw+j-1].label = labels[i*iw+j].label;
							labels[i*iw+j-1].size = 0;
						}
						isMerge = true;
						continue;
					}
					if(j+1<iw&&I[i*iw+j+1]>0&&labels[i*iw+j].label!=labels[i*iw+j+1].label){
						//both pixels are on, and they are of different labels
						//merge towards the smaller label
						if(labels[i*iw+j].label<labels[i*iw+j+1].label){
							labels[labels[i*iw+j].label].size += labels[labels[i*iw+j+1].label].size;
							labels[labels[i*iw+j+1].label].size = 0;
							labels[i*iw+j+1].label = labels[i*iw+j].label;
							labels[i*iw+j+1].size = 0;
						}
						else{
							labels[labels[i*iw+j+1].label].size += labels[labels[i*iw+j].label].size;
							labels[labels[i*iw+j].label].size = 0;
							labels[i*iw+j].label = labels[i*iw+j+1].label;
							labels[i*iw+j].size = 0;
						}
						isMerge = true;
						continue;
					}
				}
			}
		}

		/*int maxSize = 0;
		int label = 0;
		for(int i=0;i<ih;i++){
			for(int j=0;j<iw;j++){
				if(maxSize<labels[i*iw+j].size){
					maxSize = labels[i*iw+j].size;
					label = labels[i*iw+j].label;
				}
			}
		}

		printf("max size = %d.\n", maxSize);*/

	}

	int maxSize = 0;
	int label = 0;
	for(int i=0;i<ih;i++){
		for(int j=0;j<iw;j++){
			if(maxSize<labels[i*iw+j].size){
				maxSize = labels[i*iw+j].size;
				label = labels[i*iw+j].label;
			}
		}
	}

	for(int i=0;i<ih;i++){
		for(int j=0;j<iw;j++){
			if(labels[i*iw+j].label==label){
				I[i*iw+j]=255;
			}
		}
	}

	return maxSize;
}


#endif