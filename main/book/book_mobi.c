/* SPDX-License-Identifier: LicenseRef-Kiikoread-Personal-Study-1.0
 * 中文：按 PDB 记录读取 MOBI6，索引可丢弃，原书只读。
 * English: Record-based MOBI6 reader with disposable indices and read-only originals.
 */
#include "book_mobi.h"
#include "book_mobi_markup.h"
#include "book_index_cache.h"
#include "../image/image_decode.h"
#include "../diagnostics/reader_memory.h"
#include "mobi_codec.h"
#include "esp_heap_caps.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#ifdef ESP_PLATFORM
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

#define RECORD_MAX 16384u
#define RECORD_BYTES 65536u
#define TEXT_MAX (64u * 1024u * 1024u)
#define META_MAX (1024u * 1024u)
#define SEGMENT_TARGET (96u * 1024u)
// 2：目录改为优先 NCX，旧缓存需重建。/ 2: NCX-first navigation invalidates older caches.
#define CACHE_VERSION 2u
#define PATH_CAP 512u

typedef struct { uint32_t offset, raw, len; } record_t;
typedef struct { const uint8_t *data; uint32_t len; bool shared, busy; } cdic_t;
typedef struct { uint8_t *data; size_t len, cap; bool failed; } huff_buf_t;
typedef struct { uint32_t offset; char title[MOBI_TITLE_CAP]; } navigation_t;
typedef struct { char title[256], author[160]; } metadata_t;
struct book_mobi {
    FILE *file;
    uint32_t size, records, text_records, total, encoding, first_image, cover;
    uint16_t compression, extra;
    uint32_t fingerprint;
    record_t *record;
    uint32_t *segments;
    size_t count, segment_cap;
    navigation_t *navigation;
    size_t nav_count, nav_cap;
    uint32_t huff_offset, huff_count; ///< HUFF/CDIC 表位置 / HUFF-CDIC tables
    uint32_t ncx_index;               ///< INDX/NCX 主记录 / Main INDX record
    struct { uint8_t codelen; bool term; uint64_t maxcode; } dict1[256];
    uint64_t mincode[33], maxcode[33];
    cdic_t *dict;
    size_t dict_count, dict_cap;
    uint8_t **pool;
    size_t pool_count, pool_cap;
    metadata_t metadata;
    char path[PATH_CAP];
};

static uint16_t be16(const uint8_t *p) { return (uint16_t)((uint16_t)p[0] << 8 | p[1]); }
static uint32_t be32(const uint8_t *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }
static uint32_t hash_bytes(uint32_t hash, const void *data, size_t size) {
    const uint8_t *p = data;
    for (size_t i = 0; i < size; ++i) hash = (hash ^ p[i]) * 16777619u;
    return hash;
}
static void *allocate(size_t size) { return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); }
static esp_err_t read_at(book_mobi_t *b, uint32_t offset, void *out, size_t size) {
    if (offset > b->size || size > b->size - offset) return ESP_ERR_INVALID_SIZE;
    return fseek(b->file, (long)offset, SEEK_SET) == 0 && fread(out, 1, size, b->file) == size ? ESP_OK : ESP_FAIL;
}
static esp_err_t copy_title(const char *s, char *out, size_t cap) {
    if (!out || !cap) return ESP_ERR_INVALID_ARG;
    out[0] = 0;
    size_t len = strlen(s);
    if (len >= cap) return ESP_ERR_INVALID_SIZE;
    memcpy(out, s, len + 1);
    return ESP_OK;
}
static esp_err_t metadata_text(const uint8_t *s, size_t len, uint32_t enc, char *out, size_t cap) {
    char *utf8 = NULL; size_t bytes;
    esp_err_t err = book_mobi_decode((const char *)s, len, enc, &utf8, &bytes);
    if (err != ESP_OK) return err;
    size_t n = bytes < cap - 1 ? bytes : cap - 1;
    while (n && n < bytes && ((uint8_t)utf8[n] & 0xc0) == 0x80) --n;
    memcpy(out, utf8, n); out[n] = 0;
    free(utf8);
    return ESP_OK;
}

