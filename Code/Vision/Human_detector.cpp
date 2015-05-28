#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;
//--------------------------------------------------------------------------------------------------------------------------//
//Name: R1
//Input: R,G,B values of Mat Image
//Output: (true/false) whether the R,G,B values lies within the threshold.
//--------------------------------------------------------------------------------------------------------------------------//
bool R1(int R, int G, int B) {
    bool e1 = (R>95) && (G>40) && (B>20) && ((max(R,max(G,B)) - min(R, min(G,B)))>15) && (abs(R-G)>15) && (R>G) && (R>B);
    bool e2 = (R>220) && (G>210) && (B>170) && (abs(R-G)<=15) && (R>B) && (G>B);
    return (e1||e2);
}

//--------------------------------------------------------------------------------------------------------------------------//
//Name: R2
//Input: Y,Cr,Cb values of Mat Image
//Output: (true/false) whether the Y,Cr,Cb values lies within the threshold.
//--------------------------------------------------------------------------------------------------------------------------//
bool R2(float Y, float Cr, float Cb) {
    bool e3 = Cr <= 1.5862*Cb+20;
    bool e4 = Cr >= 0.3448*Cb+76.2069;
    bool e5 = Cr >= -4.5652*Cb+234.5652;
    bool e6 = Cr <= -1.15*Cb+301.75;
    bool e7 = Cr <= -2.2857*Cb+432.85;
    return e3 && e4 && e5 && e6 && e7;
}

//--------------------------------------------------------------------------------------------------------------------------//
//Name: R3
//Input: H,S,V values of Mat Image
//Output: (true/false) whether the H,S,V values lies within the threshold.
//--------------------------------------------------------------------------------------------------------------------------//
bool R3(float H, float S, float V) {
    return (H<25) || (H > 230);
}


//------------------------------------------------------------------------------------------------------------------------//
//Name: GetSkin()
//Input: ROIS at which blobs detected
//Output: (true/false) whether a human is in the ROI.
//------------------------------------------------------------------------------------------------------------------------//
bool GetSkin(Mat const &src) {
    // allocate the result matrix
    Mat dst = src.clone();
    
    Vec3b cwhite = Vec3b::all(255);
    Vec3b cblack = Vec3b::all(0);
    
    Mat src_ycrcb, src_hsv;
    // OpenCV scales the YCrCb components, so that they
    // cover the whole value range of [0,255], so there's
    // no need to scale the values:
    cvtColor(src, src_ycrcb, CV_BGR2YCrCb);
    // OpenCV scales the Hue Channel to [0,180] for
    // 8bit images, so make sure we are operating on
    // the full spectrum from [0,360] by using floating
    // point precision:
    src.convertTo(src_hsv, CV_32FC3);
    cvtColor(src_hsv, src_hsv, CV_BGR2HSV);
    // Now scale the values between [0,255]:
    normalize(src_hsv, src_hsv, 0.0, 255.0, NORM_MINMAX, CV_32FC3);
    int WhiteCount = 0;
    int blackCount = 0;
    for(int i = 0; i < src.rows; i++) {
        for(int j = 0; j < src.cols; j++) {
            
            Vec3b pix_bgr = src.ptr<Vec3b>(i)[j];
            int B = pix_bgr.val[0];
            int G = pix_bgr.val[1];
            int R = pix_bgr.val[2];
            // apply rgb rule
            bool a = R1(R,G,B);
            
            Vec3b pix_ycrcb = src_ycrcb.ptr<Vec3b>(i)[j];
            int Y = pix_ycrcb.val[0];
            int Cr = pix_ycrcb.val[1];
            int Cb = pix_ycrcb.val[2];
            // apply ycrcb rule
            bool b = R2(Y,Cr,Cb);
            
            Vec3f pix_hsv = src_hsv.ptr<Vec3f>(i)[j];
            float H = pix_hsv.val[0];
            float S = pix_hsv.val[1];
            float V = pix_hsv.val[2];
            // apply hsv rule
            bool c = R3(H,S,V);
            
            if(!(a&&b&&c))
            {
                dst.ptr<Vec3b>(i)[j] = cblack;
                blackCount++; // used to calc persentage
            }
            if (a&&b&&c)
            {
                dst.ptr<Vec3b>(i)[j] = cwhite;
                WhiteCount++;  // used to calc persentage
            }
            
        }
    }
    
    double image_size = dst.cols*dst.rows;
    if((double) WhiteCount/image_size*100 < 1) // if White pixel is less than 15 % if the image then its not a human (value determined from test)
    {
        cout << "rejected because: " << endl;
        cout <<(double) WhiteCount/image_size*100 << " " <<(double)blackCount/image_size*100<< " " << image_size << endl;
        
        imshow("This", dst);
        return false;
    }
    else
    {
        cout << "accepted because: "<< endl;
        cout <<(double) WhiteCount/image_size*100 << " " <<(double)blackCount/image_size*100<< " " << image_size << endl;
        
        imshow("This", dst);
        return true;
    }
}
//--------------------------------------------------------------------------------------------------------------------------//
//Name: ROI
//Input: BinaryImage and Original image.
//Output: A Mat at which the BinaryImage is used as mask on the Original Image.
//--------------------------------------------------------------------------------------------------------------------------//
Mat ROI(Mat binaryImg, Mat originalImg){
    
    Mat channel[3];
    split(originalImg, channel);
    Mat originalImg2=originalImg.clone();
    
    //calcHistogram(binaryImg);
    
    for(int i=0; i<binaryImg.rows; i++){
        for(int j=0; j<binaryImg.cols; j++){
            if(binaryImg.at<double>(i,j)<250){
                channel[0].at<double>(i,j)=0;
                channel[1].at<double>(i,j)=0;
                channel[2].at<double>(i,j)=0;
            }
        }
    }
    merge(channel, 3, originalImg2);
    cvtColor(originalImg2, originalImg2, CV_BGR2RGB, 0 );
    return originalImg2;
}

