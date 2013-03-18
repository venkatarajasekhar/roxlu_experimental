/*
 
 Basic BMFFont parser.
 ---------------------

 - use the BMFFont application to generate a font map + XML output. 
   http://www.angelcode.com/products/bmfont/

 - supports only one page font textures (make the size of the texture big enough)

 - supports a custom callback which is called when we generate triangles. You can use
   this callback to change the color of e.g. character

 - The `BMFFont` class is a simple wrapper which ties the `BMFFontLoader` and `BMFFontRenderer` 
   together. You can use the `BMFFontLoader` separately if you want ot handly the vertices 
   in a different way. The `BMFFontRenderer` uses an optimized openGL render path using 
   multidraw arrays and a stream draw VBO (could be more optimized by mapping the VBO 
   memory to pinned space).


 - This class is to be used in a way where you "add" texts vertices to a buffer. You can 
   either add text once (when it's static), or update it for every frame. If you text changes
   you need to add it again and make sure to call `BMFFont::reset()`.


*/


#ifndef ROXLU_BMFONT_H
#define ROXLU_BMFONT_H

#include <vector>
#include <string>
#include <bmfont/BMFLoader.h>
#include <bmfont/BMFRenderer.h>

template<class T = BMFVertex>
class BMFFont {
 public:
  BMFFont();
  ~BMFFont();
  bool setup(std::string filename, int windowW, int windowH, bool datapath = false);     
  void setColor(float r, float g, float b, float a = 1.0);
  size_t addText(std::string str, float x, float y);                                    /* add a new text and return indices to the offsets and vertex counts of the BMFRenderer, see BMFRenderer for more info */
  void update();                                                                        /* only call update when you're using the drawText() function. draw() will call update for you. Only call update() once after changing your texts */
  void draw();
  void bind();                                                                          /* bind the GL objects used for drawing; only call this manually when you're using `drawText()`, when  you use only `draw()` don't call this. Only call bind() once per frame. */
  void drawText(size_t index);                                                          /* only draw a specific text entry */
  void drawText(size_t index, const float* modelMatrix);                                 /* only draw a specific text entry + with the given model matrix */
  void reset();
  void print();
  void getStringSize(std::string str, int& w, int& h);                                   /* get the width and max height for the given string */

 public:
  BMFLoader<T> loader;
  BMFRenderer<T> renderer;
};

template<class T>
BMFFont<T>::BMFFont()
  :renderer(loader)
{
}

template<class T>
BMFFont<T>::~BMFFont() {
}

template<class T>
inline bool BMFFont<T>::setup(std::string filename, int windowW, int windowH, bool datapath) {
  bool r = true;
  r = loader.load(filename, datapath);
  renderer.setup(windowW, windowH);
  return r;
}

template<class T>
inline void BMFFont<T>::setColor(float r, float g, float b, float a) {
  loader.setColor(r, g, b, a);
}

template<class T>
inline size_t BMFFont<T>::addText(std::string str, float x, float y) {
  std::vector<T> vertices = loader.generateVertices(str, x, y);
  return renderer.addVertices(vertices);
}

template<class T>
inline void BMFFont<T>::update() {
  renderer.update();
}

template<class T>
inline void BMFFont<T>::draw() {
  renderer.update();
  renderer.draw();
}

template<class T>
inline void BMFFont<T>::drawText(size_t text) {
  renderer.drawText(text);
}

template<class T>
inline void BMFFont<T>::drawText(size_t text, const float* mm) {
  renderer.drawText(text, mm);
}

template<class T>
inline void BMFFont<T>::print() {
  loader.print();
}

template<class T>
inline void BMFFont<T>::reset() {
  renderer.reset();
}

template<class T>
inline void BMFFont<T>::bind() {
  renderer.bind();
}

template<class T>
inline void BMFFont<T>::getStringSize(std::string str, int& w, int& h) {
  loader.getStringSize(str, w, h);
}


#endif