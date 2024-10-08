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

void TagReader::readMetadata(const QString &fileName, TrackTags &result) const {
    if (fileName.isEmpty()) return;

    const auto fileRef = std::make_unique<TagLib::FileRef>(fileName.toUtf8().constData());

    if (!fileRef || fileRef->isNull() || fileRef->file() == nullptr || fileRef->tag() == nullptr) {
        qDebug() << "Cannot read metadata: file is null";
        return;
    }

    if (const auto *properties = fileRef->audioProperties()) {
        result.add(MetadataFields::BitRateRole, properties->bitrate() * 1000);
        result.add(MetadataFields::DurationRole, properties->lengthInSeconds());
        result.add(MetadataFields::ChannelsRole, properties->channels());
        result.add(MetadataFields::SampleRateRole, properties->sampleRate());
    }

    readGenericMetadata(fileRef->file()->properties(), result);

    if (auto *flac_file = dynamic_cast<TagLib::FLAC::File *>(fileRef->file())) {
        result.add(MetadataFields::FileTypeRole, QStringLiteral("FLAC"));

        if (flac_file->hasXiphComment()) {
            auto *xiphComment = flac_file->xiphComment();
            readVorbisComments(xiphComment, result);
            result.addCoverImage(extractFlacCover(xiphComment));
        } else if (flac_file->hasID3v2Tag()) {
            readID3v2Tags(flac_file->ID3v2Tag(), result);
        }

    } else if (dynamic_cast<TagLib::Ogg::File *>(fileRef->file()) != nullptr) {
        if (auto *xiphComment = dynamic_cast<TagLib::Ogg::XiphComment *>(fileRef->file()->tag())) {
            readVorbisComments(xiphComment, result);
            result.addCoverImage(extractFlacCover(xiphComment));
        }

        if (dynamic_cast<TagLib::Ogg::FLAC::File *>(fileRef->file()) != nullptr) {
            result.add(MetadataFields::FileTypeRole, QStringLiteral("FLAC"));
        } else if (dynamic_cast<TagLib::Ogg::Vorbis::File *>(fileRef->file()) != nullptr) {
            result.add(MetadataFields::FileTypeRole, QStringLiteral("Vorbis"));
        } else if (dynamic_cast<TagLib::Ogg::Speex::File *>(fileRef->file()) != nullptr) {
            result.add(MetadataFields::FileTypeRole, QStringLiteral("Speex"));
        } else if (dynamic_cast<TagLib::Ogg::Opus::File *>(fileRef->file()) != nullptr) {
            result.add(MetadataFields::FileTypeRole, QStringLiteral("Opus"));
        }

    } else if (auto *mpeg_file = dynamic_cast<TagLib::MPEG::File *>(fileRef->file())) {
        result.add(MetadataFields::FileTypeRole, QStringLiteral("MPEG"));
        if (mpeg_file->hasID3v2Tag()) {
            readID3v2Tags(mpeg_file->ID3v2Tag(), result);
        }

    } else if (const auto *mp4_file = dynamic_cast<TagLib::MP4::File *>(fileRef->file())) {
        result.add(MetadataFields::FileTypeRole, QStringLiteral("MPEG-4"));
        if (mp4_file->tag() != nullptr) {
            readMP4Tags(mp4_file->tag(), result);
        }

    } else if (const auto *wav_file = dynamic_cast<TagLib::RIFF::WAV::File *>(fileRef->file())) {
        result.add(MetadataFields::FileTypeRole, QStringLiteral("WAV"));
        if (wav_file->hasID3v2Tag()) {
            readID3v2Tags(wav_file->ID3v2Tag(), result);
        }

    } else if (const auto *riff_file = dynamic_cast<TagLib::RIFF::AIFF::File *>(fileRef->file())) {
        result.add(MetadataFields::FileTypeRole, QStringLiteral("AIFF"));
        readID3v2Tags(riff_file->tag(), result);

    } else if (auto *trueaudio_file = dynamic_cast<TagLib::TrueAudio::File *>(fileRef->file())) {
        result.add(MetadataFields::FileTypeRole, QStringLiteral("TTA"));
        if (trueaudio_file->hasID3v2Tag()) {
            readID3v2Tags(trueaudio_file->ID3v2Tag(), result);
        }
    }
}