//--------------------------------------------------------------------------------------------------------------------------//
//Name: MarkCountours
//Input: BinaryImage
//Output: ROIS with human detection
//--------------------------------------------------------------------------------------------------------------------------//
vector<Mat>  MarkCountours(Mat binaryImg, Mat originalImg) //Input binary image
{
    vector<vector<Point> > contours;
    Mat test = binaryImg.clone();
    Mat test2 = originalImg.clone();
    vector<Vec4i> hierarchy;
    RNG rng(12345);
    findContours( test, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) ); // Findcontours based from the binary image
    vector<vector<Point> > contours_poly( contours.size() );
    vector<Rect> boundRect( contours.size() );
    for( int i = 0; i < contours.size(); i++ )
    {
        approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
        boundRect[i] = boundingRect( Mat(contours_poly[i]) );
        //Creating small initialize the ROI boundary
    }
    
    vector<Mat> subregions_normal;
    vector<Mat> subregions_contour;
    for (int i = 0; i< boundRect.size(); i++)
    {
        if (contourArea(contours[i]) >= 1000 && contourArea(contours[i]) <= 1000000) //If the Area of contour
        {
            Mat roi_normal(test2,boundRect[i]); //Create ROI with approved Contour area;
            if (GetSkin(roi_normal)) { // Check if ROI has approved skin color
                subregions_normal.push_back(roi_normal); // Vector consiting of Detected humans
                rectangle(originalImg, boundRect[i], Scalar(255,0,0)); // Rectangle showing on the original image where it is.
                
            }
        }
        
    }
    
    return subregions_normal;
}




int main(int argc, const char * argv[]) {
    Mat src, dst;
    //Load image
    //src = imread("/Users/keerthikanratnarajah/Desktop/Wave1.jpeg"); // Load image
    //Load Video
    VideoCapture cap("/Users/keerthikanratnarajah/SDU-UAS-15-Group1-Report/Test_images/Oceanvideo_whalerescue.mp4"); // load Video
    while (1)
    {
        cap.read(src);
        cvtColor(src, dst, CV_RGB2HSV, 0 );
        Mat channel[3];
        split(dst, channel);
        
        //Binarize image
        Mat binaryImg;
        threshold(channel[0], binaryImg, THRESH_BINARY, 255, THRESH_OTSU); // Preproccesing at which  the image rets binarizes.
        vector<Mat> ROI = MarkCountours(binaryImg,src); // MarkCounter output ROI with detected Humans within.
        if (ROI.size() == 0) {
            cout << "No Humans detected" << endl;
            return 0;
        }
        int i = 0;
        while (i <  ROI.size())
        {
            if (waitKey(10) != 'y')
            {
                imshow("Found",ROI[i]); // press 'y' to continue
            }
            else
            {
                i++;
            }
        }
        
        imshow("Image binary", binaryImg );
        imshow("SRC" , src);
        waitKey(0);
        
        
    }
    return 0;
}