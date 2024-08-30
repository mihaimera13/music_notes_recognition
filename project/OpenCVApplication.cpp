// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"
#include <vector>
#include <random>
#include <fstream>
#include "pitches.h"

#define HALF 2
#define QUARTER 4
#define EIGHTH 8

using namespace std;

struct Stave
{
	int start;
	int end;
};

struct Label
{
	int label;
	int x;
	int y;
};

bool isInside(Mat_<uchar> img, int i, int j)
{
	if (i >= 0 && i < img.rows && j >= 0 && j < img.cols)
		return true;
	return false;
}

Mat_<uchar> project_cut(Mat_<uchar> img)
{
	int min_x = img.rows, min_y = img.cols, max_x = 0, max_y = 0;
	for (int i = 0; i < img.rows; i++)
		for (int j = 0; j < img.cols; j++)
		{
			if (img(i, j) == 0)
			{
				if (i < min_x)
					min_x = i;
				if (i > max_x)
					max_x = i;
				if (j < min_y)
					min_y = j;
				if (j > max_y)
					max_y = j;
			}
		}

	Mat_<uchar> new_img(max_x - min_x + 1, max_y - min_y + 1);
	for (int i = min_x; i <= max_x; i++)
		for (int j = min_y; j <= max_y; j++)
			new_img(i - min_x, j - min_y) = img(i, j);

	return new_img;
}

Mat_<uchar> project_cut(Mat_<int> img, int label,int &min, int &max)
{
	int min_x = img.rows, min_y = img.cols, max_x = 0, max_y = 0;
	for (int i = 0; i < img.rows; i++)
		for (int j = 0; j < img.cols; j++)
		{
			if (img(i, j) == label)
			{
				if (i < min_x)
					min_x = i;
				if (i > max_x)
					max_x = i;
				if (j < min_y)
					min_y = j;
				if (j > max_y)
					max_y = j;
			}
		}

	min = min_x;
	max = max_x;


	Mat_<uchar> new_img(max_x - min_x + 1, max_y - min_y + 1);
	for (int i = min_x; i <= max_x; i++)
		for (int j = min_y; j <= max_y; j++)
			if(img(i,j)==0)
				new_img(i - min_x, j - min_y) = 255;
			else
				new_img(i - min_x, j - min_y) = 0;
	return new_img;
}

Mat_<uchar> project_threshold(Mat_<uchar> img)
{
	Mat_<uchar> new_img(img.rows, img.cols);
	int threshold = 200;
	for (int i = 0; i < img.rows; i++)
		for (int j = 0; j < img.cols; j++)
		{
			if (img(i, j) > threshold)
				new_img(i, j) = 255;
			else
				new_img(i, j) = 0;
		}
	return new_img;
}

int project_area(Mat_<uchar> img)
{
	int area = 0;
	for (int i = 0; i < img.rows; i++)
		for (int j = 0; j < img.cols; j++)
			if (img(i, j) == 0)
				area++;
	return area;
}

vector<float> project_meanonrows(Mat_<uchar> img)
{
	vector<float> mean;
	for (int i = 0; i < img.rows; i++)
	{
		float sum = 0;
		for (int j = 0; j < img.cols; j++)
			sum += img(i, j);
		mean.push_back(sum * 1.0 / img.cols);
	}

	return mean;
}

vector<float> project_whitedensity(Mat_<uchar> img)
{
	vector<float> density;
	for (int i = 0; i < img.rows; i++)
	{
		float sum = 0;
		for (int j = 0; j < img.cols; j++)
			if (img(i, j) == 0)
				sum++;
		density.push_back(1.0 - sum * 1.0 / img.cols);
	}
	return density;
}

vector<Stave> project_getstaves(Mat_<uchar> img)
{
	vector<Stave> staves;

	vector<float> mean = project_meanonrows(img);
	vector<float> density = project_whitedensity(img);

	for (int i = 0; i < img.rows; i++)
	{
		if (mean[i] < 100 && density[i] < 0.3)
		{
			Stave s;
			s.start = i;
			while (i < img.rows && mean[i] < 100 && density[i] < 0.3)
				i++;
			s.end = i-1;
			staves.push_back(s);
		}
	}

	return staves;
}