static esp_err_t headers(const char *path, book_mobi_t **out) {
    *out = NULL;
    if (!path || !*path || strlen(path) >= PATH_CAP) return ESP_ERR_INVALID_ARG;
    book_mobi_t *b = allocate(sizeof(*b));
    if (!b) return ESP_ERR_NO_MEM;
    memset(b, 0, sizeof(*b));
    b->cover = b->first_image = UINT32_MAX;
    memcpy(b->path, path, strlen(path) + 1);
    b->file = fopen(path, "rb");
    esp_err_t err = ESP_ERR_NOT_FOUND;
    uint8_t *table = NULL, *head = NULL;
    if (!b->file) goto fail;
    err = ESP_ERR_INVALID_SIZE;
    if (fseek(b->file, 0, SEEK_END)) goto fail;
    long size = ftell(b->file);
    if (size < 78 || (uint64_t)size > UINT32_MAX) goto fail;
    b->size = (uint32_t)size;
    uint8_t pdb[78];
    if ((err = read_at(b, 0, pdb, sizeof(pdb))) != ESP_OK) goto fail;
    err = ESP_ERR_NOT_SUPPORTED;
    if (memcmp(pdb + 60, "BOOKMOBI", 8)) goto fail;
    b->records = be16(pdb + 76);
    err = ESP_ERR_INVALID_SIZE;
    if (b->records < 2 || b->records > RECORD_MAX || b->records > (b->size - 78) / 8) goto fail;
    table = allocate(b->records * 8);
    b->record = allocate((b->records + 1) * sizeof(*b->record));
    if (!table || !b->record) { err = ESP_ERR_NO_MEM; goto fail; }
    memset(b->record, 0, (b->records + 1) * sizeof(*b->record));
    if ((err = read_at(b, 78, table, b->records * 8)) != ESP_OK) goto fail;
    b->fingerprint = hash_bytes(hash_bytes(2166136261u, pdb, sizeof(pdb)), table, b->records * 8);
    uint32_t previous = 78 + b->records * 8 - 1;
    for (size_t i = 0; i < b->records; ++i) {
        uint32_t offset = be32(table + i * 8);
        if (offset <= previous || offset >= b->size) { err = ESP_ERR_INVALID_SIZE; goto fail; }
        b->record[i].offset = offset;
        previous = offset;
    }
    b->record[b->records].offset = b->size;
    free(table); table = NULL;
    size_t head_len = b->record[1].offset - b->record[0].offset;
    if (head_len < 132 || head_len > META_MAX) { err = ESP_ERR_INVALID_SIZE; goto fail; }
    head = allocate(head_len);
    if (!head) { err = ESP_ERR_NO_MEM; goto fail; }
    if ((err = read_at(b, b->record[0].offset, head, head_len)) != ESP_OK) goto fail;
    b->fingerprint = hash_bytes(b->fingerprint, head, head_len);
    err = ESP_ERR_NOT_SUPPORTED;
    if (memcmp(head + 16, "MOBI", 4) || be16(head + 12)) goto fail;
    uint32_t mobi_len = be32(head + 20);
    if (mobi_len < 116 || mobi_len > head_len - 16) { err = ESP_ERR_INVALID_SIZE; goto fail; }
    if (be32(head + 24) != 2 || be32(head + 36) != 6) goto fail;
    b->compression = be16(head);
    if (b->compression != 1 && b->compression != 2 && b->compression != 17480) goto fail;
    b->encoding = be32(head + 28);
    if (b->encoding != 65001 && b->encoding != 1252) goto fail;
    b->total = be32(head + 4); b->text_records = be16(head + 8);
    b->first_image = be32(head + 108);
    if (mobi_len >= 228) b->extra = be16(head + 242);
    // 这三个字段在记录 0 里按 PalmDOC 头之后的绝对偏移给出。
    // These three live at absolute offsets in record 0, after the PalmDOC header.
    if (head_len >= 0x78) {
        uint32_t huff = be32(head + 0x70), count = be32(head + 0x74);
        if (huff && huff < b->records && count && count < 64) { b->huff_offset = huff; b->huff_count = count; }
    }
    // 目录索引字段在 0xF4，落在声明的头长（常见 0xE8）之外，只能按记录长度判断。
    // The NCX field sits at 0xF4, past the declared header length (often 0xE8); bound it by the record.
    if (head_len >= 0xf8) {
        uint32_t ncx = be32(head + 0xf4);
        if (ncx != UINT32_MAX && ncx < b->records) b->ncx_index = ncx;
    }
    err = ESP_ERR_INVALID_SIZE;
    if (!b->total || b->total > TEXT_MAX || !b->text_records || b->text_records >= b->records ||
        !be16(head + 10)) goto fail;
    if (b->first_image != UINT32_MAX && (b->first_image <= b->text_records || b->first_image >= b->records)) goto fail;
    for (size_t i = 1; i <= b->text_records; ++i)
        if (b->record[i+1].offset - b->record[i].offset > RECORD_BYTES) goto fail;
    uint32_t name = be32(head + 84), name_len = be32(head + 88);
    if (name_len) {
        if (name > head_len || name_len > head_len - name) goto fail;
        if ((err = metadata_text(head + name, name_len, b->encoding, b->metadata.title, sizeof(b->metadata.title))) != ESP_OK) goto fail;
    }
    if (be32(head + 128) & 0x40) {
        size_t at = 16 + mobi_len;
        err = ESP_ERR_INVALID_SIZE;
        if (at > head_len || head_len - at < 12 || memcmp(head + at, "EXTH", 4)) goto fail;
        uint32_t exth_len = be32(head + at + 4), count = be32(head + at + 8);
        if (exth_len < 12 || exth_len > head_len - at || count > 4096) goto fail;
        size_t end = at + exth_len; at += 12;
        for (uint32_t i = 0; i < count; ++i) {
            err = ESP_ERR_INVALID_SIZE;
            if (end - at < 8) goto fail;
            uint32_t type = be32(head + at), n = be32(head + at + 4);
            if (n < 8 || n > end - at) goto fail;
            const uint8_t *value = head + at + 8;
            if (type == 100 || type == 503) {
                if ((err = metadata_text(value, n - 8, b->encoding,
                        type == 100 ? b->metadata.author : b->metadata.title,
                        type == 100 ? sizeof(b->metadata.author) : sizeof(b->metadata.title))) != ESP_OK) goto fail;
            } else if (type == 201 && n == 12 && b->first_image < b->records) {
                uint32_t index = be32(value);
                if (index < b->records - b->first_image) b->cover = b->first_image + index;
            }
            at += n;
        }
    }
    free(head);
    *out = b;
    return ESP_OK;
fail:
    free(table); free(head); book_mobi_close(b);
    return err;
}

static bool huff_ready(book_mobi_t *b);
static bool huff_inflate(book_mobi_t *b, const uint8_t *data, size_t len, huff_buf_t *out, int depth);

static esp_err_t decode_record(book_mobi_t *b, size_t record, uint8_t *compressed, uint8_t *raw, size_t *len) {
    size_t n = b->record[record + 1].offset - b->record[record].offset;
    esp_err_t err = read_at(b, b->record[record].offset, compressed, n);
    if (err != ESP_OK) return err;
    for (int bit = 15; bit > 0; --bit) if (b->extra & (1u << bit)) {
        uint32_t size = 0; unsigned shift = 0; size_t bytes = 0;
        bool terminal = false;
        while (bytes < n && bytes < 4) {
            uint8_t c = compressed[n - 1 - bytes++];
            size |= (uint32_t)(c & 0x7f) << shift;
            shift += 7;
            if (c & 0x80) { terminal = true; break; }
        }
        if (!terminal || size < bytes || size > n) return ESP_ERR_INVALID_SIZE;
        n -= size;
    }
    if (b->extra & 1) {
        if (!n) return ESP_ERR_INVALID_SIZE;
        size_t count = (compressed[n - 1] & 3) + 1;
        if (count > n) return ESP_ERR_INVALID_SIZE;
        n -= count;
    }
    *len = RECORD_BYTES;
    if (b->compression == 2) {
        if (!mobi_codec_palmdoc(raw, len, compressed, n)) return ESP_ERR_INVALID_RESPONSE;
    } else if (b->compression == 17480) {
        // HUFF/CDIC：码表与词典在首次解压时装载，之后只查表。
        // HUFF-CDIC: the code table and dictionary load on first decode, then lookups only.
        if (!huff_ready(b)) return ESP_ERR_NOT_SUPPORTED;
        huff_buf_t sink = {raw, 0, RECORD_BYTES, false};
        if (!huff_inflate(b, compressed, n, &sink, 0) || !sink.len) return ESP_ERR_INVALID_RESPONSE;
        *len = sink.len;
    } else { memcpy(raw, compressed, n); *len = n; }
    return ESP_OK;
}

static bool grow(void **array, size_t *capacity, size_t count, size_t item_size, size_t limit) {
    if (count < *capacity) return true;
    if (count >= limit) return false;
    size_t cap = *capacity ? *capacity * 2 : 16;
    if (cap > limit) cap = limit;
    void *p = heap_caps_realloc(*array, cap * item_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!p) return false;
    *array = p; *capacity = cap;
    return true;
}
/* ---- HUFF/CDIC 解码 / HUFF-CDIC decoding ---- */


static bool hbuf_add(huff_buf_t *b, const uint8_t *src, size_t n) {
    if (b->failed) return false;
    if (n > b->cap - b->len) { b->failed = true; return false; }
    memcpy(b->data + b->len, src, n);
    b->len += n;
    return true;
}

static uint8_t *record_read(book_mobi_t *b, size_t index, size_t *len, size_t cap) {
    if (!b || index >= b->records) return NULL;
    size_t n = b->record[index + 1].offset - b->record[index].offset;
    if (!n || n > cap) return NULL;
    uint8_t *data = allocate(n);
    if (!data) return NULL;
    if (read_at(b, b->record[index].offset, data, n) != ESP_OK) { free(data); return NULL; }
    *len = n;
    return data;
}