void TagReader::readGenericMetadata(const TagLib::PropertyMap &properties, TrackTags &result) const {
    if (properties.isEmpty()) return;

    readGenericField(properties, "TITLE", MetadataFields::TitleRole, result);

    if (!readGenericField(properties, "ARTIST", MetadataFields::ArtistRole, result)) {
        readGenericField(properties, "ARTISTS", MetadataFields::ArtistRole, result);
    }

    readGenericField(properties, "ALBUM", MetadataFields::AlbumRole, result);
    readGenericField(properties, "ALBUMARTIST", MetadataFields::AlbumArtistRole, result);

    if (!readGenericField(properties, "COMMENT", MetadataFields::CommentRole, result)) {
        readGenericField(properties, "DESCRIPTION", MetadataFields::CommentRole, result);
    }

    if (properties.contains("TRACKNUMBER")) {
        result.add(MetadataFields::TrackNumberRole, properties["TRACKNUMBER"].toString().toInt());
    } else if (properties.contains("TRACK")) {
        result.add(MetadataFields::TrackNumberRole, properties["TRACK"].toString().toInt());
    }

    if (properties.contains("DATE")) {
        result.add(MetadataFields::YearRole, properties["DATE"].toString().toInt());
    } else if (properties.contains("YEAR")) {
        result.add(MetadataFields::YearRole, properties["YEAR"].toString().toInt());
    }

    if (properties.contains("DISCNUMBER")) {
        result.add(MetadataFields::DiscNumberRole, properties["DISCNUMBER"].toString().toInt());
    } else if (properties.contains("DISC")) {
        result.add(MetadataFields::DiscNumberRole, properties["DISC"].toString().toInt());
    }

    if (properties.contains("LYRICS")) {
        result.add(
            MetadataFields::LyricsRole,
            TStringToQString(properties["LYRICS"].toString()).trimmed().replace(QLatin1Char('\r'), QLatin1Char('\n')));
    }

    readGenericField(properties, "GENRE", MetadataFields::GenreRole, result);
    readGenericField(properties, "PERFORMER", MetadataFields::PerformerRole, result);
    readGenericField(properties, "COMPOSER", MetadataFields::ComposerRole, result);
    readGenericField(properties, "LYRICIST", MetadataFields::LyricistRole, result);
}

bool TagReader::readGenericField(const TagLib::PropertyMap &properties, const std::string &tagName,
                                 const MetadataFields::ColumnsRoles role, TrackTags &result) const {
    if (properties.contains(tagName)) {
        const auto values = properties[tagName];
        for (const auto &value : values) {
            result.add(role, TStringToQString(value).trimmed());
        }
        return true;
    }
    return false;
}

void TagReader::readID3v2Tags(const TagLib::ID3v2::Tag *id3Tags, const TrackTags &result) const {
    if (id3Tags->isEmpty()) return;

    const auto &map = id3Tags->frameListMap();

    if (map.contains(ID3v2_DiscNumber)) {
        const auto discNumber = map[ID3v2_DiscNumber].front()->toString().toInt();
        result.add(MetadataFields::DiscNumberRole, discNumber);
    }

    if (map.contains(ID3v2_Composer)) {
        const auto composer = map[ID3v2_Composer].front()->toString();
        result.add(MetadataFields::ComposerRole, TStringToQString(composer));
    }

    if (map.contains(ID3v2_Artist)) {
        const auto artist = map[ID3v2_Artist].front()->toString();
        result.add(MetadataFields::ArtistRole, TStringToQString(artist));
    }

    if (map.contains(ID3v2_Performer)) {
        const auto performer = map[ID3v2_Performer].front()->toString();
        result.add(MetadataFields::PerformerRole, TStringToQString(performer));
    }

    if (map.contains(ID3v2_AlbumArtist)) {
        const auto albumArtist = map[ID3v2_AlbumArtist].front()->toString();
        result.add(MetadataFields::AlbumArtistRole, TStringToQString(albumArtist));
    }

    if (map.contains(ID3v2_SynchronizedLyrics)) {
        const auto lyrics = map[ID3v2_SynchronizedLyrics].front()->toString();
        result.add(MetadataFields::LyricsRole, TStringToQString(lyrics));
    } else if (map.contains(ID3v2_UnsychronizedLyrics)) {
        const auto lyrics = map[ID3v2_UnsychronizedLyrics].front()->toString();
        result.add(MetadataFields::LyricsRole, TStringToQString(lyrics));
    }

    if (map.contains(ID3v2_Comment)) {
        const auto comment = map[ID3v2_Comment].front()->toString();
        result.add(MetadataFields::CommentRole, TStringToQString(comment));
    }

    if (map.contains(ID3v2_CoverArt)) {
        result.addCoverImage(extractID3v2Cover(id3Tags));
    }
}

