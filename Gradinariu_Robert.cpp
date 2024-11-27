#include "stdafx.h"
#include "opencv2/opencv.hpp"
#include <opencv2/core/utils/logger.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <random>
#include <fstream>
#include <numeric> 
#include <vector>
#include <cmath>   
#include <windows.h>
#include <map>

using namespace cv;
using namespace std;


/// Structura ce retine informatii despre o nota muzicala
struct Note {
    Rect boundingBox;
    int name = -1;
    int centerX = -1;
    int centerY = -1;
    int position = -1;
    int stem = -1;
    int whole = -1;
    int duration = -1; 
    int found = 0;// Duration of the note (64 = whole, 32 = half, 16 = quarter, etc.)
};

/// frecventele notelor pentru functia Beep
map<int, int> noteFrequencies = {
    {1, 261}, // DO (C4)
    {2, 293}, // RE (D4)
    {3, 329}, // MI (E4)
    {4, 349}, // FA (F4)
    {5, 392}, // SOL (G4)
    {6, 440}, // LA (A4)
    {7, 493}  // SI (B4)
};

const int BASE_DURATION_MS = 1000;  // ex 1000 milliseconds for a whole note

/// functia ce da play la o nota
void playNote(int frequency, int duration) {
    Beep(frequency, duration);
}

/// functia de verificare pt pixeli
bool isInside(Mat img, int i, int j) {
    return i >= 0 && j >= 0 && i < img.rows&& j < img.cols;
}


Mat_<uchar> dilatare(Mat_<uchar> src, Mat_<uchar> elstr) {

    /// dst este destinatia - imaginea modificata
    /// src este sursa si nu se schimba
    /// 
    Mat_<uchar> dst(src.size());

    dst.setTo(255);

    for (int i = 0; i < src.rows; i++) {
        for (int j = 0; j < src.cols; j++) {
            if (src(i, j) == 0) {
                ///Dacă originea elementului structural coincide cu un punct “obiect” în imagine

                for (int u = 0; u < elstr.rows; u++) {
                    for (int v = 0; v < elstr.cols; v++) {
                        if (elstr(u, v) == 0) {
                            //imagine, atunci toți pixelii acoperiți de elementul structural devin pixeli “obiect” în imaginea rezultat.
                            int i2 = i + u - elstr.rows / 2;
                            int j2 = j + v - elstr.cols / 2;

                            if (isInside(src, i2, j2))
                                dst(i2, j2) = 0;
                            //dst(i2, j2) = 0;

                        }
                    }
                }
            }
        }
    }
    return dst;
}


Mat_<uchar> eroziune(Mat_<uchar> src, Mat_<uchar> elstr) {

    /// dst este destinatia - imaginea modificata
    /// src este sursa si nu se schimba
    /// 
    Mat_<uchar> dst(src.size());

    dst.setTo(255);

    for (int i = 0; i < src.rows; i++) {
        for (int j = 0; j < src.cols; j++) {
            if (src(i, j) == 0) {
                ///Dacă originea elementului structural coincide cu un punct “obiect” în imagine
                bool allinside = 1;
                for (int u = 0; u < elstr.rows; u++) {
                    for (int v = 0; v < elstr.cols; v++) {
                        if (elstr(u, v) == 0) {
                            int i2 = i + u - elstr.rows / 2;
                            int j2 = j + v - elstr.cols / 2;

                            if (isInside(src, i2, j2) && src(i2, j2) == 255)
                                allinside = 0;

                        }
                    }
                }
                if (allinside)
                    dst(i, j) = 0;
            }
        }
    }
    return dst;
}



vector<int> calcHist(Mat_<uchar> img) {
    vector<int> hist(256, 0);
    for (int i = 0; i < img.rows; i++)
        for (int j = 0; j < img.cols; j++)
            hist[img(i, j)]++;
    return hist;
}

