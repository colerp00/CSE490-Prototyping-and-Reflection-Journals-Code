#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Shape.hpp>

/**
 * Custom triangle class that extends shape
 * 
 * positive _dir means right, negative means left
 */
class Triangle : public Shape {
  protected:
    int _dir;
  
  public:
    Triangle(int x, int y, int width, int height, int dir) : Shape(x, y, width, height), _dir(dir)
    {}

    /**
     * @brief Draw the triangle
     * 
     * @param display 
     */
    void draw (const Adafruit_SSD1306& display) override{
      // Draw triangle takes in (xTip, yTip, xCorner1, yCorner1, xCorner2, yCorner2, color)
      if(_drawFill){
        display.fillTriangle(_x, _y,
                             _x - _width, _y + (_height / 2),
                             _x - _width, _y - (_height / 2),
                             SSD1306_WHITE);
      }else{
        display.drawTriangle(_x, _y,
                             _x - _width, _y + (_height / 2),
                             _x - _width, _y - (_height / 2),
                             SSD1306_WHITE);
      }
    }

    int getLeft() {
      if(_dir == 1) {
        return _x - _width;
      } else {
        return _x
      }
    }

    int getRight() {
      if(_dir == -1) {
        return _x + _width;
      } else {
        return _x
      }
    }

    int getBottom() {
      return _y - (_height / 2);
    }

    int getTop() {
      return _y + (_height / 2);
    }

    String getName() override{
      return "Triangle";
    }
};

/**
 * Custom player ship class
 */
class PlayerShip : public Triangle {
  public:
    Triangle(int x, int y, int width, int height) : Shape(x, y, width, height), _dir(1)
    {}

    String getName() override{
      return "PlayerShip";
    }
};

/**
 * Custom enemy ship class
 */
class EnemyShip : public Triangle {
  public:
    Triangle(int x, int y, int width, int height) : Shape(x, y, width, height), _dir(-1)
    {}

    String getName() override{
      return "EnemyShip";
    }
};