void TagReader::readVorbisComments(TagLib::Ogg::XiphComment *xiphComment, TrackTags &result) const {
    if (xiphComment == nullptr || xiphComment->isEmpty()) return;

    const auto &map = xiphComment->fieldListMap();

    if (map.contains(VorbisComment_Performer)) {
        const auto performer = map[VorbisComment_Performer].front();
        result.add(MetadataFields::PerformerRole, TStringToQString(performer));
    }

    if (map.contains(VorbisComment_Composer)) {
        const auto composer = map[VorbisComment_Composer].front();
        result.add(MetadataFields::ComposerRole, TStringToQString(composer));
    }

    if (map.contains(VorbisComment_AlbumArtist1)) {
        const auto albumArtist = map[VorbisComment_AlbumArtist1].front();
        result.add(MetadataFields::AlbumArtistRole, TStringToQString(albumArtist));
    } else if (map.contains(VorbisComment_AlbumArtist2)) {
        const auto albumArtist = map[VorbisComment_AlbumArtist2].front();
        result.add(MetadataFields::AlbumArtistRole, TStringToQString(albumArtist));
    }

    if (map.contains(VorbisComment_DiscNumber)) {
        const auto discNumber = map[VorbisComment_DiscNumber].front();
        result.add(MetadataFields::DiscNumberRole, discNumber.toInt());
    }

    if (map.contains(VorbisComment_Lyrics)) {
        const auto lyrics = map[VorbisComment_Lyrics].front();
        result.add(MetadataFields::LyricsRole, TStringToQString(lyrics));
    } else if (map.contains(VorbisComment_UnsyncedLyrics)) {
        const auto lyrics = map[VorbisComment_UnsyncedLyrics].front();
        result.add(MetadataFields::LyricsRole, TStringToQString(lyrics));
    }
}

void TagReader::readMP4Tags(const TagLib::MP4::Tag *mp4Tags, TrackTags &result) const {
    if (mp4Tags->isEmpty()) return;

    const auto &map = mp4Tags->itemMap();

    if (map.contains(MP4_AlbumArtist)) {
        const auto albumArtist = map[MP4_AlbumArtist].toStringList().front();
        result.add(MetadataFields::AlbumArtistRole, TStringToQString(albumArtist));
    }

    if (map.contains(MP4_Composer)) {
        const auto composer = map[MP4_Composer].toStringList().toString(", ");
        result.add(MetadataFields::ComposerRole, TStringToQString(composer));
    }

    if (map.contains(MP4_TrackNumber)) {
        const auto trackNumber = map[MP4_TrackNumber].toInt();
        result.add(MetadataFields::TrackNumberRole, trackNumber);
    } else if (map.contains(MP4_TrackNumber2)) {
        const auto trackNumber = map[MP4_TrackNumber2].toInt();
        result.add(MetadataFields::TrackNumberRole, trackNumber);
    }

    if (map.contains(MP4_DiscNumber)) {
        const auto discNumber = map[MP4_DiscNumber].toInt();
        result.add(MetadataFields::DiscNumberRole, discNumber);
    } else if (map.contains(MP4_DiscNumber2)) {
        const auto discNumber = map[MP4_DiscNumber2].toInt();
        result.add(MetadataFields::DiscNumberRole, discNumber);
    }

    if (map.contains(MP4_Lyrics)) {
        const auto lyrics = map[MP4_Lyrics].toStringList().toString(" ");
        result.add(MetadataFields::LyricsRole, TStringToQString(lyrics));
    }

    if (map.contains(MP4_Comment)) {
        const auto comment = map[MP4_Comment].toStringList().toString(" ");
        result.add(MetadataFields::CommentRole, TStringToQString(comment));
    }

    if (map.contains(MP4_CoverArt)) {
        result.addCoverImage(extractMP4Cover(mp4Tags));
    }
}

QByteArray TagReader::extractID3v2Cover(const TagLib::ID3v2::Tag *id3Tags) const {
    if (id3Tags->isEmpty()) {
        return {};
    }

    TagLib::ID3v2::FrameList frameList = id3Tags->frameList(ID3v2_CoverArt);

    if (frameList.isEmpty()) {
        return {};
    }

    const auto *pictureFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(frameList.front());
    return {pictureFrame->picture().data(), pictureFrame->picture().size()};
}

QByteArray TagReader::extractFlacCover(TagLib::Ogg::XiphComment *xiphComment) const {
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

    const auto *pictureFront = pictureList.front();
    return {pictureFront->data().data(), pictureFront->data().size()};
}

QByteArray TagReader::extractMP4Cover(const TagLib::MP4::Tag *mp4Tags) const {
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

    const auto &coverArtFront = coverArtList.front();
    return {coverArtFront.data().data(), coverArtFront.data().size()};
}
