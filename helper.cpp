/*
 * helper.cpp
 *
 * Helper code to read TGA files.
 * Adapted from OpenGL superbible 5th edition
 *
 * CPSC 453: Introduction to Computer Graphics
 */

#include "helper.h"


////////////////////////////////////////////////////////////////////
// Allocate memory and load targa bits. Returns pointer to new buffer,
// height, and width of texture, and the OpenGL format of data.
// Call free() on buffer when finished!
// This only works on pretty vanilla targas... 8, 24, or 32 bit color
// only, no palettes, no RLE encoding.
// This function also takes an optional final parameter to preallocated 
// storage for loading in the image data.
GLbyte *ReadTGABits(const char *szFileName, GLint *iWidth, GLint *iHeight, GLint *iComponents, GLenum *eFormat, GLbyte *pData)
{
  FILE *pFile;			// File pointer
  TGAHEADER tgaHeader;		// TGA file header
  unsigned long lImageSize;		// Size in bytes of image
  short sDepth;			// Pixel depth;
  GLbyte	*pBits = NULL;          // Pointer to bits
  
  // Default/Failed values
  *iWidth = 0;
  *iHeight = 0;
  *eFormat = GL_RGB;
  *iComponents = GL_RGB;
  
  // Attempt to open the file
  pFile = fopen(szFileName, "rb");
  if(pFile == NULL)
    return NULL;
	
  // Read in header (binary)
  fread(&tgaHeader, 18/* sizeof(TGAHEADER)*/, 1, pFile);
  
  // Get width, height, and depth of texture
  *iWidth = tgaHeader.width;
  *iHeight = tgaHeader.height;
  sDepth = tgaHeader.bits / 8;
  
  // Put some validity checks here. Very simply, I only understand
  // or care about 8, 24, or 32 bit targa's.
  if(tgaHeader.bits != 8 && tgaHeader.bits != 24 && tgaHeader.bits != 32)
    return NULL;
	
  // Calculate size of image buffer
  lImageSize = tgaHeader.width * tgaHeader.height * sDepth;
  
  // Allocate memory and check for success
  if(pData == NULL) 
    pBits = (GLbyte*)malloc(lImageSize * sizeof(GLbyte));
  else 
    pBits = pData; 
  
  // Read in the bits
  // Check for read error. This should catch RLE or other 
  // weird formats that I don't want to recognize
  if(fread(pBits, lImageSize, 1, pFile) != 1)
		{
      if(pBits != NULL)
        free(pBits);
      return NULL;
		}
  
  // Set OpenGL format expected
  switch(sDepth)
		{
    case 4:
      *eFormat = GL_BGRA;
      *iComponents = GL_RGBA;
      break;
    case 1:
      *eFormat = GL_LUMINANCE;
      *iComponents = GL_LUMINANCE;
      break;
    default:
      *eFormat = GL_BGR;    
          // RGB
      // If on the iPhone, TGA's are BGR, and the iPhone does not 
      // support BGR without alpha, but it does support RGB,
      // so a simple swizzle of the red and blue bytes will suffice.
      // For faster iPhone loads however, save your TGA's with an Alpha!
      
      break;
		}
	
  // Done with File
  fclose(pFile);
	
  // Return pointer to image data
  return pBits;
}
