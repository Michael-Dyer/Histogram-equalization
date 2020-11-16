#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <time.h>
#include <vector>
#include <conio.h>
#include <dirent.h>
#include <windows.h>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/core/utils/filesystem.hpp>

#include <sys/stat.h>
#include <sys/types.h>

#define G_SIZE 256

using namespace cv;
using namespace std;

void image_to_arr(Mat img, float * arr);
int get_total(float *arr);
void normalize_arr(float* arr, int total);
Mat arr_to_matlut(float* arr);//not using to implement my own
Mat lut_to_image(Mat in_img, float* lut);

void write_arr_to_file(float *arr);
void read_arr_from_file(String filename, float *arr);


//use u_char and read as greyscale


//from Color Image Processing Lecture 01
//histo[image.at<uchar>(r,c)++; <- in loop
//histo_float[i] = histo[i] / npixles; <- in loop
//histo_float[i] += histor_float[i-1] <- in loop
//now compute lookup table
//lut[i] = cv::round(255*histo_float[i]);
//now apply lookup table; opencv has one

int main(int argc, const char** argv) {

	ofstream out_histo;
	ifstream in_histo;



	uchar input = 'y';

	int histo_method = 1;
	 

	Mat image;

	try {


		//these are the keys for the comand line parser
		String keys =
			"{m histo_method|1     | change the histogram operation }"
			"{h help  |      | show help message}"     // optional, show help optional
			"{@dir | <none>        | Input file}"
			"{@histogram | <none>        | histogram file}";



		CommandLineParser parser(argc, argv, keys);
		if (parser.has("h") || parser.has("help")) {
			cout << "To run program, execute; program_name ";
			cout << " histo [-h] [-m=n] imagefile [histogram_file]" << endl;
			cout << "m=1 will be histogram equalization, m=2 is histogram matching and m=3 is histogram matching with a file (that is selected with the last argument)" << endl;
			cout << "press q to exit image" << endl;
			parser.printMessage();
			return 0;
		}


		int histo_method = parser.get<int>("m");
		

		//gets mandatory value, the original file from the parser
		String og_file = parser.get<String>(0);
		//og_file = og_file + ".png";
		


		//this program can only work on grayscale images 
		Mat og_img = imread(og_file, IMREAD_GRAYSCALE);
		
		

		if (og_img.empty())
		{
			std::cout << "Could not read the image: " << og_file << std::endl;
			return 1;
		}


		

		
		switch (histo_method) {

		case 1://normalization
		{

			cout << "option 1" << endl;
			//convert image to gray levels
			
			float histo[G_SIZE] = {};
			image_to_arr(og_img, histo);
			int total = get_total(histo);
			//normalize the histogram
			
			normalize_arr(histo, total);
			//apply the histogram to the image
			og_img = lut_to_image(og_img, histo);
		}
			break;




		case 2://matching
		{
			cout << "option 2" << endl;

			
			//Mat to_match = imread("to_match.png", IMREAD_GRAYSCALE);
			Mat to_match = imread("other.png", IMREAD_GRAYSCALE);

			if (to_match.empty()) {
				cout << "Could not read the matching image, to use option ";
				cout << "2, please make sure a file named to_match.png exists within the ";
				cout << "source folder.";
				return 1;
			}


			//now get og histo from to_match
			float histo2[G_SIZE] = {};
			image_to_arr(to_match, histo2);
			int total = get_total(histo2);
			//normalize the histogram
			
			normalize_arr(histo2, total);

			
			//apply the histogram to the image
			og_img = lut_to_image(og_img, histo2);
			}
			break;

			

		case 3:
		{
			cout << "option 3" << endl;
			String histo_name = parser.get<String>(1);
			
			//this is the histogram pulled in from file
			float histo3[G_SIZE] = {};
			read_arr_from_file(histo_name,histo3);

			og_img = lut_to_image(og_img, histo3);

		}
			break;

		default:
			cout << "You didn't enter a valid option; chose either 1, 2 or 3. Printing original image!" << endl;
		}
		
		//used this to make new histo.txt files
		/*
		Mat light = imread("name.png", IMREAD_GRAYSCALE);
		float l_histo[G_SIZE] = {};
		image_to_arr(light, l_histo);
		int total = get_total(l_histo);
		
		

		normalize_arr(l_histo, total);
		for (int y = 0; y < G_SIZE; y++) {
			cout << l_histo[y] << endl;
		}
		write_arr_to_file(l_histo);
		*/

		//this stuff is for display~~
		namedWindow("Display window", WINDOW_AUTOSIZE); // Create a window for display.
		moveWindow("Display window", 0, 0);
		do {
			imshow("Display window", og_img);
			input = waitKey();



		} while (input != 'q' && input != 'Q');


		cout << "end";


	}
	catch (std::string& str) {
		std::cerr << "Error: " << argv[0] << ": " << str << std::endl;
		return (1);
	}
	catch (cv::Exception& e) {
		std::cerr << "Error: " << argv[0] << ": " << e.msg << std::endl;
		return (1);
	}

	
	return (0);
}