Mat_<uchar> binarizareautomata(Mat_<uchar> img) {
    vector<int> hist = calcHist(img);
    Mat_<uchar> dst(img.rows, img.cols);
    int totalPixels = img.rows * img.cols;
    int sum = 0, sumB = 0;
    int wB = 0, wF = 0;
    float mB, mF, maxVar = 0;
    int threshold = 0;

    for (int i = 0; i < 256; i++) sum += i * hist[i];
    for (int i = 0; i < 256; i++) {
        wB += hist[i];
        if (wB == 0) continue;
        wF = totalPixels - wB;
        if (wF == 0) break;
        sumB += i * hist[i];
        mB = sumB / wB;
        mF = (sum - sumB) / wF;
        float varBetween = (float)wB * (float)wF * (mB - mF) * (mB - mF);
        if (varBetween > maxVar) {
            maxVar = varBetween;
            threshold = i;
        }
    }
    for (int i = 0; i < img.rows; i++)
        for (int j = 0; j < img.cols; j++)
            dst(i, j) = (img(i, j) >= threshold) ? 255 : 0;
    return dst;
}


vector<int> mergeCloseLines(const vector<int>& lines, int lineThickness) {
    vector<int> mergedLines;
    int prevLine = -lineThickness * 2;
    for (int line : lines) {
        if (line - prevLine > lineThickness) {
            mergedLines.push_back(line);
            prevLine = line;
        }
    }
    return mergedLines;
}

vector<int> calcHorizontalHist(Mat_<uchar> img) {
    vector<int> horizontalHist(img.rows, 0);
    for (int i = 0; i < img.rows; i++)
        for (int j = 0; j < img.cols; j++)
            if (img(i, j) == 0)
                horizontalHist[i]++;
    return horizontalHist;
}

vector<int> calcVerticalHist(Mat_<uchar> img) {
    vector<int> verticalHist(img.cols, 0);

    for (int j = 0; j < img.cols; j++) {
        for (int i = 0; i < img.rows; i++) {
            if (img(i, j) == 0) {
                verticalHist[j]++;
            }
        }
    }
    return verticalHist;
}

vector<int> detectStaffLines(const vector<int>& hist, int threshold) {
    vector<int> staffLines;
    for (int i = 0; i < hist.size(); i++) {
        if (hist[i] > threshold) {
            staffLines.push_back(i);
        }
    }
    return staffLines;
}

Mat_<uchar> removeStaffLines(Mat_<uchar> img, const vector<int>& horizontalHist, int threshold) {
    Mat_<uchar> imgWithoutStaff = img.clone();

    for (int i = 0; i < img.rows; i++) {
        if (horizontalHist[i] > threshold) {
            imgWithoutStaff.row(i).setTo(255);
        }
    }

    return imgWithoutStaff;
}


void bfsLabeling(Mat_<uchar> img, int thresholdHeight, int thresholdWidth, vector<Note>& notes) {
    Mat_<int> labels(img.rows, img.cols, int(0));
    int label = 0;
    int di[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
    int dj[] = { -1, 0, 1, -1, 1, -1, 0, 1 };

    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            if (img(i, j) == 0 && labels(i, j) == 0) {
                queue<pair<int, int>> Q;
                label++;
                Q.push({ i, j });
                labels(i, j) = label;

                int xmin = img.rows, xmax = 0, ymin = img.cols, ymax = 0;

                while (!Q.empty()) {
                    pair<int, int> q = Q.front();
                    Q.pop();
                    for (int k = 0; k < 8; k++) {
                        int i2 = q.first + di[k];
                        int j2 = q.second + dj[k];
                        if (isInside(img, i2, j2) && img(i2, j2) == 0 && labels(i2, j2) == 0) {
                            labels(i2, j2) = label;
                            Q.push({ i2, j2 });
                            xmin = min(xmin, i2);
                            xmax = max(xmax, i2);
                            ymin = min(ymin, j2);
                            ymax = max(ymax, j2);
                        }
                    }
                }

                int height = xmax - xmin + 2;
                int width = ymax - ymin + 2;

                if (height <= thresholdHeight && width <= thresholdWidth && height >= 3 && width >= 8 && height <= 20) {
                    Note note;
                    note.boundingBox = Rect(ymin, xmin, width, height);
                    notes.push_back(note);
                }
            }
        }
    }
}

