#include "tagreader.h"

#include <taglib/aifffile.h>
#include <taglib/apefile.h>
#include <taglib/asffile.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/flacfile.h>
#include <taglib/mp4file.h>
#include <taglib/mpcfile.h>
#include <taglib/mpegfile.h>
#include <taglib/oggfile.h>
#include <taglib/oggflacfile.h>
#include <taglib/opusfile.h>
#include <taglib/speexfile.h>
#include <taglib/tag.h>
#include <taglib/taglib.h>
#include <taglib/tpropertymap.h>
#include <taglib/trueaudiofile.h>
#include <taglib/tstring.h>
#include <taglib/vorbisfile.h>
#include <taglib/wavfile.h>
#include <taglib/wavpackfile.h>

namespace {

// Docs:    https://id3.org/id3v2.3.0
//          https://taglib.org/api/p_propertymapping.html

constexpr auto ID3v2_Title = "TIT2";
constexpr auto ID3v2_Artist = "TPE1";
constexpr auto ID3v2_Album = "TALB";
constexpr auto ID3v2_AlbumArtist = "TPE2";
constexpr auto ID3v2_Genre = "TCON";
constexpr auto ID3v2_Performer = "TPE3";
constexpr auto ID3v2_Composer = "TCOM";
constexpr auto ID3v2_Comment = "COMM";
constexpr auto ID3v2_Date = "TDRC";
constexpr auto ID3v2_TrackNumber = "TRCK";
constexpr auto ID3v2_DiscNumber = "TPOS";
constexpr auto ID3v2_UnsychronizedLyrics = "USLT";
constexpr auto ID3v2_SynchronizedLyrics = "SYLT";
constexpr auto ID3v2_CoverArt = "APIC";

constexpr auto VorbisComment_AlbumArtist1 = "ALBUMARTIST";
constexpr auto VorbisComment_AlbumArtist2 = "ALBUM ARTIST";
constexpr auto VorbisComment_Performer = "PERFORMER";
constexpr auto VorbisComment_Composer = "COMPOSER";
constexpr auto VorbisComment_DiscNumber = "DISCNUMBER";
constexpr auto VorbisComment_CoverArt = "COVERART";
constexpr auto VorbisComment_MetadataBlockPicture = "METADATA_BLOCK_PICTURE";
constexpr auto VorbisComment_Lyrics = "LYRICS";
constexpr auto VorbisComment_UnsyncedLyrics = "UNSYNCEDLYRICS";

constexpr auto MP4_Title = "\251nam";
constexpr auto MP4_Artist = "\251ART";
constexpr auto MP4_Album = "\251alb";
constexpr auto MP4_AlbumArtist = "aART";
constexpr auto MP4_Genre = "\251gen";
constexpr auto MP4_Composer = "\251wrt";
constexpr auto MP4_Comment = "\251cmt";
constexpr auto MP4_Date = "\251day";
constexpr auto MP4_TrackNumber = "trkn";
constexpr auto MP4_TrackNumber2 = "----:com.apple.iTunes:track";
constexpr auto MP4_DiscNumber = "disk";
constexpr auto MP4_DiscNumber2 = "----:com.apple.iTunes:disc";
constexpr auto MP4_Lyrics = "\251lyr";
constexpr auto MP4_CoverArt = "covr";

} // namespace

TagReader::TagReader() = default;

