#include "FuzzyOpening.hpp"

//Needs to be int now
void AASS::RSI::FuzzyOpening::addPointValueInCircle(cv::Mat& input, cv::Mat& output, int value)
{
// 	std::cout << "INPUT " << std::endl << input << std::endl;
	
// 	std::cout << "OUTPUT " << std::endl << output << std::endl;
	//Making circle;
	cv::Mat element;
	
	//TODO : make circle drawing faster ! The problem is escentially the drawing time of the circles
	//If the algorithm doesn't need to run fast, we run a circluar element for more accurate result
	if(_fast == false){
		element = cv::Mat::zeros((value * 2) + 2 , (value * 2) + 2, CV_32F);
		cv::circle(element, cv::Point2i((value), (value)), (value), cv::Scalar(1), -1);
// 		cv::circle(element, cv::Point2i((value), (value)), (value)-1, cv::Scalar(1), -1);
// 		cv::circle(element, cv::Point2i((value)-1, (value)), (value)-1, cv::Scalar(1), -1);
// 		cv::circle(element, cv::Point2i((value), (value)-1), (value)-1, cv::Scalar(1), -1);
		
		//Needs to be not even
// 		element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2*value +1, 2*value +1), cv::Point(value-1 , value-1 ) );
// 		std::cout << value << std::endl << element << std::endl;
	}
	//To go fast we use a rectangular element for comparison
	else{
		element = cv::Mat::ones(value * 2 , value * 2, CV_32F);
	}
	
// 	std::cout << "CERLCE " << std::endl << circle << std::endl;
	
	if(input.rows > element.rows || input.cols > element.cols){
		std::ostringstream str_test;
		str_test <<  "Input is bigger than elemtn at line " << __LINE__ << " in file " << __FILE__ << "." << std::endl << "Input rows and col : " << input.rows << " " << input.cols << " element : " << element.rows << " " << element.cols ;
		throw std::runtime_error(str_test.str() );
	}
	
	
	for(int row = 0 ; row < input.rows ; row++){
		float* p = input.ptr<float>(row); //point to each row
		float* p_output = output.ptr<float>(row); //point to each row
		float* p_element = element.ptr<float>(row); //point to each row
		for(int col = 0 ; col < input.cols ; col++){
			//p[j] <- how to access element
	// 				std::cout << (int)p[j]<< std::endl;
// 			std::cout << "Comparing " << (float)p[col] << " and " << value << " while circle " << p_circle[col] << std::endl;
			if((float)p[col] > 0 && (float)p_output[col] < value && (float)p_element[col] > 0){
				p_output[col] = value;
			}
// 			if((float)p[col] == 0 && p_circle[col] > 0){
// 				p[col] = value;
// 			}
		}
	}
// 	return true;
	
	

}