Mat_<uchar> project_deletestaves(Mat_<uchar> original, vector<Stave> staves)
{
	Mat_<uchar> img = original.clone();

	for (int i = 0; i < staves.size(); i++)
	{
		int above = staves[i].start - 1;
		int below = staves[i].end + 1;

		for (int k = 0; k < img.cols; k++)
		{
			if (isInside(img, above, k) && img(above, k) == 255 && isInside(img, below, k) && img(below, k) == 255)
			{
				for (int j = staves[i].start; j <= staves[i].end; j++)
					img(j, k) = 255;
			}
			else if (!isInside(img, above, k) && isInside(img, below, k) && img(below, k) == 255)
			{
				for (int j = staves[i].start; j <= staves[i].end; j++)
					img(j, k) = 255;
			}
			else if (isInside(img, above, k) && img(above, k) == 255 && !isInside(img, below, k))
			{
				for (int j = staves[i].start; j <= staves[i].end; j++)
					img(j, k) = 255;
			}
		}
	}

	return img;
}

default_random_engine gen;
uniform_int_distribution<int> d(0, 255);

void lab5_visualizecolors(Mat_<int> img, int label)
{
	vector<Vec3b> colors;

	for (int i = 0; i <= label; i++)
		colors.push_back(Vec3b(d(gen), d(gen), d(gen)));

	Mat_<Vec3b> colored(img.rows, img.cols);

	for (int i = 0; i < img.rows; i++)
		for (int j = 0; j < img.cols; j++)
			if (img(i, j) == 0)
				colored(i, j) = Vec3b(255, 255, 255);
			else
				colored(i, j) = colors[img(i, j)];

	imshow("Colored labels", colored);
	waitKey();
}

Mat_<int> project_label(Mat_<uchar> img)
{
	Mat_<int> labels(img.rows, img.cols);

	for (int i = 0; i < img.rows; i++)
		for (int j = 0; j < img.cols; j++)
			labels(i, j) = 0;

	int label = 0;

	vector<vector<int>> edges;

	int dx8[] = { 0,0,1,1,1,-1,-1,-1 };
	int dy8[] = { 1,-1,0,1,-1,0,1,-1 };

	for (int j = 0; j < img.cols; j++)
		for (int i = 0; i < img.rows; i++)
			if (img(i, j) == 0 && labels(i, j) == 0)
			{
				vector<int> L;
				
				for (int k = 0; k < 8; k++)
					if (isInside(img, i + dx8[k], j + dy8[k]))
					{
						if (labels(i + dx8[k], j + dy8[k]) > 0)
							L.push_back(labels(i + dx8[k], j + dy8[k]));
					}
				

				if (L.empty())
				{
					label++;
					edges.resize(label + 1);
					labels(i, j) = label;
				}
				else
				{
					int x = L[0];
					for (int i = 0; i < L.size(); i++)
						if (x < L[i])
							x = L[i];
					labels(i, j) = x;

					for (int i = 0; i < L.size(); i++)
						if (L[i] != x)
						{
							edges[x].push_back(L[i]);
							edges[L[i]].push_back(x);
						}
				}
			}

	vector<int> newlabels(label + 1, 0);
	int newlabel = 0;

	for (int i = 1; i <= label; i++)
		if (newlabels[i] == 0)
		{
			newlabel++;
			queue<int> Q;
			newlabels[i] = newlabel;
			Q.push(i);
			while (!Q.empty())
			{
				int x = Q.front();
				Q.pop();

				for (int j = 0; j < edges[x].size(); j++)
					if (newlabels[edges[x][j]] == 0)
					{
						newlabels[edges[x][j]] = newlabel;
						Q.push(edges[x][j]);
					}

			}
		}

	for (int i = 0; i < img.rows; i++)
		for (int j = 0; j < img.cols; j++)
			labels(i, j) = newlabels[labels(i, j)];

	return labels;
}

vector<Label> project_extractlabels(Mat_<int> img)
{
	vector<Label> labels;
	for (int i = 0; i < img.rows; i++)
		for (int j = 0; j < img.cols; j++)
		{
			if (img(i, j) != 0)
			{
				bool found = false; 
				for (int k = 0; k < labels.size(); k++)
					if (labels[k].label == img(i, j))
					{
						found = true;
						break;
					}
				if (!found)
				{
					Label l;
					l.label = img(i, j);
					l.x = i;
					l.y = j;
					labels.push_back(l);

				}
			}
		}
	return labels;
}