int getNoteName(Mat_<uchar> img, vector<int> staff, int LineSpacing, Note& note) {

    // DO - 1  RE - 2  MI - 3  FA - 4 SOL - 5 LA - 6  SI -  7 


    vector<int> notesOnLine = {
             4,   2,   7,   5,   3
             //"FA","RE","SI","SOL","MI"
    };

    int tolerance = 2; /// error for center of musical note
    int noteCenter = note.centerY;



         /// verificarea notelor pe linii
    for (int i = 0; i < staff.size(); i++) {
        for (int t = -tolerance; t <= tolerance; t++) {
            if (noteCenter == staff[i] + t) {
                return notesOnLine[i];
            }
        }
    }

    /// Verificarea notelor intre linii

    if (noteCenter > staff[0] && noteCenter < staff[1]) {
        return 3;   ///"MI"
    }
    else if (noteCenter > staff[1] && noteCenter < staff[2]) {
        return 1; /// "DO";
    }
    else if (noteCenter > staff[2] && noteCenter < staff[3]) {
        return 6; /// "LA";
    }
    else if (noteCenter > staff[3] && noteCenter < staff[4]) {
        return 4;  /// "FA";
    }

    /// note de deasupra
    if (noteCenter<staff[0] && noteCenter>staff[0] - LineSpacing) {
        return  5; /// "SOL";
    }
    if (noteCenter < staff[0] - LineSpacing && noteCenter < staff[0] - 2 * LineSpacing) {
        return  7; ///   "SI";
    }

    for (int t = -tolerance; t <= tolerance; t++) {
        if (noteCenter == staff[0] + t - LineSpacing) {
            return 6;   ///"LA";
        }
        if (noteCenter == staff[0] + t - 2 * LineSpacing) {
            return 1;  /// "DO";
        }
    }

    /// note dedesubt


    if (noteCenter > staff[4] && noteCenter < staff[4] + LineSpacing) {
        return 2;  ///"RE";
    }
    for (int t = -tolerance; t <= tolerance; t++) {
        if (noteCenter == staff[4] + t + LineSpacing) {
            return 1;  /// "DO";
        }
    }
    // }
    return 0;
}

float calculateAverageLineSpacing(const vector<int>& StaffLines) {
    int totalSpacing = 0;
    int numSpaces = StaffLines.size() - 1;

    for (int i = 1; i < StaffLines.size(); i++) {
        int spacing = StaffLines[i] - StaffLines[i - 1];
        if (spacing <= 20) {
            totalSpacing += spacing;
        }
        else {
            numSpaces--;
        }
    }

    float averageSpacing = static_cast<float>(totalSpacing) / numSpaces;
    return averageSpacing;
}



int isWhole(Mat_<uchar>& img, Note& note) {
    Rect bbox = note.boundingBox;

    int countBlackPixels = 0;
    int countWhitePixels = 0;

    for (int y = bbox.y; y < bbox.y + bbox.height; ++y) {
        for (int x = bbox.x; x < bbox.x + bbox.width; ++x) {
            if (isInside(img, x, y)) {
                if (img(y, x) == 255) {
                    countWhitePixels++;
                }
                else {
                    countBlackPixels++;
                }
            }
        }
    }
    if (countWhitePixels * 2 > countBlackPixels) {
        return 1; // (whole note)
    }

    return 0;
}


int hasStem(Mat_<uchar> originalImage, Note& note) {
    Rect bbox = note.boundingBox;
    int minStemLength = 10; 
    int extension = 10; // Extinderea bounding box-ului în sus

    // Extinderea bounding box-ului în sus
    int extendedTop =  bbox.y - extension;
    int extendedRight =  bbox.x + bbox.width + extension;

    // Verificăm în sus și la dreapta
    for (int j = bbox.x; j < extendedRight; j++) {
        int count = 0;
        for (int i = extendedTop; i < bbox.y + bbox.height; i++) {
            if (isInside(originalImage, i, j) && originalImage(i, j) == 0) {
                count++;
            }
            else {
                if (count >= minStemLength) {
                    return 1;
                }
                count = 0;
            }
        }
        if (count >= minStemLength) {
            return 1;
        }
    }
    return 0;
}

void detectValue(Mat_<uchar> img, Note& note) {

    if (hasStem(img, note) == 1) {
        note.stem = 1;
    }
    else {
        note.stem = 0;
    }

}