void TagReader::readMetadata(const QString& fileName, TrackTags& result) const {
    if (fileName.isEmpty()) return;

    const auto fileRef = std::make_unique<TagLib::FileRef>(fileName.toUtf8().constData());

    if (!fileRef || fileRef->isNull() || fileRef->file() == nullptr || fileRef->tag() == nullptr) {
        qDebug() << "Cannot read metadata: file is null";
        return;
    }

    if (const auto* properties = fileRef->audioProperties()) {
        result.add(Metadata::Fields::BitRate, properties->bitrate() * 1000);
        result.add(Metadata::Fields::Duration, properties->lengthInSeconds());
        result.add(Metadata::Fields::Channels, properties->channels());
        result.add(Metadata::Fields::SampleRate, properties->sampleRate());
    }

    TagLib::File* file = fileRef->file();
    readGenericMetadata(file->properties(), result);

    if (auto* flac_file = dynamic_cast<TagLib::FLAC::File*>(file)) {
        result.add(Metadata::Fields::FileType, QStringLiteral("FLAC"));

        if (flac_file->hasXiphComment()) {
            auto* xiphComment = flac_file->xiphComment();
            readVorbisComments(xiphComment, result);
            result.addCoverImage(extractFlacCover(xiphComment));
        } else if (flac_file->hasID3v2Tag()) {
            readID3v2Tags(flac_file->ID3v2Tag(), result);
        }

    } else if (dynamic_cast<TagLib::Ogg::File*>(file) != nullptr) {
        if (auto* xiphComment = dynamic_cast<TagLib::Ogg::XiphComment*>(file->tag())) {
            readVorbisComments(xiphComment, result);
            result.addCoverImage(extractFlacCover(xiphComment));
        }

        if (dynamic_cast<TagLib::Ogg::FLAC::File*>(file) != nullptr) {
            result.add(Metadata::Fields::FileType, QStringLiteral("FLAC"));
        } else if (dynamic_cast<TagLib::Ogg::Vorbis::File*>(file) != nullptr) {
            result.add(Metadata::Fields::FileType, QStringLiteral("Vorbis"));
        } else if (dynamic_cast<TagLib::Ogg::Speex::File*>(file) != nullptr) {
            result.add(Metadata::Fields::FileType, QStringLiteral("Speex"));
        } else if (dynamic_cast<TagLib::Ogg::Opus::File*>(file) != nullptr) {
            result.add(Metadata::Fields::FileType, QStringLiteral("Opus"));
        }

    } else if (auto* mpeg_file = dynamic_cast<TagLib::MPEG::File*>(file)) {
        result.add(Metadata::Fields::FileType, QStringLiteral("MPEG"));
        if (mpeg_file->hasID3v2Tag()) {
            readID3v2Tags(mpeg_file->ID3v2Tag(), result);
        }

    } else if (const auto* mp4_file = dynamic_cast<TagLib::MP4::File*>(file)) {
        result.add(Metadata::Fields::FileType, QStringLiteral("MPEG-4"));
        if (mp4_file->tag() != nullptr) {
            readMP4Tags(mp4_file->tag(), result);
        }

    } else if (const auto* wav_file = dynamic_cast<TagLib::RIFF::WAV::File*>(file)) {
        result.add(Metadata::Fields::FileType, QStringLiteral("WAV"));
        if (wav_file->hasID3v2Tag()) {
            readID3v2Tags(wav_file->ID3v2Tag(), result);
        }

    } else if (const auto* riff_file = dynamic_cast<TagLib::RIFF::AIFF::File*>(file)) {
        result.add(Metadata::Fields::FileType, QStringLiteral("AIFF"));
        readID3v2Tags(riff_file->tag(), result);

    } else if (auto* trueaudio_file = dynamic_cast<TagLib::TrueAudio::File*>(file)) {
        result.add(Metadata::Fields::FileType, QStringLiteral("TTA"));
        if (trueaudio_file->hasID3v2Tag()) {
            readID3v2Tags(trueaudio_file->ID3v2Tag(), result);
        }
    }
}

