#pragma once

#include <stdint.h>
#include <string.h>
#include <strings.h>

// fast special cased parallel transactional database with generic
// metadata search

// 1) create list with imageid + metadata (couple dimensions, say <=3)
// 2) morton code sort (parallel radix sort, similar to pbrt's lbvh)
//    3d morton code in 64 bits limits us to ~1M entries in the db (2^20)
// 3) offset list is kd-tree for search
// 4) query any range/sort order

// then create a vertex array object from it and copy over to GPU for display

// for 1) we need metadata as 20-bit index. needs a string pool/hashtable? look
// into concurrencykit?

typedef enum dt_image_label_t
{
  s_image_label_none   = 0,
  s_image_label_red    = 1<<0,
  s_image_label_green  = 1<<1,
  s_image_label_blue   = 1<<2,
  s_image_label_yellow = 1<<3,
  s_image_label_purple = 1<<4,
  s_image_label_selected = 1<<15,
}
dt_image_label_t;

typedef struct dt_image_t
{
  const char *filename;  // point into db.sp_filename.buf stringpool
  uint32_t    thumbnail; // index into thumbnails->thumb[] or -1u
  uint16_t    rating;    // -1u reject 0 1 2 3 4 5 stars
  uint16_t    labels;    // each bit is one colour label flag, 1<<15 is selected bit
}
dt_image_t;

// forward declare for stringpool.h so we don't have to include it here.
typedef struct dt_stringpool_entry_t dt_stringpool_entry_t;
typedef struct dt_stringpool_t
{
  uint32_t entry_max;
  dt_stringpool_entry_t *entry;

  uint32_t buf_max;
  uint32_t buf_cnt;
  char *buf;
}
dt_stringpool_t;

typedef struct dt_db_t
{
  char dirname[1024];           // full path of currently opened directory

  // list of images in database
  dt_image_t *image;
  uint32_t image_cnt;
  uint32_t image_max;

  // string pool for image file names
  dt_stringpool_t sp_filename;

  // TODO: light table edit history

  // TODO: current sort criterion for current collection

  // current query
  uint32_t *collection;
  uint32_t  collection_cnt;
  uint32_t  collection_max;

  // selection
  uint32_t *selection;
  uint32_t  selection_cnt;
  uint32_t  selection_max;

  // currently selected image (when switching to darkroom mode, e.g.)
  uint32_t current_image;
}
dt_db_t;

void dt_db_init   (dt_db_t *db);
void dt_db_cleanup(dt_db_t *db);

typedef struct dt_thumbnails_t dt_thumbnails_t;
void dt_db_load_directory(
    dt_db_t         *db,
    dt_thumbnails_t *thumbnails,
    const char      *dirname);

void dt_db_load_image(
    dt_db_t         *db,
    dt_thumbnails_t *thumbnails,
    const char      *filename);

static inline int
dt_db_accept_filename(
    const char *f)
{
  // TODO: magic number checks instead.
  const char *f2 = f + strlen(f);
  while(f2 > f && *f2 != '.') f2--;
  return !strcasecmp(f2, ".cr2") ||
         !strcasecmp(f2, ".nef") ||
         !strcasecmp(f2, ".orf") ||
         !strcasecmp(f2, ".arw") ||
         !strcasecmp(f2, ".dng") ||
         !strcasecmp(f2, ".raf") ||
         !strcasecmp(f2, ".rw2") ||
         !strcasecmp(f2, ".cfg");   // also accept config files directly (preferrably so)
}

// add image to the list of selected images, O(1).
void dt_db_selection_add   (dt_db_t *db, uint32_t imgid);
// remove image from the list of selected images, O(N).
void dt_db_selection_remove(dt_db_t *db, uint32_t imgid);
// return sorted list of selected images
const uint32_t *dt_db_selection_get(dt_db_t *db);

// work with lighttable history
// TODO: modify image rating w/ adding history
// TODO: modify image labels w/ adding history

// read and write db config in ascii
int dt_db_read (dt_db_t *db, const char *filename);
// TODO: make sure we only write to this file if we opened the *complete* directory.
// TODO: single photo/subset sessions should append to the db file in this directory.
int dt_db_write(const dt_db_t *db, const char *filename, int append);

// fill full file name with directory and extension.
// return 0 on success, else the buffer was too small.
int dt_db_image_path(const dt_db_t *db, const uint32_t imgid, char *fn, uint32_t maxlen);