void AASS::RSI::FuzzyOpening::fuzzyOpening(const cv::Mat& src, cv::Mat& output, int size)
{
	//Calcul distance image
	cv::Mat distance_image, label;
	if(src.channels() == 3){
		std::cout << "Convert" << std::endl;
		cv::cvtColor(src, distance_image, CV_RGB2GRAY);
	}
	else{
		src.copyTo(distance_image);
	}
	
// 	distance_image.convertTo(CV_);
	
	cv::threshold(distance_image, distance_image, 50, 255, CV_THRESH_BINARY_INV);
	cv::distanceTransform(distance_image, distance_image, label, CV_DIST_L2, CV_DIST_MASK_PRECISE, CV_DIST_LABEL_CCOMP);
// 	std::cout << " TYPE " << type2str(distance_image.type()) << std::endl;	
	CV_Assert(distance_image.depth() == CV_32F);
	
	int pad = distance_image.rows;
	int old_rows = distance_image.rows;
	int old_cols = distance_image.cols;
	if(pad < distance_image.cols){
		pad = distance_image.cols;
	}
	
	cv::copyMakeBorder( distance_image, distance_image, pad, pad, pad, pad, cv::BORDER_CONSTANT, 0 );
	
	output = cv::Mat::ones(distance_image.rows, distance_image.cols, CV_32F);
	cv::Mat roi_output_final = output(cv::Rect(pad, pad, old_cols, old_rows));
// 	std::cout << roi_output_final << std::endl;
	//Update result 
	int count = 0;
	for(int row = pad ; row < distance_image.rows - pad + 1 /*&& count < 15000*/ ; row++){
		float* p = distance_image.ptr<float>(row); //point to each row
		float* p_output = output.ptr<float>(row); //point to each row
		for(int col = pad ; col < distance_image.cols - pad + 1 /*&& count < 15000*/ ; col++){
			
			int dist_to_obstacle = (float)p[col];
			dist_to_obstacle;
			if(dist_to_obstacle > _size){
				dist_to_obstacle = _size;
			}
			else{
// 				std::cout << "size is good " << dist_to_obstacle <<  std::endl;
			}
// 			std::cout << row << " " << col << " " << " dist : " <<dist_to_obstacle << " same " << (float)p[col] << " | " ;
			
			cv::Mat roi = distance_image(cv::Rect(col - (dist_to_obstacle), row - (dist_to_obstacle), dist_to_obstacle * 2, dist_to_obstacle * 2));
			cv::Mat roi_output = output(cv::Rect(col - (dist_to_obstacle), row - (dist_to_obstacle), dist_to_obstacle * 2, dist_to_obstacle * 2));
			if((int)dist_to_obstacle > 0){
				addPointValueInCircle(roi, roi_output, dist_to_obstacle);
// 				std::cout << std::endl;
// 				std::cout << roi_output_final << std::endl;
// 				int a ;
// 				std::cin >> a;
// 				count++;
// 				if(count % 15000 == 0){
					
// 					std::cout << "Hello " << count << std::endl;
// 					cv::Mat copy;
// 					roi_output_final.copyTo(copy);
// 					cv::normalize(copy, copy, 0, 1, cv::NORM_MINMAX, CV_32F);
// 					cv::imshow("out", copy);
// // 					
// // 					cv::Mat roi_tmp_draw = cv::Mat::zeros(roi_output_final.rows, roi_output_final.cols, CV_32F);
// // 					if(dist_to_obstacle%2 != 0){
// // 						cv::circle(roi_tmp_draw, cv::Point2i(col - pad , row - pad ) , dist_to_obstacle, cv::Scalar(1), -1);
// // 					}
// // 					else{
// // 						cv::circle(roi_tmp_draw, cv::Point2i(col-1 - pad , row -1 - pad ), (dist_to_obstacle)-1, cv::Scalar(1), -1);
// // 						cv::circle(roi_tmp_draw, cv::Point2i(col - pad , row - pad ), (dist_to_obstacle)-1, cv::Scalar(1), -1);
// // 						cv::circle(roi_tmp_draw, cv::Point2i(col -1 - pad , row - pad ), (dist_to_obstacle)-1, cv::Scalar(1), -1);
// // 						cv::circle(roi_tmp_draw, cv::Point2i(col - pad , row - -1 - pad ), (dist_to_obstacle)-1, cv::Scalar(1), -1);
// // 					}
// // 					
// // 					cv::imshow("circle moving", roi_tmp_draw);
// // 					
// 					cv::waitKey(1);
// 				}
			}
			else{
				p_output[col] = 0;
// 				std::cout << "Wall" << std::endl;
			}
			
			
			
		}
// 		std::cout << std::endl;
	}
	
	cv::normalize(roi_output_final, roi_output_final, 0, 255, cv::NORM_MINMAX, CV_8U);
	
// 	std::cout << roi_output_final << std::endl;
	
// 	cv::imshow("out", roi_output_final);
// 	cv::waitKey(0);
	
	output = roi_output_final;

}

//I don't need that in the end 
bool AASS::RSI::FuzzyOpening::circleIsEmpty(cv::Mat& input, cv::Mat& circle)
{
	for(int row = 0 ; row < input.rows ; row++){
		uchar* p = input.ptr(row); //point to each row
		uchar* p_circle = circle.ptr(row); //point to each row
		for(int col = 0 ; col < input.cols ; col++){
			//p[j] <- how to access element
	// 				std::cout << (int)p[j]<< std::endl;
			if(p[col] > 0 && p_circle[col] > 0){
				return false;
			}
		}
	}
	return true;
	
	
}