static bool pool_keep(book_mobi_t *b, uint8_t *data) {
    if (b->pool_count == b->pool_cap) {
        size_t cap = b->pool_cap ? b->pool_cap * 2 : 4;
        uint8_t **grown = heap_caps_realloc(b->pool, cap * sizeof(*grown), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (!grown) return false;
        b->pool = grown;
        b->pool_cap = cap;
    }
    b->pool[b->pool_count++] = data;
    return true;
}

static bool dict_append(book_mobi_t *b, const cdic_t *entry) {
    if (b->dict_count == b->dict_cap) {
        size_t cap = b->dict_cap ? b->dict_cap * 2 : 512;
        cdic_t *grown = heap_caps_realloc(b->dict, cap * sizeof(*grown), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (!grown) return false;
        b->dict = grown;
        b->dict_cap = cap;
    }
    b->dict[b->dict_count++] = *entry;
    return true;
}

static bool huff_inflate(book_mobi_t *b, const uint8_t *data, size_t len, huff_buf_t *out, int depth);

// 展开一条词典短语；嵌套条目递归解码后再当作普通字节。
// Expand one dictionary phrase; a nested entry is decoded recursively and then treated as bytes.
static bool huff_phrase(book_mobi_t *b, cdic_t *entry, huff_buf_t *out, int depth) {
    if (entry->shared) return hbuf_add(out, entry->data, entry->len);
    if (entry->busy || depth > 6) return false;
    entry->busy = true;
    bool ok = huff_inflate(b, entry->data, entry->len, out, depth);
    entry->busy = false;
    return ok;
}

static bool huff_inflate(book_mobi_t *b, const uint8_t *data, size_t len, huff_buf_t *out, int depth) {
    if (!len) return false;
    uint8_t *window = heap_caps_calloc(1, len + 16, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!window) return false;
    memcpy(window, data, len);
    long bits_left = (long)len * 8;
    size_t pos = 0;
    uint64_t x = ((uint64_t)be32(window) << 32) | be32(window + 4);
    long n = 32;
    bool ok = true;
    for (;;) {
        if (n <= 0) {
            pos += 4;
            if (pos + 8 > len + 16) break;
            x = ((uint64_t)be32(window + pos) << 32) | be32(window + pos + 4);
            n += 32;
        }
        uint32_t code = (uint32_t)((x >> (unsigned)n) & 0xffffffffull);
        unsigned codelen = b->dict1[code >> 24].codelen;
        uint64_t maxcode = b->dict1[code >> 24].maxcode;
        bool term = b->dict1[code >> 24].term;
        if (!term) {
            while (codelen <= 32 && (uint64_t)code < b->mincode[codelen]) ++codelen;
            if (codelen > 32) { ok = false; break; }
            maxcode = b->maxcode[codelen];
        }
        n -= (long)codelen;
        bits_left -= (long)codelen;
        if (bits_left < 0) break;
        uint64_t index = (maxcode - code) >> (32 - codelen);
        if (index >= b->dict_count) { ok = false; break; }
        if (!huff_phrase(b, &b->dict[index], out, depth + 1)) { ok = false; break; }
        if (out->failed) { ok = false; break; }
    }
    free(window);
    return ok;
}

static bool huff_load_table(book_mobi_t *b, size_t index) {
    size_t n = 0;
    uint8_t *huff = record_read(b, index, &n, 65536u);
    if (!huff) return false;
    bool ok = false;
    do {
        if (n < 24 || memcmp(huff, "HUFF", 4) != 0) break;
        uint32_t off1 = be32(huff + 8), off2 = be32(huff + 12);
        if ((uint64_t)off1 + 1024 > n || (uint64_t)off2 + 256 > n) break;
        bool bad = false;
        for (unsigned i = 0; i < 256; ++i) {
            uint32_t v = be32(huff + off1 + i * 4);
            unsigned codelen = v & 0x1f;
            if (!codelen) { bad = true; break; }
            uint32_t maxcode = v >> 8;
            b->dict1[i].codelen = (uint8_t)codelen;
            b->dict1[i].term = (v & 0x80) != 0;
            // 参考实现用任意精度整数；64 位足以保住 33 位 maxcode。
            // The reference uses arbitrary precision; 64 bits keep a 33-bit maxcode intact.
            b->dict1[i].maxcode = (((uint64_t)maxcode + 1u) << (32 - codelen)) - 1;
        }
        if (bad) break;
        for (unsigned i = 1; i <= 32; ++i) {
            uint32_t low = be32(huff + off2 + (i - 1) * 8);
            uint32_t high = be32(huff + off2 + (i - 1) * 8 + 4);
            b->mincode[i] = (uint64_t)low << (32 - i);
            b->maxcode[i] = (((uint64_t)high + 1u) << (32 - i)) - 1;
        }
        ok = true;
    } while (0);
    free(huff);
    return ok;
}

static bool huff_load_cdic(book_mobi_t *b, size_t index) {
    size_t n = 0;
    uint8_t *cdic = record_read(b, index, &n, 1048576u);
    if (!cdic) return false;
    if (n < 16 || memcmp(cdic, "CDIC", 4) != 0 || be32(cdic + 4) != 0x10) { free(cdic); return false; }
    uint32_t phrases = be32(cdic + 8), bits = be32(cdic + 12);
    if (bits > 24) { free(cdic); return false; }
    uint64_t room = phrases > b->dict_count ? phrases - b->dict_count : 0;
    uint64_t count = 1ull << bits;
    if (room < count) count = room;
    if (16 + count * 2 > n) { free(cdic); return false; }
    if (!pool_keep(b, cdic)) { free(cdic); return false; }
    for (uint64_t i = 0; i < count; ++i) {
        uint32_t at = 16u + be16(cdic + 16 + (size_t)i * 2);
        if ((uint64_t)at + 2 > n) return false;
        uint16_t head = be16(cdic + at);
        uint32_t bytes = head & 0x7fff;
        if ((uint64_t)at + 2 + bytes > n) return false;
        cdic_t entry = {cdic + at + 2, bytes, (head & 0x8000) != 0, false};
        if (!dict_append(b, &entry)) return false;
    }
    return true;
}

static bool huff_ready(book_mobi_t *b) {
    if (b->dict_count) return true;
    if (!b->huff_offset || !b->huff_count || b->huff_offset >= b->records) return false;
    if (!huff_load_table(b, b->huff_offset)) return false;
    for (uint32_t i = 1; i < b->huff_count; ++i) {
        if (b->huff_offset + i >= b->records) return false;
        if (!huff_load_cdic(b, b->huff_offset + i)) return false;
    }
    return b->dict_count != 0;
}

/* ---- INDX/NCX 目录 / INDX-NCX navigation ---- */

typedef struct { uint8_t tag, values, mask, end; } tagx_t;
typedef struct { uint32_t key, len; const char *text; } ctoc_t;
typedef struct { bool ok; uint32_t len, start, count, nctoc; } indx_head_t;

static bool vwi_read(const uint8_t *data, size_t len, size_t *at, uint32_t *out) {
    uint32_t value = 0;
    size_t i = *at;
    for (unsigned n = 0; n < 5 && i < len; ++n) {
        uint8_t byte = data[i++];
        value = (value << 7) | (byte & 0x7f);
        if (byte & 0x80) { *at = i; *out = value; return true; }
    }
    return false;
}

static unsigned bit_count(uint8_t value) {
    unsigned count = 0;
    while (value) { count += value & 1u; value >>= 1; }
    return count;
}

static indx_head_t indx_head(const uint8_t *data, size_t len) {
    indx_head_t head = {0};
    if (len < 0x38 || memcmp(data, "INDX", 4) != 0) return head;
    head.len = be32(data + 4);
    head.start = be32(data + 20);
    head.count = be32(data + 24);
    head.nctoc = be32(data + 52);
    head.ok = head.len < len;
    return head;
}

// 按 TAGX 走完一条目录项，取回 pos / noffs。
// Walk one index entry through the TAGX control bytes and keep pos/noffs.
static bool index_entry(const uint8_t *data, size_t len, size_t begin, size_t end,
                        const tagx_t *tags, size_t ntags, size_t control_bytes,
                        uint32_t *pos, uint32_t *noffs) {
    if (begin >= len || end > len || begin >= end) return false;
    size_t text_len = data[begin];
    size_t control = begin + 1 + text_len;
    if (control > end) return false;
    size_t data_at = control + control_bytes;
    size_t ctrl_index = 0;
    for (size_t i = 0; i < ntags; ++i) {
        if (tags[i].end) { ++ctrl_index; continue; }
        if (control + ctrl_index >= end) return false;
        uint8_t raw = data[control + ctrl_index];
        uint8_t value = raw & tags[i].mask;
        if (!value) continue;
        size_t values = 0;
        if (value == tags[i].mask && bit_count(tags[i].mask) > 1) {
            uint32_t bytes = 0;
            if (!vwi_read(data, len, &data_at, &bytes)) return false;
            size_t consumed = 0;
            while (consumed < bytes) {
                size_t before = data_at;
                uint32_t ignored = 0;
                if (!vwi_read(data, len, &data_at, &ignored)) return false;
                consumed += data_at - before;
                ++values;
            }
        } else if (value == tags[i].mask) {
            values = 1;
        } else {
            uint8_t mask = tags[i].mask, shifted = value;
            while (!(mask & 1u)) { mask >>= 1; shifted >>= 1; }
            values = (size_t)shifted * (tags[i].values ? tags[i].values : 1);
        }
        for (size_t v = 0; v < values; ++v) {
            for (unsigned repeat = 0; repeat < (tags[i].values ? tags[i].values : 1); ++repeat) {
                uint32_t number = 0;
                if (!vwi_read(data, len, &data_at, &number)) return false;
                if (!v && !repeat) {
                    if (tags[i].tag == 1) *pos = number;
                    else if (tags[i].tag == 3) *noffs = number;
                }
            }
        }
    }
    return true;
}

static int ctoc_cmp(const void *a, const void *b) {
    const ctoc_t *left = a, *right = b;
    if (left->key == right->key) return 0;
    return left->key < right->key ? -1 : 1;
}

// 读 ncx 编译出的 INDX：主记录带 TAGX，随后是子记录与 CTOC 字符串块。
// Read the INDX built from the ncx: a TAGX main record, sub-records and CTOC blocks.
static esp_err_t ncx_navigation(book_mobi_t *b) {
    if (!b->ncx_index || b->ncx_index >= b->records) return ESP_OK;
    size_t idx_len = 0;
    uint8_t *main = record_read(b, b->ncx_index, &idx_len, 1048576u);
    if (!main) return ESP_ERR_NOT_FOUND;
    esp_err_t err = ESP_ERR_NOT_FOUND;
    indx_head_t head = indx_head(main, idx_len);
    uint8_t *sources[8] = {0};
    size_t source_len[8] = {0};
    ctoc_t *map = NULL;
    size_t map_count = 0, map_cap = 0;
    navigation_t *built = NULL;
    size_t built_count = 0, built_cap = 0;
    if (!head.ok || head.count > 64 || head.nctoc > 8) goto done;
    if ((uint64_t)head.len + 12 > idx_len || memcmp(main + head.len, "TAGX", 4) != 0) goto done;
    uint32_t tagx_end = be32(main + head.len + 4);
    uint32_t control_bytes = be32(main + head.len + 8);
    if (tagx_end < 12 || (uint64_t)head.len + tagx_end > idx_len || !control_bytes || control_bytes > 4) goto done;
    size_t tag_count = (tagx_end - 12) / 4;
    tagx_t tags[16];
    if (tag_count > sizeof(tags) / sizeof(tags[0])) tag_count = sizeof(tags) / sizeof(tags[0]);
    for (size_t i = 0; i < tag_count; ++i) {
        const uint8_t *raw = main + head.len + 12 + i * 4;
        tags[i] = (tagx_t){raw[0], raw[1], raw[2], raw[3]};
    }
    for (uint32_t j = 0; j < head.nctoc; ++j) {
        size_t block = (size_t)b->ncx_index + head.count + 1 + j;
        sources[j] = record_read(b, block, &source_len[j], 1048576u);
        if (!sources[j]) goto done;
        size_t at = 0;
        while (at < source_len[j] && sources[j][at]) {
            size_t begin = at;
            uint32_t name_len = 0;
            if (!vwi_read(sources[j], source_len[j], &at, &name_len)) goto done;
            if ((uint64_t)at + name_len > source_len[j]) goto done;
            if (map_count == map_cap) {
                size_t cap = map_cap ? map_cap * 2 : 256;
                ctoc_t *grown = heap_caps_realloc(map, cap * sizeof(*grown), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
                if (!grown) { err = ESP_ERR_NO_MEM; goto done; }
                map = grown; map_cap = cap;
            }
            map[map_count].key = (uint32_t)begin + j * 0x10000u;
            map[map_count].text = (const char *)sources[j] + at;
            map[map_count].len = name_len;
            ++map_count;
            at += name_len;
        }
    }
    if (!map_count) goto done;
    qsort(map, map_count, sizeof(*map), ctoc_cmp);
    for (uint32_t r = 1; r <= head.count; ++r) {
        size_t sub_len = 0;
        uint8_t *sub = record_read(b, (size_t)b->ncx_index + r, &sub_len, 1048576u);
        if (!sub) { err = ESP_ERR_INVALID_SIZE; goto done; }
        indx_head_t sh = indx_head(sub, sub_len);
        if (!sh.ok || !sh.count || (uint64_t)sh.start + 4 + (uint64_t)sh.count * 2 > sub_len) { free(sub); continue; }
        for (uint32_t e = 0; e < sh.count; ++e) {
            size_t begin = be16(sub + sh.start + 4 + (size_t)e * 2);
            size_t end = e + 1 < sh.count ? be16(sub + sh.start + 4 + (size_t)(e + 1) * 2) : sh.start;
            uint32_t pos = 0, noffs = 0;
            if (!index_entry(sub, sub_len, begin, end, tags, tag_count, control_bytes, &pos, &noffs)) continue;
            ctoc_t key = {noffs, 0, NULL};
            const ctoc_t *found = bsearch(&key, map, map_count, sizeof(*map), ctoc_cmp);
            if (!found || !found->len || pos >= b->total) continue;
            if (built_count >= MOBI_NAV_MAX) { free(sub); err = ESP_ERR_INVALID_SIZE; goto done; }
            if (built_count == built_cap) {
                size_t cap = built_cap ? built_cap * 2 : 64;
                navigation_t *grown = heap_caps_realloc(built, cap * sizeof(*grown), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
                if (!grown) { free(sub); err = ESP_ERR_NO_MEM; goto done; }
                built = grown; built_cap = cap;
            }
            char *utf8 = NULL;
            size_t bytes = 0;
            if (book_mobi_decode(found->text, found->len, b->encoding, &utf8, &bytes) != ESP_OK) { free(sub); continue; }
            navigation_t *slot = &built[built_count];
            memset(slot, 0, sizeof(*slot));
            slot->offset = pos;
            size_t n = bytes < sizeof(slot->title) - 1 ? bytes : sizeof(slot->title) - 1;
            while (n && n < bytes && ((uint8_t)utf8[n] & 0xc0) == 0x80) --n;
            memcpy(slot->title, utf8, n);
            free(utf8);
            if (n) ++built_count;
        }
        free(sub);
    }
    if (built_count < 2) goto done;
    // 目录按正文位置排序并去重，位置相同的保留先出现的标题。
    // Sort by body position and drop duplicates, keeping the first title at a position.
    for (size_t i = 1; i < built_count; ++i) {
        navigation_t item = built[i];
        size_t j = i;
        while (j && built[j - 1].offset > item.offset) { built[j] = built[j - 1]; --j; }
        built[j] = item;
    }
    size_t unique = 0;
    for (size_t i = 0; i < built_count; ++i)
        if (!unique || built[i].offset != built[unique - 1].offset) built[unique++] = built[i];
    if (unique < 2) goto done;
    free(b->navigation);
    b->navigation = built;
    b->nav_count = unique;
    b->nav_cap = built_cap;
    built = NULL;
    err = ESP_OK;
done:
    free(main);
    for (unsigned j = 0; j < 8; ++j) free(sources[j]);
    free(map);
    free(built);
    return err;
}

static esp_err_t add_segment(book_mobi_t *b, uint32_t offset) {
    if (b->count && b->segments[b->count - 1] == offset) return ESP_OK;
    if (b->count == MOBI_NAV_MAX) return ESP_ERR_INVALID_SIZE;
    if (!grow((void **)&b->segments, &b->segment_cap, b->count, sizeof(*b->segments), MOBI_NAV_MAX)) return ESP_ERR_NO_MEM;
    b->segments[b->count++] = offset;
    return ESP_OK;
}
static esp_err_t add_navigation(book_mobi_t *b, uint32_t offset, const char *raw, size_t len) {
    if (offset >= b->total || !len) return ESP_OK;
    char *utf8 = NULL; size_t bytes;
    esp_err_t err = book_mobi_decode(raw, len, b->encoding, &utf8, &bytes);
    if (err != ESP_OK) return err;
    html_text_t text = {0};
    err = html_to_blocks(utf8, bytes, &text);
    free(utf8);
    if (err != ESP_OK) return err;
    while (text.len && isspace((unsigned char)text.utf8[text.len - 1])) text.utf8[--text.len] = 0;
    if (!text.len) { html_text_free(&text); return ESP_OK; }
    if (b->nav_count == MOBI_NAV_MAX) { html_text_free(&text); return ESP_ERR_INVALID_SIZE; }
    if (!grow((void **)&b->navigation, &b->nav_cap, b->nav_count, sizeof(*b->navigation), MOBI_NAV_MAX)) {
        html_text_free(&text); return ESP_ERR_NO_MEM;
    }
    navigation_t *nav = &b->navigation[b->nav_count++];
    memset(nav, 0, sizeof(*nav)); nav->offset = offset;
    size_t n = text.len < sizeof(nav->title) - 1 ? text.len : sizeof(nav->title) - 1;
    while (n < text.len && n && ((uint8_t)text.utf8[n] & 0xc0) == 0x80) --n;
    memcpy(nav->title, text.utf8, n);
    html_text_free(&text);
    return ESP_OK;
}

typedef struct {
    uint32_t toc, link, position, cut;
    bool head, hidden, visible, capture, heading;
    char label[1024]; size_t label_len;
} scanner_t;

static esp_err_t scan_token(book_mobi_t *b, scanner_t *s, const mobi_token_t *t) {
    const char *value; size_t len; uint32_t number;
    esp_err_t err;
    uint32_t end = s->position + (uint32_t)t->len;
    if (t->tag) {
        if (!strcmp(t->name, "head")) s->head = !t->closing;
        if (!strcmp(t->name, "script") || !strcmp(t->name, "style")) s->hidden = !t->closing;
        if (!strcmp(t->name, "reference") && book_mobi_attr(t, "type", &value, &len) && len == 3 &&
            !strncasecmp(value, "toc", 3) && book_mobi_attr(t, "filepos", &value, &len) &&
            book_mobi_number(value, len, &number) && number < b->total) s->toc = number;
        bool link = !strcmp(t->name, "a");
        bool heading = t->name[0] == 'h' && t->name[1] >= '1' && t->name[1] <= '6' && !t->name[2];
        if (!t->closing && ((link && s->position >= s->toc &&
            book_mobi_attr(t, "filepos", &value, &len) && book_mobi_number(value, len, &number)) ||
            (heading && s->toc == UINT32_MAX))) {
            s->capture = true; s->heading = heading;
            s->link = heading ? s->position : number;
            s->label_len = 0;
        } else if (t->closing && s->capture && ((link && !s->heading) || (heading && s->heading))) {
            if ((err = add_navigation(b, s->link, s->label, s->label_len)) != ESP_OK) return err;
            s->capture = false;
        }
        if (!s->head && !s->hidden && !strcmp(t->name, "img")) s->visible = true;
        bool pagebreak = !strcmp(t->name, "mbp:pagebreak");
        bool safe = t->closing && (!strcmp(t->name, "p") || !strcmp(t->name, "div") ||
                    !strcmp(t->name, "blockquote") || heading);
        if (s->visible && (pagebreak || (safe && end - s->cut >= SEGMENT_TARGET))) {
            if (end - s->cut > MOBI_SEGMENT_MAX) return ESP_ERR_INVALID_SIZE;
            if (end < b->total && (err = add_segment(b, end)) != ESP_OK) return err;
            s->cut = end; s->visible = false;
        }
    } else if (!s->head && !s->hidden) {
        for (size_t i = 0; i < t->len; ++i) if (!isspace((unsigned char)t->bytes[i])) { s->visible = true; break; }
        if (s->capture) {
            if (t->len > sizeof(s->label) - s->label_len) return ESP_ERR_INVALID_SIZE;
            memcpy(s->label + s->label_len, t->bytes, t->len); s->label_len += t->len;
        }
    }
    s->position = end;
    return end - s->cut > MOBI_SEGMENT_MAX ? ESP_ERR_INVALID_SIZE : ESP_OK;
}

typedef struct { uint32_t value, minimum; unsigned remaining; } utf8_state_t;
static bool validate_bytes(utf8_state_t *s, const uint8_t *raw, size_t len, uint32_t encoding) {
    for (size_t i = 0; i < len; ++i) {
        uint8_t c = raw[i];
        if (!c) return false;
        if (encoding == 1252) continue;
        if (s->remaining) {
            if ((c & 0xc0) != 0x80) return false;
            s->value = (s->value << 6) | (c & 63);
            if (!--s->remaining && (s->value < s->minimum || s->value > 0x10ffff ||
                (s->value >= 0xd800 && s->value <= 0xdfff))) return false;
        } else if (c >= 0x80) {
            s->remaining = c >= 0xc2 && c <= 0xdf ? 1 : c >= 0xe0 && c <= 0xef ? 2 : c >= 0xf0 && c <= 0xf4 ? 3 : 0;
            if (!s->remaining) return false;
            s->minimum = s->remaining == 1 ? 0x80 : s->remaining == 2 ? 0x800 : 0x10000;
            s->value = c & ((1u << (6 - s->remaining)) - 1);
        }
    }
    return true;
}

static esp_err_t build_index(book_mobi_t *b) {
    esp_err_t err = add_segment(b, 0);
    if (err != ESP_OK) return err;
    uint8_t *compressed = allocate(RECORD_BYTES), *raw = allocate(RECORD_BYTES);
    char *pending = allocate(RECORD_BYTES + MOBI_TAG_MAX);
    scanner_t *scan = allocate(sizeof(*scan));
    if (!compressed || !raw || !pending || !scan) { err = ESP_ERR_NO_MEM; goto done; }
    *scan = (scanner_t){.toc = UINT32_MAX};
    size_t remaining = 0; uint32_t total = 0;
    utf8_state_t utf8 = {0};
#ifdef ESP_PLATFORM
    int64_t began = esp_timer_get_time();
#endif
    for (size_t i = 1; i <= b->text_records; ++i) {
        size_t n;
        if ((err = decode_record(b, i, compressed, raw, &n)) != ESP_OK) goto done;
        if (!n || total >= b->total) { err = ESP_ERR_INVALID_SIZE; goto done; }
        if (n > b->total - total) {
            if (i != b->text_records) { err = ESP_ERR_INVALID_SIZE; goto done; }
            n = b->total - total;
        }
        if (!validate_bytes(&utf8, raw, n, b->encoding)) { err = ESP_ERR_INVALID_RESPONSE; goto done; }
        b->record[i].raw = total; b->record[i].len = (uint32_t)n;
        total += (uint32_t)n;
        memcpy(pending + remaining, raw, n); remaining += n;
        size_t at = 0;
        while (at < remaining) {
            mobi_token_t token;
            size_t used = book_mobi_token(pending + at, remaining - at, i == b->text_records, &token);
            if (!used) break;
            if (token.tag && used > MOBI_TAG_MAX) { err = ESP_ERR_INVALID_SIZE; goto done; }
            if ((err = scan_token(b, scan, &token)) != ESP_OK) goto done;
            at += used;
        }
        remaining -= at;
        if (remaining > MOBI_TAG_MAX) { err = ESP_ERR_INVALID_SIZE; goto done; }
        memmove(pending, pending + at, remaining);
#ifdef ESP_PLATFORM
        if (esp_timer_get_time() - began > 30000000) { err = ESP_ERR_TIMEOUT; goto done; }
        if (!(i % 16)) vTaskDelay(1);
#endif
    }
    if (total != b->total || remaining || utf8.remaining) { err = ESP_ERR_INVALID_SIZE; goto done; }
    if (b->count > 1 && !scan->visible) --b->count;
    /* 中文：有完整编号目录时只显示正文编号项，仍保留全部分段供阅读。
     * English: Prefer numbered authored body entries without removing any readable segments. */
    size_t numbered = 0;
    for (size_t i = 0; i < b->nav_count; ++i)
        if (!strncasecmp(b->navigation[i].title, "Chapter", 7) ||
            (!strncmp(b->navigation[i].title, "第", 3) && strstr(b->navigation[i].title, "章"))) ++numbered;
    if (numbered >= 2) {
        size_t out = 0;
        for (size_t i = 0; i < b->nav_count; ++i)
            if (!strncasecmp(b->navigation[i].title, "Chapter", 7) ||
                (!strncmp(b->navigation[i].title, "第", 3) && strstr(b->navigation[i].title, "章")))
                b->navigation[out++] = b->navigation[i];
        b->nav_count = out;
    }
    // ncx 索引比正文目录页更可靠：命中两项以上就整表替换。
    // The ncx index beats an authored TOC page: two or more entries replace the table wholesale.
    if (b->nav_count < MOBI_NAV_MAX) (void)ncx_navigation(b);
done:
    free(compressed); free(raw); free(pending); free(scan);
    return err;
}

typedef struct { uint32_t fingerprint, records, segments, navigation, checksum; } cache_t;
static uint32_t index_checksum(const book_mobi_t *b) {
    uint32_t h = hash_bytes(2166136261u, b->record + 1, b->text_records * sizeof(*b->record));
    h = hash_bytes(h, b->segments, b->count * sizeof(*b->segments));
    return hash_bytes(h, b->navigation, b->nav_count * sizeof(*b->navigation));
}
static bool cache_read(book_mobi_t *b, const char *path, const book_index_cache_header_t *key) {
    FILE *file = book_index_cache_open_read(path, key);
    if (!file) return false;
    cache_t c;
    bool ok = fread(&c, sizeof(c), 1, file) == 1 && c.fingerprint == b->fingerprint &&
        c.records == b->text_records && c.segments && c.segments <= MOBI_NAV_MAX && c.navigation <= MOBI_NAV_MAX;
    record_t *records = NULL;
    if (ok) {
        records = allocate(c.records * sizeof(*records));
        b->segments = allocate(c.segments * sizeof(*b->segments));
        if (c.navigation) b->navigation = allocate(c.navigation * sizeof(*b->navigation));
        ok = records && b->segments && (!c.navigation || b->navigation);
    }
    if (ok) ok = fread(records, sizeof(*records), c.records, file) == c.records &&
        fread(b->segments, sizeof(*b->segments), c.segments, file) == c.segments &&
        (!c.navigation || fread(b->navigation, sizeof(*b->navigation), c.navigation, file) == c.navigation) && fgetc(file) == EOF;
    uint32_t total = 0;
    if (ok) for (size_t i = 0; i < c.records; ++i) {
        if (records[i].offset != b->record[i + 1].offset || records[i].raw != total ||
            !records[i].len || records[i].len > RECORD_BYTES || records[i].len > b->total - total) { ok = false; break; }
        total += records[i].len;
    }
    if (ok) ok = total == b->total && b->segments[0] == 0;
    if (ok) for (size_t i = 0; i < c.segments; ++i) {
        uint32_t end = i + 1 < c.segments ? b->segments[i + 1] : b->total;
        if (end <= b->segments[i] || end - b->segments[i] > MOBI_SEGMENT_MAX) { ok = false; break; }
    }
    if (ok) for (size_t i = 0; i < c.navigation; ++i) {
        navigation_t *n = &b->navigation[i];
        const char *end = memchr(n->title, 0, sizeof(n->title));
        if (n->offset >= b->total || !end || !book_mobi_utf8(n->title, (size_t)(end - n->title))) { ok = false; break; }
    }
    if (ok) {
        memcpy(b->record + 1, records, c.records * sizeof(*records));
        b->count = b->segment_cap = c.segments; b->nav_count = b->nav_cap = c.navigation;
        ok = index_checksum(b) == c.checksum;
    }
    free(records); fclose(file);
    if (!ok) {
        free(b->segments); free(b->navigation);
        b->segments = NULL; b->navigation = NULL;
        b->count = b->segment_cap = b->nav_count = b->nav_cap = 0;
    }
    return ok;
}
static void cache_write(book_mobi_t *b, const char *path, const book_index_cache_header_t *key) {
    char temp[PATH_CAP];
    FILE *f = book_index_cache_open_write(path, key, temp, sizeof(temp));
    if (!f) return;
    cache_t c = {b->fingerprint, b->text_records, (uint32_t)b->count, (uint32_t)b->nav_count, index_checksum(b)};
    bool ok = fwrite(&c, sizeof(c), 1, f) == 1 &&
        fwrite(b->record + 1, sizeof(*b->record), b->text_records, f) == b->text_records &&
        fwrite(b->segments, sizeof(*b->segments), b->count, f) == b->count &&
        (!b->nav_count || fwrite(b->navigation, sizeof(*b->navigation), b->nav_count, f) == b->nav_count);
    book_index_cache_finish_write(f, temp, path, ok);
}
static void metadata_write(const book_mobi_t *b) {
    char path[PATH_CAP], temp[PATH_CAP]; book_index_cache_header_t key;
    if (!book_index_cache_prepare(b->path, "mobi-meta", CACHE_VERSION, path, sizeof(path), &key)) return;
    FILE *f = book_index_cache_open_write(path, &key, temp, sizeof(temp));
    if (!f) return;
    uint32_t checksum = hash_bytes(2166136261u, &b->metadata, sizeof(b->metadata));
    bool ok = fwrite(&b->metadata, sizeof(b->metadata), 1, f) == 1 && fwrite(&checksum, sizeof(checksum), 1, f) == 1;
    book_index_cache_finish_write(f, temp, path, ok);
}

esp_err_t book_mobi_open(const char *path, book_mobi_t **out) {
    if (!out) return ESP_ERR_INVALID_ARG;
    *out = NULL;
    book_mobi_t *b = NULL;
    esp_err_t err = headers(path, &b);
    if (err != ESP_OK) return err;
    char cache[PATH_CAP]; book_index_cache_header_t key;
    bool enabled = book_index_cache_prepare(path, "mobi", CACHE_VERSION, cache, sizeof(cache), &key);
    if (!enabled || !cache_read(b, cache, &key)) {
        err = build_index(b);
        if (err != ESP_OK) { book_mobi_close(b); return err; }
        if (enabled) cache_write(b, cache, &key);
    }
    metadata_write(b);
    *out = b;
    return ESP_OK;
}
void book_mobi_close(book_mobi_t *b) {
    if (!b) return;
    if (b->file) fclose(b->file);
    for (size_t i = 0; i < b->pool_count; ++i) free(b->pool[i]);
    free(b->pool); free(b->dict);
    free(b->record); free(b->segments); free(b->navigation); free(b);
}
size_t book_mobi_chapter_count(const book_mobi_t *b) { return b ? b->count : 0; }
size_t book_mobi_navigation_count(const book_mobi_t *b) { return b ? b->nav_count ? b->nav_count : b->count : 0; }
static size_t chapter_at(const book_mobi_t *b, uint32_t offset) {
    size_t i = 0;
    while (i + 1 < b->count && b->segments[i + 1] <= offset) ++i;
    return i;
}
size_t book_mobi_navigation_chapter(const book_mobi_t *b, size_t pos) {
    if (!b || pos >= book_mobi_navigation_count(b)) return SIZE_MAX;
    return b->nav_count ? chapter_at(b, b->navigation[pos].offset) : pos;
}
size_t book_mobi_navigation_source_offset(const book_mobi_t *b, size_t pos) {
    size_t chapter = book_mobi_navigation_chapter(b, pos);
    return chapter == SIZE_MAX ? SIZE_MAX : b->nav_count ? b->navigation[pos].offset - b->segments[chapter] : 0;
}
esp_err_t book_mobi_navigation_title(const book_mobi_t *b, size_t pos, char *buf, size_t cap) {
    if (!b || pos >= book_mobi_navigation_count(b)) return ESP_ERR_INVALID_ARG;
    return b->nav_count ? copy_title(b->navigation[pos].title, buf, cap) : book_mobi_chapter_title(b, pos, buf, cap);
}
esp_err_t book_mobi_chapter_title(const book_mobi_t *b, size_t i, char *buf, size_t cap) {
    if (!b || i >= b->count) return ESP_ERR_INVALID_ARG;
    for (size_t n = 0; n < b->nav_count; ++n)
        if (chapter_at(b, b->navigation[n].offset) == i) return copy_title(b->navigation[n].title, buf, cap);
    char name[64]; snprintf(name, sizeof(name), "第%u节", (unsigned)i + 1);
    return copy_title(name, buf, cap);
}
esp_err_t book_mobi_chapter_name(const book_mobi_t *b, size_t i, char *buf, size_t cap) {
    if (!b || i >= b->count) return ESP_ERR_INVALID_ARG;
    char name[48]; snprintf(name, sizeof(name), "mobi-%u.html", (unsigned)i);
    return copy_title(name, buf, cap);
}
uint32_t book_mobi_total_bytes(const book_mobi_t *b) { return b ? b->total : 0; }
uint32_t book_mobi_chapter_byte_offset(const book_mobi_t *b, size_t i) { return b && i < b->count ? b->segments[i] : 0; }

esp_err_t book_mobi_load_target(book_mobi_t *b, size_t i, const char *anchor, size_t source_offset,
                                size_t *text_offset, html_text_t *out) {
    if (!out) return ESP_ERR_INVALID_ARG;
    memset(out, 0, sizeof(*out));
    if (text_offset) *text_offset = 0;
    if (!b || i >= b->count) return ESP_ERR_INVALID_ARG;
    uint32_t begin = b->segments[i], end = i + 1 < b->count ? b->segments[i + 1] : b->total;
    size_t len = end - begin;
    if (!len || len > MOBI_SEGMENT_MAX) return ESP_ERR_INVALID_SIZE;
    char *text = allocate(len);
    uint8_t *compressed = allocate(RECORD_BYTES), *raw = allocate(RECORD_BYTES);
    esp_err_t err = ESP_ERR_NO_MEM;
    if (!text || !compressed || !raw) goto done;
    size_t written = 0;
    for (size_t r = 1; r <= b->text_records; ++r) {
        record_t *record = &b->record[r];
        if (record->raw >= end) break;
        if (record->raw + record->len <= begin) continue;
        size_t n;
        if ((err = decode_record(b, r, compressed, raw, &n)) != ESP_OK) goto done;
        if (n < record->len) { err = ESP_ERR_INVALID_SIZE; goto done; }
        uint32_t from = record->raw < begin ? begin - record->raw : 0;
        uint32_t to = record->raw + record->len > end ? end - record->raw : record->len;
        memcpy(text + written, raw + from, to - from); written += to - from;
    }
    if (written != len) { err = ESP_ERR_INVALID_SIZE; goto done; }
    /* 中文：尽早交还解压暂存，再构建规范化文本和块表。
     * English: Release codec scratch before building normalized text and blocks. */
    free(compressed); compressed = NULL; free(raw); raw = NULL;
    err = book_mobi_markup(text, len, begin, b->encoding, b->first_image, b->records,
                           anchor, source_offset, text_offset, out);
    /* 中文：目录点击按"章内源偏移"定位。锚点链在个别书上会返回 0 或落不到标题上，
     * 此时退回已核实的章首块；块没有独立成段时再退回正文里标题第一次出现的位置。
     * English: TOC clicks locate by an in-segment source offset. On some books the anchor
     * chain returns 0 or misses the heading, so fall back to the verified chapter-start
     * block, and when no block carries the title alone, to the title's first body hit. */
    const bool source_request = !anchor && source_offset != SIZE_MAX && source_offset < len;
    const uint32_t requested = source_request ? begin + (uint32_t)source_offset : UINT32_MAX;
    const char *requested_title = NULL;
    size_t snap = SIZE_MAX;
    if (err == ESP_OK) for (size_t n = 0; n < b->nav_count; ++n) {
        if (chapter_at(b, b->navigation[n].offset) != i) continue;
        size_t title_len = strlen(b->navigation[n].title);
        for (size_t j = 0; j < out->count; ++j) {
            blk_t *block = &out->blocks[j];
            size_t from = block->offset, size = block->len;
            while (size && isspace((unsigned char)out->utf8[from])) { ++from; --size; }
            while (size && isspace((unsigned char)out->utf8[from + size - 1])) --size;
            if (block->image < 0 && !block->linked && !block->auxiliary && size == title_len &&
                !memcmp(out->utf8 + from, b->navigation[n].title, size)) {
                block->heading = true; block->heading_level = 1; block->chapter_start = true;
                if (b->navigation[n].offset == requested) {
                    requested_title = b->navigation[n].title;
                    snap = block->offset;
                }
                break;
            }
        }
    }
    if (err == ESP_OK && text_offset && source_request && requested_title && requested_title[0]) {
        bool at_title = *text_offset <= out->len;
        if (at_title) {
            const char *landed = out->utf8 + *text_offset;
            while (isspace((unsigned char)*landed)) ++landed;
            at_title = strncmp(landed, requested_title, strlen(requested_title)) == 0;
        }
        if (!at_title) {
            if (snap != SIZE_MAX) *text_offset = snap;
            else {
                const char *hit = strstr(out->utf8, requested_title);
                if (hit) *text_offset = (size_t)(hit - out->utf8);
            }
        }
    }
done:
    free(text); free(compressed); free(raw);
    return err;
}

static bool image_record(book_mobi_t *b, const char *src, uint32_t *index) {
    return b && src && !strncmp(src, "mobi-", 5) && book_mobi_number(src + 5, strlen(src + 5), index) &&
        *index >= b->first_image && *index < b->records;
}
static esp_err_t load_image(book_mobi_t *b, uint32_t index, uint8_t **data, size_t *size, bool *png, size_t budget) {
    *data = NULL; *size = 0; *png = false;
    if (index < b->first_image || index >= b->records) return ESP_ERR_NOT_FOUND;
    size_t len = b->record[index + 1].offset - b->record[index].offset;
    if (len < 8) return ESP_ERR_INVALID_SIZE;
    size_t free_bytes = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t available = free_bytes > 2u * 1024u * 1024u ? free_bytes - 2u * 1024u * 1024u : 0;
    if (budget > available) budget = available;
    if (len > 4u * 1024u * 1024u || len > budget) return ESP_ERR_NO_MEM;
    uint8_t signature[8];
    esp_err_t err = read_at(b, b->record[index].offset, signature, sizeof(signature));
    if (err != ESP_OK) return err;
    *png = !memcmp(signature, "\x89PNG\r\n\x1a\n", 8);
    if (!*png && (signature[0] != 0xff || signature[1] != 0xd8)) return ESP_ERR_NOT_SUPPORTED;
    uint8_t *bytes = allocate(len);
    if (!bytes) return ESP_ERR_NO_MEM;
    err = read_at(b, b->record[index].offset, bytes, len);
    if (err != ESP_OK) { free(bytes); return err; }
    *data = bytes; *size = len;
    return ESP_OK;
}
esp_err_t book_mobi_image(book_mobi_t *b, size_t chapter, const char *src, uint8_t **data, size_t *size, bool *png) {
    if (!data || !size || !png) return ESP_ERR_INVALID_ARG;
    *data = NULL; *size = 0; *png = false;
    uint32_t index;
    if (!b || chapter >= b->count || !image_record(b, src, &index)) return ESP_ERR_NOT_FOUND;
    size_t free_bytes = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    return load_image(b, index, data, size, png, free_bytes > 2u * 1024u * 1024u ? free_bytes - 2u * 1024u * 1024u : 0);
}
esp_err_t book_mobi_image_dimensions(book_mobi_t *b, size_t chapter, const char *src, unsigned *w, unsigned *h) {
    if (!w || !h) return ESP_ERR_INVALID_ARG;
    *w = *h = 0;
    uint32_t index;
    if (!b || chapter >= b->count || !image_record(b, src, &index)) return ESP_ERR_NOT_FOUND;
    size_t len = b->record[index + 1].offset - b->record[index].offset;
    if (len > RECORD_BYTES) len = RECORD_BYTES;
    uint8_t *data = allocate(len);
    if (!data) return ESP_ERR_NO_MEM;
    esp_err_t err = read_at(b, b->record[index].offset, data, len);
    if (err != ESP_OK) { free(data); return err; }
    bool png = len >= 8 && !memcmp(data, "\x89PNG\r\n\x1a\n", 8);
    bool supported = png || (len >= 2 && data[0] == 0xff && data[1] == 0xd8);
    if (err == ESP_OK && (!supported || !book_image_dimensions(data, len, png, w, h))) err = ESP_ERR_NOT_SUPPORTED;
    free(data); return err;
}
esp_err_t book_mobi_cover_bounded(const char *path, uint8_t **data, size_t *size, bool *png, size_t budget) {
    if (!data || !size || !png) return ESP_ERR_INVALID_ARG;
    *data = NULL; *size = 0; *png = false;
    book_mobi_t *b = NULL;
    esp_err_t err = headers(path, &b);
    if (err == ESP_OK) {
        err = load_image(b, b->cover, data, size, png, budget);
        book_mobi_close(b);
    }
    return err;
}
esp_err_t book_mobi_metadata(const char *path, char *title, size_t title_cap, char *author, size_t author_cap) {
    if (!title || !title_cap || !author || !author_cap) return ESP_ERR_INVALID_ARG;
    title[0] = author[0] = 0;
    book_mobi_t *b = NULL;
    esp_err_t err = headers(path, &b);
    if (err != ESP_OK) return err;
    metadata_write(b);
    err = copy_title(b->metadata.title, title, title_cap);
    if (err == ESP_OK) err = copy_title(b->metadata.author, author, author_cap);
    book_mobi_close(b);
    return err;
}
esp_err_t book_mobi_metadata_cached(const char *source, char *title, size_t title_cap, char *author, size_t author_cap) {
    if (!title || !title_cap || !author || !author_cap) return ESP_ERR_INVALID_ARG;
    title[0] = author[0] = 0;
    char path[PATH_CAP]; book_index_cache_header_t key;
    if (!book_index_cache_prepare(source, "mobi-meta", CACHE_VERSION, path, sizeof(path), &key)) return ESP_ERR_NOT_FOUND;
    FILE *f = book_index_cache_open_read(path, &key);
    if (!f) return ESP_ERR_NOT_FOUND;
    metadata_t meta; uint32_t checksum;
    bool ok = fread(&meta, sizeof(meta), 1, f) == 1 && fread(&checksum, sizeof(checksum), 1, f) == 1 &&
        fgetc(f) == EOF && checksum == hash_bytes(2166136261u, &meta, sizeof(meta)) &&
        memchr(meta.title, 0, sizeof(meta.title)) && memchr(meta.author, 0, sizeof(meta.author));
    fclose(f);
    if (!ok || !book_mobi_utf8(meta.title, strlen(meta.title)) || !book_mobi_utf8(meta.author, strlen(meta.author))) return ESP_ERR_INVALID_RESPONSE;
    esp_err_t err = copy_title(meta.title, title, title_cap);
    return err == ESP_OK ? copy_title(meta.author, author, author_cap) : err;
}

#if CONFIG_READER_PERF && CONFIG_READER_MEMORY
void book_mobi_memory_stats(const book_mobi_t *b, reader_source_memory_stats_t *stats) {
    if (!b) return;
    stats->mobi_state = reader_memory_owned(b);
    stats->mobi_index = reader_memory_owned(b->record) + reader_memory_owned(b->segments) + reader_memory_owned(b->navigation);
    stats->open_files += b->file != NULL;
}
#endif
