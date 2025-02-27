#ifndef TAGREADER_H
#define TAGREADER_H

#include "tracktags.h"

#include <QString>
#include <taglib/fileref.h>
#include <taglib/id3v2tag.h>
#include <taglib/mp4tag.h>
#include <taglib/xiphcomment.h>

class TagReader {
public:
    TagReader();
    ~TagReader() = default;
    void readMetadata(const QString& fileName, TrackTags& result) const;
    void extractCoverArt(const QString& fileName, TrackTags& result) const;

private:
    void readGenericMetadata(const TagLib::PropertyMap& properties, TrackTags& result) const;

    bool readGenericField(const TagLib::PropertyMap& properties, const std::string& tagName, Metadata::Fields field,
                          TrackTags& result) const;

    void readID3v2Tags(const TagLib::ID3v2::Tag* id3Tags, const TrackTags& result) const;
    void readVorbisComments(TagLib::Ogg::XiphComment* xiphComment, TrackTags& result) const;
    void readMP4Tags(const TagLib::MP4::Tag* mp4Tags, TrackTags& result) const;

    QByteArray extractID3v2Cover(const TagLib::ID3v2::Tag* id3Tags) const;
    QByteArray extractFlacCover(TagLib::Ogg::XiphComment* xiphComment) const;
    QByteArray extractMP4Cover(const TagLib::MP4::Tag* mp4Tags) const;
};

#endif // TAGREADER_H
