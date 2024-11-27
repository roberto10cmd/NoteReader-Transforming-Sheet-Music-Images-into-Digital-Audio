# Sheet Music Reader - Transforming Sheet Music Images into Digital Audio
Project Description
The project involves processing an image of sheet music and converting it into playable notes based on their pitch and duration. The general approach for this project includes:

Automatic Binarization: Using Otsu's method to binarize the image.
Image Segmentation: Detecting staff lines based on the horizontal histogram.
Removal of Staff Lines: Using morphological operations (dilation + erosion) to remove the staff lines for cleaner note processing.
Stem Removal: Identifying the pitch and duration of notes by removing the note stems.
Similar Publications:
Authors: Yuan-Hsiang Chang, Zhong-Xian Peng, Li-Der Jeng. Their approach involves the following steps:

Convert the image into a binary image.
Apply horizontal projection (histogram).
Segment regions for independent processing.
Remove staff lines.
Apply morphological closing operation.
Label connected components to identify notes.
Remove stems.
Filter note sizes based on staff lines.
Detect note heads by calculating symmetry.
Classify notes based on their position relative to the staff and properties (solid, stem, number of tails).
Recognize accidentals (sharp, flat, and natural) and rests that modify the note.
Image Processing Results:
Binarization
Dilation
Erosion
Staff Lines and Stem Removal
Final Processed Image
Implementation Details
The implementation includes the following components:

Rectangle Class: This class holds information about the note, such as:

Height, width, and position (top-left corner).
Center coordinates (X, Y).
Whether the note has a stem.
Whether the note is solid or not.
Whether the note was found during the iteration over the image.
Image Preprocessing: The image is binarized, and staff lines are segmented to make the note processing smoother.

Labeling Algorithm: Used to label notes for easier recognition.

Morphological Operations: Dilation and erosion are applied to close the notes.

Note Height Determination: The height of the note is calculated based on the distance from its center to the nearest staff line.

Note Type Detection: The note's type (quarter, eighth, etc.) is determined based on its characteristics.

Thinning Staff Lines: Thicker staff lines are thinned to a single pixel to avoid errors in note height calculation.

Note Duration Calculation: The note's duration is determined based on its features.

Sorting Notes: Notes are sorted from right to left and top to bottom to ensure a correct order for playback.

Note Feature Highlighting: Features of the notes are highlighted on the image for better understanding.

How to Use
Development Environment: The project is developed using Visual Studio.
Image Input: The project uses a pre-loaded image located in the project’s image folder.
Running the Project: Simply run the project within Visual Studio to process the sheet music image. The output will display the notes detected and processed, and they can be played back as audio.
Conclusion
Extracting notes from sheet music was an interesting challenge, introducing us to the vast field of image processing. From simple visual representations on "paper" to audio representations, the project achieved good accuracy. Challenges included noise in the image and the level of detail in the notes, which sometimes interfered with predicting symbols.

Future Improvements
Noise Reduction: Implementing more advanced noise filtering techniques.
Complex Notation Support: Expanding the system to handle more complex music notation (e.g., ties, triplets).
Real-Time Audio Playback: Adding real-time playback and tempo adjustments.
Dependencies
OpenCV: Used for image processing tasks.
C++11 or later: The project uses modern C++ features.
Visual Studio: Recommended IDE for development and debugging.