int computeDuration(Note& note) {
    if (note.whole == 1 && note.stem == 0) {   ///  pure whole note
        return 64;
    }
    else if (note.whole == 1 && note.stem == 1) {   ///  half note
        return  32;
    }
    else if (note.whole == 0 && note.stem == 1) {   /// quarter note 
        return  16;
    }
    else if (note.stem == 2) {
        return 8;
    }
        return -1;

}

void play(Note note) {
    int frequency = noteFrequencies[note.name];
    int duration;

    if (note.duration != -1) {
        duration = (note.duration * BASE_DURATION_MS) / 64;
    }
    else {
        duration = (64 * BASE_DURATION_MS) / 64;   

    }

    // Play the note
    playNote(frequency, duration);
}

/// de sus in jos
bool compareByCenterY(Note& a, Note& b) {
    return a.centerY < b.centerY;
}
/// de la stanga la dreapta
bool compareByCenterX(const Note& a, const Note& b) {
    return a.centerX < b.centerX;
}

void sortNotes(vector<Note>& notes) {
    // Sort notes by centerX (left to right)
    sort(notes.begin(), notes.end(), compareByCenterX);

    int start = 0;
    while (start < notes.size()) {
        int end = start + 1;
        while (end < notes.size() && notes[end].centerX - notes[start].centerX < 20) {
            end++;
        }
        sort(notes.begin() + start, notes.begin() + end, compareByCenterY);
        start = end;
    }


}

Mat drawNotes(Mat_<uchar> img, vector<Note>& notes, vector<int>& hist, int threshold) {


    Mat colorImage;
    cvtColor(img, colorImage, COLOR_GRAY2BGR);

    // Detect staff lines
    vector<int> StaffLines = detectStaffLines(hist, threshold);

    // Merge close lines to handle thicker lines
    StaffLines = mergeCloseLines(StaffLines, 5); 

    float LineSpacing = calculateAverageLineSpacing(StaffLines);


    // Find staves (groups of 5 consecutive lines with relatively equal spacing)
    vector<vector<int>> staves;
    vector<int> currentStaff;

    for (size_t i = 0; i < StaffLines.size(); ++i) {
        if (currentStaff.empty() || (StaffLines[i] - currentStaff.back() < 20)) {
            currentStaff.push_back(StaffLines[i]);
            if (currentStaff.size() == 5) {
                staves.push_back(currentStaff);
                currentStaff.clear();
            }
        }
        else {
            currentStaff.clear();
            currentStaff.push_back(StaffLines[i]);
        }
    }


    for (int i = 0; i < notes.size(); ++i) {

        int noteCenterY = notes[i].boundingBox.y + notes[i].boundingBox.height / 2;
        int noteCenterX = notes[i].boundingBox.x + notes[i].boundingBox.width / 2;
        notes[i].centerY = noteCenterY;
        notes[i].centerX = noteCenterX;
    }

    sortNotes(notes);

    for (int idx = 0; idx < staves.size(); ++idx) {

        const auto& staff = staves[idx];


        for (int i = 0; i < notes.size(); ++i) {

            if (notes[i].found == 0) {

                int noteName = getNoteName(img, staff, round(LineSpacing), notes[i]);
                notes[i].name = noteName;

                if (notes[i].centerY > staff.front() - 20 && notes[i].centerY < staff.back() + 20 && notes[i].boundingBox.x > 50 && notes[i].boundingBox.height <= notes[i].boundingBox.width) {

                    notes[i].found = 1;
                    rectangle(colorImage, notes[i].boundingBox, Scalar(0, 0, 255), 1);  
                    // putText(img, to_string(abs(distanceFromFirstLine)), Point(note.boundingBox.x, note.boundingBox.y - 10), FONT_HERSHEY_SIMPLEX, 0.35, Scalar(0, 0, 255), 1); // Display the distance above the note

                    detectValue(img, notes[i]);

                    notes[i].name = getNoteName(img, staff, round(LineSpacing), notes[i]);

                    int x = isWhole(img, notes[i]);
                    notes[i].whole = x;

                    notes[i].duration = computeDuration(notes[i]);



                    string durationLabel;
                    switch (notes[i].duration) {
                    case 64: durationLabel = "WH"; break;
                    case 32: durationLabel = "HLF"; break;
                    case 16: durationLabel = "Q"; break;
                    case 8: durationLabel = "S"; break;
                    default: durationLabel = "?"; break;
                    }
                    putText(colorImage, durationLabel, Point(notes[i].boundingBox.x, notes[i].boundingBox.y - 10),
                        FONT_HERSHEY_SIMPLEX, 0.35, Scalar(0, 0, 255), 1);

                    string noteLabel;
                    switch (notes[i].name) {
                    case 1: noteLabel = "DO"; break;
                    case 2: noteLabel = "RE"; break;
                    case 3: noteLabel = "MI"; break;
                    case 4: noteLabel = "FA"; break;
                    case 5: noteLabel = "SOL"; break;
                    case 6: noteLabel = "LA"; break;
                    case 7: noteLabel = "SI"; break;
                    default: noteLabel = "?"; break;
                    }
                    putText(colorImage, noteLabel, Point(notes[i].boundingBox.x, notes[i].boundingBox.y + 18),
                        FONT_HERSHEY_SIMPLEX, 0.35, Scalar(255, 0, 0), 1);

                    play(notes[i]);
                }

            }
        }
    }

    return colorImage;
}

