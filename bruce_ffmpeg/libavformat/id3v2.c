


static void id3v2_read_internal(AVIOContext *pb, AVDictionary **metadata, AVFormatContext *s, const char *magic, ID3v2ExtraMeta **extra_meta, int64_t max_search_size)
{
    int len, ret;
    uint8_t buf[ID3v2_HEADER_SIZE];
    int found_header;
    int64_t start, off;

    if (max_search_size && max_search_size < ID3v2_HEADER_SIZE)
        return;

    start = avio_tell(pb);
    do {
        /* save the current offset in case there's nothing to read/skip */
        off = avio_tell(pb);
        if (max_search_size && off - start >= max_search_size - ID3v2_HEADER_SIZE) {
            avio_seek(pb, off, SEEK_SET);
            break;
        }

        ret = avio_ensure_seekback(pb, ID3v2_HEADER_SIZE);
        if (ret >= 0)
            ret = avio_read(pb, buf, ID3v2_HEADER_SIZE);
        if (ret != ID3v2_HEADER_SIZE) {
            avio_seek(pb, off, SEEK_SET);
            break;
        }
        found_header = ff_id3v2_match(buf, magic);
        if (found_header) {
            /* parse ID3v2 header */
            len = ((buf[6] & 0x7f) << 21) |
                  ((buf[7] & 0x7f) << 14) |
                  ((buf[8] & 0x7f) << 7) |
                   (buf[9] & 0x7f);
            id3v2_parse(pb, metadata, s, len, buf[3], buf[5], extra_meta);
        } else {
            avio_seek(pb, off, SEEK_SET);
        }
    } while (found_header);
    ff_metadata_conv(metadata, NULL, ff_id3v2_34_metadata_conv);
    ff_metadata_conv(metadata, NULL, id3v2_2_metadata_conv);
    ff_metadata_conv(metadata, NULL, ff_id3v2_4_metadata_conv);
    merge_date(metadata);
}

void id3v2_read_dict(AVIOContext *pb, AVDictionary **metadata, const char *magic, ID3v2ExtraMeta **extra_meta)
{
    id3v2_read_internal(pb, metadata, NULL, magic, extra_meta, 0);
}

int id3v2_parse_apic(AVFormatContext *s, ID3v2ExtraMeta **extra_meta)
{
    ID3v2ExtraMeta *cur;

    for (cur = *extra_meta; cur; cur = cur->next) {
        ID3v2ExtraMetaAPIC *apic;
        AVStream *st;

        if (strcmp(cur->tag, "APIC"))
            continue;
        apic = cur->data;

        if (!(st = avformat_new_stream(s, NULL)))
            return AVERROR(ENOMEM);

        st->disposition      |= AV_DISPOSITION_ATTACHED_PIC;
        st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
        st->codecpar->codec_id   = apic->id;

        if (AV_RB64(apic->buf->data) == 0x89504e470d0a1a0a)
            st->codecpar->codec_id = AV_CODEC_ID_PNG;

        if (apic->description[0])
            av_dict_set(&st->metadata, "title", apic->description, 0);

        av_dict_set(&st->metadata, "comment", apic->type, 0);

        av_init_packet(&st->attached_pic);
        st->attached_pic.buf          = apic->buf;
        st->attached_pic.data         = apic->buf->data;
        st->attached_pic.size         = apic->buf->size - AV_INPUT_BUFFER_PADDING_SIZE;
        st->attached_pic.stream_index = st->index;
        st->attached_pic.flags       |= AV_PKT_FLAG_KEY;

        apic->buf = NULL;
    }

    return 0;
}

void ff_id3v2_free_extra_meta(ID3v2ExtraMeta **extra_meta)
{
    ID3v2ExtraMeta *current = *extra_meta, *next;
    const ID3v2EMFunc *extra_func;

    while (current) {
        if ((extra_func = get_extra_meta_func(current->tag, 1)))
            extra_func->free(current->data);
        next = current->next;
        av_freep(&current);
        current = next;
    }

    *extra_meta = NULL;
}

int ff_id3v2_match(const uint8_t *buf, const char *magic)
{
    return  buf[0]         == magic[0] &&
            buf[1]         == magic[1] &&
            buf[2]         == magic[2] &&
            buf[3]         != 0xff     &&
            buf[4]         != 0xff     &&
           (buf[6] & 0x80) == 0        &&
           (buf[7] & 0x80) == 0        &&
           (buf[8] & 0x80) == 0        &&
           (buf[9] & 0x80) == 0;
}

int ff_id3v2_tag_len(const uint8_t *buf)
{
    int len = ((buf[6] & 0x7f) << 21) +
              ((buf[7] & 0x7f) << 14) +
              ((buf[8] & 0x7f) << 7) +
              (buf[9] & 0x7f) +
              ID3v2_HEADER_SIZE;
    if (buf[5] & 0x10)
        len += ID3v2_HEADER_SIZE;
    return len;
}

