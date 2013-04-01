//! \file Image.hpp
//! Interface of the Image class and helper functions.

#ifndef GOSU_IMAGE_HPP
#define GOSU_IMAGE_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/Color.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <Gosu/TR1.hpp>
#include <Gosu/Platform.hpp>
#include <memory>
#include <vector>
#if defined(GOSU_CPP11_ENABLED)
#include <type_traits>
#endif

namespace Gosu
{
#if defined(GOSU_CPP11_ENABLED)
    class DrawModifier {};
    struct Position : public DrawModifier
    {
        public:
        double x, y;
        explicit Position(double x, double y):x(x),y(y) {}
    };

    struct PositionOfCenter : public DrawModifier
    {
        public:
        double x, y;
        explicit PositionOfCenter(double x, double y):x(x),y(y) {}
    };

    struct Scale : public DrawModifier
    {
        public:
        double x, y;
        explicit Scale(double x, double y):x(x),y(y) {}
    };

    struct ScaleAbsolute : public DrawModifier
    {
        public:
        double x, y;
        explicit ScaleAbsolute(double x, double y):x(x),y(y) {}
    };

    struct Rotate : public DrawModifier
    {
        public:
        double degrees;
        explicit Rotate(double degrees):degrees(degrees) {}
    };

    struct ColorSet : public DrawModifier
    {
        public:
        Color c[4];
        ColorSet(Color c1, Color c2, Color c3, Color c4):c({c1, c2, c3, c4}) {}
    };

    struct ZPosSet : public DrawModifier
    {
        public:
        ZPos z;
        explicit ZPosSet(ZPos z):z(z) {}
    };

    template<typename... Args>
    class is_DrawModifier
    {
        public:
        static const bool value = true;
    };

    template<class T, typename... Args>
    class is_DrawModifier<T, Args...>
    {
        public:
        static const bool value = std::is_base_of<DrawModifier, T>::value && is_DrawModifier<Args...>::value;
    };

    // allow AlphaMode directly without a DrawModifier class
    template<typename... Args>
    class is_DrawModifier<AlphaMode, Args...>
    {
        public:
        static const bool value = is_DrawModifier<Args...>::value;
    };

    // allow Color directly without a DrawModifier class
    template<typename... Args>
    class is_DrawModifier<Color, Args...>
    {
        public:
        static const bool value = is_DrawModifier<Args...>::value;
    };
#endif //defined(GOSU_CPP11_ENABLED)

    //! Provides functionality for drawing rectangular images.
    class Image
    {
        std::tr1::shared_ptr<ImageData> data;

    public:
        //! Loads an image from a given filename that can be drawn onto
        //! graphics.
        //! This constructor can handle PNG and BMP images. A color key of #ff00ff is
        //! automatically applied to BMP type images. For more flexibility, use the
        //! corresponding constructor that uses a Bitmap object.
        Image(Graphics& graphics, const std::wstring& filename,
              bool tileable = false);
        //! Loads a portion of the the image at the given filename that can be
        //! drawn onto graphics.
        //! This constructor can handle PNG and BMP images. A color key of #ff00ff is
        //! automatically applied to BMP type images. For more flexibility, use the
        //! corresponding constructor that uses a Bitmap object.
        Image(Graphics& graphics, const std::wstring& filename, unsigned srcX,
              unsigned srcY, unsigned srcWidth, unsigned srcHeight,
              bool tileable = false);
        
        //! Converts the given bitmap into an image that can be drawn onto
        //! graphics.
        Image(Graphics& graphics, const Bitmap& source,
            bool tileable = false);
        //! Converts a portion of the given bitmap into an image that can be
        //! drawn onto graphics.
        Image(Graphics& graphics, const Bitmap& source, unsigned srcX,
            unsigned srcY, unsigned srcWidth, unsigned srcHeight,
            bool tileable = false);
        
        //! Creates an Image from a user-supplied instance of the ImageData interface.
        explicit Image(std::auto_ptr<ImageData> data);

        unsigned width() const;
        unsigned height() const;

        //! Draws the image so its upper left corner is at (x; y).
        void draw(double x, double y, ZPos z,
            double factorX = 1, double factorY = 1,
            Color c = Color::WHITE,
            AlphaMode mode = amDefault) const;
        //! Like draw(), but allows to give modulation colors for all four
        //! corners.
        void drawMod(double x, double y, ZPos z,
            double factorX, double factorY,
            Color c1, Color c2, Color c3, Color c4,
            AlphaMode mode = amDefault) const;

