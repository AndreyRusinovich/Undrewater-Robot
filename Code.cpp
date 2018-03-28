#include <murAPI.hpp> // ����������� ���������� murAPI

Object detectBlack(cv::Mat img) {                                       // �� 2 �� 30 ������ ������� ��� ����������� ������� ��������                                               
	cv::Scalar lower(0, 0, 0);
	cv::Scalar upper(0, 0, 255);
	cv::cvtColor(img, img, CV_BGR2HSV);
	cv::inRange(img, lower, upper, img);
	std::vector<std::vector<cv::Point>>contours;
	cv::findContours(img, contours, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
	Object objectToRet;

	for (std::size_t i = 0; i < contours.size(); i++) {
		if (contours.at(i).size() < 100) {
			continue;
		}
		if (std::fabs(cv::contourArea(contours.at(i))) < 300.0) {
			continue;
		}

		cv::RotatedRect bEllipse = cv::fitEllipse(contours.at(i));
		objectToRet.x = (int)bEllipse.center.x;
		objectToRet.y = (int)bEllipse.center.y;
		objectToRet.angle = bEllipse.angle;
		for (auto obj : mur.getDetectedObjectsList(0)) {
			if (obj.type == Object::RECTANGLE) objectToRet.type = Object::RECTANGLE;
		}
		return objectToRet;
	}
	return objectToRet;
}

void keepYD(double &yts, double &dth, bool &black) {   // � 32 �� 47 ������ ������� ��� ��������� ����� � �������
	float power = 20.0;
	double yaw = mur.getYaw(),
		depth = mur.getInputAOne();
	if (!black) {
		if (yaw > yts) {
			mur.setPortA(power / 3.2);
			mur.setPortB(power);
		}
		if (yaw < yts) {
			mur.setPortA(power);
			mur.setPortB(power / 3);
		}
		mur.setPortC((dth - depth) * 10 * (-1));
	}
}

void rotateTo(double &yts, bool &line, int &i, double &dth) {                              // � 49 �� 64 ������ ������� ��� �������� �� ����� �����
	if (!line) {
		for (auto obj : mur.getDetectedObjectsList(0)) {
			if (obj.type == Object::RECTANGLE) {
				i++;
				if (obj.angle < 90) {
					yts += obj.angle;
				}
				else {
					yts -= std::fabs(90 - obj.angle);
				}
				if (i < 2) {
					mur.removeDetectorFromList(Object::RECTANGLE, 0);
					mur.addDetectorToList(Object::CIRCLE, 0);
				}
				if (i == 2) {
					dth = 100.0;
				}
				line = true;
			}
		}
	}
}

void findCirc(bool &line) {                                             // � 66 �� 80 ������ ������� ��� ������ �����
	if (line) {
		for (auto obj : mur.getDetectedObjectsList(0)) {
			if (obj.type == Object::CIRCLE) {
				mur.setPortA(0);
				mur.setPortB(0);
				mur.setPortC(-40);
				sleepFor(5000);
				mur.removeDetectorFromList(Object::CIRCLE, 0);
				mur.addDetectorToList(Object::RECTANGLE, 0);
				line = false;
			}
		}
	}
}


void findBlack(bool &black) {                                       // � 83 �� 96 ������ ������� ��� ������ ������� ��������
	Object obj = detectBlack(mur.getCameraOneFrame());
	if (obj.type == Object::RECTANGLE) {
		black = true;
		mur.setPortA(0);
		mur.setPortB(0);
		mur.setPortC(-70);
		sleepFor(7000);
		mur.setPortA(0);
		mur.setPortB(0);
		mur.setPortC(80);
		sleepFor(1000000);
	}
}

int main() {
	double yts = 0.0,   // ��������� ����
		dth = 60.0;  // ��������� �������
	bool line = false, black = false;
	int i = 0;
	mur.addDetectorToList(Object::RECTANGLE, 0); // ���������� ����� � ������ ������� ��������

	while (1) {
		keepYD(yts, dth, black);  // ������� ��� ��������� ������� � �����
		rotateTo(yts, line, i, dth);      // ������� ��� �������� �� ����� �����
		findBlack(black);          // ������� ��� ������ �����      
		findCirc(line);         // ������� ��� ������ ������� ��������
	}
	return 0;
}