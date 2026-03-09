#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "controller.h"

// Declare external global variable for block metadata
extern std::vector<int> g_blockMetadata;

using namespace std;
namespace fs = std::filesystem;

/**
 * Display an image using WxWidgets.
 * https://www.wxwidgets.org/
 */

/** Declarations*/

/**
 * Class that implements wxApp
 */
class MyApp : public wxApp {
 public:
  bool OnInit() override;
};

/**
 * Class that implements wxFrame.
 * This frame serves as the top level window for the program
 */
class MyFrame : public wxFrame {
 public:
  MyFrame(const wxString &title, unsigned char * pixelData, int w, int h);

 private:
  void OnPaint(wxPaintEvent &event);
  wxImage inImage;
  wxScrolledWindow *scrolledWindow;
  int width;
  int height;
  bool showBlocks;
};

/** Utility function to read image data */
unsigned char *readImageData(string imagePath, int width, int height);

/** Definitions */

/**
 * Init method for the app.
 * Here we process the command line arguments and
 * instantiate the frame.
 */
bool MyApp::OnInit() {
  wxInitAllImageHandlers();

  // deal with command line arguments here
  cout << "Number of command line arguments: " << wxApp::argc << endl;
  if (wxApp::argc != 5) {
    cerr << "The executable should be invoked with exactly one filepath "
            "argument. Example ./MyImageApplication '../../Lena_512_512.rgb'"
         << endl;
    exit(1);
  }

  string imagePath = wxApp::argv[1].ToStdString();
  int M = stoi(wxApp::argv[2].ToStdString());
  int Q = stoi(wxApp::argv[3].ToStdString());
  double B = stod(wxApp::argv[4].ToStdString());

  unsigned char *data = nullptr;
  int width = 512, height = 512;

  data = adaptivePipeline(imagePath, width, height, M, Q, B);
  if (data == nullptr) {
    cerr << "Controller failed to return image data\n";
    exit(1);
  }

  MyFrame *frame = new MyFrame("Image Display", data, width, height);
  frame->Show(true);

  // return true to continue, false to exit the application
  return true;
}

/**
 * Constructor for the frame.
 * Here we set up the scrollable window and bind the paint event handler.
 * We also bind a key event handler to toggle block boundaries when 'B' is pressed.
 */
MyFrame::MyFrame(const wxString &title, unsigned char * pixelData, int w, int h)
  : wxFrame(NULL, wxID_ANY, title), width(w), height(h), showBlocks(false) {
    inImage.SetData(pixelData, width, height, false);

    scrolledWindow = new wxScrolledWindow(this, wxID_ANY);
    scrolledWindow->SetScrollbars(10, 10, width, height);
    scrolledWindow->SetVirtualSize(width, height);
    scrolledWindow->Bind(wxEVT_PAINT, &MyFrame::OnPaint, this);
    SetClientSize(width, height);
    SetBackgroundColour(*wxBLACK);

    Bind(wxEVT_CHAR_HOOK, [this](wxKeyEvent &evt){
      int key = evt.GetKeyCode();
      if (key == 'B' || key == 'b') {
        this->showBlocks = !this->showBlocks;
        this->Refresh();
      }
      evt.Skip();
    });
  }
/**
 * The OnPaint handler that paints the UI.
 * Here we paint the image pixels into the scrollable window.
 */
void MyFrame::OnPaint(wxPaintEvent &event) {
  wxBufferedPaintDC dc(scrolledWindow);
  scrolledWindow->DoPrepareDC(dc);

  wxBitmap inImageBitmap = wxBitmap(inImage);
  dc.DrawBitmap(inImageBitmap, 0, 0, false);

  // Draw block boundaries if showBlocks is enabled
  if (showBlocks && !g_blockMetadata.empty()) {
    dc.SetPen(wxPen(*wxGREEN, 2));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);

    // g_blockMetadata contains triplets: x0, y0, N for each block
    for (size_t i = 0; i + 2 < g_blockMetadata.size(); i += 3) {
      int x0 = g_blockMetadata[i];
      int y0 = g_blockMetadata[i + 1];
      int N = g_blockMetadata[i + 2];

      // Draw rectangle for this block
      dc.DrawRectangle(x0, y0, N, N);
    }
  }
}

/** Utility function to read image data */
unsigned char *readImageData(string imagePath, int width, int height) {

  // Open the file in binary mode
  ifstream inputFile(imagePath, ios::binary);

  if (!inputFile.is_open()) {
    cerr << "Error Opening File for Reading" << endl;
    exit(1);
  }

  // Create and populate RGB buffers
  vector<char> Rbuf(width * height);
  vector<char> Gbuf(width * height);
  vector<char> Bbuf(width * height);

  /**
   * The input RGB file is formatted as RRRR.....GGGG....BBBB.
   * i.e the R values of all the pixels followed by the G values
   * of all the pixels followed by the B values of all pixels.
   * Hence we read the data in that order.
   */

  inputFile.read(Rbuf.data(), width * height);
  inputFile.read(Gbuf.data(), width * height);
  inputFile.read(Bbuf.data(), width * height);

  inputFile.close();

  /**
   * Allocate a buffer to store the pixel values
   * The data must be allocated with malloc(), NOT with operator new. wxWidgets
   * library requires this.
   */
  unsigned char *inData =
      (unsigned char *)malloc(width * height * 3 * sizeof(unsigned char));
      
  for (int i = 0; i < height * width; i++) {
    // We populate RGB values of each pixel in that order
    // RGB.RGB.RGB and so on for all pixels
    inData[3 * i] = Rbuf[i];
    inData[3 * i + 1] = Gbuf[i];
    inData[3 * i + 2] = Bbuf[i];
  }

  return inData;
}

wxIMPLEMENT_APP(MyApp);