//make an array of number of gray levels in an image
void image_to_arr(Mat img, float *arr) {
	

	int total = 0;
	for (int r = 0; r < img.rows; r++) {
		for (int c = 0; c < img.cols; c++) {
			arr[img.at<uchar>(r, c)]++;
			total++;
		}
	}

	
	
}

//simply count number of pixles in table 
int get_total(float *arr) {
	int total = 0;
	for (int i = 0; i < G_SIZE; i++) {
		total = total + arr[i];
	}

	return total;
}

//the math provided to normalize the histogram
void normalize_arr(float *arr, int total) {
	


	
	for (int i = 0; i < G_SIZE; i++) {
		arr[i] = arr[i] / total;
	}

	//start at 1 to avoid oob error
	for (int i = 1; i < G_SIZE; i++) {
		arr[i] += arr[i-1];
	}

	//this is to change the values between 0-1 to 1-256
	for (int i = 0; i < G_SIZE; i++) {
		arr[i] = round(255 * arr[i]);
	}
	
	
}


//converts the arr to a lut to be used by cv::LUT(IN_MAT, LUT_MAT, OUT_MAT); 
//(from opencv documentation)
//I'm not using this to program my own from scratch; see below 
Mat arr_to_matlut(float* arr) {
	Mat lookUpTable(1, 256, CV_8U);

	uchar* p = lookUpTable.ptr();
	for (int i = 0; i < 256; ++i) {
		p[i] = arr[i];
	}

	return lookUpTable;
}

//providing the changes from image to lookup table for efficiency 
Mat lut_to_image(Mat in_img, float* lut) {
	Mat new_mat = cv::Mat::zeros(in_img.rows, in_img.cols, in_img.type());

	for (int r = 0; r < in_img.rows; r++) {
		for (int c = 0; c < in_img.cols; c++) {

			//maps old gray level to new gray level
			//had to convert types
			int old_gray = (int) in_img.at<uchar>(r, c);
			char new_gray = (char) lut[old_gray];

			new_mat.at<uchar>(r, c) = new_gray;

		}
	}


	return new_mat;
}

//this function is used to write an array to a txt file to read to later
void write_arr_to_file(float* arr) {
	ofstream out_file("new_histo.txt");

	for (int i = 0; i < G_SIZE; i++) {
		out_file << arr[i] << endl;
	}

	out_file.close();

}

//reads the files made with the above function and returns an array
void read_arr_from_file(String filename, float *arr) {
	ifstream in_file(filename);

	if (!in_file) {
		cout << "can't open the file" << endl;
		exit(EXIT_FAILURE);
	}

	if (in_file.is_open()) {

		for (int i = 0; i < G_SIZE; i++) {
			in_file >> arr[i];
			
		}

	}

	in_file.close();
}