Mat_<uchar> removeNoteStems(Mat_<uchar> img) {
    Mat_<uchar> imgWithoutStems = img.clone();
    int minStemLength = 15;

    for (int col = 0; col < img.cols; col++) {
        int count = 0;

        for (int row = 0; row < img.rows; row++) {
            if (img(row, col) == 0) {
                count++;
            }
            else {
                if (count >= minStemLength) {
                    for (int k = row - count; k < row; k++) {
                        imgWithoutStems(k, col) = 255;
                    }
                }
                count = 0;
            }
        }

        // verifica ultimul segmnent din coloana
        if (count >= minStemLength) {
            for (int k = img.rows - count; k < img.rows; k++) {
                imgWithoutStems(k, col) = 255;
            }
        }
    }
    return imgWithoutStems;
}


void drawDetectedLines(Mat_<uchar> img, const vector<int>& lines) {
    for (int line : lines) {
        cv::line(img, Point(0, line), Point(img.cols, line), Scalar(0, 0, 0), 1);
    }
}


int main() {
    Mat_<uchar> img = imread("./a_poze_PI_proiect/22.PNG", 0);
    ///Let-it-Go-_easy-piano_.bmp
    /// Partiture2.jpg   v
    ///download.bmp   v
    /// Itsy Bitsy Spider - Free Piano Sheet Music   v
    /// This-Old-man-Free-Piano-Sheet-Music  v
    /// partiture.PNG
    ///
    /// 

    imshow("Imagine initiala", img);

    Mat_<uchar> binaryImage = binarizareautomata(img);
    imshow("Imagine binarizata", binaryImage);


    vector<int> horizontalHist = calcHorizontalHist(binaryImage);
    vector<int> staffLines = detectStaffLines(horizontalHist, 400);
    vector<int> mergedStaffLines = mergeCloseLines(staffLines, 2);

    int threshold = 120;
    Mat_<uchar> WithoutLines = removeStaffLines(binaryImage, horizontalHist, threshold);
    imshow("Imagine fara linii de portativ", WithoutLines);



    Mat_<uchar> elStructural(3, 3);
    for (int i = 0; i < elStructural.rows; ++i) {
        for (int j = 0; j < elStructural.cols; ++j) {
            elStructural(i, j) = 0;
        }
    }


    Mat_<uchar> ImgDilatata = dilatare(WithoutLines, elStructural);
    imshow("Imagine dilatata", ImgDilatata);

    Mat_<uchar> ImgErodata = eroziune(ImgDilatata, elStructural);
    imshow("Imagine erodata", ImgErodata);

    Mat_<uchar> ImgWithoutStems = removeNoteStems(ImgErodata);
    imshow("Imagine fara bastonasele notelor", ImgWithoutStems);


    vector<Note> notes;
    bfsLabeling(ImgWithoutStems, 50, 50, notes);

    vector<int> StaffLines = detectStaffLines(horizontalHist, threshold);


    drawDetectedLines(ImgErodata, mergedStaffLines);

    Mat colorImage = drawNotes(ImgErodata, notes, horizontalHist, 400);

    imshow("Imaginea finala procesata", colorImage);



    waitKey(0);
    return 0;
}