void TagReader::extractCoverArt(const QString& fileName, TrackTags& result) const {
    if (fileName.isEmpty()) return;

    const auto fileRef = std::make_unique<TagLib::FileRef>(fileName.toUtf8().constData());

    if (!fileRef || fileRef->isNull() || fileRef->file() == nullptr || fileRef->tag() == nullptr) return;

    TagLib::File* file = fileRef->file();

    if (auto* flac_file = dynamic_cast<TagLib::FLAC::File*>(file)) {
        if (flac_file->hasXiphComment()) {
            result.addCoverImage(extractFlacCover(flac_file->xiphComment()));
        } else if (flac_file->hasID3v2Tag()) {
            result.addCoverImage(extractID3v2Cover(flac_file->ID3v2Tag()));
        }
    } else if (auto* ogg_file = dynamic_cast<TagLib::Ogg::File*>(file)) {
        if (auto* xiphComment = dynamic_cast<TagLib::Ogg::XiphComment*>(ogg_file->tag())) {
            result.addCoverImage(extractFlacCover(xiphComment));
        }
    } else if (auto* mpeg_file = dynamic_cast<TagLib::MPEG::File*>(file)) {
        if (mpeg_file->hasID3v2Tag()) {
            result.addCoverImage(extractID3v2Cover(mpeg_file->ID3v2Tag()));
        }
    } else if (auto* mp4_file = dynamic_cast<TagLib::MP4::File*>(file)) {
        if (mp4_file->tag() != nullptr) {
            result.addCoverImage(extractMP4Cover(mp4_file->tag()));
        }
    } else if (auto* wav_file = dynamic_cast<TagLib::RIFF::WAV::File*>(file)) {
        if (wav_file->hasID3v2Tag()) {
            result.addCoverImage(extractID3v2Cover(wav_file->ID3v2Tag()));
        }
    } else if (auto* riff_file = dynamic_cast<TagLib::RIFF::AIFF::File*>(file)) {
        result.addCoverImage(extractID3v2Cover(riff_file->tag()));
    } else if (auto* trueaudio_file = dynamic_cast<TagLib::TrueAudio::File*>(file)) {
        if (trueaudio_file->hasID3v2Tag()) {
            result.addCoverImage(extractID3v2Cover(trueaudio_file->ID3v2Tag()));
        }
    }
}

void TagReader::readGenericMetadata(const TagLib::PropertyMap& properties, TrackTags& result) const {
    if (properties.isEmpty()) return;

    readGenericField(properties, "TITLE", Metadata::Fields::Title, result);

    if (!readGenericField(properties, "ARTIST", Metadata::Fields::Artist, result)) {
        readGenericField(properties, "ARTISTS", Metadata::Fields::Artist, result);
    }

    readGenericField(properties, "ALBUM", Metadata::Fields::Album, result);
    readGenericField(properties, "ALBUMARTIST", Metadata::Fields::AlbumArtist, result);

    if (!readGenericField(properties, "COMMENT", Metadata::Fields::Comment, result)) {
        readGenericField(properties, "DESCRIPTION", Metadata::Fields::Comment, result);
    }

    if (properties.contains("TRACKNUMBER")) {
        result.add(Metadata::Fields::TrackNumber, properties["TRACKNUMBER"].toString().toInt());
    } else if (properties.contains("TRACK")) {
        result.add(Metadata::Fields::TrackNumber, properties["TRACK"].toString().toInt());
    }

    if (properties.contains("DATE")) {
        result.add(Metadata::Fields::Year, properties["DATE"].toString().toInt());
    } else if (properties.contains("YEAR")) {
        result.add(Metadata::Fields::Year, properties["YEAR"].toString().toInt());
    }

    if (properties.contains("DISCNUMBER")) {
        result.add(Metadata::Fields::DiscNumber, properties["DISCNUMBER"].toString().toInt());
    } else if (properties.contains("DISC")) {
        result.add(Metadata::Fields::DiscNumber, properties["DISC"].toString().toInt());
    }

    if (properties.contains("LYRICS")) {
        result.add(
            Metadata::Fields::Lyrics,
            TStringToQString(properties["LYRICS"].toString()).trimmed().replace(QLatin1Char('\r'), QLatin1Char('\n')));
    }

    readGenericField(properties, "GENRE", Metadata::Fields::Genre, result);
    readGenericField(properties, "PERFORMER", Metadata::Fields::Performer, result);
    readGenericField(properties, "COMPOSER", Metadata::Fields::Composer, result);
    readGenericField(properties, "LYRICIST", Metadata::Fields::Lyricist, result);
}