void project_sortlabels(vector<Label>& labels)
{
	for (int i = 0; i < labels.size() - 1; i++)
		for (int j = i + 1; j < labels.size(); j++)
			if (labels[i].y > labels[j].y)
			{
				Label aux = labels[i];
				labels[i] = labels[j];
				labels[j] = aux;
			}
}

bool project_isupsidedown(Mat_<uchar> note)
{
	int uphalf, downhalf;
	uphalf = downhalf = 0;

	for (int i = 0; i < note.rows / 2; i++)
		for (int j = 0; j < note.cols; j++)
			if (note(i, j) == 0)
				uphalf++;

	for (int i = note.rows / 2; i < note.rows; i++)
		for (int j = 0; j < note.cols; j++)
			if (note(i, j) == 0)
				downhalf++;

	return uphalf > downhalf;
}

bool project_hasflag_normal(Mat_<uchar> note)
{
	bool found = false;

	int x, y;

	for(int i=0;!found && i<note.rows;i++)
		for(int j=0;!found && j<note.cols;j++)
			if (note(i, j) == 0)
			{
				x=i;
				y=j;
				found = true;
			}

	while (y < note.cols && note(x, y) == 0)
	{
		y++;
	}

	y--;

	while (x < note.rows && note(x, y) == 0)
	{
		if (isInside(note, x, y + 1) && note(x, y + 1) == 0)
			return true;
		x++;
	}

	return false;
}


bool project_hasflag_upsidedown(Mat_<uchar> note)
{
	bool found = false;

	int x, y;

	for (int i = note.rows-1; !found && i >= 0; i--)
		for (int j = 0; !found && j < note.cols; j++)
			if (note(i, j) == 0)
			{
				x = i;
				y = j;
				found = true;
			}

	while (y < note.cols && note(x, y) == 0)
	{
		y++;
	}

	y--;

	while (x >= note.rows/2 && note(x, y) == 0)
	{
		if (isInside(note, x, y + 1) && note(x, y + 1) == 0)
			return true;
		x--;
	}

	return false;
}

bool project_iscircleempty(Mat_<uchar> note, int direction)
{
	int count = 0;
	int min;
	int max;

	if (direction == 1)
	{
		min = note.rows/2;
		max = note.rows;
	}
	else
	{
		min = 0;
		max = note.rows/2;
	}

	for (int i = min; i < max; i++)
	{
		bool first = false;
		bool second = false;

		for(int j=0;j<note.cols-1;j++)
			if (first)
			{
				if(note(i,j)==255 && note(i,j+1)==0)
					second = true;
			}
			else
			{
				if(note(i,j)==0 && note(i,j+1)==255)
					first = true;
			}

		if (first && second)
			count++;
	}

	return count > 10;
}

int project_recognizeduration(Mat_<uchar> note, bool ud)
{
	int area = project_area(note);
	int size = note.rows * note.cols;

	float ratio = (float)area / size;

	if (ratio < 0.9)
	{
		bool flag;
		bool circle;

		if (ud) 
		{
			flag = project_hasflag_upsidedown(note);
			circle = project_iscircleempty(note, 0);
		}
		else
		{
			flag = project_hasflag_normal(note);
			circle = project_iscircleempty(note, 1);
		}

		if (circle)
			return HALF;
		else
			if (flag)
				return EIGHTH;
			else
				return QUARTER;
	}
	
	return -1;
}

float project_averagedistance(vector<Stave> staves)
{
	float sum = 0;
	for(int i=0;i<staves.size()-1;i++)
		sum += staves[i+1].start - staves[i].end;

	return sum / (staves.size() - 1);
}

bool project_around(int value1, int value2, int threshold)
{
	int diff = abs(value1 - value2);
	return diff<=threshold;
}

