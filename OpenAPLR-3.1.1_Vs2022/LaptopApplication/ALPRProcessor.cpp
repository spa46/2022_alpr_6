
#include "LaptopModules.h"
#include "opencv2/opencv.hpp"

using namespace client;

#define NUMBEROFPREVIOUSPLATES 10
char LastPlates[NUMBEROFPREVIOUSPLATES][64] = { "","","","","" };
unsigned int CurrentPlate = 0;
Point2i last_point(0, 0);
int process_count = 0;

int counter = 0;

void ALPRProcessor::process(Mat frame)
{
	std::vector<AlprRegionOfInterest> regionsOfInterest;
	regionsOfInterest.push_back(AlprRegionOfInterest(0, 0, frame.cols, frame.rows));
	Rect totalrect(0, 0, frame.cols, frame.rows);
	AlprResults results;

	if (regionsOfInterest.size() > 0)
		results = recognize(frame.data, (int)frame.elemSize(), frame.cols, frame.rows,
			regionsOfInterest);

	for (int i = 0; i < results.plates.size(); i++)
	{
		char textbuffer[1024];
		bool found = false;
		std::vector<cv::Point2f> pointset;
		std::vector<cv::Point2i> psi;

		for (int z = 0; z < 4; z++) {
			pointset.push_back(Point2i(results.plates[i].plate_points[z].x,
				results.plates[i].plate_points[z].y));
			psi.push_back(Point2i(pointset[i]));
		}

		cv::Rect rect = cv::boundingRect(pointset);
		cv::rectangle(frame, rect, cv::Scalar(0, 255, 0), 2);
		sprintf_s(textbuffer, "%s - %.2f", results.plates[i].bestPlate.characters.c_str(),
			results.plates[i].bestPlate.overall_confidence);

		cv::putText(frame, textbuffer,
			cv::Point(rect.x, rect.y - 5), //top-left position
			FONT_HERSHEY_COMPLEX_SMALL, 1,
			Scalar(0, 255, 0), 0, LINE_AA, false);

		for (int x = 0; x < NUMBEROFPREVIOUSPLATES; x++)
		{
			if (strcmp(results.plates[i].bestPlate.characters.c_str(), LastPlates[x]) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			Mat plate_cropped;
			string rs = results.plates[i].bestPlate.characters.c_str();

			if (process_count == 0 || abs(last_point.x - psi[0].x) > 80)
			{
				plate_uid++;
				plate_cropped = frame(rect & totalrect);
			}

			printf("str:%s pid:%d rect(x:%d y:%d) image_empty:%d \n", 
				rs.c_str(), plate_uid, rect.x, rect.y,
				plate_cropped.empty());

			viManager->setRecognizedInfo(rs, plate_uid, plate_cropped);

			if (!plate_cropped.empty())
			{
				char fn[128];
				sprintf_s(fn, sizeof(fn), "cropped_%d.jpg", counter++);
				imwrite(fn, plate_cropped);
			}
		}

		strcpy_s(LastPlates[CurrentPlate], results.plates[i].bestPlate.characters.c_str());
		CurrentPlate = (CurrentPlate + 1) % NUMBEROFPREVIOUSPLATES;
		last_point = psi[0];
		process_count++;
	}
}