        //! Draws the image rotated by the given angle so that its rotation
        //! center is at (x; y). Note that this is different from how all the
        //! other drawing functions work!
        //! \param angle See Math.hpp for an explanation of how Gosu interprets
        //! angles.
        //! \param centerX Relative horizontal position of the rotation center
        //! on the image. 0 is the left border, 1 is the right border, 0.5 is
        //! the center (and default).
        //! \param centerY See centerX.
        void drawRot(double x, double y, ZPos z,
            double angle, double centerX = 0.5, double centerY = 0.5,
            double factorX = 1, double factorY = 1,
            Color c = Color::WHITE,
            AlphaMode mode = amDefault) const;

        //! Provides access to the underlying image data object.
        ImageData& getData() const;

#if defined(GOSU_CPP11_ENABLED)
        template<typename... Args, typename = typename std::enable_if<is_DrawModifier<Args...>::value>::type>
        void draw(double x, double y, ZPos z, Args... args) const
        {
            draw_temp(  x, y,
                        z,
                        1.0, 1.0,
                        0.0,
                        0.0, 0.0,
                        Color::WHITE, Color::WHITE, Color::WHITE, Color::WHITE,
                        AlphaMode::Default,
                        args...);
        }
        template<typename... Args, typename = typename std::enable_if<is_DrawModifier<Args...>::value>::type>
        void draw(Args... args) const
        {
            draw(0.0, 0.0, 0.0, args...);
        }
    private:
        // final function call
        void draw_temp( double x, double y,
                        ZPos z,
                        double factorX, double factorY,
                        double angle,
                        double centerX, double centerY,
                        Color c1, Color c2, Color c3, Color c4,
                        AlphaMode mode) const;

        // specialized call on Position
        template<typename... Args>
        void draw_temp( double x, double y,
                        ZPos z,
                        double factorX, double factorY,
                        double angle,
                        double centerX, double centerY,
                        Color c1, Color c2, Color c3, Color c4,
                        AlphaMode mode,
                        Position pos,
                        Args... rest) const
        {
            draw_temp(  pos.x, pos.y,
                        z,
                        factorX, factorY,
                        angle,
                        centerX, centerY,
                        c1, c2, c3, c4,
                        mode,
                        rest...);
        }
        // specialized call on PositionOfCenter
        template<typename... Args>
        void draw_temp( double x, double y,
                        ZPos z,
                        double factorX, double factorY,
                        double angle,
                        double centerX, double centerY,
                        Color c1, Color c2, Color c3, Color c4,
                        AlphaMode mode,
                        PositionOfCenter pos,
                        Args... rest) const
        {
            draw_temp(  pos.x, pos.y,
                        z,
                        factorX, factorY,
                        angle,
                        0.5, 0.5,
                        c1, c2, c3, c4,
                        mode,
                        rest...);
        }
        // specialized call on Scale
        template<typename... Args>
        void draw_temp( double x, double y,
                        ZPos z,
                        double factorX, double factorY,
                        double angle,
                        double centerX, double centerY,
                        Color c1, Color c2, Color c3, Color c4,
                        AlphaMode mode,
                        Scale scale,
                        Args... rest) const
        {
            draw_temp(  x, y,
                        z,
                        scale.x, scale.y,
                        angle,
                        centerX, centerY,
                        c1, c2, c3, c4,
                        mode,
                        rest...);
        }
        // specialized call on ScaleAbsolute
        template<typename... Args>
        void draw_temp( double x, double y,
                        ZPos z,
                        double factorX, double factorY,
                        double angle,
                        double centerX, double centerY,
                        Color c1, Color c2, Color c3, Color c4,
                        AlphaMode mode,
                        ScaleAbsolute scale,
                        Args... rest) const
        {
            draw_temp(  x, y,
                        z,
                        scale.x/width(), scale.y/height(),
                        angle,
                        centerX, centerY,
                        c1, c2, c3, c4,
                        mode,
                        rest...);
        }
        // specialized call on Rotate
        template<typename... Args>
        void draw_temp( double x, double y,
                        ZPos z,
                        double factorX, double factorY,
                        double angle,
                        double centerX, double centerY,
                        Color c1, Color c2, Color c3, Color c4,
                        AlphaMode mode,
                        Rotate rot,
                        Args... rest) const
        {
            draw_temp(  x, y,
                        z,
                        factorX, factorY,
                        rot.degrees,
                        centerX, centerY,
                        c1, c2, c3, c4,
                        mode,
                        rest...);
        }
        // specialized call on ColorSet
        template<typename... Args>
        void draw_temp( double x, double y,
                        ZPos z,
                        double factorX, double factorY,
                        double angle,
                        double centerX, double centerY,
                        Color c1, Color c2, Color c3, Color c4,
                        AlphaMode mode,
                        ColorSet col,
                        Args... rest) const
        {
            draw_temp(  x, y,
                        z,
                        factorX, factorY,
                        angle,
                        centerX, centerY,
                        col.c[0], col.c[1], col.c[2], col.c[3],
                        mode,
                        rest...);
        }
        // specialized call on Color
        template<typename... Args>
        void draw_temp( double x, double y,
                        ZPos z,
                        double factorX, double factorY,
                        double angle,
                        double centerX, double centerY,
                        Color c1, Color c2, Color c3, Color c4,
                        AlphaMode mode,
                        Color col,
                        Args... rest) const
        {
            draw_temp(  x, y,
                        z,
                        factorX, factorY,
                        angle,
                        centerX, centerY,
                        col, col, col, col,
                        mode,
                        rest...);
        }
        // specialized call on AlphaMode
        template<typename... Args>
        void draw_temp( double x, double y,
                        ZPos z,
                        double factorX, double factorY,
                        double angle,
                        double centerX, double centerY,
                        Color c1, Color c2, Color c3, Color c4,
                        AlphaMode mode,
                        AlphaMode am,
                        Args... rest) const
        {
            draw_temp(  x, y,
                        z,
                        factorX, factorY,
                        angle,
                        centerX, centerY,
                        c1, c2, c3, c4,
                        am,
                        rest...);
        }
        // specialized call on ZPosSet
        template<typename... Args>
        void draw_temp( double x, double y,
                        ZPos z,
                        double factorX, double factorY,
                        double angle,
                        double centerX, double centerY,
                        Color c1, Color c2, Color c3, Color c4,
                        AlphaMode mode,
                        ZPosSet zpos,
                        Args... rest) const
        {
            draw_temp(  x, y,
                        zpos.z,
                        factorX, factorY,
                        angle,
                        centerX, centerY,
                        c1, c2, c3, c4,
                        mode,
                        rest...);
        }
#endif //defined(GOSU_CPP11_ENABLED)

    };
    
