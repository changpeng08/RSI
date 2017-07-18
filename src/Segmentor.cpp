#include "Segmentor.hpp"

double AASS::RSI::Segmentor::segmentImage(cv::Mat& src, AASS::RSI::GraphZone& graph_src)
{
						
//Find contours
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours( src, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

	//Draw the outer contours
	cv::Mat outer = cv::Mat::zeros(src.rows, src.cols, CV_8UC1);
	// iterate through all contours, filling holes
	for(int i = 0 ; i < contours.size() ; ++i)
	{
// 					if(hierarchy[i][3] != -1){
			cv::drawContours( outer, contours, i, 255 , CV_FILLED, 8, hierarchy );
// 					}
	}
// 	
// 	cv::imshow("input", src);
// 	cv::waitKey(0);
	
// 	cv::threshold(src, src, 20, 255, cv::THRESH_BINARY);
// 	cv::threshold(src, src, 20, 255, cv::THRESH_BINARY_INV);
	
	double begin_process, end_process, decompose_time;
	std::cout << "/************ FUZZY OPENING*************/ \n";
	AASS::RSI::FuzzyOpening fuzzy_slam;
	fuzzy_slam.fast(false);
	
	cv::Mat out_slam;
	
	begin_process = getTime();	
	fuzzy_slam.fuzzyOpening(outer, out_slam, 500);
	end_process = getTime();	decompose_time = end_process - begin_process;
	double time = decompose_time;
	
	std::cout << "Fuzzy opening time: " << time << std::endl;
	
	out_slam.convertTo(out_slam, CV_8U);
		
	std::cout << "/************ REDUCING THE SPACE OF VALUES *****************/\n";
	cv::Mat out_tmp_slam;
	AASS::RSI::ZoneExtractor zone_maker;
	zone_maker.addValueToIgnore(0);
	
	begin_process = getTime();
	AASS::RSI::reduceZone(out_slam, out_tmp_slam, 5);
	zone_maker.extract(out_tmp_slam);
	end_process = getTime();	decompose_time = end_process - begin_process;
	time = time + decompose_time;
	
	std::cout << "Zone reducing: " << decompose_time << std::endl;
		
	std::cout << "/*********** MAKING AND TRIMMING THE GRAPH ***************/\n";
	
	int size_to_remove2 = 10;
	
	begin_process = getTime();
	graph_src = zone_maker.getGraph();
	graph_src.setThreshold(0.25);
// 	graph_src.removeVertexValue(0);	
	graph_src.removeVertexUnderSize(size_to_remove2, true);

	graph_src.useCvMat(true);
// 	graph_src.updatePCA();
	graph_src.updateContours();
	graph_src.removeRiplesv3();
	
	end_process = getTime();	decompose_time = end_process - begin_process;
	time = time + decompose_time;
	std::cout << "Ripples: " << decompose_time << std::endl;
	
	begin_process = getTime();
// 	graph_src.updatePCA();
	graph_src.updateContours();
	
	//Watershed Algorithm
	graph_src.watershed();
	
	int size_to_remove = 100;
	graph_src.removeVertexUnderSize(size_to_remove, true);
	graph_src.removeLonelyVertices();
	end_process = getTime();	decompose_time = end_process - begin_process;
	time = time + decompose_time;
	
	std::cout << "watershed: " << decompose_time << std::endl;
	
	if(graph_src.lonelyVertices())
		throw std::runtime_error("Fuck you lonelyness");	
	
	
// 	addHoles(src, contours, hierarchy, graph_src);
	
	return time;
	
}


void AASS::RSI::Segmentor::addHoles(const cv::Mat& src, std::vector< std::vector< cv::Point > > contours, std::vector< cv::Vec4i > hierarchy, AASS::RSI::GraphZone& graph_src)
{
				
	//Draw all holes on a Mat
	cv::Mat drawing = cv::Mat::zeros(src.rows, src.cols, CV_8UC1);
	// iterate through all the top-level contours,
	for(int i = 0 ; i < contours.size() ; ++i)
	{
		if(hierarchy[i][3] != -1){
			cv::drawContours( drawing, contours, i, 255 , CV_FILLED, 8, hierarchy );
		}
	}
	
	for(int row = 0; row < drawing.rows; ++row) {
		uchar* p = drawing.ptr(row);
		for(int col = 0; col < drawing.cols; ++col) {
			//If the pixel is part of a hole
			if(p[col] != 0){
				//For all zone remove it if it is part of the zone
				std::pair<AASS::RSI::GraphZone::VertexIteratorZone, AASS::RSI::GraphZone::VertexIteratorZone> vp;
				//vertices access all the vertix
// 				std::cout << "NEW start lonely" << std::endl;
		// 		std::cout << "num of vertices " << getNumVertices() << std::endl; 
				for (vp = boost::vertices(graph_src); vp.first != vp.second;) {
		// 			std::cout << "Looking up vertex " << std::endl;
					auto v = *vp.first;
					++vp.first;
					
					graph_src[v].removePoint(row, col);
					
				}
				
				
			}
		}
	}
	
}


void AASS::RSI::Segmentor::findLimits(AASS::RSI::GraphZone& graph_src)
{
	
	auto contacttest = [](int x, int y, int x2, int y2) -> bool{
		if(x + 1 >= x2 && x - 1 <= x2){
			if(y + 1 >= y2 && y - 1 <= y2){
				return true;
			}
		}
		return false;
	};
	
	
	std::pair<AASS::RSI::GraphZone::VertexIteratorZone, AASS::RSI::GraphZone::VertexIteratorZone> vp;
	//vertices access all the vertix
// 				std::cout << "NEW start lonely" << std::endl;
// 		std::cout << "num of vertices " << getNumVertices() << std::endl; 
	for (vp = boost::vertices(graph_src); vp.first != vp.second;) {
// 			std::cout << "Looking up vertex " << std::endl;
		auto v = *vp.first;
		++vp.first;
		AASS::RSI::GraphZone::EdgeIteratorZone out_i, out_end;
		//Since we fuse the old zone in biggest we only need to link them to biggest
		for (boost::tie(out_i, out_end) = boost::out_edges(v, graph_src); 
			out_i != out_end; out_i = ++out_i) {
			AASS::RSI::GraphZone::EdgeZone e_second = *out_i;
			AASS::RSI::GraphZone::VertexZone targ = boost::target(e_second, graph_src);
			AASS::RSI::GraphZone::VertexZone src = boost::source(e_second, graph_src);
		
			auto contact = graph_src[src].getContactPoint(graph_src[targ]);
			
			std::pair<cv::Point2i, cv::Point2i> limits;
			int min = 15;
			int min2 = 15;
		
		
			for(auto it = contact.begin(); it != contact.end() ; ++it){
				int nb_contact = 0;
				for(auto it2 = contact.begin(); it2 != contact.end() ; ++it2){
					if(contacttest(it->x, it->y, it2->x, it2->y)){
						++nb_contact;
					}
				}
				if(nb_contact < min){
					limits.first = *it;
				}
				else if(nb_contact < min2){
					limits.second = *it;
				}
			}
			
			
		}
		
		
		
		
		
	}

}