bool TagReader::readGenericField(const TagLib::PropertyMap& properties, const std::string& tagName,
                                 const Metadata::Fields field, TrackTags& result) const {
    if (properties.contains(tagName)) {
        const auto values = properties[tagName];
        for (const auto& value : values) {
            result.add(field, TStringToQString(value).trimmed());
        }
        return true;
    }
    return false;
}

void TagReader::readID3v2Tags(const TagLib::ID3v2::Tag* id3Tags, const TrackTags& result) const {
    if (id3Tags->isEmpty()) return;

    const auto& map = id3Tags->frameListMap();

    if (map.contains(ID3v2_DiscNumber)) {
        const auto discNumber = map[ID3v2_DiscNumber].front()->toString().toInt();
        result.add(Metadata::Fields::DiscNumber, discNumber);
    }

    if (map.contains(ID3v2_Composer)) {
        const auto composer = map[ID3v2_Composer].front()->toString();
        result.add(Metadata::Fields::Composer, TStringToQString(composer));
    }

    if (map.contains(ID3v2_Artist)) {
        const auto artist = map[ID3v2_Artist].front()->toString();
        result.add(Metadata::Fields::Artist, TStringToQString(artist));
    }

    if (map.contains(ID3v2_Performer)) {
        const auto performer = map[ID3v2_Performer].front()->toString();
        result.add(Metadata::Fields::Performer, TStringToQString(performer));
    }

    if (map.contains(ID3v2_AlbumArtist)) {
        const auto albumArtist = map[ID3v2_AlbumArtist].front()->toString();
        result.add(Metadata::Fields::AlbumArtist, TStringToQString(albumArtist));
    }

    if (map.contains(ID3v2_SynchronizedLyrics)) {
        const auto lyrics = map[ID3v2_SynchronizedLyrics].front()->toString();
        result.add(Metadata::Fields::Lyrics, TStringToQString(lyrics));
    } else if (map.contains(ID3v2_UnsychronizedLyrics)) {
        const auto lyrics = map[ID3v2_UnsychronizedLyrics].front()->toString();
        result.add(Metadata::Fields::Lyrics, TStringToQString(lyrics));
    }

    if (map.contains(ID3v2_Comment)) {
        const auto comment = map[ID3v2_Comment].front()->toString();
        result.add(Metadata::Fields::Comment, TStringToQString(comment));
    }

    if (map.contains(ID3v2_CoverArt)) {
        result.addCoverImage(extractID3v2Cover(id3Tags));
    }
}

void TagReader::readVorbisComments(TagLib::Ogg::XiphComment* xiphComment, TrackTags& result) const {
    if (xiphComment == nullptr || xiphComment->isEmpty()) return;

    const auto& map = xiphComment->fieldListMap();

    if (map.contains(VorbisComment_Performer)) {
        const auto performer = map[VorbisComment_Performer].front();
        result.add(Metadata::Fields::Performer, TStringToQString(performer));
    }

    if (map.contains(VorbisComment_Composer)) {
        const auto composer = map[VorbisComment_Composer].front();
        result.add(Metadata::Fields::Composer, TStringToQString(composer));
    }

    if (map.contains(VorbisComment_AlbumArtist1)) {
        const auto albumArtist = map[VorbisComment_AlbumArtist1].front();
        result.add(Metadata::Fields::AlbumArtist, TStringToQString(albumArtist));
    } else if (map.contains(VorbisComment_AlbumArtist2)) {
        const auto albumArtist = map[VorbisComment_AlbumArtist2].front();
        result.add(Metadata::Fields::AlbumArtist, TStringToQString(albumArtist));
    }

    if (map.contains(VorbisComment_DiscNumber)) {
        const auto discNumber = map[VorbisComment_DiscNumber].front();
        result.add(Metadata::Fields::DiscNumber, discNumber.toInt());
    }

    if (map.contains(VorbisComment_Lyrics)) {
        const auto lyrics = map[VorbisComment_Lyrics].front();
        result.add(Metadata::Fields::Lyrics, TStringToQString(lyrics));
    } else if (map.contains(VorbisComment_UnsyncedLyrics)) {
        const auto lyrics = map[VorbisComment_UnsyncedLyrics].front();
        result.add(Metadata::Fields::Lyrics, TStringToQString(lyrics));
    }
}

