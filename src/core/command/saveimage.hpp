#ifndef COMMAND_SAVEIMAGE_HPP
#define COMMAND_SAVEIMAGE_HPP

#include "command.hpp"

namespace sword
{

class Region;

namespace command
{

class SaveSwapToPng : public Command
{
public:
    void execute(Application*) override;
    const char* getName() const override {return "SaveSwapToPng";};
    void set( const std::string_view name) { fileName = name; }
private:
    std::string fileName;
};

class SaveAttachmentToPng : public Command
{
public:
    void execute(Application*) override;
    const char* getName() const override {return "SaveAttachmentToPng";};
    void set( const std::string_view attachmentName, const std::string_view fileName, int offset_x, int offset_y, int width, int height, bool fullPath = false ) 
    {
        this->x = offset_x;
        this->y = offset_y;
        this->width = width;
        this->height = height;
        this->attachmentName = attachmentName;
        this->fileName = fileName;
        this->fullPath = fullPath;
    }
private:
    int32_t x{0};
    int32_t y{0};
    uint32_t width{100};
    uint32_t height{100};
    std::string attachmentName;
    std::string fileName;
    bool fullPath;
};

class CopyAttachmentToUndoStack : public Command
{
public:
    void execute(Application*) override;
    void reverse(Application*);
    const char* getName() const override {return "CopyAttachmentToUndoStack";};
    void set(Region* region, std::string_view attachmentName, int offset_x, int offset_y, int width, int height ) 
    {
        this->undoStack = region;
        this->x = offset_x;
        this->y = offset_y;
        this->width = width;
        this->height = height;
        this->attachmentName = attachmentName;
    }
private:
    Region* undoStack;
    int32_t x{0};
    int32_t y{0};
    uint32_t width{100};
    uint32_t height{100};
    std::string attachmentName;
};

}; // namespace command

}; // namespace sword

#endif /* end of include guard: COMMAND_SAVEIMAGE_HPP */
