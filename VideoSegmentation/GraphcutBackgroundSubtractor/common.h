#ifndef COMMON_H
#define COMMON_H

#define		MAX_FEATURE		1000
#define		pi				3.14159265358979323846

/* This is just an inline that allocates images. I did this to reduce clutter in the
* actual computer vision algorithmic code. Basically it allocates the requested image
* unless that image is already non-NULL. It always leaves a non-NULL image as-is even
* if that image's size, depth, and/or channels are different than the request.
*/

inline static void allocateOnDemand( IplImage **img, CvSize size, int depth, int channels)
{
	if ( *img != NULL ) return;
	*img = cvCreateImage( size, depth, channels );
	if ( *img == NULL )
	{
		fprintf(stderr, "Error: Couldn't allocate image. Out of memory?\n");
		exit(-1);
	}
}

char *generateFileName(int imageNum, char* ext, char* fileName, int digit, char *fileAbsoluteName, char *dir)
{
	char *num = (char*)malloc(sizeof(char)*digit);
	char *formattedNum = (char*)malloc(sizeof(char)*digit);
	for(int i=0;i<digit;i++){
		num[i]=0;
		formattedNum[i]=0;
	}
	_itoa(imageNum, num, 10);
	int numZero = digit - strlen(num);
	for(int i=0;i<numZero;i++){
		formattedNum[i] = '0';
	}

	sprintf(fileAbsoluteName, "%s%s%s%s%s", dir, fileName, formattedNum, num, ext);
	if(digit>1){
		free(num);
		free(formattedNum);
	}
	return fileAbsoluteName;
}

#endif