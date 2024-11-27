# Sheet Music Reader - Transforming Sheet Music Images into Digital Audio

## Project Description

The project involves processing an image of sheet music and converting it into playable notes based on their pitch and duration. The general approach for this project includes:

1. **Automatic Binarization**: Using Otsu's method to binarize the image.
2. **Image Segmentation**: Detecting staff lines based on the horizontal histogram.
3. **Removal of Staff Lines**: Using morphological operations (dilation + erosion) to remove the staff lines for cleaner note processing.
4. **Stem Removal**: Identifying the pitch and duration of notes by removing the note stems.

### Similar Publications:
Authors: **Yuan-Hsiang Chang, Zhong-Xian Peng, Li-Der Jeng**. Their approach involves the following steps:

1. Convert the image into a binary image.
2. Apply horizontal projection (histogram).
3. Segment regions for independent processing.
4. Remove staff lines.
5. Apply morphological closing operation.
6. Label connected components to identify notes.
7. Remove stems.
8. Filter note sizes based on staff lines.
9. Detect note heads by calculating symmetry.
10. Classify notes based on their position relative to the staff and properties (solid, stem, number of tails).
11. Recognize accidentals (sharp, flat, and natural) and rests that modify the note.

### Image Processing Results:
1. **Binarization**
2. **Dilation**
3. **Erosion**
4. **Staff Lines and Stem Removal**
5. **Final Processed Image**

## Implementation Details

The implementation includes the following components:

### 1. **Rectangle Class**
This class holds information about the note, such as:
  - Height, width, and position (top-left corner).
  - Center coordinates (X, Y).
  - Whether the note has a stem.
  - Whether the note is solid or not.
  - Whether the note was found during the iteration over the image.

### 2. **Image Preprocessing**
The image is binarized, and staff lines are segmented to make the note processing smoother.

### 3. **Labeling Algorithm**
Used to label notes for easier recognition.

### 4. **Morphological Operations**
Dilation and erosion are applied to close the notes.

### 5. **Note Height Determination**
The height of the note is calculated based on the distance from its center to the nearest staff line.

### 6. **Note Type Detection**
The note's type (quarter, eighth, etc.) is determined based on its characteristics.

### 7. **Thinning Staff Lines**
Thicker staff lines are thinned to a single pixel to avoid errors in note height calculation.

### 8. **Note Duration Calculation**
The note's duration is determined based on its features.

### 9. **Sorting Notes**
Notes are sorted from right to left and top to bottom to ensure a correct order for playback.

### 10. **Note Feature Highlighting**
Features of the notes are highlighted on the image for better understanding.

## How to Use

1. **Development Environment**: The project is developed using **Visual Studio**.
2. **Image Input**: The project uses a pre-loaded image located in the project’s image folder.
3. **Running the Project**: Simply run the project within Visual Studio to process the sheet music image. The output will display the notes detected and processed, and they can be played back as audio.

## Conclusion

Extracting notes from sheet music was an interesting challenge, introducing us to the vast field of image processing. From simple visual representations on "paper" to audio representations, the project achieved good accuracy. Challenges included noise in the image and the level of detail in the notes, which sometimes interfered with predicting symbols.

## Future Improvements
- **Noise Reduction**: Implementing more advanced noise filtering techniques.
- **Complex Notation Support**: Expanding the system to handle more complex music notation (e.g., ties, triplets).
- **Real-Time Audio Playback**: Adding real-time playback and tempo adjustments.

## Dependencies
- **OpenCV**: Used for image processing tasks.
- **C++11** or later: The project uses modern C++ features.
- **Visual Studio**: Recommended IDE for development and debugging.

---

## License
[MIT License](https://opensource.org/licenses/MIT)