void TagReader::readMP4Tags(const TagLib::MP4::Tag* mp4Tags, TrackTags& result) const {
    if (mp4Tags->isEmpty()) return;

    const auto& map = mp4Tags->itemMap();

    if (map.contains(MP4_AlbumArtist)) {
        const auto albumArtist = map[MP4_AlbumArtist].toStringList().front();
        result.add(Metadata::Fields::AlbumArtist, TStringToQString(albumArtist));
    }

    if (map.contains(MP4_Composer)) {
        const auto composer = map[MP4_Composer].toStringList().toString(", ");
        result.add(Metadata::Fields::Composer, TStringToQString(composer));
    }

    if (map.contains(MP4_TrackNumber)) {
        const auto trackNumber = map[MP4_TrackNumber].toInt();
        result.add(Metadata::Fields::TrackNumber, trackNumber);
    } else if (map.contains(MP4_TrackNumber2)) {
        const auto trackNumber = map[MP4_TrackNumber2].toInt();
        result.add(Metadata::Fields::TrackNumber, trackNumber);
    }

    if (map.contains(MP4_DiscNumber)) {
        const auto discNumber = map[MP4_DiscNumber].toInt();
        result.add(Metadata::Fields::DiscNumber, discNumber);
    } else if (map.contains(MP4_DiscNumber2)) {
        const auto discNumber = map[MP4_DiscNumber2].toInt();
        result.add(Metadata::Fields::DiscNumber, discNumber);
    }

    if (map.contains(MP4_Lyrics)) {
        const auto lyrics = map[MP4_Lyrics].toStringList().toString(" ");
        result.add(Metadata::Fields::Lyrics, TStringToQString(lyrics));
    }

    if (map.contains(MP4_Comment)) {
        const auto comment = map[MP4_Comment].toStringList().toString(" ");
        result.add(Metadata::Fields::Comment, TStringToQString(comment));
    }

    if (map.contains(MP4_CoverArt)) {
        result.addCoverImage(extractMP4Cover(mp4Tags));
    }
}

QByteArray TagReader::extractID3v2Cover(const TagLib::ID3v2::Tag* id3Tags) const {
    if (id3Tags->isEmpty()) {
        return {};
    }

    TagLib::ID3v2::FrameList frameList = id3Tags->frameList(ID3v2_CoverArt);

    if (frameList.isEmpty()) {
        return {};
    }

    const auto* pictureFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frameList.front());
    return {pictureFrame->picture().data(), pictureFrame->picture().size()};
}

QByteArray TagReader::extractFlacCover(TagLib::Ogg::XiphComment* xiphComment) const {
    if (xiphComment->isEmpty()) {
        return {};
    }

    const auto pictureList = xiphComment->pictureList();

    if (pictureList.isEmpty()) {
        return {};
    }

    const auto imageType = pictureList.front()->type();
    if (imageType != TagLib::FLAC::Picture::FrontCover) {
        return {};
    }

    const auto* pictureFront = pictureList.front();
    return {pictureFront->data().data(), pictureFront->data().size()};
}

QByteArray TagReader::extractMP4Cover(const TagLib::MP4::Tag* mp4Tags) const {
    if (mp4Tags->isEmpty()) {
        return {};
    }

    const auto coverArt = mp4Tags->item(MP4_CoverArt);
    if (!coverArt.isValid()) {
        return {};
    }

    const auto coverArtList = coverArt.toCoverArtList();
    if (coverArtList.isEmpty()) {
        return {};
    }

    const auto& coverArtFront = coverArtList.front();
    return {coverArtFront.data().data(), coverArtFront.data().size()};
}
