//------------------------------------------------------------------------------
// !START!
//------------------------------------------------------------------------------

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include "qu_image.h"
#include "qu_log.h"

//------------------------------------------------------------------------------

static int read(void *user, char *data, int size)
{
    libqu_file *file = (libqu_file *) user;
    return (int) libqu_fread(data, size, file);
}

static void skip(void *user, int n)
{
    libqu_file *file = (libqu_file *) user;
    libqu_fseek(file, n, SEEK_CUR);
}

static int eof(void *user)
{
    libqu_file *file = (libqu_file *) user;
    int64_t position = libqu_ftell(file);
    int64_t size = libqu_file_size(file);

    return (position == size);
}

libqu_image *libqu_load_image(libqu_file *file)
{
    // stbi_set_flip_vertically_on_load(1);

    libqu_image *image = malloc(sizeof(libqu_image));

    if (!image) {
        return NULL;
    }

    image->pixels = stbi_load_from_callbacks(
        &(stbi_io_callbacks) {
            .read = read,
            .skip = skip,
            .eof = eof,
        }, file,
        &image->width,
        &image->height,
        &image->channels, 0
    );

    if (!image->pixels) {
        libqu_error("Failed to load image from %s. stbi error: %s\n",
            libqu_file_repr(file), stbi_failure_reason());

        free(image);
        return NULL;
    }

    libqu_info("Loaded %dx%dx%d image from %s.\n",
        image->width, image->height,
        image->channels, libqu_file_repr(file));

    return image;
}

void libqu_delete_image(libqu_image *image)
{
    stbi_image_free(image->pixels);
    free(image);
}

//------------------------------------------------------------------------------