    std::vector<Gosu::Image> loadTiles(Graphics& graphics, const Bitmap& bmp, int tileWidth, int tileHeight, bool tileable);
    std::vector<Gosu::Image> loadTiles(Graphics& graphics, const std::wstring& bmp, int tileWidth, int tileHeight, bool tileable);
    
    //! Convenience function that splits a BMP or PNG file into an array
    //! of small rectangles and creates images from them.
    //! \param tileWidth If positive, specifies the width of one tile in
    //! pixels. If negative, the bitmap is divided into -tileWidth rows.
    //! \param tileHeight See tileWidth.
    //! \param appendTo STL container to which the images will be appended.
    //! Must provide a push_back member function; vector<tr1::shared_ptr<Image>>
    //! or boost::ptr_vector<Image> are good choices.
    template<typename Container>
    void imagesFromTiledBitmap(Graphics& graphics, const std::wstring& filename, int tileWidth, int tileHeight, bool tileable, Container& appendTo)
    {
        std::vector<Gosu::Image> tiles = loadTiles(graphics, filename, tileWidth, tileHeight, tileable);
        for (int i = 0, num = tiles.size(); i < num; ++i)
            appendTo.push_back(typename Container::value_type(new Gosu::Image(tiles[i])));
    }
    
    //! Convenience function that splits a bitmap into an area of array 
    //! rectangles and creates images from them.
    //! \param tileWidth If positive, specifies the width of one tile in
    //! pixels. If negative, the bitmap is divided into -tileWidth rows.
    //! \param tileHeight See tileWidth.
    //! \param appendTo STL container to which the images will be appended.
    //! Must provide a push_back member function; std::vector<std::tr1::shared_ptr<Image>>
    //! or boost::ptr_vector<Image> are good choices.
    template<typename Container>
    void imagesFromTiledBitmap(Graphics& graphics, const Bitmap& bmp,
        int tileWidth, int tileHeight, bool tileable, Container& appendTo)
    {
        std::vector<Gosu::Image> tiles = loadTiles(graphics, bmp, tileWidth, tileHeight, tileable);
        for (int i = 0, num = tiles.size(); i < num; ++i)
            appendTo.push_back(typename Container::value_type(new Gosu::Image(tiles[i])));
    }
}

#endif