int project_recognizefrequency(int max,vector<Stave> staves)
{
	float avg = project_averagedistance(staves);
	int mid_circle = max - avg / 2;

	int value_to_consider = max >= staves[staves.size() - 1].end ? mid_circle : max;

	int closest = 0;
	int closest_distance = abs(staves[0].end - value_to_consider);

	for (int i = 1; i < staves.size(); i++)
	{
		int dist = abs(staves[i].end - value_to_consider);
		if (dist <= closest_distance)
		{
			closest_distance = dist;
			closest = i;
		}
	}

	switch (closest)
	{
	case 4:
		if (project_around(closest_distance, 2 * avg, 2))
			return NOTE_A3;
		if (project_around(closest_distance, 1.5 * avg, 2))
			return NOTE_B3;
		else if (project_around(closest_distance, avg, 2))
			return NOTE_C4;
		else if (project_around(closest_distance, avg / 2, 2))
			return NOTE_D4;
		else if (project_around(closest_distance, 0, 2) && project_around(mid_circle, staves[staves.size() - 1].end, 2))
			return NOTE_E4;
		else if (project_around(closest_distance, 0, 2))
			return NOTE_F4;
		else return 0;
	case 3:
		if (project_around(closest_distance, 0, 2))
			return NOTE_A4;
		else if (project_around(closest_distance, avg / 2, 2))
			return NOTE_G4;
		else return 0;
	case 2:
		if (project_around(closest_distance, 0, 2))
			return NOTE_C5;
		else if (project_around(closest_distance, avg / 2, 2))
			return NOTE_B4;
		else return 0;
	case 1:
		if (project_around(closest_distance, 0, 2))
			return NOTE_E5;
		else if (project_around(closest_distance, avg / 2, 2))
			return NOTE_D5;
		else return 0;
	case 0:
		if (project_around(closest_distance, 0, 2))
			return NOTE_E5;
		else if (project_around(closest_distance, avg / 2, 2))
			return NOTE_D5;
		else return 0;
	default:
		return 0;
	}
}

pair<int,int> project_recognize(Mat_<uchar> note,int max, vector<Stave> staves)
{
	bool ud = project_isupsidedown(note);
	int duration = project_recognizeduration(note, ud);
	int frequency = project_recognizefrequency(max, staves);

	if (duration > 0)
		return pair<int, int>(duration, frequency);
	else return pair<int, int>(0, 0);
}

void project_playsong(vector<pair<int,int>> song)
{
	for (int i = 0; i < song.size(); i++)
	{
		if (song[i].first != 0 && song[i].second != 0)
		{
			float duration = 1.0 / song[i].first * 1000;
			int frequency = song[i].second;

			cout << duration << " " << frequency << endl;

			Beep(frequency, duration);
			Sleep(100);
		}
	}
}

void project_execute(char filename[])
{
	vector<pair<int, int>> song;

	char path[100];
	strcpy(path, filename);

	Mat_<uchar> original = imread(path, 0);

	imshow("Original", original);
	waitKey();

	original = project_threshold(original);
	

	Mat_<int> labels = project_label(original);
		
	vector<Label> lbls = project_extractlabels(labels);
	lab5_visualizecolors(labels, lbls.size());

	for (int i = 0; i < lbls.size(); i++)
	{
		int m, M;
		Mat_<uchar> current = project_cut(labels, lbls[i].label,m,M);
		imshow("Current", current);
		waitKey();


		vector<Stave> staves = project_getstaves(current);

		float avg = project_averagedistance(staves);

		Mat_<uchar> current_no_staves = project_deletestaves(current, staves);
		imshow("Current no staves", current_no_staves);
		waitKey();


		Mat_<int> current_labels = project_label(current_no_staves);
		vector<Label> current_lbls = project_extractlabels(current_labels);
		lab5_visualizecolors(current_labels, current_lbls.size());
		project_sortlabels(current_lbls);

		for (int j = 0; j < current_lbls.size(); j++)
		{
			int min, max;
			Mat_<uchar> note = project_cut(current_labels, current_lbls[j].label, min, max);

			if (note.rows * note.cols >= 100)
			{
				bool is_upside_down = project_isupsidedown(note);

				pair<int, int> value;

				if (is_upside_down)
					value = project_recognize(note, min, staves);
				else
					value = project_recognize(note, max, staves);

				song.push_back(value);

				char name[100];
				strcpy(name, "Note");
				strcat(name, to_string(i).c_str());
				strcat(name, to_string(j).c_str());
				imshow(name, note);
				waitKey();
			}
			
		}
	}

	project_playsong(song);
}

int main()
{
	project_execute("Mera_Mihai.png");

	return 0;
}
